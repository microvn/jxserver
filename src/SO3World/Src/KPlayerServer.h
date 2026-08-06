/************************************************************************/
/* ��Ϸ��������ͻ��˵�����                                             */
/* Copyright : Kingsoft 2005										    */
/* Author	 : Zhu Jianqiu												*/
/* Modify	 : Spe														*/
/* Remark	 : ��Ҫ�����޸ķָ������ע�ͣ����Զ����ɴ���ĺ�����Ҫʹ�� */
/* History	 :															*/
/*		2005.01.26	Create												*/
/************************************************************************/
#ifndef _KPLAYER_SERVER_H_
#define _KPLAYER_SERVER_H_

#if defined(_SERVER)
#include "Common/KG_Socket.h"
#include "Common/KG_Package.h"

#include <map>
#include <vector>
#include "IServer.h"
#include "KConnectionData.h"
#include "KExteriorDef.h"
#include "GS_Client_Protocol.h"
#include "KRecorderFactory.h"

class KSceneObject;
class KCharacter;
class KNpc;
class KPlayer;
class KDoodad;
class IKG_Buffer;
class KTarget;
class KRegion;
class KItem;
class KQuest;
class KShop;
class KSkill;
class KScene;
struct KPARTY_INFO;
struct KPARTY_MEMBER_INFO;
struct KG_SKILL_RECIPE_LIST_NODE;
struct KSKILL_CHAIN_TARGET;
struct KG_FELLOWSHIP_ALL_GS;
struct KG_FOE_ALL_GS;
struct KG_BLACK_LIST_ALL_GS;
struct KTongMemberInfo;
struct KPQ_STATISTICS_DATA;
struct KTEAM_MEMBER_INFO;
struct KTeam;

class KPlayerServer
{
public:
	KPlayerServer(void);
	~KPlayerServer(void);

	BOOL Init(IRecorderFactory* piFactory);

	BOOL UnInit(void);

	void Breathe(void);

    BOOL Attach(KPlayer* pPlayer, int nConnIndex);
    BOOL Detach(int nConnIndex);

	BOOL Send(int nConnIndex, void* pvData, size_t uDataLen);
	void FlushSend();

    KConnectionData* GetConnection(int nConnIndex)
    {
        KConnectionData* pConnection = NULL;
        if (nConnIndex >= 0 && nConnIndex < m_nMaxConnection)
        {
            pConnection = &m_ConnectionDataList[nConnIndex];
        }
        return pConnection;
    }

    int   GetConnectionCount()
    {
        return m_nMaxConnection - (int)m_ConnectionDataListFreeVector.size();
    }

    size_t GetNetworkFlux()
    {
        return m_uSecondFlux;
    }
    
public:
	int					m_nListenPort;		
	DWORD				m_dwInternetAddr;
    char                m_szLocalIP[16];

private:
    int                 m_nPingCycle;
	KConnectionData*	m_ConnectionDataList;
	int					m_nMaxConnection;
    int                 m_nMaxClientPackPerFrame;
    int                 m_nClientGM;
    int                 m_nNextCheckPingConnection;
    DWORD               m_dwTimeNow;
    size_t              m_uAddUpFlux;
    size_t              m_uSecondFlux;
    DWORD               m_dwNextFluxUpdateTime;

    ISocketServer*      m_piSocketServer;
    int                 m_nSocketEventCount;                            
    KG_SOCKET_EVENT*    m_pSocketEventArray;

    typedef vector<unsigned>                KG_CONNECTION_DATA_LIST_FREE_VECTOR;
    KG_CONNECTION_DATA_LIST_FREE_VECTOR     m_ConnectionDataListFreeVector;

	typedef void (KPlayerServer::*PROCESS_PROTOCOL_FUNC)(char*, size_t, int, int);
	PROCESS_PROTOCOL_FUNC	m_ProcessProtocolFuns[client_gs_connection_end];

	int m_nProtocolSize[client_gs_connection_end];

    struct KPROTOCOL_STAT_INFO
    {
        DWORD   dwPackCount;
        int64_t nTotalSize;
    };
    KPROTOCOL_STAT_INFO m_DownwardProtocolStatTable[gs_client_connection_end];
    BYTE                m_byTempData[MAX_EXTERNAL_PACKAGE_SIZE];

    int _Construct();
    int _Destroy();

    inline KPlayer* GetPlayerByConnection(int nConnIndex)
    {
        if (nConnIndex != -1)
        {
            assert(nConnIndex >= 0 && nConnIndex < m_nMaxConnection);
            return m_ConnectionDataList[nConnIndex].pPlayer;
        }
        return NULL;
    }

	BOOL Shutdown(int nConnIndex);

    BOOL ProcessNewConnection(IKG_SocketStream* piSocket);
    BOOL ProcessPackage(IKG_SocketStream* piSocket);
    BOOL DumpPakckageStat();
	BOOL CheckPackage(BYTE* pbyData, size_t uDataLen);

public:
	// ����Ӧ����Ϣ
	BOOL DoMessageNotify(int nConnIndex, int nType, int nCode, const void* pvData = NULL, size_t uDataLen = 0);
	// ͬ���������
	BOOL DoSyncPlayerBaseInfo(int nConnIndex, KPlayer* pPlayer);
	// ͬ����������
	BOOL DoSyncQuestData(int nConnIndex, DWORD dwDestPlayerID, BYTE* pbyData, size_t uDataLen, BYTE byDataType);
	// ֪ͨ�ͻ����Զ������ߺ�
	BOOL DoAccountKickout(int nConnIndex);
	// ֪ͨ�ͻ����л�������
	BOOL DoSwitchGS(int nConnIndex, DWORD dwPlayerID, GUID& Guid, DWORD dwIPAddr, WORD wPort);
	// ֪ͨ�ͻ��˻���ͼ
    BOOL DoSwitchMap(KPlayer* pPlayer, DWORD dwMapID, int nCopyIndex, int nX, int nY, int nZ);

    // ---------------- Player, NPC, Doodad �ĳ���/��ʧͬ�� --------------------------
    // ͬ��ĳ��Player�������ͻ������
    BOOL DoSyncNewPlayer(int nConnIndex, KPlayer* pPlayer);

    // ͬ��ĳ��NPC�������ͻ������
    BOOL DoSyncNewNpc(int nConnIndex, KNpc* pNpc);

    // ͬ��ĳ��doodad��ĳ���ͻ������
    BOOL DoSyncNewDoodad(int nConnIndex, KDoodad* pDoodad);
    
	// �㲥Doodad��״̬ͬ��
	BOOL DoSyncDoodadState(KDoodad* pDoodad);
	// ֪ͨ�ͻ���ĳ����ɫ���Ƴ���
	BOOL DoRemoveCharacter(KCharacter* pCharacter, BOOL bKilled);
    // �Ƴ�Doodad
    BOOL DoRemoveDoodad(KDoodad* pDoodad);
    
