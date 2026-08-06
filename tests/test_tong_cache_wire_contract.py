"""Target-derived contract tests for the tong-cache / tong-diplomacy wire slice.

Oracle = target DWARF (`jx3_dwarf/SO3GameServerD`), read live through
`llvm-dwarfdump`. The candidate side is parsed out of the source headers.
The two sides are produced by different readers on purpose: no test here
round-trips a model against itself.

Boundary: compare-engine/staging/blocker/krelay-tong-cache-wire-20260806
"""

import os
import re
import subprocess
import unittest

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DWARF = os.path.join(os.path.dirname(REPO), "jx3_dwarf", "SO3GameServerD")
DWARFDUMP = "/opt/homebrew/opt/llvm/bin/llvm-dwarfdump"

RELAY_H = os.path.join(REPO, "include", "Include", "Relay_GS_Protocol.h")
CLIENT_H = os.path.join(REPO, "include", "Include", "GS_Client_Protocol.h")
TONGDEF_H = os.path.join(REPO, "include", "Include", "KTongDef.h")
TONGSERVER_H = os.path.join(REPO, "include", "Include", "SO3World", "KTongServer.h")
RELAY_CPP = os.path.join(REPO, "src", "SO3World", "Src", "KRelayClient.cpp")

WIDTH = {
    "BYTE": 1, "unsigned char": 1, "char": 1, "bool": 1,
    "WORD": 2, "unsigned short": 2, "short": 2,
    "DWORD": 4, "int": 4, "BOOL": 4, "time_t": 4, "unsigned long": 4,
    "long": 4, "size_t": 4, "float": 4,
    "TONG_DIPLOMACY_RELATION_TYPE": 4,
}


def have_dwarf():
    return os.path.isfile(DWARF) and os.path.isfile(DWARFDUMP)


def read_source(path):
    return open(path, "rb").read().decode("gbk", "replace")


def dwarfdump(*args):
    out = subprocess.run(
        [DWARFDUMP] + list(args) + [DWARF],
        capture_output=True, text=True, errors="replace",
    )
    return out.stdout


def target_enum(name):
    """Return {member: value} for a target enum, pairing each name with the
    const_value that FOLLOWS it (llvm-dwarfdump emits name before value)."""
    text = dwarfdump("--name=%s" % name, "-c")
    values, pending = {}, None
    for line in text.splitlines():
        m = re.search(r'DW_AT_name\s+\("([^"]+)"\)', line)
        if m:
            pending = m.group(1)
            continue
        m = re.search(r"DW_AT_const_value\s+\((-?\d+)\)", line)
        if m and pending:
            values.setdefault(pending, int(m.group(1)))
            pending = None
    return values


def target_struct(name):
    """Return (byte_size, [(field, offset), ...]) for the FIRST matching target
    structure DIE. DIEs are separated by blank lines; only DW_TAG_member blocks
    are fields, so DW_TAG_inheritance and duplicate compile-unit copies are
    dropped instead of inflating the field list."""
    text = dwarfdump("--name=%s" % name, "-c")
    blocks = re.split(r"\n\s*\n", text)
    size, fields, started = None, [], False
    for block in blocks:
        if "DW_TAG_structure_type" in block or "DW_TAG_class_type" in block:
            if started:
                break                    # second copy of the same type: stop
            m = re.search(r"DW_AT_byte_size\s+\((0x[0-9a-f]+|\d+)\)", block)
            n = re.search(r'DW_AT_name\s+\("([^"]+)"\)', block)
            if not n or n.group(1) != name:
                continue
            size = int(m.group(1), 0) if m else None
            started = True
            continue
        if not started:
            continue
        if "DW_TAG_member" not in block:
            continue
        n = re.search(r'DW_AT_name\s+\("([^"]+)"\)', block)
        o = re.search(r"DW_AT_data_member_location\s+\(DW_OP_plus_uconst (0x[0-9a-f]+|\d+)\)", block)
        if n and o:
            fields.append((n.group(1), int(o.group(1), 0)))
    return size, fields


def candidate_enum(path, enum_name):
    src = read_source(path)
    i = src.index("enum %s" % enum_name)
    body = src[i:src.index("};", i)]
    values, val = {}, -1
    for line in body.split("\n")[2:]:
        line = line.split("//")[0].split("/*")[0].strip()
        m = re.match(r"([A-Za-z_0-9]+)\s*(=\s*(\d+))?\s*,?$", line)
        if not m:
            continue
        val = int(m.group(3)) if m.group(3) else val + 1
        values[m.group(1)] = val
    return values


