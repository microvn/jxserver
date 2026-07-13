/************************************************************************/
/* NPC			                                                        */
/* Copyright : Kingsoft 2005										    */
/* Author	 : Zhu Jianqiu												*/
/* History	 :															*/
/*		2004.12.30	Create												*/
/************************************************************************/
#ifndef _KNPC_H_
#define _KNPC_H_

#include "KShop.h"
#include "KCharacter.h"
#include "INpcFile.h"

enum  KNPC_SPECIES
{
    ensInvalid = 0, 
    ensHumanoid,    // ���͹�
    ensBeast,       // Ұ��
    ensMechanical,  // ľ��
    ensUndead,      // ��ʬ, ��ʬ����
    ensGhost,       // ����
    ensPlant,       // ֲ��
    ensLegendary,   // ���ޣ�ʲô�������׻�����ȸ������ȵ�
    ensCritter,     // С����,�����
    ensOther,       // δ����
    ensTotal
};

class KPlayer;
class KNpcTemplate;
class S2C_NPC_DISPLAY_DATA;

class KNpc : public KCharacter
{
public:
	BOOL Init();
	void UnInit();
	BOOL Activate(void);

public:
    int             m_nIntensity;       // ǿ��
	DWORD			m_dwScriptID;		// �󶨽ű�ID		
	KShop*          m_pShop;            // �󶨵��̵�
	DWORD			m_dwTemplateID;		// ģ����
    int             m_nReviveTime;      // ����ʱ�䣬��λ��
	int				m_nDisappearFrames;	// ��ʧ֡��
	BOOL			m_bDisappearToReviveList; //��ʧ��ɾ��,���ǽ���������
    KNPC_SPECIES    m_eSpecies;

    // ���Npc�����Ƿ��п��ԶԻ��Ľű�,���������Ϊfalse�Ļ�,������MarkҲ����ʾ
    // ע��,������Ϊֻ��Ϊ�ͻ��˱��ֶ����õ�,��û���߼��ϵı�Ȼ��ϵ
	BOOL			m_bDialogFlag;

    // ���Npc�Ƿ�ÿ֡���Զ�Active
    BOOL            m_bActiveMark;

	BOOL			m_bDisableAutoTurn;	// ���Ի�ʱ�Զ�ת��
    MOVE_MODE       m_eMoveMode;

	DWORD			m_dwReliveID;					//����Ⱥ��ı��
    DWORD           m_dwRandomID;                   //����Ⱥ�M���S�C��̖����0�r���S�C
	int				m_nNpcTeamID;					//NpcС��ID
	int				m_nNpcTeamIndex;				//NPCС���еı�ţ�û�ж����npcΪ-1
	int				m_nSkillCommomCD;						//Npc���ܵĹ���CD,��ֹNpcһ���ͷŶ������
	int				m_nSkillCastFrame[MAX_NPC_AI_SKILL];	//NPCÿ�����ܵ�����ͷ�ʱ��

#if defined(_CLIENT)
	int				m_nUpdateMiniMapMarkFrame;		// ����С��ͼ��ǵļ�ʱ��
#endif 

#if defined(_SERVER)
	BOOL			m_bBank;						// �Ƿ�������
    BOOL            m_bMailbox;                     // �Ƿ�����
    DWORD           m_dwSkillMasterID;              // �书������ѵ��ID
    DWORD           m_dwCraftMasterID;              // �������ѵ��ID
    BOOL            m_bAuction;                     // �Ƿ������
    BOOL            m_bTongRepertory;               // �Ƿ���ֿ�

    int             m_nPathFindAstar;
    
    BOOL            m_bDropNotQuestItemFlag;        // �Ƿ������������
    int             m_nAllAttackerLevel;            // ������Ŀ���⣬���й����ҵĽ�ɫ�ĵȼ�֮��
#endif 
    DWORD           m_dwDropTargetPlayerID;         // ����Ŀ��

	KNpcTemplate*	m_pTemplate;					// ָ��ģ���ָ��
    DWORD           m_dwEmployer;                   // ���NPC�Ĺ�����Ҳ�������ˣ�һ�������NPC��û�й�����

#ifdef _SERVER
    // -- ��AI��� ------------------------------------------------------->
    int             m_nSkillSelectIndex;            // ����ѡ�����Ľ��
    // <----------------------------------------------------------------------
#endif // _SERVER

#ifdef _SERVER
    int             m_nReviveGameLoop;
    int             m_nInFightGameLoop;

    int             m_nProgressID;                  // ��NPC�����Ľ���ID
#endif

public:
#ifdef _CLIENT
	void UpdateMiniMapMark();
#endif

	BOOL Load(const KNPC_DATA& crNpcData);

	virtual BOOL ChangeRegion(KRegion* pDestRegion);

#ifdef _SERVER
    void RegisterToViewRegion(KRegion* pFromRegion, KRegion* pToRegion);
#endif

	int GetNpcRelation(KNpc* pTarget);
	int GetPlayerRelation(KPlayer* pTarget);