    // ----------------- �ƶ�ͬ�� --------------------------------------------
    BOOL DoSyncMoveCtrl(KPlayer* pPlayer);
    BOOL DoSyncMoveParam(KCharacter* pCharacter);
    BOOL DoAdjustPlayerMove(KPlayer* pPlayer);
    BOOL DoSyncMoveState(KCharacter* pCharacter, BOOL bSyncSelf);
	BOOL DoCharacterDeath(KCharacter* pCharacter, DWORD dwKillerID);
	BOOL DoMoveCharacter(KCharacter* pCharacter, int nX, int nY, int nZ, BOOL bIgnoreBlock);

    // ------------------------------------------------------------------------------

	// ��ɫ��������ͬ��
	BOOL DoCharacterAction(KCharacter* pCharacter, DWORD dwCharacterID, DWORD dwAction);
	// ��ɫת��ͬ��
	BOOL DoCharacterTurn(KCharacter* pCharacter, BOOL bForceTurn, BOOL bTurnImmediately);

    BOOL DoOverTimeAction(KCharacter* pCharacter, int nActionType, DWORD dwActionID, int nDirection, int nDurationFrame);

	// ���������ʾ����
	BOOL DoPlayerDisplayData(DWORD dwPlayerID, DWORD dwCheckSum, int nConnIndex);
	// ����Npc��ʾ����
	BOOL DoNpcDisplayData(DWORD dwNpcID, DWORD dwCheckSum, int nConnIndex);

	// ͬ����ǰ����ֵ,Э��������,˫���������ֵ
	BOOL DoSyncExperience(KPlayer* pPlayer, int nDeltaExp);

    BOOL DoPlayerLevelUp(KPlayer* pPlayer);

    BOOL DoCommonHealth(KCharacter* pCharacter, int nDeltaLife);

	BOOL DoFrameSignal(int nConnIndex);

    // ----------------- fight about --------------------------

	BOOL DoSyncSkillData(int nConnIndex, DWORD dwSkillID, DWORD dwSkillLevel, DWORD dwSkillExp, BOOL bInitial);

    BOOL DoSyncSkillRecipe(int nConnIndex, size_t uArrayCount, KG_SKILL_RECIPE_LIST_NODE* pdwRecipeArray);

	BOOL DoSkillPrepare(
        KCharacter* pCharacter, DWORD dwSkillID, DWORD dwSkillLevel, 
        int nPrepareFrame, KTarget &rTarget
    );
    BOOL DoSkillCast(KCharacter* pCharacter, KSkill* pSkill, KTarget &rTarget,  BOOL bSubCast);
    BOOL DoSyncCommonSkillAutoCastState(int nConnIndex, BOOL bStartOrStop, BOOL bMeleeOrRange);
    BOOL DoResetCooldown(int nConnIndex, DWORD dwCoolDownID, int nEndFrame, int nInterval);
    BOOL DoSkillChainEffect(KCharacter* pCaster, DWORD dwSkillID, DWORD dwSkillLevel, size_t uTargetCount, KSKILL_CHAIN_TARGET* pTarget);

    BOOL DoSkillEffectResult(
        KCharacter* pSkillSrc, KCharacter* pSkillDst, KCharacter* pEffectDst, 
        struct KSKILL_CALCULATE_RESULT *pSkillResult, DWORD dwBulletID
    );

    BOOL DoSyncBehitRepresent(
        KCharacter* pSkillSrc, KCharacter* pSkillDst, KCharacter* pEffectDst,
        struct KSKILL_CALCULATE_RESULT *pSkillResult, DWORD dwBulletID
    );

    // ÿ����ʱ��,���Player��Ŀ���BuffList�Ƿ�仯,����仯,��ͬ��Ŀ�������List
    BOOL DoSyncTargetBuffList(KPlayer* pPlayer);
    BOOL DoSyncTargetTargetBuffList(KPlayer* pPlayer);

    // ͬ�������ѡĿ�������������Ϣ
    BOOL DoSyncTargetDropID(KPlayer* pPlayer);

    // ͬ������Լ���Buff list(ĿǰӦ���ڵ�½��ʱ��)
	BOOL DoSyncBuffList(KPlayer* pPlayer, KPlayer* pDstPlayer);
    // ����Լ���Buff�仯ʱ,����������ͬ��(����Buff)
    BOOL DoSyncBuffSingle(
        KCharacter* pCharacter, int nIndex,
        DWORD dwBuffID, int nLevel, DWORD dwRecipeKey[],
        int nStackNum, int nEndFrame
    );

    BOOL DoSyncRepresentState(KCharacter* pCharacter);

    BOOL DoSyncKungfuMount(int nConnIndex, DWORD dwKungfuID, DWORD dwKungfuLevel);

    BOOL DoSkillBeatBack(KCharacter* pCharacter, int nBeatBackFrame, BOOL bBreak = false);

    BOOL DoSyncSheathState(KCharacter* pCharacter);

    // ---------------- ��ֵͬ�� -------------------------------------

    // �ο��ĵ�: ������ֵͬ����Ҫ.txt
    // L,M,S, �ֱ���Life, Mana, Stamina������ĸ

    BOOL DoSyncSelfMaxLMRS(KPlayer* pPlayer);
    BOOL DoSyncSelfCurrentLMRS(KPlayer* pPlayer);
    BOOL DoSyncSelfWeakInfo(KPlayer* pPlayer);
    BOOL DoSyncTargetMaxLMR(KPlayer* pPlayer);
    BOOL DoSyncTargetCurrentLMR(KPlayer* pPlayer);
    BOOL DoSyncTargetWeakInfo(KPlayer* pPlayer);
    BOOL DoSyncTargetTargetMaxLMR(KPlayer* pPlayer);
    BOOL DoSyncTargetTargetCurrentLMR(KPlayer* pPlayer);
    BOOL DoSyncTeamMemberMaxLMR(DWORD dwTeamID, KTEAM_MEMBER_INFO* pMember);
    BOOL DoSyncTeamMemberCurrentLMRGlobal(DWORD dwTeamID, KTEAM_MEMBER_INFO* pMember);
    BOOL DoSyncTeamMemberCurrentLMRLocal(KPlayer* pPlayer);
    BOOL DoSyncTeamMemberMisc(DWORD dwTeamID, KTEAM_MEMBER_INFO* pMember);
    BOOL DoSyncTeamMemberPositionGlobal(DWORD dwTeamID, KTEAM_MEMBER_INFO* pMember);
    BOOL DoSyncTeamMemberPositionLocal(KPlayer* pPlayer);
	BOOL DoBroadcastCharacterLife(KCharacter* pCharacter);
    BOOL DoSyncPlayerTrain(KPlayer* pPlayer);
    BOOL DoBroadcastPlayerSchoolID(KPlayer* pPlayer);