def candidate_struct_fields(path, struct_name):
    """Declared field order for a packed struct, as (type, name, count)."""
    src = read_source(path)
    m = re.search(r"struct\s+%s\s*(:[^\{]*)?\{(.*?)\n\};" % struct_name, src, re.S)
    if not m:
        raise AssertionError("struct %s not found in %s" % (struct_name, path))
    fields = []
    for line in m.group(2).split("\n"):
        line = line.split("//")[0].split("/*")[0].strip()
        if not line or line.endswith(")") or "(" in line:
            continue
        d = re.match(r"([A-Za-z_][A-Za-z_0-9:< >]*?)\s+([A-Za-z_][A-Za-z_0-9]*)\s*(\[([A-Za-z_0-9]*)\])?\s*;$", line)
        if not d:
            continue
        fields.append((d.group(1).strip(), d.group(2), d.group(4)))
    return fields


class TestWireIds(unittest.TestCase):
    """Numeric wire IDs must equal the target enum values, not just match by name."""

    @classmethod
    def setUpClass(cls):
        if not have_dwarf():
            raise unittest.SkipTest("target DWARF or llvm-dwarfdump unavailable")
        cls.r2s_t = target_enum("KR2S_PROTOCOL")
        cls.s2r_t = target_enum("KS2R_PROTOCOL")
        cls.r2s_c = candidate_enum(RELAY_H, "KR2S_PROTOCOL")
        cls.s2r_c = candidate_enum(RELAY_H, "KS2R_PROTOCOL")

    def test_new_r2s_routes_sit_at_target_ids(self):
        for name in ("r2s_sync_tong_diplomacy_data",
                     "r2s_apply_tong_cachce_data_respond",
                     "r2s_sync_tong_cache_change"):
            self.assertIn(name, self.r2s_c, "%s missing from candidate enum" % name)
            self.assertEqual(self.r2s_c[name], self.r2s_t[name], name)

    def test_auction_lookup_moved_off_the_size_collision_slot(self):
        self.assertEqual(self.r2s_c["r2s_auction_lookup_respond"],
                         self.r2s_t["r2s_auction_lookup_respond"])
        self.assertNotEqual(self.r2s_c["r2s_auction_lookup_respond"],
                            self.r2s_c["r2s_sync_tong_diplomacy_data"])

    def test_s2r_request_id(self):
        self.assertEqual(self.s2r_c["s2r_apply_tong_cache_request"],
                         self.s2r_t["s2r_apply_tong_cache_request"])
        self.assertEqual(self.s2r_c["s2r_apply_tong_cache_request"], 0x81)

    def test_no_live_handler_moved(self):
        """Every registered non-noop r2s handler must still sit at its target ID."""
        cpp = read_source(RELAY_CPP)
        regs = re.findall(
            r"REGISTER_INTERNAL_FUNC\(\s*(r2s_\w+)\s*,\s*&KRelayClient::(\w+)\s*,\s*(\d+)", cpp)
        live = [(n, h, int(s)) for n, h, s in regs if h != "OnNoOpRespond"]
        self.assertGreater(len(live), 50)
        alias = candidate_enum(RELAY_H, "KR2S_TARGET_COIN_SHOP_PROTOCOL")
        for name, handler, _size in live:
            if name not in self.r2s_t:
                continue
            cand = self.r2s_c.get(name, alias.get(name))
            self.assertIsNotNone(cand, "%s is registered but not declared" % name)
            self.assertEqual(cand, self.r2s_t[name],
                             "%s (%s) drifted off its target ID" % (name, handler))

    def test_registered_sizes_match_target_structs(self):
        cpp = read_source(RELAY_CPP)
        expect = {
            "r2s_sync_tong_diplomacy_data": "R2S_SYNC_TONG_DIPLOMACY_DATA",
            "r2s_apply_tong_cachce_data_respond": "R2S_APPLY_TONG_CACHCE_DATA_RESPOND",
            "r2s_sync_tong_cache_change": "R2S_SYNC_TONG_CACHE_CHANGE",
        }
        for slot, struct in expect.items():
            m = re.search(r"REGISTER_INTERNAL_FUNC\(\s*%s\s*,[^,]+,\s*(\d+)\)" % slot, cpp)
            self.assertIsNotNone(m, "%s is not registered" % slot)
            size, _ = target_struct(struct)
            self.assertEqual(int(m.group(1)), size,
                             "%s registered size != target %s byte_size" % (slot, struct))


