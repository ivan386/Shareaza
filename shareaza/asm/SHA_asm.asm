//
// SHA_asm.asm
//
// Copyright (c) Shareaza Development Team, 2002-2004.
// This file is part of SHAREAZA (www.shareaza.com)
//
// Shareaza is free software; you can redistribute it
// and/or modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// Shareaza is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Shareaza; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

; ##########################################################################################
;
; SHA_asm - Implementation of SHA-1 for x86 - use together with SHA.cpp and SHA.h
;
; created              4.7.2004         by Camper
;
; last modified        7.7.2004         by Camper
;
; ##########################################################################################

                        .586p
                        .model      flat, C 
                        option      casemap:none                    ; case sensitive
                        option      prologue:none                   ; we generate our own entry/exit code
                        option      epilogue:none

; ##########################################################################################


m_nCount0                equ         0                              ; offsets as laid out in SHA.h
m_nCount1                equ         4

m_nHash0                 equ         8
m_nHash1                 equ         12
m_nHash2                 equ         16
m_nHash3                 equ         20
m_nHash4                 equ         24

m_nBuffer                equ         28

                         .data?


; we place variables in the data segment to save stack space
; note: it's easy to change back to local vars on the stack, just add a frame and replace _w and _t
; sub   esp, 81*4
; _w    textequ    <esp+4>
; _t    textequ    <esp>
; __this needs to be changed too
; we will lose ideal alignment for _w (unless we add additional code, i doubt it would improve anything)

                        ALIGN       16                          ; best alignment for _w, useful with SSE2

_w                      dd          80 dup (?)
                        public      _w
_t                      dd          ?                           ; temporal storage

; #define rnd(f,k)   t = a; a = rotl32(a,5) + f(b,c,d) + e + k + w[i]; e = d; d = c; c = rotl32(b, 30); b = t
; ##define ch(x,y,z)       (((x) & (y)) ^ (~(x) & (z)))
; ##define parity(x,y,z)   ((x) ^ (y) ^ (z))
; #define maj(x,y,z)      (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))

; we use aliases for registers that hold vars as follows
; those aliases change within the macros as the registers that hold those vars change - this saves a few mov commands
; we'll initialize them at the appropriate time

;reg_a                   textequ     <eax>
;reg_b                   textequ     <ebx>
;reg_c                   textequ     <ecx>
;reg_d                   textequ     <edx>
;reg_e                   textequ     <esi>

RND_CH                  MACRO       const:REQ

; t=a; a=rotl32(a,5)+e+k+w[i]+((b&c)^(~b&d)); e=d; d=c; c=rotl32(b,30); b=t

                        mov         reg_temp1, reg_a                        ; t=a
                        mov         reg_temp2, reg_c                        ; save c

                        rol         reg_a, 5
                        add         reg_a, reg_e
                        add         reg_a, const
                        add         reg_a, [_w+count*4]

                        mov         reg_e, reg_b
                        and         reg_b, reg_c                            ; b&c
                        mov         reg_c, reg_e
                        not         reg_e
                        ror         reg_c, 2                                ; c=rotl32(b,30)
                        and         reg_e, reg_d                            ; ~b&d
                        xor         reg_b, reg_e                            ; ( )^( )
                        add         reg_a, reg_b

reg_t                   textequ     reg_e
reg_e                   textequ     reg_d                                   ; e=d, saves mov reg_e,reg_d
reg_d                   textequ     reg_temp2                               ; d=c, saves mov reg_d,reg_temp2
reg_temp2               textequ     reg_t

reg_t                   textequ     reg_b                                   ; b=t
reg_b                   textequ     reg_temp1                               ; save mov reg_d,reg_temp1
reg_temp1               textequ     reg_t

count                   =           count + 1

                        ENDM                                                ; RND_CH

RND_PARITY              MACRO       const:REQ

; t=a; a=rotl32(a,5)+e+k+w[i]+(b^c^d); e=d; d=c; c=rotl32(b,30); b=t

                        mov         reg_temp1, reg_a                        ; t=a

                        rol         reg_a, 5
                        add         reg_a, reg_e
                        add         reg_a, const
                        add         reg_a, [_w+count*4]

                        mov         reg_e, reg_b

reg_t                   textequ     reg_b
reg_b                   textequ     reg_e                                   ; b<->e
reg_e                   textequ     reg_t
                        
                        xor         reg_e, reg_c
                        xor         reg_e, reg_d
                        add         reg_a, reg_e

reg_t                   textequ     reg_e
reg_e                   textequ     reg_d                                   ; e=d
reg_d                   textequ     reg_c                                   ; d=c
reg_c                   textequ     reg_b
                        ror         reg_c, 2                                ; c=rotl32(b,30)
reg_b                   textequ     reg_temp1                               ; b=t
reg_temp1               textequ     reg_t