    // ----------------- party about --------------------------------

    // ��ҵ�½������ʱͬ����������
    BOOL DoLoginTeamSync(KPlayer* pPlayer);
    // ͬ��������ڶ���Ļ�����Ϣ
    BOOL DoSyncTeamBaseInfo(DWORD dwTeamID, KPlayer* pPlayer, BOOL bAddTeamMemberFlag = false);
    // ��������뷢�͵�Ŀ�����
    BOOL DoInvitePlayerJoinTeamRequest(DWORD dwInviteDst, const char cszInviteSrc[]);
    // ��Relayת���������������,���͵��ӳ��ͻ���
    BOOL DoApplyJoinTeamRequest(DWORD dwLeader, const char cszApplySrc[]);
    // ����������Ա��֪ͨ
    BOOL DoTeamAddMemberNotify(DWORD dwTeamID, int nGroupIndex, DWORD dwMemberID);
    // xxx�뿪�����֪ͨ
    BOOL DoTeamDelMemberNotify(DWORD dwTeamID, int nGroupIndex, DWORD dwMemberID);
    BOOL DoTeamChangeAuthorityNotify(DWORD dwTeamID, KTEAM_AUTHORITY_TYPE eType, DWORD dwTargetID);
    BOOL DoTeamSetLootModeNotify(DWORD dwTeamID, int nLootMode, int nRollQuality);
    
    BOOL DoTeamSetFormationLeaderNotify(DWORD dwTeamID, int nGroupIndex, DWORD dwNewFormationLeader);

    // �����ɢ֪ͨ
    BOOL DoTeamDisbandNotify(DWORD dwTeamID);
    // ͬ����Ա����״̬�ĸı�
    BOOL DoSyncTeamMemberOnlineFlag(DWORD dwTeamID, DWORD dwMemberID, BOOL bOnlineFlag);
    // ͬ�����״̬��Ϣ
    BOOL DoSyncPlayerStateInfo(KPlayer* pPlayer);

    // ������
    BOOL DoTeamSetMarkRespond(DWORD dwTeamID, int nMarkType, DWORD dwTargetID);

    // �����Ӫ�����仯
    BOOL DoTeamCampChange(DWORD dwTeamID, KCAMP eCamp);

    BOOL DoTeamLevelUpRaidNotify(DWORD dwTeamID, int nGroupNum);
    
    BOOL DoTeamChangeMemberGroupNotify(
        DWORD dwTeamID, int nSrcGroupIndex, int nDstGroupIndex, DWORD dwDstMemberID, DWORD dwMemberID
    );

    // ----------------- shop about -------------------------------

    BOOL DoOpenShopNotify(int nConnectIndex, DWORD dwShopID);
    BOOL DoSyncShopItem(int nConnIndex, const KShop *pcShop, DWORD dwPageIndex, DWORD dwPosIndex, BOOL bIDChanged);
    BOOL DoSyncTimeLimitReturnItem(
        int nConnIndex, DWORD dwItemID, DWORD dwShopTemplateID, int nShopItemIndex, time_t nEndTime
    );
    BOOL DoSyncTimeLimitSoldListInfo(int nConnIndex, DWORD dwItemID, time_t nEndTime);

    // ------------------ Trading about ---------------------------
    BOOL DoTradingInviteRequest(KPlayer* pTradingInviteSrc, KPlayer *pTradingInviteDst);
    BOOL DoOpenTradingBoxNotify(int nConnIndex, DWORD dwOtherPlayer);
    BOOL DoSyncTradingConfirm(int nConnIndex, DWORD dwPlayerID, BOOL bConfirm);
    BOOL DoSyncTradingItemData(
        int nConnIndex, KItem* pcItem, DWORD dwGridIndex, DWORD dwItemOwnerID, 
        DWORD dwBoxIndex, DWORD dwPosIndex, int nKey
    );
    BOOL DoSyncTradingMoney(int nConnIndex, DWORD dwPlayerID, int nMoney, int nKey);
    BOOL DoTradingComplete(int nConnIndex, BOOL bComplete);
    // ------------------------------------------------------------

	// ͬ����Ǯ����
	BOOL DoSyncMoney(int nConnIndex, int nMoney, BOOL bShowMsg);
    BOOL DoSyncContribution(int nConnIndex, int nContribution);
	// ͬ����������
	BOOL DoSyncItemData(int nConnIndex, DWORD dwPlayerID, KItem* pItem, DWORD dwBox, DWORD dwX);
	// ͬ������ɾ��
	BOOL DoSyncItemDestroy(int nConnIndex, DWORD dwOwnerID, DWORD dwBox, DWORD dwX);
	// ����װ��
	BOOL DoExchangeItem(int nConnIndex, DWORD dwBox1, DWORD dwX1, DWORD dwBox2, DWORD dwX2, DWORD dwAmount);
	// ������Ʒ�������;�
	BOOL DoUpdateItemAmount(int nConnIndex, DWORD dwBox, DWORD dwX, int nAmount);
    // ��һ�װ�㲥
    BOOL DoSyncEquipRepresent(KPlayer* pPlayer, int nIndex, DWORD dwRepresentID);
    // sync exterior (wai-guan) box contents + apply-flag to the owning client
    BOOL DoSyncExteriorBoxData(KPlayer* pPlayer, size_t uCount, const KEXTERIOR_ITEM* pItems);
    BOOL DoSyncApplyExteriorFlag(KPlayer* pPlayer, DWORD dwApplyFlag);
	// ͬ�����߻�õ���Ϣ
	BOOL DoAddItemNotify(KPlayer* pAddItemPlayer, DWORD dwItemID, int nCount, int nDestConnIndex = -1);
    
	// ѧϰ�����
	BOOL DoLearnProfessionNotify(int nConnIndex, DWORD dwProfessionID);
	// ѧϰ����ܷ�֧
	BOOL DoLearnBranchNotify(int nConnIndex, DWORD dwProfessionID, DWORD dwBranchID);
	// ��������
	BOOL DoForgetProfessionNotify(int nConnIndex, DWORD dwProfessionID);
	// ��������ܾ���
	BOOL DoAddProfessionProficiency(int nConnIndex, DWORD dwProfessionID, DWORD dwExp);
    // ͬ������ܱ���������Ӱ��ĵ����ȼ�
    BOOL DoSyncProfessionAdjustLevel(int nConnIndex, DWORD dwProfessionID, DWORD dwAdjustLevel);
	// ������������ȼ�
	BOOL DoSetMaxProfessionLevelNotify(int nConnIndex, DWORD dwProfessionID, DWORD dwLevel);
	// ͬ��������������
	BOOL DoUpdatePlayerProfession(
        int nConnIndex, DWORD dwProfessionID, DWORD dwMaxLevel, 
		DWORD dwCurrentLevel, DWORD dwCurrentProficiency, DWORD dwBranchID
    );

