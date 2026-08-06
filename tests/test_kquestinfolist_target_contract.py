#!/usr/bin/env python3
"""Static, target-derived contract checks for the KQuestInfoList file port."""

from pathlib import Path
import re
import unittest


ROOT = Path(__file__).resolve().parents[1]
HEADER = (ROOT / "include/Include/SO3World/KQuestInfoList.h").read_text(encoding="latin-1")
SOURCE = (ROOT / "src/SO3World/Src/KQuestInfoList.cpp").read_text(encoding="latin-1")


def function_body(name: str) -> str:
    start = SOURCE.index("KQuestInfoList::%s" % name)
    opening = SOURCE.index("{", start)
    depth = 0
    for index in range(opening, len(SOURCE)):
        if SOURCE[index] == "{":
            depth += 1
        elif SOURCE[index] == "}":
            depth -= 1
            if depth == 0:
                return SOURCE[opening + 1:index]
    raise AssertionError("unbalanced function body: %s" % name)


class KQuestInfoListTargetContract(unittest.TestCase):
    def test_group_record_uses_target_bool_abi_and_vector_owner(self):
        record = HEADER[HEADER.index("struct KQUEST_GROUP_INFO"):HEADER.index("class KQuestInfoList")]
        self.assertRegex(record, r"(?s)DWORD\s+dwGroupID;.*DWORD\s+dwScriptID;.*int\s+nMaxCount;")
        self.assertRegex(record, r"(?s)BOOL\s+bPlayerDiff;.*BOOL\s+bRandomDiff;.*BOOL\s+bDaily;")
        self.assertIn("std::vector<DWORD, KMemory::KAllocator<DWORD> > QuestGroup;", record)

    def test_default_quest_file_only_loads_default_row(self):
        body = function_body("LoadQuestFile")
        default_at = body.index("if (bIsLoadDefultInfo)")
        normal_at = body.index("else", default_at)
        default_branch = body[default_at:normal_at]
        self.assertIn("LoadQuestInfo(piTabFile, 2, &m_DefaultQuestInfo)", default_branch)
        self.assertNotIn("for (", default_branch)
        normal_branch = body[normal_at:]
        self.assertIn("for (int nIndex = 2; nIndex <= nHeight; ++nIndex)", normal_branch)
        self.assertNotIn("memset(&TempQuestInfo", body)

    def test_quest_list_zeros_default_once_before_row_dispatch(self):
        body = function_body("LoadQuestListFile")
        self.assertLess(body.index("memset(&m_DefaultQuestInfo"), body.index("for (int nIndex = 2"))
        self.assertIn("LoadQuestFile(szFilePath, nIndex == 2)", body)

    def test_random_group_target_branch_and_ownership_policy(self):
        body = function_body("LoadRandomQuestGroup")
        self.assertLess(body.index("m_mapNpc2RandomQuestGroup.insert"), body.index('"ScriptName"'))
        self.assertIn('"MaxCount", 1', body)
        self.assertIn("if (!QuestGroupInfo.dwGroupID)\n            continue;", body)
        self.assertIn("if (!dwNpcTemplateID)\n            continue;", body)
        self.assertIn("if (QuestGroupInfo.nMaxCount < 1)\n            continue;", body)
        self.assertNotIn("KGLOG_PROCESS_ERROR(QuestGroupInfo.dwGroupID)", body)
        self.assertNotIn("KGLOG_PROCESS_ERROR(dwNpcTemplateID)", body)
        self.assertNotIn("KGLOG_PROCESS_ERROR(QuestGroupInfo.nMaxCount >= 1)", body)
        self.assertIn("if (nData == 0)\n                break;", body)
        self.assertNotIn("if (nData <= 0)", body)
        self.assertIn("pQuestInfo->bRepeat != true", body)
        self.assertIn("if (QuestGroupInfo.QuestGroup.empty())\n            continue;", body)
        self.assertIn("if (QuestGroupInfo.bRandomDiff)", body)
        group_insert = re.search(r"m_QuestGroupMap\.insert\([^\n]+\);", body)
        self.assertIsNotNone(group_insert)
        self.assertNotIn(".second", group_insert.group(0))
        self.assertNotIn("memset(&QuestGroupInfo", body)

    def test_optional_tab_reads_and_force_present_invariant_match_target(self):
        body = function_body("LoadQuestInfo")
        for field in (
            '"Activity"',
            '"PresentExp2Money"',
            '"RewardSkillID"',
            '"RewardSkillMaxLevel"',
            '"PresentArenaAward"',
            '"PresentActivityAward"',
        ):
            start = body.index(field)
            self.assertIn("(void)bRetCode;", body[start:body.index(";", start) + 80])
        self.assertIn("!pQuestInfo->bPresentAccordToForce[0] || pQuestInfo->bPresentAll[0]", body)
        self.assertIn("!pQuestInfo->bPresentAccordToForce[1] || pQuestInfo->bPresentAll[1]", body)
        self.assertLess(body.index('"CanRepeat"'), body.index('"Activity"'))
        self.assertLess(body.index('"Activity"'), body.index('"CanAssist"'))
        self.assertLess(body.index('"DropDoodadTemplateID%d"'), body.index('"DropNpcTemplateID%d"'))
        self.assertLess(body.index('"DropNpcTemplateID%d"'), body.index('"IsDeleteEndRequireItem%d"'))

    def test_quest_file_loaders_disable_tab_error_log_before_column_reads(self):
        quest_file = function_body("LoadQuestFile")
        quest_list = function_body("LoadQuestListFile")
        self.assertLess(quest_file.index("SetErrorLog(false)"), quest_file.index("GetHeight()"))
        self.assertLess(quest_list.index("SetErrorLog(false)"), quest_list.index("GetHeight()"))

    def test_init_and_group_lookup_contract(self):
        init = function_body("Init")
        self.assertLess(init.index("LoadQuestListFile"), init.index("LoadRandomQuestGroup"))
        group_lookup = SOURCE[SOURCE.index("KQuestInfoList::GetQuestGroupInfo(DWORD dwNpcTemplateID"):SOURCE.index("KQuestInfoList::GetQuestInfo")]
        self.assertIn("it->second.at((unsigned int)i)", group_lookup)


if __name__ == "__main__":
    unittest.main()
