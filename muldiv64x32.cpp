/* -----------------------------------------
* muldiv64x32.cpp for x86-32 bits compiler only:
* function asmMulDiv64x32(UINT64 nM1, UINT64 nM2, UINT64 nDiv)
* calculates (nM1*nM2)/nDiv for with 128 bits intermediate result.
* There is NO inline assembler in MSVC for 64 bits and all commands
* here are for 32 bits mode.
* This code is based on D.Knuth, Vol2 along with MS .asm sources
* for runtime library of some old * 32 bits MSVC compiler in
* old good time when Windows 95 was new...
* Compiler at that time support 64 bits integers and multiply/divide
* was implemented in .asm in runtime library, something like
* lmul64.asm and ldiv64.asm
* Compile this file with /D__WITHTEST to have .exe with test call
* or compile it as is to library ot with your own C/CPP test caller.
* See #ifdef __WITHTEST test trive at thed od this file
* -----------------------------------------
*/
#pragma warning(disable:4001)	/* "//" comment in *.c: no warning W4 */
#pragma warning(disable:4201)	// nameless struct/unions: OK in new standart
#pragma warning(disable:4214)	// 'unsigned' bit fields: OK in new standart
#pragma warning(disable:4514)   // unreferenced inline func: nu i hooy s ney
#pragma warning(disable:4115)   // not yet defined "struct ..." in prototype
#pragma warning(disable:4100)   // unreferenced formal parameter

#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#include "stdio.h"

typedef unsigned int     UINT32;
typedef unsigned __int64 UINT64;

