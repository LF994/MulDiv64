; --------------------------------------------------------------------------------
; extern "C" UINT64 asmMulDiv64(UINT64 a, UINT64 b, UINT64 c)
; Calculates (a*b)/c using intermediate 128 bits for a*b production. 
; This is asm for 64 bits (ML64.exe), so you have to compile C/CPP for 64 bits respectively.
; Shortly about MSVC-64 calling conventions (see more details in MS Docs):
; Up to 4 ptr or int args passed as of: RCX, RDX, R8, R9 (64 bits registers)
; HERE: a in RCX, b in RDX and d in R8
; (if more: push on stack starting fifth one + some issues with float point, see MS Docs)
; The result (up to 64 bits) returned in RAX (can hold 64 bits ptr or up to 64 bits integer).
; Registers RAX, RCX, RDX, R8, R9, R10, R11 (? SSX 8-15 ?).
; are considered volatile and must be considered destroyed on function calls
; It means: use it safely in your asm, assume destroyed if you call another function.
; The registers RBX, RBP, RDI, RSI, R12, R13, R14 and R15 (? SSX 0-7 ?)
; are considered nonvolatile and must be saved and restored by your asm function
; if one uses them.
; Here: all non-volatile registers are untouched, rsp also OK.
; Exception can happen if c=0 or when resulting quotient going to be above (2^64-1)
; It can happen when after mul we have [rdx] > [r8], check it and do whatever you want
; or handle exceprion or allow program to crash. 
; --------------------------------------------------------------------------------
include listing.inc

_TEXT	SEGMENT
  PUBLIC asmMulDiv64

asmMulDiv64 PROC
  mov  rax,rdx    ; rax <- b
  mul  rcx        ; 128 bits of rax*rcx placed into rdx+rax (a was in rcx) 
  div  r8         ; [rdx,rax] / r8: result->rax, reminder->rdx (c was in r8)
; Quotient already in rax, place here "mov rax,rdx" if you want to return remainder instead.
  ret  0          ; 0: do not remove parameters from stack
asmMulDiv64 ENDP

_TEXT	ENDS
END
; eof