	// ѧϰ�䷽
	BOOL DoLearnRecipeNotify(int nConnIndex, DWORD dwCraftID, DWORD dwRecipeID);
	// �����䷽
	BOOL DoForgetRecipeNotify(int nConnIndex, DWORD dwCraftID);
	// ͬ���䷽�б�
	BOOL DoSyncRecipeState(int nConnIndex, BYTE* pbyData, size_t uDataLen, int nVersion);

	// ��һ���ͻ��˴���
	BOOL DoOpenWindow(DWORD dwIndex, TARGET_TYPE eTarget, DWORD dwTargetID, const char* pcszText, int nConnIndex);

	// ͬ���䷽��ʹ��
	BOOL DoStartUseRecipe(KPlayer* pPlayer, DWORD dwCraftID, DWORD dwRecipeID, KTarget &rTarget);

	// ��������
	BOOL DoAcceptQuest(int nQuestIndex, DWORD dwQuestID, int nConnIndex, BOOL bDailyQuest = false);
	// �������
	BOOL DoFinishQuest(DWORD dwQuestID, int nConnIndex, BOOL bForceFinished);
	// ȡ������
	BOOL DoCancelQuest(int nQuestIndex, int nConnIndex);
	// �������
	BOOL DoClearQuest(int nConnIndex, DWORD dwQuestID);
	// ��������
	BOOL DoShareQuest(KPlayer* pDstPlayer, KPlayer* pPlayer, DWORD dwQuestID);
	// ͬ���������
	BOOL DoSyncQuestValue(int nQuestIndex, int nConnIndex);
	// ͬ����ʼС������
	BOOL DoStartEscortQuest(int nConnIndex, DWORD dwStartPlayerID, DWORD dwQuestID);

	// ͬ��ʰȡ�б�
	BOOL DoSyncLootList(KPlayer* pPlayer, KDoodad* pDoodad);

	BOOL DoRollItemMessage(int nDestConnIndex, KPlayer* pRoller, int nRollPoint, DWORD dwDoodadID, DWORD dwItemID);

	// ��ʰȡ���
	BOOL DoOpenLootList(int nConnIndex, KPlayer *pPlayer, KDoodad *pDoodad);

	// ͬ��ս������
	BOOL DoSyncFightFlag(int nConnIndex, DWORD dwForceID, BOOL bFight);

	// ͬ������
	BOOL DoSyncRepute(int nConnIndex, DWORD dwForceID, int nLevel, int nRepute, int nDiffValue);
	// ͬ����������ս������
	BOOL DoSyncFightflagList(int nConnIndex, int nCount, KPlayer* pPlayer);
	// ͬ�����е���������
	BOOL DoSyncReputeList(int nConnIndex, int nCount, KPlayer* pPlayer);
	// ͬ�������޸ı��
	BOOL DoShowReputation(int nConnIndex, DWORD dwForceID, BOOL bLoginLoading);

    BOOL DoCharacterOpen(KCharacter *pCharacter, KDoodad* pDoodad);
	BOOL DoDistributeItem(int nConnIndex, DWORD dwDestPlayerID, DWORD dwItemID);

    BOOL DoFinishLoot(KPlayer* pPlayer, KDoodad* pDoodad);
	BOOL DoBeginRollItem(int nConnIndex, KItem* pItem, KDoodad* pDoodad, int nLeftRollFrame);

	// ͬ�����Ƶ��û���Ϣ
	BOOL DoCustomOTAction(
        KPlayer* pPlayer, int nType, BOOL bIsBreakable, int nFrame, int nTargetType, 
        DWORD dwTargetID, DWORD dwScriptID, const char* pszActionName
    );

	// ���ӳ��
	BOOL DoModifyThreat(int nConnIndex, DWORD dwCharacterID);

	// �޸ı����
	BOOL DoModifyThreaten(int nConnIndex, DWORD dwCharacterID);

	// ������
	BOOL DoClearThreat(int nConnIndex, DWORD dwCharacterID);

	// ��������
	BOOL DoClearThreaten(int nConnIndex, DWORD dwCharacterID);

    // ������Ϣ
    BOOL DoTalkMessage(
        int nMsgType, 
        DWORD dwTalkerID, const char cszTalker[], 
        DWORD dwReceiverID, const char cszReceiver[], 
        size_t uDataLen, BYTE* pbyTalkData, 
        int nRange = 0
    );

    BOOL DoSceneMessage(KScene* pScene, const char* pszMessage);

	// ��������
	BOOL DoSyncSelfCurrentST(KPlayer* pPlayer);

	// 
	BOOL DoDoodadLootOver(KDoodad* pDoodad);

    BOOL DoSyncFellowshipList(DWORD dwPlayerID, int nCount, KG_FELLOWSHIP_ALL_GS* pData);
    BOOL DoSyncFoeList(DWORD dwPlayerID, int nCount, KG_FOE_ALL_GS* pData);
    BOOL DoSyncBlackList(DWORD dwPlayerID, int nCount, KG_BLACK_LIST_ALL_GS* pData);
    BOOL DoSyncFellowshipGroupName(DWORD dwPlayerID);

	BOOL DoNpcTalkToPlayer(KNpc* pNpc, DWORD dwDialogID, NPC_TALK_ACTION_TYPE eTalkActionType, DWORD dwTalkIndex, CHARACTER_TALK_TYPE eTalkType, KTarget& rTarget);

	BOOL DoSetForce(KCharacter* pCharacter);
	
    // -------------------- �ʼ�ϵͳ ----------------------------------->
    BOOL DoSendMailRespond(int nConnIndex, int nRespondID, int nResult);
    BOOL DoGetMailListRespond(int nConnIndex, KMailListInfo* pMailListInfo, int nMailCount);
    BOOL DoNewMailNotify(int nConnIndex, KMailListInfo* pMailListInfo);
    BOOL DoSyncMailContent(DWORD dwPlayerID, DWORD dwMailID, int nResult, KMailContent* pContent);
    BOOL DoMailGeneralRespond(DWORD dwPlayerID, DWORD dwMailID, int nResult);
    // <------------------- �ʼ�ϵͳ ------------------------------------

	// --------------------- PK About ----------------------------------->
	BOOL DoSyncPKState(int nConnIndex, KPlayer* pPlayer, int nEndFrame = 0, BOOL bRefuseDuelFlag = false);
    BOOL DoBroadcastPKState(KPlayer* pPlayer);
	BOOL DoWinDuelBroadcast(DWORD dwWinnerID, DWORD dwLosserID);