extern "C" UINT64 __declspec(naked) __stdcall
asmMulDiv64x32(UINT64 nM1, UINT64 nM2, UINT64 nDiv) {
  _asm {
	push  ebp
	mov   ebp, esp
    sub   esp, 28   ;; allocate local copy of partial dividend & full prod
;; stack layout relatively our new ebp, all 32 bit [d]words:
#define DVD_0       DWORD PTR [ebp - 28]
#define DVD_1       DWORD PTR [ebp - 24]
#define DVD_2       DWORD PTR [ebp - 20]
#define ACC_0       DWORD PTR [ebp - 16]
#define ACC_1       DWORD PTR [ebp - 12]
#define ACC_2       DWORD PTR [ebp - 8]
#define ACC_3       DWORD PTR [ebp - 4]
;; ^^^^ 16 bytes of the local variable, will keep production in 0-3
;; saved old ebp:   [ebp]
;; return adress:   [ebp + 4]
;; Here arguments passed in the stack:
#define LO_M1   DWORD PTR [ebp + 8]
#define HI_M1   DWORD PTR [ebp + 12]
#define LO_M2   DWORD PTR [ebp + 16]
#define HI_M2   DWORD PTR [ebp + 20]
#define LO_DIV  DWORD PTR [ebp + 24]
#define HI_DIV  DWORD PTR [ebp + 28]
;; names: either HI/LO prefixes or 0-1-... postfix, 0==lowest digit
;; ---------------------------------------------------
   push  ebx
   push  esi
   push  edi
;; ---------------------------------------------------
;; multiply M1 and M2 and place result into ACC_0 - ACC_3:
   mov   eax, LO_M1
   mul   LO_M2   ;; eax*Arg --> edx:eax
   mov   ACC_0, eax
   mov   ACC_1, edx
   mov   eax, HI_M1
   mul   HI_M2   ;; eax*Arg --> edx:eax
   mov   ACC_2, eax
   mov   ACC_3, edx
;; partial result in ACC, AddWithCarry LO*HI productions to 1-2, carry to 3:
   xor   ebx, ebx
   mov   eax, HI_M1
   mul   LO_M2   ;; eax*Arg --> edx:eax
   add   ACC_1, eax
   adc   ACC_2, edx
   adc   ACC_3, ebx
;;  next cross:
   mov   eax, HI_M2
   mul   LO_M1   ;; eax*Arg --> edx:eax
   add   ACC_1, eax
   adc   ACC_2, edx
   adc   ACC_3, ebx
;; division of the 4 digits in ACC_0--ACC_3 into 2 digits in hi/lo _DIV
   mov   edx, HI_DIV
   test  edx,edx
   jnz   short do_long_div
   mov   ecx, LO_DIV  ;; HI_DIV == 0
   mov   eax, ACC_3   ;; edx already 0
   test  ecx, ecx
   jz    short epilogue  ;; divide by zero, return smth stupid instead crash
   div   ecx          ;; edx::eax / ecx -> eax ;; edx <-- remainder
   mov   ACC_3, eax   ;; senior digit of the result done
   mov   eax, ACC_2
   div   ecx          ;; edx::eax / ecx -> eax; edx <-- remainder; ecx const
   mov   ACC_2, eax
   mov   eax, ACC_1
   div   ecx          ;; edx::eax / ecx -> eax ;; edx <-- remainder
   mov   ACC_1, eax
   mov   eax, ACC_0
   div   ecx          ;; edx::eax / ecx -> eax ;; edx <-- remainder
   mov   ACC_0, eax   ;; junior digit done, remainder in edx
   jmp    epilogue
;;
do_long_div:      ;; use __Div3By2 to divide 3by2 vs 2by1.
;; __Div3By2: on entry ebx:edx:ecx == divident, on exit: ebx:edx=remainder
;; quotient in eax, fit eax only when: ebx:edx < divisor > 2^32 (!!!)
;; ecx,esi,edi registers may be destroyed during call, ebp/esp OK
   xor   ebx,ebx
   mov   edx, ACC_3
   mov   eax, ACC_2
   call  __Div3By2
   mov   ACC_3, 0
   mov   ACC_2, eax
   mov   eax, ACC_1
   call  __Div3By2
   mov   ACC_1, eax
   mov   eax, ACC_0
   call  __Div3By2
   mov   ACC_0, eax    ;; junior done, remainder in ebx:edx
epilogue:      ;; division result in ACC_ 0-3, least significant first
   ;; mov  eax, ACC_0   ;; only 2 low digits (32 bit in each 'digit')
   ;; ^^^ ACC_0 already in eax
   mov  edx, ACC_1   ;; will be returned
;; ------------------------------------------
  pop   edi
  pop   esi
  pop   ebx
  mov   esp,ebp
  pop   ebp
  ret   24    ;; 3 args, 8 byte each removed from the stack
;; -------------------------------------------
;;
__Div3By2:    ;; internal subprogram for division of 3 digit by 2 digit.
;; number for divide in ebx:edx:eax and divisor in HI_DIV:LO_DIV.
;; divisor assumed > 2^32 (i.e. HI > 0) and also ebx:edx < DIV (!!! must).
;; In this case result always will be < 2^32 and it will be returned in eax.
;; Remainder will be returned in ebx:edx, it is < DIV and can be > 2^32
;; Do not touch ebp here, use it for access to on-stack variables
;; other registers can be destroyed (ecx,esi,edi)
;;
   mov DVD_0, eax  ;; save partial 3-digit dividend in local
   mov DVD_1, edx  ;; variable DVD (ebp same as in caller)
   mov DVD_2, ebx  ;;
;;
;; 2-digit divisor, shift right until HI digit 0, shift value into ECX
   mov  eax, HI_DIV
   mov  ebx, LO_DIV
;; shift value can be 32, in this case we can do it simple way
;; (!!! needed, because shrd do nothing for shift = 32 !!!)
   bt  eax,31    ;; senior bit to CF
   jnc  do_shift
;; here make shift for 32 to have dividend in edx:eax and divisor in ebx
   mov  ebx, eax     ;; HI_DIV as shifted by 32 divisor
   mov  eax, DVD_1   ;; 32 bit shifted dividend
   mov  edx, DVD_2
   jmp  short do_div
do_shift:
   xor  ecx, ecx     ;; zero
loop_shift:
   inc  ecx
   shr  eax, 1
   rcr  ebx, 1
   or   eax, eax  ;; is it zero ?
   jnz  loop_shift
;;
;; 'reduced'  divisor now in ebx
;; note: number in 2 high digits in DVD < divisor, so after same shift
;; of divident it will fit in 2 digits and result of division this shifted
;; value into reduced divisor will give us 1 digit result which is
;; approximation of the actual quotient
;;
   mov   esi, DVD_2
   mov   edx, DVD_1
   mov   eax, DVD_0
;;
   shrd  eax, edx, cl   ;; shifted eax::edx into eax; edx unchanged
   shrd  edx, esi, cl   ;; (edx:esi) >> cl  ->  edx
   sar   esi, cl
   or    esi, esi
   jz    short do_div
   ;; non-zero (maybe never happen), use upper estimation for the result:
   xor   esi, esi
   dec   esi
   jmp   short done_div    ;; without any division
;;
do_div:
;; ready for 'approximating' division, now we have reduced divisor in ebx
;; end reduced dividend in the edx:eax
;; before division: exclude division overflow, if overflow
;; than use upper estimation instead of division
;;
   cmp  edx, ebx      ;; reduced divisor was & remains ebx
   jb   short  no_overflow
;; division overflow will happen if divide, set estimation:
   xor   esi, esi
   dec   esi    ;; maximal 'single-digit' number
   jmp   short done_div
;;
no_overflow:
;; now: shifted dividend in edx:eax, shifted divisor in ebx
   div   ebx   ;; result -> eax; remainder into edx (not needed)
   mov   esi, eax
;;
done_div:
;; esi  now contains 'approximated' result. It can be either correct or
;; a little bigger (1 or 2, see details in D.Knuth, Vol 2)
;; both: actual & 'approximated' result NEVER can be bigger than 1 digit
;; Multiply it with actual divisor and subtruct until correct
;; result reached (compare with actual dividend, only 1 or 2 sub needed)
;; multiply result to be placed into exc:edx:eax
;;
   mov   eax, HI_DIV
   mul   esi         ;; eax*HI_DIV --> edx::eax
   mov   ecx, edx
   mov   edi, eax
   mov   eax, LO_DIV
   mul   esi
   add   edx, edi
   adc   ecx, 0
;;
;; ready -  ecx:edx:eax contains production, it can be bigger than
;; original dividend, need correct it by subtraction until OK.
;; 0-1-2 times only, as D.Knuth promises. Even Intel has bugs in division,
;; so here I limit number of iterations: if I am like Intel - I will return
;; some stupid numbers, but no infinite/too long loop or crash
;; If no bug - it just work a little slower, so:
;;
   mov   ebx, 20    ;; !!! stupid patch
corr_loop:
   dec   ebx        ;; !!! stupid patch
   jz    corr_done  ;; !!! stupid patch for possible bug
   cmp   ecx, DVD_2
   ja    short  corr_subtract
   jb    short  corr_done
;; else: equality, go to lower digit
   cmp   edx, DVD_1
   ja    short  corr_subtract
   jb    short  corr_done
;;  we are on lowest digit, equal means correction done (& zero remainder)
   cmp   eax, DVD_0
   jbe   corr_done
;;
corr_subtract:  ;;  ecx:edx:eax -= divisor  (with carry !!!)
   sub   eax, LO_DIV
   sbb   edx, HI_DIV
   sbb   ecx, 0  ;; carry flag from the last subtract
   dec   esi     ;; estimated quotient correction by 1
   jmp   short corr_loop
;;
corr_done:  // now: esi contains proper quotient; rem = DVD - ecx:edx:eax
   mov   edi, DVD_0
   sub   edi, eax   ;; edi <- remainder_0
   mov   eax, DVD_1
   sbb   eax, edx   ;; eax <- remainder_1
   mov   ebx, DVD_2
   sbb   ebx, edx   ;; here MUST be 0 (or my log error in program)
   mov   edx, edi   ;; LO_remainder
   mov   ebx, eax   ;; HI_remainder
   mov   eax, esi  ;; quotient
   ret   ;; remainder & quotient as needed
  }
} /* void args: no difference if stdcall/cdecl */


