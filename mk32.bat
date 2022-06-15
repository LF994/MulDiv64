@echo off
:: compile with test enabled, run from 32 bits command prompt
:: or enjoy compiler error messages from 64 bits compiler
:: option /D will define preprocessor variable, see #ifdef in source

call clexe.bat /D__WITH_TEST_MAIN muldiv64x32.cpp

:: eof