	// ���䷽��
	BOOL DoOpenRecipeBook(int nConnIndex, DWORD dwRecipeBookItemID);

	// ���Ķ�����
	BOOL DoOpenBook(int nConnIndex, DWORD dwBox, DWORD dwX);

	BOOL DoSyncUserPreferences(int nConnIndex, int nOffset, int nLength, BYTE* pbyData);
 
	BOOL DoSyncAllBookState(int nConnIndex, KPlayer* pPlayer);

	BOOL DoSyncBookState(int nConnIndex, DWORD dwBookID, BYTE byState);
 
	BOOL DoCharacterChangeModel(KCharacter* pCharacter);

	BOOL DoSyncNpcDialogFlag(KNpc* pNpc);

    BOOL DoSyncRoleDataOver(int nConnIndex);
    BOOL DoSyncRoleDataSectionCheckRequest(int nConnIndex, BYTE bySectionType);
    BOOL DoSyncRegressionPlayerData(int nConnIndex, int nGradeID, int nDailyCount, BYTE* pbyItemMark);

	BOOL DoSyncEnableBankPackage(int nConnIndex, int nEnabledCount);

	BOOL DoUpdateMapMark(int nConnIndex, int nX, int nY, int nZ, int nType, const char* pszComment);

	BOOL DoSyncMidMapMark(DWORD dwTeamID, DWORD dwSrcPlayerID, int nMapID, int nX, int nY, int nType, const char* pszComment);

	BOOL DoPlaySound(int nConnIndex, DWORD dwSoundID);
	BOOL DoPlayMiniGame(int nConnIndex, DWORD dwGameID);

    BOOL DoSyncHorseFlag(KPlayer* pPlayer);

    // ---------------------- ��ͨ��� --------------------------------
    BOOL DoRoadTrackForceRespond(int nConnIndex);
    BOOL DoSyncRouteNodeOpenList(int nConnIndex, int nCount, int* pnNodeList);
    
    BOOL DoSyncPrinkForce(KPlayer* pPlayer, DWORD dwForceID, int nLevel);

    BOOL DoSyncVisitMapInfo(KPlayer* pPlayer);

    BOOL DoSyncAccumulateValue(KPlayer* pPlayer);

    BOOL DoSetTarget(KPlayer* pPlayer, KCharacter* pCharacter);
    BOOL DoBroadcastTarget(KCharacter* pCharacter);
    BOOL DoSyncKillPoint(KPlayer* pPlayer);
    BOOL DoSetPlayerRedName(KPlayer* pPlayer);
    BOOL DoSetPlayerDialogFlag(KPlayer* pPlayer);
    BOOL DoSetOnlyReviveInSitu(KPlayer* pPlayer);
    BOOL DoSyncFormationCoefficient(KPlayer* pPlayer, DWORD dwMemberID, int nCoefficient);

    BOOL DoJoinBattleFieldRespond(DWORD dwPlayerID, DWORD dwMapID, int nErrorCode);
    BOOL DoLeaveBattleFieldRespond(DWORD dwPlayerID, DWORD dwMapID, int nErrorCode);
    BOOL DoBattleFieldNotify(
        DWORD dwPlayerID, time_t nAvgQueueTime, int nPassTime,
        DWORD dwMapID, int nCopyIndex, BATTLE_FIELD_NOTIFY_TYPE eType
    );
    BOOL DoSyncBattleFieldList(DWORD dwPlayerID, DWORD dwMapID, int nCopyIndex);
    BOOL DoSyncBattleFieldSide(KPlayer* pPlayer);
    BOOL DoBattleFieldEnd(DWORD dwMapID, int nCopyIndex);

    BOOL DoSyncCampInfo(KPlayer* pPlayer);
    BOOL DoSyncCurrentPrestige(KPlayer* pPlayer);
    
    // ---------------------- ������ --------------------------------
    BOOL DoSyncTongMemberInfo(DWORD dwPlayerID, const KTongMemberInfo& crTongMember);
    BOOL DoSyncCorpsChangeValue(DWORD dwPlayerID, int* pnCorpsLevel, int* pnCorpsRoleLevel);
    BOOL DoTongDeleteMemberNotify(DWORD dwPlayerID, DWORD dwMemberID);
    BOOL DoApplyTongInfoRespond(DWORD dwPlayerID, BYTE byRespondType, BYTE* pbyRespondData, size_t uDataLen);
    BOOL DoUpdateTongClientDataVersion(DWORD dwPlayerID, KTONG_UPDATE_FLAG eFrameType, int nUpdateFrame);    
    BOOL DoSyncTongRepertoryPageRespond(
        DWORD dwPlayerID, BYTE byPageIndex, int nServerCounter, BYTE byGetRight, BYTE byPutRight,
        BYTE byUpdateItemCount, BYTE byInvailedItemCount, size_t uDataLen, BYTE byData[]
    );
    BOOL DoInvitePlayerJoinTongRequest(DWORD dwPlayerID, DWORD dwInviterID, DWORD dwTongID, char szInviterName[], char szTongName[]);
    BOOL DoChangePlayerTongNotify(int nConnIndex, DWORD dwTongID, BYTE byReason, char szTongName[]);
    BOOL DoBroadcastTongMessage(int nConnIndex, BYTE byCode, BYTE* pbyData, size_t uDataLen);
    BOOL DoGetTongDescriptionRespond(int nConnIndex, DWORD dwTongID, char szTongName[]);
    BOOL DoBroadcastPlayerChangeTong(KPlayer* pPlayer);
    BOOL DoSyncTongOnlineMessage(int nConnIndex, char szOnlineMessage[], size_t uDataLen);
    BOOL DoApplyOpenTongRepertoryRespond(int nConnIndex, DWORD dwPlayerID, const char szTongName[], BYTE byRepertoryPageNum);
    BOOL DoSyncTongHistoryRespond(int nConnIndex, BYTE byType, DWORD dwStartIndex, int nCount, BYTE byData[], size_t uDataLen);
    BOOL DoSyncTongDiplomacyData(int nConnIndex, const vector<KTONG_DIPLOMACY_RELATION_INFO>& crDiplomacyInfoVector);
    // target: _ZN13KPlayerServer20DoSyncTongTotalCacheEiPKh@0805edce
    BOOL DoSyncTongTotalCache(int nConnIndex, const BYTE* byCacheData);
    // target: _ZN13KPlayerServer25DoBroadcastTongTotalCacheEmPKh@0805ed68
    BOOL DoBroadcastTongTotalCache(DWORD dwTongID, const BYTE* byCacheData);
    // target: _ZN13KPlayerServer26DoBroadcastTongCacheChangeEm15TongTechNodeTagh@0805ae8e
    BOOL DoBroadcastTongCacheChange(DWORD dwTongID, TongTechNodeTag eType, BYTE byValue);
    //-----------------------------------------------------------------
     
