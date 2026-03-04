[Setup]
AppId={{E1A4F1C3-8B3D-4C8A-9F12-EMSNOTIFY001}}
#define MyAppVersion "1.0.4"
AppName=EMS Notify
AppVersion={#MyAppVersion}
AppPublisher=Harsha
DefaultDirName={pf}\EmsNotify
DefaultGroupName=EmsNotify
OutputDir=.
OutputBaseFilename=EMS_Notify_v{#MyAppVersion}
Compression=lzma
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64
SetupIconFile=resources\icons\clock.ico

[Files]
Source: "out\build\release\*"; DestDir: "{app}"; Flags: recursesubdirs

[Icons]
Name: "{group}\EMS Notify"; Filename: "{app}\EmsNotify.exe"
Name: "{commondesktop}\EMS Notify"; Filename: "{app}\EmsNotify.exe"

[Tasks]
Name: "startup"; Description: "Run EMS Notify at Windows startup"; GroupDescription: "Additional options:"

[Registry]
Root: HKCU; Subkey: "Software\Microsoft\Windows\CurrentVersion\Run"; \
    ValueType: string; ValueName: "EmsNotify"; \
    ValueData: "{app}\EmsNotify.exe"; \
    Flags: uninsdeletevalue; Tasks: startup

[Run]
Filename: "{app}\EmsNotify.exe"; Description: "Launch EMS Notify"; Flags: nowait postinstall skipifsilent