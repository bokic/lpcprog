[Setup]
AppName=LPCProg
AppVerName=LPCProg version 0.1.0.4
AppPublisher=BokiCSoft
AppPublisherURL=http://lpcprog.bokicsoft.com/
AppVersion=0.1.0.4
DefaultDirName={pf}\BokiCSoft\LPCProg
DefaultGroupName=LPCProg
UninstallDisplayIcon={app}\lpcprog.exe
Compression=lzma
;Compression=none
SolidCompression=yes
OutputBaseFilename=lpcprog-0.1
OutputDir=.
VersionInfoVersion=0.1.0.4
VersionInfoDescription=LPCProg is opensource, multiplatform LPC214x programmer

[Files]
; MinGW 4.8 runtime
Source: "C:\Qt\5.1.0\mingw48_32\bin\libgcc_s_dw2-1.dll"; DestDir: "{app}";
Source: "C:\Qt\5.1.0\mingw48_32\bin\libstdc++-6.dll"; DestDir: "{app}";
Source: "C:\Qt\5.1.0\mingw48_32\bin\libwinpthread-1.dll"; DestDir: "{app}";

; Text codec stuff(or something else not sure)
Source: "C:\Qt\5.1.0\mingw48_32\bin\icudt51.dll"; DestDir: "{app}";
Source: "C:\Qt\5.1.0\mingw48_32\bin\icuin51.dll"; DestDir: "{app}";
Source: "C:\Qt\5.1.0\mingw48_32\bin\icuuc51.dll"; DestDir: "{app}";

; QT 5.1 library
Source: "C:\Qt\5.1.0\mingw48_32\bin\Qt5Core.dll"; DestDir: "{app}";
Source: "C:\Qt\5.1.0\mingw48_32\bin\Qt5Gui.dll"; DestDir: "{app}";
Source: "C:\Qt\5.1.0\mingw48_32\bin\Qt5SerialPort.dll"; DestDir: "{app}";
Source: "C:\Qt\5.1.0\mingw48_32\bin\Qt5Widgets.dll"; DestDir: "{app}";

; main program
Source: "..\build-lpcprog-Desktop_Qt_5_1_0_MinGW_32bit-Release\release\lpcprog.exe"; DestDir: "{app}";

[Icons]
Name: "{group}\LPCProg"; Filename: "{app}\lpcprog.exe"
