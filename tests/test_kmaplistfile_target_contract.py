import re
import shutil
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).parents[1]
SOURCE = ROOT / "src/SO3World/Src/KMapListFile.cpp"


def compact(text):
    """Keep the contract check structural while ignoring whitespace/comments."""
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    text = re.sub(r"//[^\n]*", "", text)
    return re.sub(r"\s+", " ", text)


def require_order(text, fragments):
    cursor = 0
    for fragment in fragments:
        found = text.find(fragment, cursor)
        if found < 0:
            raise AssertionError("target operation missing or out of order: %s" % fragment)
        cursor = found + len(fragment)


def verify_target_contract(source_path):
    text = compact(source_path.read_text(encoding="latin-1"))

    # These facts are independently transcribed from Init at 0x0835986c,
    # especially the three locations legacy source does not represent.
    require_order(text, [
        "DWORD dwMapID = 0;",
        "int nMaxMapLevel = 0;",
        'GetInteger(nRowIndex, "ID", 0, (int*)&dwMapID)',
        "KGLOG_PROCESS_ERROR(dwMapID <= MAX_MAP_ID)",
        'GetString(nRowIndex, "Name", "", MapParam.szMapName',
        "KGLOG_PROCESS_ERROR(MapParam.szMapName[0] != '\\0')",
        'GetString(nRowIndex, "DisplayName", MapParam.szMapName, MapParam.szDisplayName',
        "KGLOG_PROCESS_ERROR(MapParam.szDisplayName[0] != '\\0')",
        'GetInteger(nRowIndex, "FightList", 0, &MapParam.bFightList)',
        'GetInteger(nRowIndex, "MaxMapLevel", 0, &nMaxMapLevel)',
        'GetInteger(nRowIndex, "MaxLootRange", 1, &MapParam.nMaxLootRange)',
        "MapParam.nRefreshCycle *= 60;",
        "MapParam.nRefreshOffset *= 60;",
        "MapParam.nRefreshOffset += timezone;",
        "nCopyIndex <= nMaxMapLevel",
        "MapParam.dwMapID = KScene::MakeMapKey(dwMapID, nCopyIndex);",
        "m_MapParamTable[MapParam.dwMapID] = MapParam;",
    ])

    if 'GetInteger(nRowIndex, "FightList", 0, &nMaxMapLevel)' in text:
        raise AssertionError("FightList must populate bFightList, not the copy bound")
    if 'GetInteger(nRowIndex, "MaxMapLevel", 0, &MapParam.bFightList)' in text:
        raise AssertionError("MaxMapLevel must populate the local copy bound")
    if "bRetCode && dwMapID <= MAX_MAP_ID" in text:
        raise AssertionError("target ID guard consumes the output value, not the virtual call return")
    if "bRetCode && MapParam.szMapName[0]" in text or "bRetCode && MapParam.szDisplayName[0]" in text:
        raise AssertionError("target string guards consume the output buffers, not the virtual call return")


class KMapListFileTargetContractTest(unittest.TestCase):
    def test_candidate_satisfies_target_derived_contract(self):
        verify_target_contract(SOURCE)

    def test_plausible_target_contract_mutations_are_detected(self):
        workspace = Path(tempfile.mkdtemp(prefix="kmaplistfile-contract-"))
        self.addCleanup(shutil.rmtree, workspace, True)
        original = SOURCE.read_text(encoding="latin-1")
        mutations = {
            "display-name-fallback": (
                'GetString(nRowIndex, "DisplayName", MapParam.szMapName',
                'GetString(nRowIndex, "DisplayName", ""',
            ),
            "fight-list-as-copy-bound": (
                'GetInteger(nRowIndex, "FightList", 0, &MapParam.bFightList)',
                'GetInteger(nRowIndex, "FightList", 0, &nMaxMapLevel)',
            ),
            "missing-copy-key-writeback": (
                "MapParam.dwMapID = KScene::MakeMapKey(dwMapID, nCopyIndex);",
                "m_MapParamTable[KScene::MakeMapKey(dwMapID, nCopyIndex)] = MapParam;",
            ),
            "virtual-call-return-guard": (
                "KGLOG_PROCESS_ERROR(dwMapID <= MAX_MAP_ID)",
                "KGLOG_PROCESS_ERROR(bRetCode && dwMapID <= MAX_MAP_ID)",
            ),
        }
        for name, (before, after) in mutations.items():
            with self.subTest(name=name):
                mutated = workspace / (name + ".cpp")
                mutated.write_text(original.replace(before, after, 1), encoding="latin-1")
                with self.assertRaises(AssertionError):
                    verify_target_contract(mutated)


if __name__ == "__main__":
    unittest.main()
