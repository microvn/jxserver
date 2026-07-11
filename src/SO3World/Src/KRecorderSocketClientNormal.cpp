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

    /* [R8] plain Connect (NO security handshake). RE of libcommon.a proved the
       center's GS listener runs KG_SocketServerAcceptor with ENCODE_DECODE_MODE_NONE
       (_WaitProcessAccept -> _SendSecurityKey(mode) which is a no-op for mode -1, and
       the center sends zero bytes on accept). The earlier ConnectSecurity path (R7)
       was the wrong layer: brute-forcing all 5 modes RST-ed identically. The real
       reject is the center's app-level S2R_HANDSHAKE_REQUEST handler. */
    piSocketStream = Connector.Connect(cszIP, nPort);
	KGLOG_PROCESS_ERROR(piSocketStream);

Exit0:
    return piSocketStream;
}

