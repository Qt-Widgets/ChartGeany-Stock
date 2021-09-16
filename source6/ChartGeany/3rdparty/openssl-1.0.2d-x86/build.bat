perl Configure no-asm VC-WIN32 --prefix=..\..\..\binaries\msvc32

CALL ms\do_nasm.bat; 

nmake.exe /f ms\ntdll.mak clean

nmake.exe /f ms\ntdll.mak 

nmake /f ms\ntdll.mak test

nmake /f ms\ntdll.mak install
