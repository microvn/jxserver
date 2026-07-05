/* Reconstructed KG_Socket / KG_Packer layer (the `common` base networking lib
 * didn't leak; only the interface headers kg_socket.h / kg_package.h did).
 * Linux/POSIX + epoll implementation matching those interfaces. Framing is a
 * 4-byte little-endian length prefix (protocol-interop with the real client is a
 * separate validation; this makes the server LINK and self-consistently talk).
 * Encode/decode modes are treated as pass-through (NONE) for now. */
#include "windows.h"
#include "kg_socket.h"
#include "kg_package.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <cstring>
#include <cstdlib>
#include <vector>

extern IKG_Buffer *KG_MemoryCreateBuffer(unsigned uSize);

/* ---- IKG_SocketStream implementation over a blocking/nonblocking fd ---- */
namespace {

class KG_SocketStreamImpl : public IKG_SocketStream
{
	long   m_lRef;
	int    m_nFd;
	void  *m_pvUserData;
	int    m_nLastError;
public:
	explicit KG_SocketStreamImpl(int nFd)
		: m_lRef(1), m_nFd(nFd), m_pvUserData(NULL), m_nLastError(0) {}
	~KG_SocketStreamImpl() { if (m_nFd >= 0) ::close(m_nFd); }
	int Fd() const { return m_nFd; }

	/* IUnknown */
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, void **ppv)
		{ if (ppv) { *ppv = this; AddRef(); return S_OK; } return E_POINTER; }
	ULONG STDMETHODCALLTYPE AddRef()  { return (ULONG)(++m_lRef); }
	ULONG STDMETHODCALLTYPE Release() { long r = --m_lRef; if (r == 0) delete this; return (ULONG)r; }

	int SetTimeout(const timeval *) { return 1; }

	int CheckCanSend(const timeval *) { return 1; }

	/* [len:4 LE][payload] */
	int Send(IKG_Buffer *piBuffer)
	{
		if (!piBuffer) return -1;
		unsigned uSize = piBuffer->GetSize();
		unsigned char head[4] = {
			(unsigned char)(uSize & 0xff), (unsigned char)((uSize >> 8) & 0xff),
			(unsigned char)((uSize >> 16) & 0xff), (unsigned char)((uSize >> 24) & 0xff) };
		if (_SendAll(head, 4) != 1) return -1;
		if (uSize && _SendAll((unsigned char *)piBuffer->GetData(), uSize) != 1) return -1;
		return 1;
	}

	int CheckCanRecv(const timeval *) { return 1; }

	int Recv(IKG_Buffer **ppiRetBuffer)
	{
		if (!ppiRetBuffer) return -1;
		*ppiRetBuffer = NULL;
		unsigned char head[4];
		int n = _RecvAll(head, 4);
		if (n <= 0) return n;            /* -2 again / -1 err / 0 closed */
		unsigned uSize = (unsigned)head[0] | ((unsigned)head[1] << 8)
		               | ((unsigned)head[2] << 16) | ((unsigned)head[3] << 24);
		if (uSize > MAX_PACKAGE) return -1;
		IKG_Buffer *piBuffer = KG_MemoryCreateBuffer(uSize ? uSize : 1);
		if (!piBuffer) return -1;
		if (uSize && _RecvAll((unsigned char *)piBuffer->GetData(), uSize) != 1)
			{ piBuffer->Release(); return -1; }
		*ppiRetBuffer = piBuffer;
		return 1;
	}

	int IsAlive()
	{
		char c; int r = ::recv(m_nFd, &c, 1, MSG_PEEK | MSG_DONTWAIT);
		if (r == 0) return 0;
		if (r < 0 && errno != EAGAIN && errno != EWOULDBLOCK) return 0;
		return 1;
	}

	int GetRemoteAddress(struct in_addr *pRemoteIP, u_short *pusRemotePort)
	{
		struct sockaddr_in addr; socklen_t len = sizeof(addr);
		if (::getpeername(m_nFd, (struct sockaddr *)&addr, &len) != 0) return -1;
		if (pRemoteIP)   *pRemoteIP   = addr.sin_addr;
		if (pusRemotePort) *pusRemotePort = ntohs(addr.sin_port);
		return 1;
	}

