/* Reconstructed KG_Memory buffer layer (the `common` base lib didn't leak; only
 * its interface header kg_memory.h did). Implements the COM-style IKG_Buffer +
 * IKG_Buffer_ReSize + the factory KG_MemoryCreateBuffer and KG_MemoryInit/UnInit.
 * GUID values taken verbatim from the kg_memory.h header comments. */
#include "windows.h"
#include "kg_memory.h"
#include <cstdlib>
#include <cstring>

/* GUIDs */
const GUID IID_IUnknown =
	{0x00000000,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}};
const GUID IID_IKG_Buffer =
	{0x9B459A98,0xCB94,0x4f06,{0x81,0xEF,0x22,0x6C,0xAE,0x16,0xC5,0x94}};
const GUID IID_IKG_Buffer_ReSize =
	{0x3E8AE0B1,0x724D,0x42d7,{0xB1,0x87,0x5A,0xC5,0x90,0xAD,0x25,0x01}};

namespace {
class KG_BufferImpl : public IKG_Buffer, public IKG_Buffer_ReSize
{
	long           m_lRef;
	unsigned       m_uSize;
	unsigned       m_uReserve;
	unsigned char *m_pData;
public:
	explicit KG_BufferImpl(unsigned uSize)
		: m_lRef(1), m_uSize(uSize), m_uReserve(uSize)
	{
		m_pData = (unsigned char *)std::malloc(uSize ? uSize : 1);
	}
	~KG_BufferImpl() { std::free(m_pData); }

	/* IUnknown */
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppv)
	{
		if (!ppv) return E_POINTER;
		if (IsEqualGUID(riid, IID_IUnknown) || IsEqualGUID(riid, IID_IKG_Buffer))
			{ *ppv = static_cast<IKG_Buffer *>(this); AddRef(); return S_OK; }
		if (IsEqualGUID(riid, IID_IKG_Buffer_ReSize))
			{ *ppv = static_cast<IKG_Buffer_ReSize *>(this); AddRef(); return S_OK; }
		*ppv = NULL; return E_NOINTERFACE;
	}
	ULONG STDMETHODCALLTYPE AddRef()  { return (ULONG)(++m_lRef); }
	ULONG STDMETHODCALLTYPE Release() { long r = --m_lRef; if (r == 0) delete this; return (ULONG)r; }

	/* IKG_Buffer */
	void    *GetData()        { return m_pData; }
	unsigned GetSize()        { return m_uSize; }
	unsigned GetReserveSize() { return m_uReserve; }

	/* IKG_Buffer_ReSize */
	int ResetSize()                    { m_uSize = m_uReserve; return 1; }
	int SetSmallerSize(unsigned uNew)  { if (uNew <= m_uReserve) { m_uSize = uNew; return 1; } return 0; }
};
} // anon

IKG_Buffer *KG_MemoryCreateBuffer(unsigned uSize) { return new KG_BufferImpl(uSize); }
int KG_MemoryInit(void *)   { return 1; }   /* pool init is a no-op: malloc-backed */
int KG_MemoryUnInit(void *) { return 1; }
