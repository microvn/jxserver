#include "windows.h"
#ifndef KMEMORY_H
#define KMEMORY_H

#include <cstddef>
#include <cstdlib>
#include <new>

/* Reconstructed KMemory. The real engine header (songpo/Sword3-FullSource) makes
 * KMemory a namespace whose kstring is a POOLED basic_string<char,...,KAllocator>.
 * Adopting that verbatim into this stub-based tree regressed the build badly
 * (pooled-kstring vs the std::string the rest of the reconstruction uses -> mass
 * type mismatches). So this keeps a self-consistent std::string-compatible KMemory
 * (class form; KMemory::X resolves the same as the namespace form) with the full
 * New/RawNew/Delete/RawDelete + KAllocator surface the source uses. */
class KMemory
{
public:
	static int   Initialize(const char*){return 1;}
	static void  Finalize(){}
	static void* Alloc(std::size_t s){ return ::malloc(s); }
	static void  Free(void* p){ ::free(p); }
	static void* RawAlloc(std::size_t s){ return ::malloc(s); }
	static void  RawFree(void* p, std::size_t){ ::free(p); }
	static void  RawDelete(void* p){ ::free(p); }
	static int   GetUsedSize(){ return 0; }
	/* Lua allocator callback — defined in libEngine_Lua5D.so (matches
	 * _ZN7KMemory12LuaAllocatorEPvS0_jj). Declaration only. */
	static void* LuaAllocator(void*, void*, unsigned int, unsigned int);

	template<class T> static T* New()      { return new T(); }
	template<class T> static T* RawNew()   { return new T(); }
	template<class T> static void Delete(T* p)    { delete p; }
	template<class T> static void RawDelete(T* p) { delete p; }

	template<class T>
	class KAllocator
	{
	public:
		typedef T                 value_type;
		typedef T*                pointer;
		typedef const T*          const_pointer;
		typedef T&                reference;
		typedef const T&          const_reference;
		typedef std::size_t       size_type;
		typedef std::ptrdiff_t    difference_type;
		template<class U> struct rebind { typedef KAllocator<U> other; };

		KAllocator() throw() {}
		KAllocator(const KAllocator&) throw() {}
		template<class U> KAllocator(const KAllocator<U>&) throw() {}
		~KAllocator() throw() {}

		pointer       address(reference x)       const { return &x; }
		const_pointer address(const_reference x) const { return &x; }
		pointer allocate(size_type n, const void* = 0)
			{ return (pointer)::operator new(n * sizeof(T)); }
		void deallocate(pointer p, size_type) { ::operator delete((void*)p); }
		size_type max_size() const throw() { return size_type(-1) / sizeof(T); }
		void construct(pointer p, const_reference val) { new((void*)p) T(val); }
		void destroy(pointer p) { p->~T(); }
	};
};

template<class T, class U>
inline bool operator==(const KMemory::KAllocator<T>&, const KMemory::KAllocator<U>&) { return true; }
template<class T, class U>
inline bool operator!=(const KMemory::KAllocator<T>&, const KMemory::KAllocator<U>&) { return false; }

#endif
