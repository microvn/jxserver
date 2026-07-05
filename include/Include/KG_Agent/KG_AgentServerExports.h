////////////////////////////////////////////////////////////////////////////////
//
//  FileName    : KG_AgentServerExports.h
//  Version     : 1.0
//  Creator     : tongxuehu
//  Create Date : 01/18/2007
//  Comment     : 
//  
//  Copyright (C) 2007 - All Rights Reserved
////////////////////////////////////////////////////////////////////////////////

#ifndef _KG_AGENT_SERVER_EXPORTS_H_
#define _KG_AGENT_SERVER_EXPORTS_H_ 1

#ifdef KG_AGENTSERVER_EXPORTS
#define KG_AGENT_SERVER_API  extern "C" __declspec(dllexport)
#else
#define KG_AGENT_SERVER_API  extern "C" __declspec(dllimport)
#endif

KG_AGENT_SERVER_API int CreateAgentServerInstance(
    void **ppvAgentServerHandel
);

KG_AGENT_SERVER_API int DestroyAgentServerInstance(
    void **ppvAgentServerHandel
);

#endif // _KG_AGENT_SERVER_EXPORTS_H_