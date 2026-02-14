[Setup]
AppId={{E1A4F1C3-8B3D-4C8A-9F12-EMSNOTIFY001}}
AppName=EMS Notify
AppVersion=1.0
AppPublisher=Centre for Smart Governance
DefaultDirName={pf}\EmsNotify
DefaultGroupName=EmsNotify
OutputDir=.
OutputBaseFilename=EMSNotifySetup
Compression=lzma
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64
SetupIconFile=resources\icons\clock.ico

[Files]
Source: "out\build\release\*"; DestDir: "{app}"; Flags: recursesubdirs

[Icons]
Name: "{group}\EMS Notify"; Filename: "{app}\EmsNotify.exe"
Name: "{commondesktop}\EMS Notify"; Filename: "{app}\EmsNotify.exe"

[Run]
Filename: "{app}\EmsNotify.exe"; Description: "Launch EMS Notify"; Flags: nowait postinstall skipifsilent