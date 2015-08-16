pstat | find "jdenet_n" > nul
if Not ErrorLevel 1 goto e01
start jdenet_n.exe

ping 1.1.1.1 -n 1 -w 30000 >NUL

:e01
owdcmenu.exe %1

REM taskkill /IM jdenet_n.exe