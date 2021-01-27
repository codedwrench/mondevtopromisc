@ECHO OFF
if not exist ..\Objects (mkdir ..\Objects)
cd ..\Objects
gendef %WINDIR%\System32\Npcap\wpcap.dll
dlltool -dllname %WINDIR%\System32\Npcap\wpcap.dll --def wpcap.def --output-delaylib wpcap.a
cd ..