    // ---------------------- ��������� --------------------------------
    BOOL DoAuctionLookupRespond(DWORD dwPlayerID, BYTE byRespondID, BYTE byCode, KAUCTION_PAGE_HEAD* pPage);
    BOOL DoAuctionBidRespond(DWORD dwPlayerID, BYTE byCode);
    BOOL DoAuctionSellRespond(DWORD dwPlayerID, BYTE byCode);
    BOOL DoAuctionCancelRespond(DWORD dwPlayerID, BYTE byCode);
    BOOL DoAuctionMessageNotify(DWORD dwPlayerID, BYTE byCode, const char* pszSaleName, int nPrice);

    BOOL DoSetCamp(KPlayer* pPlayer);
    
    BOOL DoSyncBattleFieldStatistics(int nConnIndex, KPlayer* pPlayer, const KPQ_STATISTICS_DATA& crStatisticsData);
    BOOL DoSyncBattleFieldObjective(int nConnIndex, int nIndex, int nCurrentValue, int nTargetValue);
    BOOL DoSyncBattleFieldPQInfo(int nConnIndex, DWORD dwPQID, DWORD dwPQTemplateID, int nLapseTime, int nLeftTime);
    BOOL DoSyncSafeLockInfo(int nConnIndex, DWORD dwMask);

    BOOL DoSyncHeroFlag(KPlayer* pPlayer);
    BOOL DoRemoteLuaCall(int nConnIndex, const char cszFunction[], BYTE* pbyData, size_t uDataLen);

    BOOL DoSyncPlayerReviveCtrl(KPlayer* pPlayer);

    BOOL DoSyncAchievementData(int nConnIndex, KPlayer* pPlayer);
    BOOL DoNewAchievementNotify(KPlayer* pPlayer, int nAchievementID);
    BOOL BroadcastAchievementAnnounce(DWORD dwPlayerID, BYTE byType, const char* pszRoleName, int nAchievementID);
    BOOL DoSyncAchievementPoint(KPlayer* pPlayer, int nDeltaPoint);
    BOOL DoSyncAchievementCount(KPlayer* pPlayer, int nID, int nDeltaCount);

    BOOL DoSyncDesignationData(KPlayer* pPlayer);
    BOOL DoSetGenerationNotify(KPlayer* pPlayer, int nGenerationFlag, int nNameInForceIndex);
    BOOL DoAcquireDesignation(KPlayer* pPlayer, int nPrefix, int nPostfix);
    BOOL BroadcastDesignationAnnounce(DWORD dwPlayerID, const char* szRoleName, int nPrefix, int nPostFix, BYTE byType);
    BOOL DoRemoveDesignation(KPlayer* pPlayer, int nPrefix, int nPostfix);
    BOOL DoSyncPlayerDesignation(KPlayer* pPlayer, int nPrefix, int nPostfix, int nGeneretion, int nByname, BOOL bDisplay);

    // v2.5 NEW: sync one currency (or all 6) to the owning client (s2c_sync_currency).
    BOOL DoSyncCurrency(KPlayer* pPlayer, class KCurrency* pCurrency);
    BOOL DoSyncCurrencyList(KPlayer* pPlayer);

    BOOL DoSyncMaxLevel(int nConnIndex, int nMaxLevel);

    BOOL DoUseItemPrepare(KPlayer* pPlayer, int nFrames, DWORD dwOTAniID, DWORD dwEndAniID, const char* pcszText);

    BOOL DoCreateViewPointRespond(KPlayer* pPlayer);
    BOOL DoRemoveViewPointRespond(KPlayer* pPlayer);
    BOOL DoMoveViewPointRespond(KPlayer* pPlayer);

    BOOL DoAccountEndOfDayTime(KPlayer* pPlayer, int nLeftTime);
    BOOL DoSyncFreeLimitFlagInfo(KPlayer* pPlayer);

    BOOL DoApexProtocol(DWORD dwPlayerID, BYTE* pbyData, size_t nSize);

    BOOL DoGameCardSellRespond(KPlayer* pPlayer, BYTE byCode);
    BOOL DoGameCardBuyRespond(KPlayer* pPlayer, BYTE byCode);
    BOOL DoGameCardLookupRespond(KPlayer* pPlayer, int nTotalCount, int nCount, KGAME_CARD* pCardList);
    BOOL DoGameCardCancelRespond(KPlayer* pPlayer, BYTE byCode);

    BOOL DoSyncCoin(KPlayer* pPlayer);
    BOOL BroadcastSeekMentorYell(
        const char cszRoleName[], const char cszTongName[], BYTE byForceID, BYTE byRoleLevel,BYTE byRoleType
    );
    BOOL BroadcastSeekApprenticeYell(
        const char cszRoleName[], const char cszTongName[], BYTE byForceID, BYTE byRoleLevel,BYTE byRoleType
    );
    BOOL DoSyncMentorData(int nConnIndex, DWORD dwMentor, DWORD dwApprentice, const KMentorRecordBase& crMentorInfo);
    BOOL DoDeleteMentorData(int nConnIndex, DWORD dwMentor, DWORD dwApprentice);

    BOOL DoSyncPlayerCampFlag(KPlayer* pPlayer);

    BOOL DoCharacterThreatRankListRespond(int nConnIndex, DWORD dwThreatListOwerID, int nRankCount, S2C_CHARACTER_THREAT_RANKLIST::_THREAT_RANK_LIST* pRankList);

    //AutoCode:-����Э�麯������-

private:
	// �����������
	void OnHandshakeRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
	void OnClientConfirmReady(char* pData, size_t nSize, int nConnIndex, int nFrame);
	void OnSyncRoleDataSectionCheckRespond(char* pData, size_t nSize, int nConnIndex, int nFrame);
	// �ͻ��˼�����Դ���,������볡��
    void OnApplyEnterScene(char* pData, size_t nSize, int nConnIndex, int nFrame);
	void OnSyncNewPlayerRespond(char* pData, size_t nSize, int nConnIndex, int nFrame);
	void OnSyncNewNpcRespond(char* pData, size_t nSize, int nConnIndex, int nFrame);
	void OnSyncNewDoodadRespond(char* pData, size_t nSize, int nConnIndex, int nFrame);
	void OnAddRegressionRewardItem(char* pData, size_t nSize, int nConnIndex, int nFrame);
	// ��ҵǳ�
	void OnPlayerLogout(char* pData, size_t nSize, int nConnIndex, int nFrame);
    // ����Ping
    void OnPingSignal(char* pData, size_t nSize, int nConnIndex, int nFrame);