count                   =           count + 1

                        ENDM                                                ; RND_PARITY

RND_MAJ                 MACRO       const:REQ

; t=a; a=rotl32(a,5)+e+k+w[i]+((b&c)^(b&d)^(c&d)); e=d; d=c; c=rotl32(b,30); b=t

                        mov         _t, reg_a
                        mov         reg_temp1, reg_c
                        mov         reg_temp2, reg_d

                        rol         reg_a, 5
                        add         reg_a, reg_e
                        add         reg_a, const
                        add         reg_a, [_w+count*4]

                        mov         reg_e, reg_b
reg_t                   textequ     reg_b
reg_b                   textequ     reg_e
reg_e                   textequ     reg_t
                        and         reg_e, reg_c
                        and         reg_c, reg_d
                        and         reg_d, reg_b
                        xor         reg_e, reg_c
                        xor         reg_e, reg_d
                        add         reg_a, reg_e

reg_t                   textequ     reg_c
reg_c                   textequ     reg_b                                   ; c=rotl32(b,30)
reg_b                   textequ     reg_t
                        mov         reg_b, _t                               ; b=t
                        ror         reg_c, 2

reg_t                   textequ     reg_e
reg_e                   textequ     reg_temp2                               ; e=d
reg_temp2               textequ     reg_t

reg_t                   textequ     reg_d
reg_d                   textequ     reg_temp1                               ; d=c
reg_temp1               textequ     reg_t
                        
count                   =           count + 1

                        ENDM                                                ; RND_MAJ
                        
INIT_REG_ALIAS          MACRO
reg_accu                textequ     <eax>
reg_base                textequ     <ebp>
reg_i_1                 textequ     <ebx>
reg_i_2                 textequ     <ecx>
reg_i_3                 textequ     <edx>
reg_i_15                textequ     <esi>
reg_i_16                textequ     <edi>
                        ENDM
                        
                        .code

SHA_Compile_p5          PROC

__this                  textequ     <[esp+32+4+4]>                          ; pusha + 2 * ret addr in between

;     for(i = 0; i < SHA1_BLOCK_SIZE / 4; ++i)   w[i] = swap_b32(m_nBuffer[i]);
;     for(i = SHA1_BLOCK_SIZE / 4; i < 80; ++i)   w[i] = rotl32(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);

; ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
; This part can be vectorized
; ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
; we unroll the loops and remember w[i] and w[i-14] cos we need them again a little bit later

                        INIT_REG_ALIAS

; we expect reg_i_16, reg_i_15, reg_i_3, reg_i_2, reg_i_1 to be initialized, the first 16 words in the buffer are filled in little endian order

;                        mov         reg_base, __this
;count                   =           0
;                        REPEAT      16
;                        mov         reg_accu, [reg_base+m_nBuffer+count*4]
;                        bswap       reg_accu
;                        IF          count eq 0
;                        mov         reg_i_16, reg_accu
;                        ELSEIF      count eq 1
;                        mov         reg_i_15, reg_accu
;                        ELSEIF      count eq 13
;                        mov         reg_i_3, reg_accu
;                        ELSEIF      count eq 14
;                        mov         reg_i_2, reg_accu
;                        ELSEIF      count eq 15
;                        mov         reg_i_1, reg_accu
;                        ENDIF
;                        mov         [_w+count*4], reg_accu
;count                   =           count + 1
;                        ENDM
count                   =           16
                        REPEAT      64
                        xor         reg_i_3, reg_i_16                       ; w[i-16]^w[i-3]
reg_i_14                textequ     reg_i_16                                ; we forget w[i-16]
                        IF          count le 77
                        mov         reg_i_14, [_w+(count-14)*4]
                        xor         reg_i_3, reg_i_14
                        ELSE
                        xor         reg_i_3, [_w+(count-14)*4]
                        ENDIF
                        xor         reg_i_3, [_w+(count-8)*4]
                        rol         reg_i_3, 1
                        mov         [_w+count*4], reg_i_3
;now we prepare for the next iteration                        
reg_i_0                 textequ     reg_i_3
reg_i_3                 textequ     reg_i_2
reg_i_2                 textequ     reg_i_1
reg_i_1                 textequ     reg_i_0
reg_i_16                textequ     reg_i_15
reg_i_15                textequ     reg_i_14
count                   =           count + 1
                        ENDM

; ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
; I see no potential for vectorization here
; ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

;
;    a = m_nHash[0];
;    b = m_nHash[1];
;    c = m_nHash[2];
;    d = m_nHash[3];
;    e = m_nHash[4];

