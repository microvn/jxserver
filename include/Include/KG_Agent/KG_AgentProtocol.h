////////////////////////////////////////////////////////////////////////////////
//
//  FileName    : KG_AgentProtocol.h
//  Version     : 1.0
//  Creater     : zoukewei
//  Create Date : 2006-12-6 16:00:00
//  Comment     : 
//
////////////////////////////////////////////////////////////////////////////////

#ifndef _INCLUDE_KG_AGENT_PROTOCOL_H_
#define _INCLUDE_KG_AGENT_PROTOCOL_H_ 1

#include <map>
#include <vector>

const char KG_AGENT_STRING_CLIENT_TYPE_AGENT_CLIENT[] = "ClientType_AgentClient";
const char KG_AGENT_STRING_CLIENT_TYPE_GM[]           = "ClientType_GM";
const char KG_AGENT_STRING_CLIENT_TYPE_GAME_CENTER[]  = "ClientType_GameCenter";

const char KG_AGENT_STRING_COMMAND_LOGIN[]            = "Login";
const char KG_AGENT_STRING_COMMAND_LOGIN_RESULT[]     = "LoginResult";
const char KG_AGENT_STRING_COMMAND_LOGOUT[]           = "Logout";
const char KG_AGENT_STRING_COMMAND_PING[]             = "Ping";
const char KG_AGENT_STRING_COMMAND_PING_RESULT[]      = "PingResult";

const char KG_AGENT_STRING_COMMAND_EXECUTE_COMMAND_LINE[]        = "ExecuteCommandLine";
const char KG_AGENT_STRING_COMMAND_EXECUTE_COMMAND_LINE_RESULT[] = "ExecuteCommandLineResult";
const char KG_AGENT_STRING_COMMAND_START_SERVER_GROUP[]          = "StartServerGroup";
const char KG_AGENT_STRING_COMMAND_STOP_SERVER_GROUP[]           = "StopServerGroup";
const char KG_AGENT_STRING_COMMAND_PROCESS[]                     = "Process";

const char KG_AGENT_STRING_PARAMETER_NEW_THREAD[]          = "NewThered";
const char KG_AGENT_STRING_PARAMETER_WAIT_EXECUTE_FINISH[] = "WaitExecuteFinish";
const char KG_AGENT_STRING_PARAMETER_WAIT_PING_RESULT[]    = "WaitPingResult";

const int  KG_AGENT_STRING_MAX_SIZE                   = 64;
const int  KG_AGENT_PROTOCOL_MAX_SIZE                 = 0xFFFF;
const int  KG_AGENT_SEQUENCE_ID_INVALID               = 0;
const int  KG_AGENT_MAX_RECEIVE_LOOP_COUNT            = 64;

const char KG_AGENT_STRING_PARAMETER_SEQUENCE_ID[]            = "SequenceID";
const char KG_AGENT_STRING_PARAMETER_RESULT[]                 = "Result";
const char KG_AGENT_STRING_PARAMETER_CLIENT_NAME[]            = "ClientName";
const char KG_AGENT_STRING_PARAMETER_CLIENT_TYPE[]            = "ClientType";
const char KG_AGENT_STRING_PARAMETER_OS_NAME[]                = "OS_Name";
const char KG_AGENT_STRING_PARAMETER_OS_VERSION[]             = "OS_Version";
const char KG_AGENT_STRING_PARAMETER_FIRST_TIME_LOGIN[]       = "FirstTimeLogin";
const char KG_AGENT_STRING_PARAMETER_DSET_AGENT_CLIENT_NAME[] = "DestAgentClientName";
const char KG_AGENT_STRING_PARAMETER_MAX_SEQUENCE_ID[]        = "MaxSequenceID";
const char KG_AGENT_STRING_PARAMETER_MIN_SEQUENCE_ID[]        = "MinSequenceID";

typedef enum _KG_AGENT_ERRORNO
{
    KG_AGENT_ERRORNO_CLIENT_NAME_NOT_EXIST = -5,
    KG_AGENT_ERRORNO_CLIENT_ALREADY_LOGIN  = -4,
    KG_AGENT_ERRORNO_FAIL                  = -1,

    KG_AGENT_ERRORNO_INVALID = 0,
    KG_AGENT_ERRORNO_SUCCESS,
    KG_AGENT_ERRORNO_PING,
    KG_AGENT_ERRORNO_STARTING_PING,

    KG_AGENT_ERRORNO_STARTING_GROUP = 20,
    KG_AGENT_ERRORNO_START_PROGRAM_FAIL,
    KG_AGENT_ERRORNO_START_PROGRAM_SUCCESS,
    KG_AGENT_ERRORNO_START_GROUP_FAIL,
    KG_AGENT_ERRORNO_START_GROUP_SUCCESS,

    KG_AGENT_ERRORNO_STOPPING_GROUP = 30,
    KG_AGENT_ERRORNO_STOP_PROGRAM_FAIL,
    KG_AGENT_ERRORNO_STOP_PROGRAM_SUCCESS,
    KG_AGENT_ERRORNO_STOP_GROUP_FAIL,
    KG_AGENT_ERRORNO_STOP_GROUP_SUCCESS
} KG_AGENT_ERRORNO;

typedef enum KG_AGENT_PROTOCOL_TYPE
{
    KG_AGENT_PROTOCOL_TYPE_INVALID = 0,
    KG_AGENT_PROTOCOL_TYPE_REQUEST,
    KG_AGENT_PROTOCOL_TYPE_RESPONSE,
};

typedef struct
{
    bool operator()(const char *X, const char *Y) const
	{
        return (strcmp(X, Y) < 0);
	}
} KG_AGENT_STRICMP_LESS;

typedef std::map<char *, char *, KG_AGENT_STRICMP_LESS> COMMAND_PARAMETER_MAP;

typedef struct _KG_AGENT_PROTOCOL
{
    char                   *pszCommand;
    int                     nSequenceID;
    int                     nResult;
    int                     nProtocolType;
    COMMAND_PARAMETER_MAP   CommandParameterMap;
    char                   *pszContext;
} KG_AGENT_PROTOCOL;

#endif // _INCLUDE_KG_AGENT_PROTOCOL_H_
