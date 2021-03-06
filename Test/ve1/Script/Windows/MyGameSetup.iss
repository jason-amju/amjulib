; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{027E36B-5981-4B37-9C8F-2ACF9E33EA4D}
AppName=Crash Site
AppVersion=0.7
AppPublisher=Jason Colman
AppPublisherURL=http://www.amju.com/
AppSupportURL=http://www.amju.com/
AppUpdatesURL=http://www.amju.com/
DefaultDirName={pf}\Crash Site
DefaultGroupName=Crash Site
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
Source: "..\..\..\..\Build\ve1\Windows\Release\ve1.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\..\..\Source\GLUT\glut32.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\..\..\Source\Network\curl\lib\libcurl.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\..\..\Source\Network\curl\lib\zlib1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\..\..\Source\SDL\lib\sdl.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\..\..\Source\SoundBass\Bass2.3\Win\bass.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\..\..\Source\SoundBass\Bass2.3\Win\bassmidi.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\Assets\*.*"; DestDir: "{app}\Data\"; Flags: ignoreversion
Source: "..\..\Assets\characters\*.*"; DestDir: "{app}\Data\characters\"; Flags: ignoreversion recursesubdirs
Source: "..\..\Assets\font2d\*.*"; DestDir: "{app}\Data\font2d\"; Flags: ignoreversion
Source: "..\..\..\..\Assets\font2d\*.*"; DestDir: "{app}\Data\font2d\"; Flags: ignoreversion
Source: "..\..\..\..\Assets\font3d\*.*"; DestDir: "{app}\Data\font3d\"; Flags: ignoreversion
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{group}\Crash Site"; Filename: "{app}\ve1.exe"
Name: "{commondesktop}\Crash Site"; Filename: "{app}\ve1.exe"; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\Crash Site"; Filename: "{app}\ve1.exe"; Tasks: quicklaunchicon

[Run]
Filename: "{app}\ve1.exe"; Description: "{cm:LaunchProgram,Crash Site}"; Flags: nowait postinstall skipifsilent