reg_a                   textequ     reg_accu
reg_b                   textequ     reg_i_1
reg_c                   textequ     reg_i_2
reg_d                   textequ     reg_i_3
reg_e                   textequ     reg_i_15
reg_temp1               textequ     reg_i_16
reg_temp2               textequ     reg_base

                        mov         reg_temp2, __this
                        mov         reg_a, [reg_temp2+m_nHash0]
                        mov         reg_b, [reg_temp2+m_nHash1]
                        mov         reg_c, [reg_temp2+m_nHash2]
                        mov         reg_d, [reg_temp2+m_nHash3]
                        mov         reg_e, [reg_temp2+m_nHash4]

count                   =           0

                        REPEAT      20
                        RND_CH      05a827999H
                        ENDM
                        REPEAT      20
                        RND_PARITY  06ed9eba1H
                        ENDM
                        REPEAT      20
                        RND_MAJ     08f1bbcdcH
                        ENDM
                        REPEAT      20
                        RND_PARITY  0ca62c1d6H
                        ENDM

                        mov         reg_temp2, __this
                        add         [reg_temp2+m_nHash0], reg_a
                        add         [reg_temp2+m_nHash1], reg_b
                        add         [reg_temp2+m_nHash2], reg_c
                        add         [reg_temp2+m_nHash3], reg_d
                        add         [reg_temp2+m_nHash4], reg_e
 
                        ret

SHA_Compile_p5          ENDP

; we eliminate m_nBuffer[16] and copy data from pData directly to w[i]
; if the buffer can be filled completely we swap and copy at once and thus save time

SHA_Add_p5              PROC        PUBLIC, _this:DWORD, _Data:DWORD, _nLength:DWORD

                        pusha
__this                  textequ     <[esp+36]>                              ; different offset due to pusha
__Data                  textequ     <[esp+40]>
__nLength               textequ     <[esp+44]>
; just incase someone tries to add 0 bytes...
                        xor         ecx, ecx
                        add         ecx, __nLength
                        jz          get_out                                 ; we get out of here

                        mov         edi,__this
                        mov         esi,__Data
; _Data points to the Datastream
; now we need to calculate if the buffer is filled already partially
                        mov         eax, 64-1
                        mov         ebx, [edi+m_nCount0]
                        and         eax, ebx
                        jnz         partial_buffer                          ; special handling required if not empty
                        add         ebx, __nLength
                        mov         [edi+m_nCount0], ebx
                        adc         [edi+m_nCount1], eax
full_blocks:            mov         ecx,__nLength                           ; we spend most of the time here
                        and         ecx,ecx                                 ; we copy full blocks of 64 bytes
                        jz          get_out                                 ; all done?
                        sub         ecx, 64
                        jb          end_of_stream
                        mov         __nLength, ecx
                        INIT_REG_ALIAS
                        mov         reg_base, __Data
count                   =           0
                        REPEAT      16
                        mov         reg_accu, [reg_base+count*4]
                        bswap       reg_accu
                        IF          count eq 0
                        mov         reg_i_16, reg_accu
                        ELSEIF      count eq 1
                        mov         reg_i_15, reg_accu
                        ELSEIF      count eq 13
                        mov         reg_i_3, reg_accu
                        ELSEIF      count eq 14
                        mov         reg_i_2, reg_accu
                        ELSEIF      count eq 15
                        mov         reg_i_1, reg_accu
                        ENDIF
                        mov         [_w+count*4], reg_accu
count                   =           count + 1                        
                        ENDM                                                ; REPEAT
                        add         reg_base, 16*4
                        mov         __Data, reg_base
                        call        SHA_Compile_p5                          ; the first 64 bytes are prepared, now we compile
                        jmp         full_blocks                             ; LOOP

end_of_stream:          mov         esi, __Data                             ; we just copy from the stream directly
                        lea         edx, _w
                        mov         ebx, 4
                        add         ecx, 64
@@:                     mov         eax, [esi]
                        mov         [edx], eax
                        add         esi, ebx
                        add         edx, ebx
                        sub         ecx, ebx
                        ja          @B                                      ; all done?
                        jmp         get_out
                        
partial_buffer:         xor         edx, edx                                ; eax = offset in buffer
                        mov         ecx, __nLength
                        add         [edi+m_nCount0], ecx
                        adc         [edi+m_nCount1], edx
                        add         ecx, eax
                        cmp         ecx, 64
                        jc          short_stream                            ; we can't fill the buffer
                        mov         ecx, -64
                        add         ecx, eax
                        add         __nLength, ecx
                        mov         esi, __Data
@@:                     mov         bl, [esi]
                        inc         esi
                        mov         byte ptr [_w+64+ecx], bl
                        inc         ecx
                        jnz         @B
                        mov         __Data, esi
                        INIT_REG_ALIAS
count                   =           0
                        REPEAT      16
                        mov         reg_accu, [_w+count*4]
                        bswap       reg_accu
                        IF          count eq 0
                        mov         reg_i_16, reg_accu
                        ELSEIF      count eq 1
                        mov         reg_i_15, reg_accu
                        ELSEIF      count eq 13
                        mov         reg_i_3, reg_accu
                        ELSEIF      count eq 14
                        mov         reg_i_2, reg_accu
                        ELSEIF      count eq 15
                        mov         reg_i_1, reg_accu
                        ENDIF
                        mov         [_w+count*4], reg_accu
