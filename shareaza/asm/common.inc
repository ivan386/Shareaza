;/*
;assembler directives follow here

MAX_PARALLEL					equ		6
LIBRARY_BUILDER_BLOCK_SIZE		equ		51200

COMMENT $

assembler will ignore everything from here on
*/
#ifndef MAX_PARALLEL
#define MAX_PARALLEL 6
#endif
#ifndef LIBRARY_BUILDER_BLOCK_SIZE
#define LIBRARY_BUILDER_BLOCK_SIZE 51200		// needs to be divisor of ED2K_PART_SIZE
#endif
#ifndef AFX_COMMON_INC__F47D3B65_FA52__B430_543274BA8CFE__INCLUDED_
#define AFX_COMMON_INC__F47D3B65_FA52__B430_543274BA8CFE__INCLUDED_

inline int GetCPUFlags()
{
	__asm {
		mov	eax, 1
		cpuid
		mov eax, edx
	}
}

inline int SupportsMMX()
{
	return ( GetCPUFlags() & 0x00800000 );
}

inline int SupportsSSE()
{
	return ( GetCPUFlags() & 0x02000000 );
}

inline int SupportsSSE2()
{
	return ( GetCPUFlags() & 0x04000000 );
}

#endif  // #ifndef AFX_COMMON_INC__F47D3B65_FA52__B430_543274BA8CFE__INCLUDED_

// $