import pathlib
import re
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[1]
HEADER = ROOT / "src/SO3World/Src/KTeamServer.h"
SOURCE = ROOT / "src/SO3World/Src/KTeamServer.cpp"


def field_names(text):
    body = re.search(r"struct KTEAM_MEMBER_INFO\s*\{(.*?)\n\};", text, re.S).group(1)
    return re.findall(r"\b(?:char|DWORD|KPORTRAIT_INFO|BOOL|KCAMP|int)\s+([A-Za-z_][A-Za-z0-9_]*)", body)


class KTeamServerTargetContract(unittest.TestCase):
    def test_member_record_target_field_order(self):
        expected = [
            "szRoleName", "dwMemberID", "PortraitInfo", "bOnlineFlag", "dwMapID",
            "nMapCopyIndex", "dwForceID", "eCamp", "nLevel", "nMaxLife", "nMaxMana",
            "nMaxRage", "nMaxEnergy", "nMaxSunEnergy", "nMaxMoonEnergy", "nLifePercent",
            "nManaPercent", "nRagePercent", "nEnergyPercent", "nSunEnergyPercent",
            "nMoonEnergyPercent", "nPosX", "nPosY", "nRoleType", "bDeathFlag",
            "dwMiniAvatarID", "dwMountKungfuID",
        ]
        self.assertEqual(field_names(HEADER.read_text()), expected)

    def test_get_team_miss_is_quiet(self):
        body = re.search(r"KTeam\* KTeamServer::GetTeam\(.*?\n\}\n", SOURCE.read_text(), re.S).group(0)
        self.assertNotIn("KGLOG_PROCESS_ERROR", body)
        self.assertIn("m_TeamTable.find(dwTeamID)", body)
        self.assertIn("return pTeam", body)

    def test_formation_effect_order_is_target_order(self):
        body = re.search(r"BOOL KTeamServer::SetFormationLeader\(.*?\n\}\n\nBOOL KTeamServer::SetMark", SOURCE.read_text(), re.S).group(0)
        get_player = body.index("m_PlayerSet.GetObj(dwTargetID)")
        leader_write = body.index("dwFormationLeader = dwTargetID")
        script_exists = body.index("PlayerAutoCastFormation.lua")
        safe_begin = body.index("SafeCallBegin")
        push_player = body.index("PushValueToStack(pPlayer)")
        call_function = body.index('CallFunction("scripts/player/PlayerAutoCastFormation.lua", "OnCastFormation", 0)')
        safe_end = body.index("SafeCallEnd")
        self.assertLess(get_player, leader_write)
        self.assertLess(leader_write, script_exists)
        self.assertLess(script_exists, safe_begin)
        self.assertLess(safe_begin, push_player)
        self.assertLess(push_player, call_function)
        self.assertLess(call_function, safe_end)

    def test_formation_missing_script_keeps_target_debug_failure(self):
        body = re.search(r"BOOL KTeamServer::SetFormationLeader\(.*?\n\}\n\nBOOL KTeamServer::SetMark", SOURCE.read_text(), re.S).group(0)
        self.assertIn('KGLogPrintf(KGLOG_DEBUG, "KGLOG_PROCESS_ERROR(%s) at line %d in %s\\n", "bRetCode", 0x172b, "BOOL KPlayer::CallAutoCastFormationScript()")', body)
        self.assertIn('PORT-DEFERRED_WIRING[IMPORT]', body)


if __name__ == "__main__":
    unittest.main()
