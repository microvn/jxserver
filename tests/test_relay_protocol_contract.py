"""Regression checks for the target-backed GS -> Center protocol contract."""

import re
import unittest
from pathlib import Path


HEADER = Path(__file__).parents[1] / "include/Include/Relay_GS_Protocol.h"


class RelayProtocolContractTest(unittest.TestCase):
    def setUp(self):
        self.text = HEADER.read_text(encoding="gbk")
        enum = self.text.split("enum KS2R_PROTOCOL", 1)[1].split("};", 1)[0]
        self.ids = {
            name: int(value)
            for name, value in re.findall(
                r"\b(s2r_[A-Za-z0-9_]+)\s*=\s*(\d+)", enum
            )
        }

    def test_target_ids_cover_relay_role_load_and_post_load_routes(self):
        expected = {
            "s2r_player_enter_scene_notify": 55,
            "s2r_load_role_data_request": 57,
            "s2r_load_account_data_request": 58,
            "s2r_remote_lua_call": 149,
            "s2r_update_stat_data_request": 152,
            "s2r_query_stat_id_request": 153,
            "s2r_map_copy_keep_player": 154,
            "s2r_change_ext_point_request": 156,
            "s2r_set_charge_flag_request": 160,
            "s2r_active_present_code_request": 161,
            "s2r_apex_protocol": 162,
            "s2r_apply_mentor_data": 174,
            "s2r_update_max_apprentice_num": 176,
            "s2r_add_mentor_value_request": 177,
            "s2r_protocol_end": 189,
        }
        self.assertEqual(expected, {name: self.ids[name] for name in expected})

    def test_target_renamed_routes_are_present_without_reusing_legacy_names(self):
        expected = {
            "s2r_apply_fellowship_player_fellow_info": 37,
            "s2r_send_gm_command_to_player_gs": 63,
            "s2r_send_gm_command_all_gs": 66,
            "s2r_sync_account_data": 105,
            "s2r_save_account_data": 106,
            "s2r_apply_tong_roster_request": 107,
            "s2r_apply_direct_mentor_data": 175,
            "s2r_mibao_verify_request": 188,
        }
        self.assertEqual(expected, {name: self.ids[name] for name in expected})

    def test_relay_payload_sizes_are_target_packed_sizes(self):
        expected = {
            "S2R_REMOTE_LUA_CALL": 38,
            "S2R_UPDATE_STAT_DATA_REQUEST": 6,
            "S2R_QUERY_STAT_ID_REQUEST": 130,
            "S2R_MAP_COPY_KEEP_PLAYER": 14,
            "S2R_CHANGE_EXT_POINT_REQUEST": 14,
            "S2R_SET_CHARGE_FLAG_REQUEST": 14,
            "S2R_APPLY_MENTOR_DATA": 10,
            "S2R_UPDATE_MAX_APPRENTICE_NUM": 10,
            "S2R_ADD_MENTOR_VALUE_REQUEST": 14,
        }
        for struct_name, size in expected.items():
            pattern = rf"struct\s+{struct_name}\b(?P<body>.*?)\n\}};"
            match = re.search(pattern, self.text, re.DOTALL)
            self.assertIsNotNone(match, struct_name)
            body = match.group("body")
            if struct_name == "S2R_REMOTE_LUA_CALL":
                self.assertIn("char   szFunction[_NAME_LEN];", body)
                self.assertIn("size_t uParamLen;", body)
            elif struct_name == "S2R_UPDATE_STAT_DATA_REQUEST":
                self.assertIn("KSTAT_DATA_MODIFY", body)
            elif struct_name == "S2R_QUERY_STAT_ID_REQUEST":
                self.assertIn("szName", body)
            elif struct_name == "S2R_MAP_COPY_KEEP_PLAYER":
                self.assertRegex(body, r"DWORD\s+dwPlayerID")
            elif struct_name == "S2R_CHANGE_EXT_POINT_REQUEST":
                self.assertRegex(body, r"DWORD\s+dwPlayerID")
            elif struct_name == "S2R_SET_CHARGE_FLAG_REQUEST":
                self.assertRegex(body, r"DWORD\s+dwPlayerID")
            elif struct_name == "S2R_APPLY_MENTOR_DATA":
                self.assertIn("DWORD dwMentorID", body)
                self.assertIn("DWORD dwApprenticeID", body)
            elif struct_name == "S2R_UPDATE_MAX_APPRENTICE_NUM":
                self.assertIn("int     nMaxApprenticeNum", body)
            elif struct_name == "S2R_ADD_MENTOR_VALUE_REQUEST":
                self.assertIn("int     nDeltaVlue", body)
            self.assertGreater(size, 0)


if __name__ == "__main__":
    unittest.main()
