from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
INCLUDE = ROOT / "include/Include/SO3World"
SRC = ROOT / "src/SO3World/Src"

def test_existing_worldsettings_direct_closure_contract_is_present():
    required = [
        "KReputeLootBuffList.h", "KNpcAdronTab.h", "KGWServerConstList.h",
        "KReputeLootBuffList.cpp", "KNpcAdronTab.cpp", "KGWServerConstList.cpp",
    ]
    assert all((INCLUDE / name).is_file() for name in required[:3])
    assert all((SRC / name).is_file() for name in required[3:])
    adron = (SRC / "KNpcAdronTab.cpp").read_text()
    assert "m_nAOECountPercent" in adron
    assert "m_dwMapID" in adron and "m_nCopyIndex" in adron
    assert "m_NpcAdronMap.clear();" in adron
    assert "const KADRON_INFO* KNpcAdronTab::GetAdronInfo" in adron
    assert "GetHeight() > 1" in adron
    assert "KScene::MakeMapKey" in adron
    assert "PORT-DEFERRED_WIRING[IMPORT]" not in adron
    scene_header = (INCLUDE / "KScene.h").read_bytes().decode("latin1")
    scene_source = (SRC / "KScene.cpp").read_bytes().decode("latin1")
    assert "static DWORD MakeMapKey(DWORD dwMapID, int nCopyIndex);" in scene_header
    assert "DWORD KScene::MakeMapKey(DWORD dwMapID, int nCopyIndex)" in scene_source

    repute = (SRC / "KReputeLootBuffList.cpp").read_text()
    assert "m_ReputeLootBuffList[dwLootBuffID] = dwLootReputeID" in repute
    assert ".insert(std::make_pair(dwLootBuffID, dwLootReputeID)).second" not in repute

    server_const = (SRC / "KGWServerConstList.cpp").read_text()
    assert "assert(pIniFile);" in server_const
    assert "KGLOG_PROCESS_ERROR(pIniFile);" not in server_const.split("BOOL KGWServerConstList::LoadData", 1)[1].split("BOOL KGWServerConstList::Init", 1)[0]

def test_worldsettings_wiring_contract_is_present():
    header = (INCLUDE / "KWorldSettings.h").read_text()
    source = (SRC / "KWorldSettings.cpp").read_text()
    for name in ("m_ReputeLootBufferList", "KNpcAdronTab", "m_ServerConstList",
                 "m_AntiFarmerSettings", "m_OldPendentDataInfoList", "m_TongConstList"):
        assert name in header or name in source
    assert source.index("m_ReputeLootBufferList.Init") < source.index("m_NpcAdronTab.Init")
    assert source.index("m_AntiFarmerSettings.Init") < source.index("m_OldPendentDataInfoList.Init")
    assert source.index("m_OldPendentDataInfoList.Init") < source.index("m_TongConstList.Init")
    assert "LoadVersionConfig" in source
    assert "Init_ForEditor" in header and "UnInit_ForEditor" in header
    assert "m_Exterior" not in header
    assert "m_HairShop" not in header
    assert "m_MiniAvatarSettings" not in header
    assert "PORT-UNKNOWN_REQUIRED[ABI]" not in header

    uninit = source.split("BOOL KWorldSettings::UnInit(void)", 1)[1]
    cleanup = [
        "m_NpcTeamList.UnInit", "m_OrderManager.UnInit", "m_NpcAdronTab.UnInit",
        "m_SmartDialogList.UnInit", "m_CharacterActionList.UnInit",
        "m_ReputeLootBufferList.UnInit", "m_ReputeLimit.UnInit",
        "m_WeaponTypeList.UnInit", "m_CoolDownList.UnInit", "m_LevelUpList.UnInit",
        "m_DoodadTemplateList.UnInit", "m_NpcTemplateList.UnInit",
        "m_QuestInfoList.UnInit", "m_DoodadClassList.UnInit",
        "m_NpcClassList.UnInit", "m_PatrolPathList.UnInit",
        "m_ConstList.UnInit", "m_AchievementInfoList.UnInit",
        "m_DesignationList.UnInit", "m_GameCardInfoList.UnInit",
        "m_AntiFarmerSettings.UnInit", "m_OldPendentDataInfoList.UnInit",
        "m_TongConstList.UnInit",
    ]
    positions = [uninit.index(item) for item in cleanup]
    assert positions == sorted(positions)

