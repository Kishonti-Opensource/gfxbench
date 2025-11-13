#ifndef OSINCLUDE_DUMMY_H
#define OSINCLUDE_DUMMY_H

#define OS_INVALID_TLS_INDEX -1

typedef int OS_TLSIndex;

inline void* OS_GetTLSValue( OS_TLSIndex i)
{
	extern void *TLSpool;
	return TLSpool;
}

inline void OS_SetTLSValue( OS_TLSIndex i, void *d)
{
	extern void *TLSpool;
	TLSpool = d;
}

inline OS_TLSIndex OS_AllocTLSIndex()
{
	extern void *TLSpool;
	TLSpool = 0;
	return 0;
}

inline void OS_FreeTLSIndex( OS_TLSIndex i)
{
}


#endif
