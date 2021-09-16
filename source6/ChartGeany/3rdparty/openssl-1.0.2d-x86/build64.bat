perl Configure no-asm VC-WIN64A --prefix=..\..\..\binaries\msvc64

CALL ms\do_win64.bat; 

nmake.exe /f ms\ntdll.mak clean

nmake.exe /f ms\ntdll.mak 

nmake /f ms\ntdll.mak test

nmake /f ms\ntdll.mak install