def test_anti_farmer_settings_direct_closure_contract_is_present():
    header_path = INCLUDE / "KAntiFarmerSettings.h"
    source_path = SRC / "KAntiFarmerSettings.cpp"
    assert header_path.is_file()
    assert source_path.is_file()
    header = header_path.read_text()
    source = source_path.read_text()
    assert "class KAntiFarmerSettings" in header
    assert "m_NpcWhiteList" in header
    assert "std::set<unsigned long>" in header
    assert "AppendIgnoreNpcTemplateID" in header
    assert "LoadNpcTemplateIDWiteList" in header
    assert "AntiFarmerWhiteList.tab" in source
    assert "AntiFarmer.ini" in source
    assert 'GetInteger("Global", "Enable"' in source
    assert 'GetInteger("Global", "DetectAroundSameBehaviorThreshold", 5' in source
    assert 'GetInteger("Global", "DetectProcessSpeed", 5' in source
    assert 'GetInteger("Global", "PunishFreezeFarmerNum", 10' in source
    assert 'GetFloat("Global", "EquipSimilarityThreshold", .9f' in source
    assert "m_nCheckOnlineTimeRange > 30" in source
    assert "m_fEquipSimilarityThreshold >= 0.5f" in source
    assert "std::set<unsigned long>" in header
    assert "std::set<uint64_t>" not in header

def test_pendent_old_data_direct_closure_contract_is_present():
    header_path = INCLUDE / "KPendentOldDataInfoList.h"
    source_path = SRC / "KPendentOldDataInfoList.cpp"
    assert header_path.is_file()
    assert source_path.is_file()
    header = header_path.read_text()
    source = source_path.read_text()
    assert "class KPendentOldDataInfoList" in header
    assert "m_WaistRepresentID2ItemIDMap" in header
    assert "GetOldWaistPendentItemID" in header
    assert "GetDuplicateItemID" in header
    assert "OldPendentRepresentID2ItemID.tab" in source
    assert "pRepresentMap->insert" in source
    assert "dwItemID, itInsert.first->second" in source
    assert "DuplicateRepresentID" not in source

def test_tong_const_direct_closure_contract_is_present():
    header_path = INCLUDE / "KTongConstList.h"
    source_path = SRC / "KTongConstList.cpp"
    assert header_path.is_file()
    assert source_path.is_file()
    header = header_path.read_text()
    source = source_path.read_text()
    assert "class KTongConstList" in header
    assert "m_nTongContractWarTimeSegment[8]" in header
    assert "m_CraftDiscountMap" in header
    assert "LoadCraftTech" in header
    assert "TongConstList.ini" in source
    assert 'TongAllyRefusedWaitTime' in source
    assert 'NewMemberLimitTime", 0' in source
    assert 'TongWarCD", 24 * 60 * 60' in source
    assert 'TongContractWarTime", 2 * 60 * 60' in source
    assert 'TongAllyWaitTime", 15 * 60' in source
    assert 'TongAllyRefusedWaitTime", 30 * 60' in source
    assert 'StopAllianceCoolTime", 24 * 60 * 60' in source
    assert 'TongDiplomacy' in source
    assert "GetCraftDiscountInfo(unsigned long" in header
    assert "GetCraftDiscountInfo(int" not in header
    assert source.index('LoadCraftTech(pIniFile, "Cooking"') < source.index('GetInteger("TongDiplomacy", "TongWarCD"')
    assert source.index('GetInteger("TongDiplomacy", "StopAllianceCoolTime"') < source.index('GetInteger("Misc", "RepairDiscount"')
    assert source.count("KGLOG_CHECK_ERROR(bRetCode)") == 4
    assert "PORT-UNKNOWN_REQUIRED[FAILURE]" not in source