	int SetUserData(void *pv) { m_pvUserData = pv; return 1; }
	void *GetUserData()       { return m_pvUserData; }
	int GetLastError()        { return m_nLastError; }

private:
	int _SendAll(const unsigned char *p, unsigned n)
	{
		unsigned off = 0;
		while (off < n) {
			ssize_t w = ::send(m_nFd, p + off, n - off, MSG_NOSIGNAL);
			if (w > 0) { off += (unsigned)w; continue; }
			if (w < 0 && (errno == EINTR)) continue;
			m_nLastError = errno; return -1;
		}
		return 1;
	}
	int _RecvAll(unsigned char *p, unsigned n)
	{
		unsigned off = 0;
		while (off < n) {
			ssize_t r = ::recv(m_nFd, p + off, n - off, 0);
			if (r > 0) { off += (unsigned)r; continue; }
			if (r == 0) return 0;                       /* peer closed */
			if (errno == EINTR) continue;
			if (errno == EAGAIN || errno == EWOULDBLOCK) return off ? -1 : -2;
			m_nLastError = errno; return -1;
		}
		return 1;
	}
};

} // anon

/* ---- KG_SocketConnector ---- */
KG_SocketConnector::KG_SocketConnector() { std::memset(&m_BindLoaclAddr, 0, sizeof(m_BindLoaclAddr)); }

IKG_SocketStream *KG_SocketConnector::Connect(const char cszIPAddress[], int nPort)
{
	int fd = ::socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) return NULL;
	int one = 1; ::setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
	struct sockaddr_in addr; std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port   = htons((u_short)nPort);
	addr.sin_addr.s_addr = (cszIPAddress && *cszIPAddress) ? inet_addr(cszIPAddress) : htonl(INADDR_LOOPBACK);
	if (::connect(fd, (struct sockaddr *)&addr, sizeof(addr)) != 0) { ::close(fd); return NULL; }
	return new KG_SocketStreamImpl(fd);
}

IKG_SocketStream *KG_SocketConnector::ConnectSecurity(
	const char cszIPAddress[], int nPort, ENCODE_DECODE_MODE)
{ return Connect(cszIPAddress, nPort); }   /* encode/decode = pass-through for now */

IKG_SocketStream *KG_SocketConnector::ProxyConnect(
	const char[], int, const char[], const char[], const char cszIPAddress[], int nPort)
{ return Connect(cszIPAddress, nPort); }    /* no proxy support: direct connect */

IKG_SocketStream *KG_SocketConnector::ProxyConnectSecurity(
	const char[], int, const char[], const char[], const char cszIPAddress[], int nPort, ENCODE_DECODE_MODE)
{ return Connect(cszIPAddress, nPort); }

int KG_SocketConnector::Bind(const char cszLocalIPAddress[], int nPort)
{
	std::memset(&m_BindLoaclAddr, 0, sizeof(m_BindLoaclAddr));
	m_BindLoaclAddr.sin_family = AF_INET;
	m_BindLoaclAddr.sin_port   = htons((u_short)nPort);
	m_BindLoaclAddr.sin_addr.s_addr =
		(cszLocalIPAddress && *cszLocalIPAddress) ? inet_addr(cszLocalIPAddress) : htonl(INADDR_ANY);
	return 1;
}

