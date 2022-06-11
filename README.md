# MulDiv64
MulDiv64 function written on assembler for Intel x86-64. It have to be compiled in 64 bits Windows with Microsoft Visual Studio version 2010 and above.
There are some manifest-related issues in VS 2008, but no problem on older MSVC versions supporting 64 bits.

You have to verify what call sequence is used by Linux or Apple IOS running on Intel-X86-64 CPU. It may or may not be the same. 

It can be your first program in assembler for x86-64. It has very few commands, but it is useful.
In CPP you have to prototype it as:

extern "C" UINT64 asmMulDiv64(UINT64 a, UINT64 b, UINT64 c);

I hope that you khow how it should look for C source code.

Function calculates (a * b)/c using intermediate 128 bits result for a*b production. Integers are unsighed.
This is asm for 64 bits (ML64.exe), so you have to compile C/CPP for 64 bits respectively.

Problem is that no more inline assembler in MSVC x86-64.
You may need this function to perform scale conversions between huge integers representing time in different units and so on.
Exception can happen if c=0 or when resulting quotient going to be above (2^64-1)
It can happen when after mul we have [rdx] > [r8], check it and do whatever you want
or handle exceprion or allow program to crash.

Assembler source can be found in muldiv64.asm. Command line test caller in CPP with proper exception handling can be found in tsmuldiv.cpp.
Use mk.bat to compile CPP and asm to tsmuldiv.exe.
Below presented build output while run from MSVC x64 command prompt:
```
**********************************************************************
** Visual Studio 2022 Developer Command Prompt v17.2.3
** HostCPU=AMD64 Tools=x64 Target=x64
** Copyright (c) 2022 Microsoft Corporation
**********************************************************************
D:\My\Proj\MulDiv64>mk
ML64.exe at: C:\Progs\VStud\VC\Tools\MSVC\14.32.31326\bin\Hostx64\x64\ml64.exe
Microsoft (R) Macro Assembler (x64) Version 14.32.31329.0
Copyright (C) Microsoft Corporation.  All rights reserved.
 Assembling: muldiv64.asm
Microsoft (R) C/C++ Optimizing Compiler Version 19.32.31329 for x64
Copyright (C) Microsoft Corporation.  All rights reserved.
tsmuldiv.cpp
Microsoft (R) Incremental Linker Version 14.32.31329.0
Copyright (C) Microsoft Corporation.  All rights reserved.
/out:tsmuldiv.exe
/MAP
tsmuldiv.obj
muldiv64.obj
D:\My\Proj\MulDiv64>
```
Now we we can test from command line, tsmuldiv.exe receives 3 arguments for a, b and c. 
```
D:\My\Proj\MulDiv64>tsmuldiv 3 4 5
tsmuldiv: (3 * 4) / 5
In Hex: (3 * 4) / 5
Res=2 (Hex=2)
End of test
D:\My\Proj\MulDiv64>tsmuldiv 10 20 0
tsmuldiv: (10 * 20) / 0
In Hex: (A * 14) / 0
Exception catched: integer division problem
End of test

```

All numbers for MulDiv are unsigned, but test caller can receive negative values and treat it as huge unsigned numbers. I hope you know how signed integer negative values represented as "two-complement". See test output below:
```
D:\My\Proj\MulDiv64>tsmuldiv -1 100 200
tsmuldiv: (18446744073709551615 * 100) / 200
In Hex: (FFFFFFFFFFFFFFFF * 64) / C8
Res=9223372036854775807 (Hex=7FFFFFFFFFFFFFFF)
End of test
```
If expected result out of 64 bits - error happen (integer division overflow):
```
D:\My\Proj\MulDiv64>tsmuldiv -1 -1 200
tsmuldiv: (18446744073709551615 * 18446744073709551615) / 200
In Hex: (FFFFFFFFFFFFFFFF * FFFFFFFFFFFFFFFF) / C8
Exception catched: integer division problem
End of test
```
It is possible to modify code to avoid "integer division overflow", but decision have to be made what "wrong value" to calculate and return. One of the options can be  to return maximal unsigned integer. It can be done very easy. If it is needed to return correct least significant 64 bits of the result - it may need to perform division twice.