	BOOL Say(char* pszText, int nRange = 0);		    // ˵��(��Ļ����)
	BOOL Yell(char* pszText);						    // ˵��(��������)
    BOOL Whisper(KPlayer* pTarget, char* pszText);	    // ˵��(����)
    BOOL SayTo(KPlayer* pTarget, char* pszText);	    // ��ָ�����˵��
    BOOL YellTo(KPlayer* pTarget, char* pszText);	    // ��ָ����Ҵ�
    BOOL SayToCamp(KCAMP eCamp, char* pszText);        // NPC��ӪƵ��

    BOOL SayToTargetUseSentenceID(KPlayer* pTarget, DWORD dwSentenceID);	// ��ָ�����˵��

    BOOL SetRevivePoint(int nX, int nY, int nZ, int nDirection);            // ����NPC������λ��
    BOOL GetRevivePoint(KScene* pScene, int* pnRetX, int* pnRetY, int* pnRetZ, int* pnRetDirection);    // ��ȡNPC������λ��

#ifdef _SERVER
    BOOL SetScript(const char* pszFileName);
    void LootExp(KPlayer* pDropTarget);
    void LootPrestige(KPlayer* pDropTarget);
    void LootContribution(KPlayer* pDropTarget);
    void LootJustice(KPlayer* pDropTarget);        // v2.5: drop cbtJustice to eligible players
    void LootExamPrint(KPlayer* pDropTarget);       // v2.5: drop cbtExamPrint
    void LootActivityAward(KPlayer* pDropTarget);   // v2.5: drop cbtActivityAward
    void LootCurrencyToEligible(KPlayer* pDropTarget, int nType, int nAmount);  // v2.5 shared helper
    void LootAchievement(KPlayer* pDropTarget);
    void LootCampScore();
	BOOL AdjustLevel();
	BOOL SetDisappearFrames(int nFrames, BOOL bDisappearToReviveList = false);
	// �Լ��ص������б�,�൱�ڱ�����
	BOOL GotoReviveList(); 
	// Npc�����Զ��Ի����ı�
	BOOL GetAutoDialogString(KPlayer* pPlayer, char* szBuffer, size_t uBufferSize);
    
    KPlayer*    GetDropTarget();
    KDoodad*    GenerateCorpse(void);
    void        ClearDropTarget();
    void        InitDialogFlag();
#endif //_SERVER

protected:

#if defined(_CLIENT)
	int m_nNextActionFrame;
#endif	//_CLIENT

public:

#ifdef _SERVER
    BOOL Revive();
    BOOL CallDeathScript(DWORD dwKiller);
#endif

public:
	DECLARE_LUA_CLASS(KNpc);

    DECLARE_LUA_INTEGER(Intensity);
	DECLARE_LUA_DWORD(ScriptID);
	DECLARE_LUA_DWORD(TemplateID);
    DECLARE_LUA_INTEGER(ReviveTime);
	DECLARE_LUA_BOOL(DialogFlag);
	DECLARE_LUA_BOOL(DisableAutoTurn);
    DECLARE_LUA_ENUM(Species);
    DECLARE_LUA_DWORD(DropTargetPlayerID);
    DECLARE_LUA_DWORD(Employer);

#ifdef _SERVER
	DECLARE_LUA_DWORD(ReliveID);
    DECLARE_LUA_DWORD(RandomID);
	DECLARE_LUA_INTEGER(NpcTeamID);
	DECLARE_LUA_INTEGER(NpcTeamIndex);
#endif //_SERVER

#ifdef _SERVER
	int LuaGetQuestString(Lua_State* L);	
	int LuaGetAutoDialogString(Lua_State* L);
    int LuaBindNpcShop(Lua_State* L);
    int LuaUnbindNpcShop(Lua_State* L);
    int LuaGetShop(Lua_State* L);
	int	LuaSetBank(Lua_State* L);

	int LuaFollowTarget(Lua_State* L);
	int LuaModifyThreat(Lua_State* L);		// �ű��޸ĳ��
    int LuaCopyThreatList(Lua_State* L);
    int LuaSetActiveMark(Lua_State* L);
	int LuaSetPatrolPath(Lua_State* L);
    int LuaSetNpcTeamID(Lua_State* L);
	int LuaSetScript(Lua_State *L);

	int LuaSetDisappearFrames(Lua_State* L);
    int LuaGotoReviveList(Lua_State* L);
    int LuaGetTemplateReviveTime(Lua_State* L);

	int LuaSetDialogFlag(Lua_State* L);

	int LuaSay(Lua_State* L);
	int LuaYell(Lua_State* L);
    int LuaWhisper(Lua_State* L);
    int LuaSayTo(Lua_State* L);
    int LuaYellTo(Lua_State* L);
    int LuaSayToCamp(Lua_State* L);

    int LuaGetAISelectSkill(Lua_State* L);
    int LuaCopyAITarget(Lua_State* L);
#endif
    int LuaIsSelectable(Lua_State* L);
    int LuaCanSeeLifeBar(Lua_State* L);
    int LuaCanSeeName(Lua_State* L);
    int LuaGetNpcQuest(Lua_State* L);

#ifdef _SERVER
    int LuaSetBattleZone(Lua_State* L);
#endif
};
#endif	//_KNPC_H_