    void OnMoveCtrl(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnApplyMoveAdjust(char* pData, size_t nSize, int nConnIndex, int nFrame);
    // ��ɫվ��ָ��
	void OnCharacterStand(char* pData, size_t nSize, int nConnIndex, int nFrame);
	// ��ɫ�߶�ָ��
	void OnCharacterWalk(char* pData, size_t nSize, int nConnIndex, int nFrame);
	// ��ɫ�ܶ�ָ��
	void OnCharacterRun(char* pData, size_t nSize, int nConnIndex, int nFrame);
	// ��ɫ��Ӿָ��
	void OnCharacterSwim(char* pData, size_t nSize, int nConnIndex, int nFrame);
	// ��ɫ��Ծָ��
	void OnCharacterJump(char* pData, size_t nSize, int nConnIndex, int nFrame);
	// ��ɫ����ָ��
	void OnCharacterSit(char* pData, size_t nSize, int nConnIndex, int nFrame);
	// ��ɫת��ָ��
	void OnSyncDirection(char* pData, size_t nSize, int nConnIndex, int nFrame);

    // �ж���ҵ�ǰ��Ϊ
    void OnStopCurrentAction(char* pData, size_t nSize, int nConnIndex, int nFrame);
    // ��ɫ�ͷż���ָ��
	void OnCharacterSkill(char* pData, size_t nSize, int nConnIndex, int nFrame);
	// ��ɫ��������ָ��
	void OnCharacterAction(char* pData, size_t nSize, int nConnIndex, int nFrame);

	// �����ʾ����
	void OnApplyPlayerDisplayData(char* pData, size_t nSize, int nConnIndex, int nFrame);
	// �����ƵĿ��Ͷ�������
	void OnApplyLoFreqData(char* pData, size_t nSize, int nConnIndex, int nFrame);

    void OnMountKungfu(char* pData, size_t nSize, int nConnIndex, int nFrame);

    void OnCancelBuff(char* pData, size_t nSize, int nConnIndex, int nFrame);

    void OnSetSheathState(char* pData, size_t nSize, int nConnIndex, int nFrame);

    void OnSetHideHatFlag(char* pData, size_t nSize, int nConnIndex, int nFrame);

    void OnApplyPlayerBuffList(char* pData, size_t nSize, int nConnIndex, int nFrame);

    // ---------------------- party about ---------------------->

    //����ĳ������
    void OnInviteJoinTeamRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    //�������Ӧ��
    void OnInviteJoinTeamRespond(char* pData, size_t nSize, int nConnIndex, int nFrame);
    //�������xxx�Ķ���
    void OnApplyJoinTeamRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    //��xxx��������Ӧ��
    void OnApplyJoinTeamRespond(char* pData, size_t nSize, int nConnIndex, int nFrame);
    //����ɾ����Ա: ��ӻ�������
    void OnTeamDelMemberRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnTeamChangeAuthorityRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    
    void OnTeamSetLootMode(char* pData, size_t nSize, int nConnIndex, int nFrame);
	void OnTeamSetRollQuality(char* pData, size_t nSize, int nConnIndex, int nFrame);
    
    void OnTeamSetFormationLeader(char* pData, size_t nSize, int nConnIndex, int nFrame);

    // ����С��ͼ�ϵı�Ǹ�����
    void OnTeamMemberNotifySignpost(char* pData, size_t nSize, int nConnIndex, int nFrame);

    // �յ�С�ӱ��
    void OnTeamSetMarkRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);

    void OnTeamLevelUpRaidRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnTeamChangeMemberGroupRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);

    // <---------------------- party about ----------------------

	//��������
	void OnAcceptQuest(char* pData, size_t nSize, int nConnIndex, int nFrame);
	//�������
	void OnFinishQuest(char* pData, size_t nSize, int nConnIndex, int nFrame);
	//ȡ������
	void OnCancelQuest(char* pData, size_t nSize, int nConnIndex, int nFrame);
	// ��������
	void OnShareQuest(char* pData, size_t nSize, int nConnIndex, int nFrame);
	//���ܻ�������
	void OnAcceptEscortQuest(char* pData, size_t nSize, int nConnIndex, int nFrame);

	//Gmָ��
	void OnGmCommand(char* pData, size_t nSize, int nConnIndex, int nFrame);

    // ---------------------- shop about ---------------------->
    void OnOpenShopRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnQueryShopItems(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnShopBuyRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnShopSellRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnShopRepairRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnTimeLimitSoldListBuyRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    // ���󹺻ػع��б����һ����Ʒ
    void OnSoldListBuyRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    // <---------------------- shop about ----------------------

    // ----------------------- Trading about ----------------------------
    void OnTradingInviteRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnTradingInviteRespond(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnTradingAddItem(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnTradingDeleteItem(char*pData, size_t nSize, int nConnIndex, int nFrame);
    void OnTradingSetMoney(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnTradingConfirm(char* pData, size_t nSize, int nConnIndex, int nFrame);
    // <----------------------- Trading about ----------------------------

    void OnRequestTarget(char* pData, size_t nSize, int nConnIndex, int nFrame);

	void OnApplyExchangeItem(char* pData, size_t nSize, int nConnIndex, int nFrame);

	void OnWindowSelect(char* pData, size_t nSize, int nConnIndex, int nFrame);
	void OnCastProfessionSkill(char* pData, size_t nSize, int nConnIndex, int nFrame);

	void OnLootItem(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnRollItem(char* pData, size_t nSize, int nConnIndex, int nFrame);
    
	void OnApplyLootList(char* pData, size_t nSize, int nConnIndex, int nFrame);
	void OnCharacterOpen(char* pData, size_t nSize, int nConnIndex, int nFrame);
	void OnApplyUseItem(char* pData, size_t nSize, int nConnIndex, int nFrame);
	void OnApplyFightFlag(char* pData, size_t nSize, int nConnIndex, int nFrame);
	void OnFinishLoot(char* pData, size_t nSize, int nConnIndex, int nFrame);
	void OnLootMoney(char* pData, size_t nSize, int nConnIndex, int nFrame);
	void OnDistributeItem(char* pData, size_t nSize, int nConnIndex, int nFrame);

    // ---------------------- talk about ---------------------->
    // ����
    void OnTalkMessage(char* pData, size_t nSize, int nConnIndex, int nFrame);


    // ---------------------- ����ϵͳ ----------------------->
    void OnGetFellowshipListRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnAddFellowshipRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnDelFellowshipRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnAddFellowshipGroupRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnDelFellowshipGroupRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnRenameFellowshipGroupRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnSetFellowshipRemarkRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnSetFellowshipGroupRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);

    // <--------------------- ����ϵͳ -----------------------

    // ---------------------- �ʼ�ϵͳ ----------------------->
    void OnSendMailRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnGetMaillistRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnQueryMailContent(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnAcquireMailMoneyRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnAcquireMailItemRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnSetMailRead(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnDeleteMail(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnReturnMail(char* pData, size_t nSize, int nConnIndex, int nFrame);
    
    // <--------------------- �ʼ�ϵͳ -----------------------

	void OnApplyPKOperate(char* pData, size_t nSize, int nConnIndex, int nFrame);
	void OnSyncUserPreferences(char* pData, size_t nSize, int nConnIndex, int nFrame);

	void OnSetBankOpenedFlag(char* pData, size_t nSize, int nConnIndex, int nFrame);
	void OnEnableBankPackage(char* pData, size_t nSize, int nConnIndex, int nFrame);

	void OnSyncMidMapMark(char* pData, size_t nSize, int nConnIndex, int nFrame);

    void OnPlayerLearnSkill(char* pData, size_t nSize, int nConnIndex, int nFrame);
	void OnPlayerLearnRecipe(char* pData, size_t nSize, int nConnIndex, int nFrame);

    void OnPlayerActiveSkillRecipe(char* pData, size_t nSize, int nConnIndex, int nFrame);

    void OnPeekOtherPlayerEquip(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnPeekOtherPlayerBook(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnPeekOtherPlayerQuest(char* pData, size_t nSize, int nConnIndex, int nFrame);

    // ------------------------- ��ͨ��� -----------------------------------
    void OnRoadTrackStartOut(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnRoadTrackForceRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);

    // ------------------------- ������� -----------------------------------
    void OnOpenVenation(char* pData, size_t nSize, int nConnIndex, int nFrame);

    void OnSendGmMessage(char* pData, size_t nSize, int nConnIndex, int nFrame);

    void OnJoinBattleFieldQueueRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnLeaveBattleFieldQueueRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);

    void OnAcceptJoinBattleField(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnGetBattleFieldList(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnLeaveBattleField(char* pData, size_t nSize, int nConnIndex, int nFrame);
    
    // ------------------------- ������ -----------------------------------
    void OnApplyTongRosterRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnApplyTongInfoRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnApplyTongRepertoryPageRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnInvitePlayerJoinTongRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnInvitePlayerJoinTongRespond(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnApplyKickOutTongMemberRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnModifyTongInfoRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnModifyTongSchemaRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnApplyQuitTongRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);

    void OnChangeTongMemberGroupRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnChangeTongMasterRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnChangeTongMemberRemarkRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnGetTongDescriptionRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnSaveMoneyInTongRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnPayTongSalaryRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnGetTongSalaryRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnApplyOpenTongRepertoryRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);

    void OnTakeTongRepertoryItemToPosRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnTakeTongRepertoryItemRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnPutTongRepertoryItemToPosRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnPutTongRepertoryItemRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);

    void OnExchangeTongRepertoryItemPosRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnStackTongRepertoryItemRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);

    // ------------------------------------------------------------------------
    void OnApplyBattleFieldStatistics(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnSetHeroFlag(char* pData, size_t nSize, int nConnIndex, int nFrame);

    // ------------------------- ��������� -----------------------------------
    void OnAuctionLookupRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnAuctionBidRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnAuctionSellRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnAuctionCancelRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    
    void OnApplySetCampRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnRemoteLuaCall(char* pData, size_t nSize, int nConnIndex, int nFrame);

    void OnPlayerReviveRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);

    void OnApplyAchievementDataRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnQueryProductIdentity(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnSetCurrentDesignationRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);

    // ------------------------- �۲����� -----------------------------------
    void OnCreateViewPointRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnRemoveViewPointRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnMoveViewPointRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);

    // ------------------------- ����Ҵ��� -----------------------------------
    void OnApexProtocol(char* pData, size_t nSize, int nConnIndex, int nFrame);

    // ------------------------- �¿����� -----------------------------------
    void OnGameCardSellRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnGameCardBuyRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnGameCardLookupRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);
    void OnGameCardCancelRequest(char* pData, size_t nSize, int nConnIndex, int nFrame);

    void OnApplyCharacterThreatRankList(char* pData, size_t nSize, int nConnIndex, int nFrame);

    //AutoCode:-����Э�麯������-

private:
    void TeamBroadcast(DWORD dwTeamID, void* pvData, size_t uSize);
    BOOL TeamBroadcastTalk(DWORD dwTeamID, DWORD dwTalkerID, void* pvData, size_t uSize);
    BOOL RaidBroadcastTalk(DWORD dwTeamID, DWORD dwTalkerID, void* pvData, size_t uSize);
};

extern KPlayerServer g_PlayerServer;

struct KBroadcastFunc
{
    KBroadcastFunc();
	BOOL operator()(KPlayer* pPlayer);
	void*	        m_pvData;			
	size_t	        m_uSize;			
	int	            m_nMaxSyncCount;
    DWORD           m_dwExcept;         // �㲥ʱ�ų����ID�����
    KSceneObject*   m_pCenterObject;    // �Դ˶���Ϊ�������������
    int             m_nRange;           // �����ڴ˷�Χ�ڲŹ㲥(��λ��),���Ǹ�ֵΪ0
    DWORD           m_dwExclusive;      // �㲥ʱ,ֻ����ʽ�ϵĹ㲥,��ʵֻ�������ID�����
};

struct KBroadcastBeHitFunc
{
    KBroadcastBeHitFunc();
	BOOL operator()(KPlayer* pPlayer);
	void*	        m_pvData;
	size_t	        m_uSize;
	int	            m_nMaxSyncCount;
    KPlayer*        m_pExceptSrc;       // �㲥ʱ�ų������Ҽ������
    KPlayer*        m_pExceptDst;       // �㲥ʱ�ų������Ҽ������
};

struct KBroadcastTalkFunc
{
    KBroadcastTalkFunc();
    BOOL operator()(KPlayer* pPlayer);
    void*	        m_pvData;
    size_t	        m_uSize;
    int	            m_nMaxSyncCount;
    DWORD           m_dwTalkerID;       // ˵���ˣ��㲥ʱ�ų�������������������ˡ�
    KSceneObject*   m_pCenterObject;    // �Դ˶���Ϊ�������������
    int             m_nRange;           // �����ڴ˷�Χ�ڲŹ㲥(��λ��),���Ǹ�ֵΪ0
    DWORD           m_dwExclusive;
};

struct KBroadcastTargetFunc
{
    KBroadcastTargetFunc() : m_pSrcCharacter(NULL){};
    BOOL operator()(KPlayer* pPlayer);
    KCharacter* m_pSrcCharacter;
};

struct KTeamBroadcast
{
    DWORD  m_dwTeamID;
    DWORD  m_dwExceptID;
    void*  m_pvData;
    size_t m_uSize;
    
    KTeamBroadcast();
    BOOL operator () (KTEAM_MEMBER_INFO* pMemberInfo);
};

#endif	//_SERVER
#endif	//_KPLAYER_SERVER_H_