#ifdef __WITH_TEST_MAIN  /* use /D__WITH_TEST_MAIN option for compiler */

static void do_tst(UINT64 nM1, UINT64 nM2, UINT64 nDiv) {
  printf("%I64u * %I64u / %I64u",
        nM1,  nM2, nDiv);
  UINT64 nRes = asmMulDiv64x32(nM1, nM2, nDiv);
  printf(" = %I64u \n", nRes );
}

static void loop_tst(UINT64 nM1, UINT64 nM2, int nCnt) {
  UINT64 nDiv = nM2;
  int j;

  for (j = 0; j < nCnt; ++j) {
    do_tst(nM1, nM2, nDiv);
    nM1  *= nM2;
    nDiv *= nM2;
  }
}


void main() {
   UINT64 nM1  = 12;
   UINT64 nM2  = 5;
   UINT64 nDiv = 20;

   do_tst(nM1, nM2, nDiv);

   printf("more test\n");
   loop_tst(7, 13, 5);
   loop_tst(17, 3, 5);
   loop_tst(13, 17, 5);
   loop_tst(11, 19, 5);
   loop_tst(2, 2, 30);
   loop_tst(3, 3, 10);

   printf("Can we crash with givision overflow when result out of 64 bits?\n");
   do_tst(-1, -1, 2); // -1 to undigned gives all bits 1 (max for given size)

   printf("Can we crash with givision by 0?\n");
   do_tst(3, 4, 0);

   printf("all done\n");
}  /* end main() */

#endif

/* eof */
