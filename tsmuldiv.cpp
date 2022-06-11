// -------------------------------------------------------------------------
// Call from CPP to external C funcion in assembler
// Compile CPP it 64 bits when .asm written for 64 bits
// There is problem with MSVC-x64: it does not have inline assembler.
// External MASM works on any platform, just follow call convensions. 
// Tested with MS Visual Studio 2022, should work on any MSVC with 64 bits 
// -------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef unsigned __int64 UINT64;

// Calculates UINT64 (a*b)/c using intermediate 128 bits a*b production. 
// Gives integer divide overflow crash if final result going to be bigger than 2^64
// Integer _atoi64() used below for purpose (to avoid printing too large number): 
// assign integer -1 to unsigned gives positive value (2^64 - 1) (all bits as one)
// "tst -1 -1 200" gives crash, while "-2 -2 -1" works fine
// Below I use Windows SEH to handle exceptions, CPP try/catch does not work here...
extern "C" UINT64 asmMulDiv64(UINT64 a, UINT64 b, UINT64 c);



int main(int nCnt, char** pVals) {
  if (nCnt < 4) {
    printf("Use: test n1 n2 n3\nto calculate (n1 * n2) / n3");
    return 1;  
  }

  UINT64 n1 = _atoi64(pVals[1]);
  UINT64 n2 = _atoi64(pVals[2]);
  UINT64 n3 = _atoi64(pVals[3]);

  printf("tsmuldiv: (%I64u * %I64u) / %I64u \n" , n1, n2, n3);
  printf("In Hex: (%I64X * %I64X) / %I64X \n" , n1, n2, n3);

  __try {
    UINT64 res = asmMulDiv64(n1, n2, n3);
    printf("Res=%I64u (Hex=%I64X)\n" , res, res);
  }
  __except(EXCEPTION_EXECUTE_HANDLER)  {
    printf("Exception catched: integer division problem\n");
  }

  printf("End of test\n");
  return 0;
}

/* eof */