count                   =           count + 1                        
                        ENDM                                                ; REPEAT
                        call        SHA_Compile_p5                          ; the first 64 bytes are prepared, now we compile
                        jmp         full_blocks

short_stream:           mov         ecx, __nLength
                        mov         esi, __Data
                        add         eax, offset _w
@@:                     mov         bl, byte ptr [esi]
                        inc         esi
                        mov         byte ptr [eax], bl
                        inc         eax
                        dec         ecx
                        jnz          @B

get_out:                popa
                        ret

SHA_Add_p5              ENDP

SHA_Finish_p5           PROC        PUBLIC, _this:DWORD

                        pusha
__this                  textequ     <[esp+36]>                              ; different offset due to pusha
                        INIT_REG_ALIAS
                        mov         edi, __this
                        mov         ecx, [edi+m_nCount0]
                        and         ecx, 64-1
                        mov         byte ptr [_w+ecx], 80h                  ; we have at least one byte left in the buffer
                        sub         ecx, 64-9                               ; we need 9 bytes left in the buffer for padding or we need to start a 2nd one
                        ja          use_2nd_buffer
                        jz          exactly_9_left
                        xor         eax, eax
@@:                     inc         ecx
                        mov         byte ptr [_w+ecx+(64-9)], al
                        jnz         @B
exactly_9_left:         INIT_REG_ALIAS
count                   =           0
                        REPEAT      14
                        mov         reg_accu, [_w+count*4]
                        bswap       reg_accu
                        IF          count eq 0
                        mov         reg_i_16, reg_accu
                        ELSEIF      count eq 1
                        mov         reg_i_15, reg_accu
                        ELSEIF      count eq 13
                        mov         reg_i_3, reg_accu
                        ENDIF
                        mov         [_w+count*4], reg_accu
count                   =           count + 1                        
                        ENDM                                                ; REPEAT
                        mov         reg_base, __this
                        mov         reg_i_2, [reg_base+m_nCount1]
                        mov         reg_i_1, [reg_base+m_nCount0]
                        shld        reg_i_2, reg_i_1, 3                     ; message length in bits, not bytes
                        shl         reg_i_1, 3
                        mov         [_w+count*4], reg_i_2                   ; count=14
count                   =           count + 1
                        mov         [_w+count*4], reg_i_1                   ; count=15
count                   =           count + 1                        
                        call        SHA_Compile_p5
                        jmp         get_out

use_2nd_buffer:         sub         ecx, 8
                        jz          use_2nd_buffer_1_left
                        xor         eax, eax
@@:                     inc         ecx
                        mov         byte ptr [_w+ecx+(64-1)],al
                        jnz         @B
use_2nd_buffer_1_left:  INIT_REG_ALIAS
count                   =           0
                        REPEAT      16
                        mov         reg_accu, [_w+count*4]
                        bswap       reg_accu
                        IF          count eq 0
                        mov         reg_i_16, reg_accu
                        ELSEIF      count eq 1
                        mov         reg_i_15, reg_accu
                        ELSEIF      count eq 13
                        mov         reg_i_3, reg_accu
                        ELSEIF      count eq 14
                        mov         reg_i_2, reg_accu
                        ELSEIF      count eq 15
                        mov         reg_i_1, reg_accu
                        ENDIF
                        mov         [_w+count*4], reg_accu
count                   =           count + 1                        
                        ENDM                                                ; REPEAT
                        call        SHA_Compile_p5
                        INIT_REG_ALIAS
count                   =           0
                        xor         reg_accu, reg_accu
                        REPEAT      14
                        IF          count eq 0
                        mov         reg_i_16, reg_accu
                        ELSEIF          count eq 1
                        mov         reg_i_15, reg_accu
                        ELSEIF      count eq 13
                        mov         reg_i_3, reg_accu
                        ENDIF
                        mov         [_w+count*4], reg_accu
count                   =           count + 1                        
                        ENDM                                                ; REPEAT
                        mov         reg_base, __this
                        mov         reg_i_2, [reg_base+m_nCount1]
                        mov         reg_i_1, [reg_base+m_nCount0]
                        shld        reg_i_2, reg_i_1, 3                     ; message length in bits, not bytes
                        shl         reg_i_1, 3
                        mov         [_w+count*4], reg_i_2                   ; count=14
count                   =           count + 1
                        mov         [_w+count*4], reg_i_1                   ; count=15
count                   =           count + 1                        
                        call        SHA_Compile_p5

get_out:                popa
                        ret

SHA_Finish_p5           ENDP

        end
