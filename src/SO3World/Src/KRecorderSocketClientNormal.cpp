#include "stdafx.h"
#include "KRecorderSocketClientNormal.h"

HRESULT STDMETHODCALLTYPE KRecorderSocketClientNormal::QueryInterface(REFIID riid, void** ppvObject)
{
    KG_USE_ARGUMENT(riid);
    KG_USE_ARGUMENT(ppvObject);
    return KG_E_FAIL;
}

ULONG STDMETHODCALLTYPE KRecorderSocketClientNormal::AddRef(void)
{
    return (ULONG)KG_InterlockedIncrement((long*)&m_ulRefCount);
}

ULONG STDMETHODCALLTYPE KRecorderSocketClientNormal::Release(void)
{
    ULONG ulCount = KG_InterlockedDecrement((long*)&m_ulRefCount);
    if (ulCount == 0)
    {
        delete this;
    }
    return ulCount;
}

IKG_SocketStream* KRecorderSocketClientNormal::Connect(const char cszIP[], int nPort)
{
    IKG_SocketStream*   piSocketStream = NULL;
    KG_SocketConnector  Connector;
    
    assert(cszIP);

    /* [R7] real KG security handshake (ConnectSecurity) instead of plain Connect --
       center's game-server listener does AcceptSecurity(KSG_ENCODE_DECODE); our recon
       socket skipped this -> center rejected at accept. libcommon.a provides the real impl. */
    piSocketStream = Connector.ConnectSecurity(cszIP, nPort, KSG_ENCODE_DECODE);
	KGLOG_PROCESS_ERROR(piSocketStream);

Exit0:
    return piSocketStream;
}