class TestWireLayout(unittest.TestCase):
    """Field order and packed offsets must reproduce the target layout."""

    @classmethod
    def setUpClass(cls):
        if not have_dwarf():
            raise unittest.SkipTest("target DWARF or llvm-dwarfdump unavailable")

    def assert_layout(self, path, struct, header_bytes, extra_widths=None):
        widths = dict(WIDTH)
        widths.update(extra_widths or {})
        t_size, t_fields = target_struct(struct)
        c_fields = candidate_struct_fields(path, struct)

        t_named = [(n, o) for n, o in t_fields]
        self.assertEqual(len(c_fields), len(t_named),
                         "%s: field count %d != target %d" % (struct, len(c_fields), len(t_named)))

        offset = header_bytes
        for (ctype, cname, ccount), (tname, toff) in zip(c_fields, t_named):
            self.assertEqual(cname, tname, "%s: field order drift" % struct)
            self.assertEqual(offset, toff,
                             "%s.%s at %d, target %d" % (struct, cname, offset, toff))
            if ccount == "":            # flexible tail: contributes 0 bytes
                continue
            width = widths.get(ctype)
            self.assertIsNotNone(width, "%s.%s: unknown width for %s" % (struct, cname, ctype))
            offset += width * (int(ccount) if ccount and ccount.isdigit() else
                               (7 if ccount == "ttntTotal" else 1))
        return t_size, offset

    def test_s2r_apply_tong_cache_request(self):
        size, end = self.assert_layout(RELAY_H, "S2R_APPLY_TONG_CACHE_REQUEST", 2)
        self.assertEqual((size, end), (6, 6))

    def test_r2s_apply_tong_cachce_data_respond(self):
        size, _ = self.assert_layout(RELAY_H, "R2S_APPLY_TONG_CACHCE_DATA_RESPOND", 2)
        self.assertEqual(size, 6)

    def test_r2s_sync_tong_cache_change(self):
        size, end = self.assert_layout(RELAY_H, "R2S_SYNC_TONG_CACHE_CHANGE", 2)
        self.assertEqual((size, end), (8, 8))

    def test_r2s_sync_tong_diplomacy_data(self):
        size, _ = self.assert_layout(
            RELAY_H, "R2S_SYNC_TONG_DIPLOMACY_DATA", 2,
            {"KTONG_DIPLOMACY_RELATION_INFO": 30})
        self.assertEqual(size, 6)

    def test_s2c_sync_tong_total_cache(self):
        size, end = self.assert_layout(CLIENT_H, "S2C_SYNC_TONG_TOTAL_CACHE", 2)
        self.assertEqual((size, end), (9, 9))

    def test_s2c_sync_tong_cache_change(self):
        size, end = self.assert_layout(CLIENT_H, "S2C_SYNC_TONG_CACHE_CHANGE", 2)
        self.assertEqual((size, end), (4, 4))

    def test_relation_info_offsets(self):
        """The 30-byte record consumed by the diplomacy handler."""
        t_size, t_fields = target_struct("KTONG_DIPLOMACY_RELATION_INFO")
        self.assertEqual(t_size, 30)
        # candidate names differ from target names; compare offsets positionally
        c_fields = candidate_struct_fields(TONGDEF_H, "KTONG_DIPLOMACY_RELATION_INFO")
        self.assertEqual(len(c_fields), len(t_fields))
        offset = 0
        for (ctype, _cname, _cnt), (_tname, toff) in zip(c_fields, t_fields):
            self.assertEqual(offset, toff)
            offset += WIDTH[ctype]
        self.assertEqual(offset, 30)
        # byIsAdd / bAdd is the Add-vs-Del selector read at record offset 0x1d
        self.assertEqual(dict(t_fields)["byIsAdd"], 0x1D)
        self.assertEqual(c_fields[-1][1], "bAdd")


