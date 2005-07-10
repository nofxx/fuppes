[Setup]
AppName=Free UPnP Entertainment Service
AppVerName=Free UPnP Entertainment Service 0.1.1
AppPublisher=fuppes development team
AppPublisherURL=http://sourceforge.net/projects/fuppes
AppSupportURL=http://sourceforge.net/projects/fuppes
AppUpdatesURL=http://sourceforge.net/projects/fuppes
DefaultDirName={pf}\Free UPnP Entertainment Service\
DefaultGroupName=Free UPnP Entertainment Service
LicenseFile=E:\Entwicklung\fuppes\trunk\COPYING
OutputDir=E:\Entwicklung\fuppes\trunk\setup
OutputBaseFilename=fuppes-0.1.1-win32-setup
SolidCompression=true
ShowTasksTreeLines=true
SourceDir=E:\Entwicklung\fuppes\trunk\win32
AppCopyright=Ulrich Völkel
WizardImageFile=compiler:wizmodernimage-is.bmp
WizardSmallImageFile=compiler:wizmodernsmallimage-is.bmp
ShowLanguageDialog=auto

[Languages]
Name: eng; Messagesfile: compiler:Default.isl; LicenseFile: E:\Entwicklung\fuppes\trunk\setup\license-eng.rtf
Name: ger; Messagesfile: compiler:Languages\German.isl; LicenseFile: E:\Entwicklung\fuppes\trunk\setup\license-eng.rtf

[Tasks]
Name: desktopicon; Description: {cm:CreateDesktopIcon}; GroupDescription: {cm:AdditionalIcons}; Flags: unchecked
Name: quicklaunchicon; Description: {cm:CreateQuickLaunchIcon}; GroupDescription: {cm:AdditionalIcons}; Flags: unchecked

[Files]
Source: ..\win32\fuppes.exe; DestDir: {app}; Flags: ignoreversion
Source: ..\win32\iconv.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\win32\pcre.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\fuppes.cfg.example; DestDir: {userappdata}\Free UPnP Entertainment Service; Flags: ignoreversion; DestName: fuppes.cfg
Source: ..\AUTHORS; DestDir: {app}; Flags: ignoreversion
Source: ..\ChangeLog; DestDir: {app}; Flags: ignoreversion
Source: ..\COPYING; DestDir: {app}; Flags: ignoreversion
Source: ..\NEWS; DestDir: {app}; Flags: ignoreversion
Source: ..\README; DestDir: {app}; Flags: ignoreversion

[Icons]
Name: {group}\Free UPnP Entertainment Service; Filename: {app}\fuppes.exe
Name: {group}\{cm:UninstallProgram,Free UPnP Entertainment Service}; Filename: {uninstallexe}
Name: {userdesktop}\Free UPnP Entertainment Service; Filename: {app}\fuppes.exe; Tasks: desktopicon
Name: {userappdata}\Microsoft\Internet Explorer\Quick Launch\Free UPnP Entertainment Service; Filename: {app}\fuppes.exe; Tasks: quicklaunchicon; Languages: 

[Run]
Filename: {app}\fuppes.exe; Description: {cm:LaunchProgram,Free UPnP Entertainment Service}; Flags: nowait postinstall skipifsilent
