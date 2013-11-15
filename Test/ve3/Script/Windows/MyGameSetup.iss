; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{12E97154-C5DD-454F-88F6-3A69775AFAF5}
AppName=Hungry People
AppVersion=1.1
AppPublisher=Jason Colman
AppPublisherURL=http://www.amju.com/
AppSupportURL=http://www.amju.com/
AppUpdatesURL=http://www.amju.com/
DefaultDirName={pf}\Hungry People
DefaultGroupName=Hungry People
AllowNoIcons=yes
OutputBaseFilename=setup
Compression=lzma
SolidCompression=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked; OnlyBelowVersion: 0,6.1

[Files]
Source: "..\..\..\..\Build\ve2\Windows\Release\ve2.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\..\..\Source\GLUT\glut32.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\..\..\Source\Network\curl\lib\libcurl.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\..\..\Source\Network\curl\lib\zlib1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\..\..\Source\SDL\lib\sdl.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\..\..\Source\SoundBass\Bass2.3\Win\bass.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\..\..\Source\SoundBass\Bass2.3\Win\bassmidi.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\..\..\Build\CompiledAssets\data-win.glue"; DestDir: "{app}\Data\"; Flags: ignoreversion
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{group}\Hungry People"; Filename: "{app}\ve2.exe"
Name: "{commondesktop}\Hungry People"; Filename: "{app}\ve2.exe"; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\Hungry People"; Filename: "{app}\ve2.exe"; Tasks: quicklaunchicon

[Run]
Filename: "{app}\ve2.exe"; Description: "{cm:LaunchProgram,Hungry People}"; Flags: nowait postinstall skipifsilent