class TestCacheState(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        if not have_dwarf():
            raise unittest.SkipTest("target DWARF or llvm-dwarfdump unavailable")

    def test_tech_node_tag_matches_target(self):
        t = target_enum("TongTechNodeTag")
        c = candidate_enum(TONGDEF_H, "TongTechNodeTag")
        self.assertEqual(c, t)
        self.assertEqual(c["ttntTotal"], 7)
        self.assertIn("ttntRepairDiscount", c)

    def test_tong_cache_data_is_one_array_of_ttnt_total(self):
        size, fields = target_struct("TongCacheData")
        self.assertEqual(size, 7)
        self.assertEqual([n for n, _ in fields], ["byTechNodeTag"])
        c = candidate_struct_fields(TONGSERVER_H, "TongCacheData")
        self.assertEqual(len(c), 1, "candidate TongCacheData must be a single array member")
        self.assertEqual(c[0][1], "byTechNodeTag")
        self.assertEqual(c[0][2], "ttntTotal")


class TestClosureIsWired(unittest.TestCase):
    """The proven target edges must exist as real calls, not markers."""

    def test_relay_handlers_have_bodies(self):
        cpp = read_source(RELAY_CPP)
        for fn in ("DoApplyTongCacheRequest", "OnApplyTongCacheRespond",
                   "OnSyncTongCacheChange", "OnSyncTongDiplomacyData"):
            self.assertIn("KRelayClient::%s(" % fn, cpp, "%s has no definition" % fn)

    def test_target_call_edges(self):
        cpp = read_source(RELAY_CPP)
        tong = read_source(os.path.join(REPO, "src", "SO3World", "Src", "KTongServer.cpp"))
        player = read_source(os.path.join(REPO, "src", "SO3World", "Src", "KPlayerServer.cpp"))
        edges = [
            (cpp, "InsertTongChache", "OnApplyTongCacheRespond -> InsertTongChache"),
            (cpp, "UpdateTongChacheChange", "OnSyncTongCacheChange -> UpdateTongChacheChange"),
            (cpp, "AddDiplomacyRelation", "OnSyncTongDiplomacyData -> AddDiplomacyRelation"),
            (cpp, "DelDiplomacyRelation", "OnSyncTongDiplomacyData -> DelDiplomacyRelation"),
            (tong, "DoBroadcastTongTotalCache", "InsertTongChache -> DoBroadcastTongTotalCache"),
            (tong, "DoBroadcastTongCacheChange", "UpdateTongChacheChange -> DoBroadcastTongCacheChange"),
            (tong, "DoSyncTongTotalCache", "SyncTongTotalCache -> DoSyncTongTotalCache"),
            (tong, "DoApplyTongCacheRequest", "RegisterTongMember -> DoApplyTongCacheRequest"),
            (player, "m_TongServer.Traverse", "DoBroadcast* -> KTongServer::Traverse"),
        ]
        for text, needle, label in edges:
            self.assertIn(needle, text, "missing target edge: %s" % label)

    def test_no_unknown_marker_left_in_the_slice(self):
        for path in (RELAY_CPP,
                     os.path.join(REPO, "src", "SO3World", "Src", "KTongServer.cpp")):
            text = read_source(path)
            self.assertNotIn("PORT-UNKNOWN_REQUIRED", text,
                             "%s still carries an in-path UNKNOWN marker" % path)


class TestMutationsAreDetected(unittest.TestCase):
    """A checker that cannot fail proves nothing. Each mutation below must be
    caught by the same helpers the positive tests use."""

    @classmethod
    def setUpClass(cls):
        if not have_dwarf():
            raise unittest.SkipTest("target DWARF or llvm-dwarfdump unavailable")

    def mutate(self, path, old, new):
        import tempfile
        raw = open(path, "rb").read()
        self.assertIn(old, raw, "mutation anchor not found")
        fd, tmp = tempfile.mkstemp(suffix=".h")
        os.write(fd, raw.replace(old, new, 1))
        os.close(fd)
        self.addCleanup(os.unlink, tmp)
        return tmp

    def test_swapped_field_order_is_caught(self):
        tmp = self.mutate(
            RELAY_H,
            b"struct R2S_SYNC_TONG_CACHE_CHANGE : INTERNAL_PROTOCOL_HEADER\n{\n"
            b"    DWORD   dwTongID;\n    BYTE    byType;\n    BYTE    byValue;\n};",
            b"struct R2S_SYNC_TONG_CACHE_CHANGE : INTERNAL_PROTOCOL_HEADER\n{\n"
            b"    BYTE    byType;\n    BYTE    byValue;\n    DWORD   dwTongID;\n};")
        fields = candidate_struct_fields(tmp, "R2S_SYNC_TONG_CACHE_CHANGE")
        _size, t_fields = target_struct("R2S_SYNC_TONG_CACHE_CHANGE")
        self.assertNotEqual([f[1] for f in fields], [f[0] for f in t_fields])

    def test_wrong_wire_id_is_caught(self):
        tmp = self.mutate(
            RELAY_H,
            b"    s2r_apply_tong_cache_request = 129,",
            b"    s2r_apply_tong_cache_request = 128,")
        c = candidate_enum(tmp, "KS2R_PROTOCOL")
        t = target_enum("KS2R_PROTOCOL")
        self.assertNotEqual(c["s2r_apply_tong_cache_request"],
                            t["s2r_apply_tong_cache_request"])

    def test_shrunk_cache_array_is_caught(self):
        """The exact regression this slice fixes: ttntTotal 7 -> 6."""
        tmp = self.mutate(
            TONGDEF_H,
            b"    ttntRepairDiscount = 6,\n    ttntTotal\n",
            b"    ttntTotal\n")
        c = candidate_enum(tmp, "TongTechNodeTag")
        self.assertEqual(c["ttntTotal"], 6)
        self.assertNotEqual(c, target_enum("TongTechNodeTag"))


if __name__ == "__main__":
    unittest.main(verbosity=2)