/* ---- KG_SocketServerAcceptor (epoll) ---- */
int KG_SocketServerAcceptor::Init(
	const char cszIPAddress[], int nPort, int nMaxAcceptEachWait,
	int nMaxRecvBufSizePerSocket, int nMaxSendBufSizePerSocket,
	ENCODE_DECODE_MODE EncodeDecodeMode, void *)
{
	m_nMaxAcceptEachWait        = nMaxAcceptEachWait > 0 ? nMaxAcceptEachWait : 64;
	m_nMaxRecvBufSizePerSocket  = nMaxRecvBufSizePerSocket;
	m_nMaxSendBufSizePerSocket  = nMaxSendBufSizePerSocket;
	m_EncodeDecodeMode          = EncodeDecodeMode;

	m_nListenSocket = ::socket(AF_INET, SOCK_STREAM, 0);
	if (m_nListenSocket < 0) return 0;
	int one = 1; ::setsockopt(m_nListenSocket, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
	struct sockaddr_in addr; std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port   = htons((u_short)nPort);
	addr.sin_addr.s_addr = (cszIPAddress && *cszIPAddress) ? inet_addr(cszIPAddress) : htonl(INADDR_ANY);
	if (::bind(m_nListenSocket, (struct sockaddr *)&addr, sizeof(addr)) != 0) { ::close(m_nListenSocket); return 0; }
	if (::listen(m_nListenSocket, 128) != 0) { ::close(m_nListenSocket); return 0; }
	::fcntl(m_nListenSocket, F_SETFL, ::fcntl(m_nListenSocket, F_GETFL, 0) | O_NONBLOCK);

	m_nEpollHandle = ::epoll_create(1024);
	if (m_nEpollHandle < 0) { ::close(m_nListenSocket); return 0; }
	struct epoll_event ev; std::memset(&ev, 0, sizeof(ev));
	ev.events = EPOLLIN; ev.data.fd = m_nListenSocket;
	::epoll_ctl(m_nEpollHandle, EPOLL_CTL_ADD, m_nListenSocket, &ev);
	return 1;
}

int KG_SocketServerAcceptor::UnInit(void *)
{
	if (m_nEpollHandle >= 0) { ::close(m_nEpollHandle); m_nEpollHandle = -1; }
	if (m_nListenSocket >= 0) { ::close(m_nListenSocket); m_nListenSocket = -1; }
	return 1;
}

int KG_SocketServerAcceptor::Wait(int nEventCount, KG_SOCKET_EVENT *pEvent, int *pnRetEventCount)
{
	if (pnRetEventCount) *pnRetEventCount = 0;
	if (!pEvent || nEventCount <= 0) return 0;
	std::vector<struct epoll_event> events(nEventCount);
	int n = ::epoll_wait(m_nEpollHandle, &events[0], nEventCount, 1);
	if (n < 0) return (errno == EINTR) ? 1 : 0;
	int out = 0;
	for (int i = 0; i < n && out < nEventCount; i++) {
		int fd = events[i].data.fd;
		if (fd == m_nListenSocket) {
			for (int a = 0; a < m_nMaxAcceptEachWait; a++) {
				int c = ::accept(m_nListenSocket, NULL, NULL);
				if (c < 0) break;
				int one = 1; ::setsockopt(c, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
				KG_SocketStreamImpl *pStream = new KG_SocketStreamImpl(c);
				struct epoll_event ev; std::memset(&ev, 0, sizeof(ev));
				ev.events = EPOLLIN; ev.data.ptr = pStream;
				::epoll_ctl(m_nEpollHandle, EPOLL_CTL_ADD, c, &ev);
				if (out < nEventCount) {
					pEvent[out].uEventFlag = KG_SOCKET_EVENT_ACCEPT;
					pEvent[out].piSocket   = pStream;
					out++;
				}
			}
		} else {
			pEvent[out].uEventFlag = KG_SOCKET_EVENT_IN;
			pEvent[out].piSocket   = (IKG_SocketStream *)events[i].data.ptr;
			out++;
		}
	}
	if (pnRetEventCount) *pnRetEventCount = out;
	return 1;
}

int KG_SocketServerAcceptor::_WaitProcessAccept(int, KG_SOCKET_EVENT *, int *) { return 1; }
int KG_SocketServerAcceptor::_WaitProcessRecv(int, KG_SOCKET_EVENT *, int *)   { return 1; }

/* ---- KG_Packer (send buffering) ---- */
int KG_Packer::Send(IKG_SocketStream *piSocketStream, unsigned uBufferSize, const unsigned char cbyBuffer[])
{
	if (!piSocketStream) return -1;
	if (!m_piBuffer) {
		m_piBuffer = KG_MemoryCreateBuffer(m_uPackSize);
		if (!m_piBuffer) return -1;
		m_piBuffer->QueryInterface(IID_IKG_Buffer_ReSize, (void **)&m_piBufferReSize);
		m_pbyBufferBegin = m_pbyBufferPos = (unsigned char *)m_piBuffer->GetData();
		m_pbyBufferEnd   = m_pbyBufferBegin + m_piBuffer->GetSize();
	}
	if (m_pbyBufferPos + uBufferSize > m_pbyBufferEnd) {
		int r = FlushSend(piSocketStream);
		if (r != 1) return r;
	}
	if (uBufferSize > m_uPackSize) {           /* oversized: send directly */
		IKG_Buffer *pTmp = KG_MemoryCreateBuffer(uBufferSize);
		if (!pTmp) return -1;
		std::memcpy(pTmp->GetData(), cbyBuffer, uBufferSize);
		int r = piSocketStream->Send(pTmp);
		pTmp->Release();
		return r;
	}
	std::memcpy(m_pbyBufferPos, cbyBuffer, uBufferSize);
	m_pbyBufferPos += uBufferSize;
	return 1;
}

int KG_Packer::FlushSend(IKG_SocketStream *piSocketStream)
{
	if (!piSocketStream || !m_piBuffer) return 1;
	unsigned uUsed = (unsigned)(m_pbyBufferPos - m_pbyBufferBegin);
	if (uUsed == 0) return 1;
	int r = 1;
	if (m_piBufferReSize) {
		m_piBufferReSize->SetSmallerSize(uUsed);
		r = piSocketStream->Send(m_piBuffer);
		m_piBufferReSize->ResetSize();
	}
	m_pbyBufferPos = m_pbyBufferBegin;
	return r;
}

int KG_Packer::Reset()
{
	m_pbyBufferPos = m_pbyBufferBegin;
	return 1;
}
