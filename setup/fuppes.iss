[Setup]
AppName=Free UPnP Entertainment Service
AppVerName=Free UPnP Entertainment Service 0.7.2a-20070529
AppPublisher=Ulrich Völkel
AppPublisherURL=http://sourceforge.net/projects/fuppes
AppSupportURL=http://sourceforge.net/projects/fuppes
AppUpdatesURL=http://sourceforge.net/projects/fuppes
DefaultDirName={pf}\Free UPnP Entertainment Service\
DefaultGroupName=Free UPnP Entertainment Service
LicenseFile=..\setup\license-eng.rtf
OutputDir=..\setup
OutputBaseFilename=fuppes-0.7.2a-20070529-win32-setup
SolidCompression=true
ShowTasksTreeLines=true
SourceDir=..\win32
AppCopyright=Ulrich Völkel
WizardImageFile=compiler:wizmodernimage-is.bmp
WizardSmallImageFile=compiler:wizmodernsmallimage-is.bmp
ShowLanguageDialog=auto
SetupIconFile=..\setup\fuppes.ico

[Languages]
Name: eng; Messagesfile: compiler:Default.isl; LicenseFile: ..\setup\license-eng.rtf
Name: ger; Messagesfile: compiler:Languages\German.isl; LicenseFile: ..\setup\license-eng.rtf

[Tasks]
Name: desktopicon; Description: {cm:CreateDesktopIcon}; GroupDescription: {cm:AdditionalIcons}; Flags: unchecked
Name: quicklaunchicon; Description: {cm:CreateQuickLaunchIcon}; GroupDescription: {cm:AdditionalIcons}; Flags: unchecked

[Files]
Source: ..\win32\fuppes.exe; DestDir: {app}; Flags: ignoreversion
Source: ..\win32\fuppescmd.exe; DestDir: {app}
Source: ..\win32\fuppes.dll; DestDir: {app}
Source: ..\win32\vorbisfile.dll; DestDir: {app}
Source: ..\win32\avcodec-51.dll; DestDir: {app}
Source: ..\win32\avformat-51.dll; DestDir: {app}
Source: ..\win32\avutil-49.dll; DestDir: {app}
Source: ..\win32\libFLAC.dll; DestDir: {app}
Source: ..\win32\libiconv-2.dll; DestDir: {app}
Source: ..\win32\libMagick-10.dll; DestDir: {app}
Source: ..\win32\libMagick++-10.dll; DestDir: {app}
Source: ..\win32\libpcre-0.dll; DestDir: {app}
Source: ..\win32\libWand-10.dll; DestDir: {app}
Source: ..\win32\libxml2-2.dll; DestDir: {app}
Source: ..\win32\mpcdec.dll; DestDir: {app}
Source: ..\win32\ogg.dll; DestDir: {app}
Source: ..\win32\sqlite3.dll; DestDir: {app}
Source: ..\win32\taglib.dll; DestDir: {app}
Source: ..\win32\vorbis.dll; DestDir: {app}
Source: ..\vfolder.cfg; DestDir: {app}
Source: ..\setup\ImageMagick-License.txt; DestDir: {app}
Source: ..\AUTHORS; DestDir: {app}; Flags: ignoreversion; DestName: AUTHORS.txt
Source: ..\ChangeLog; DestDir: {app}; Flags: ignoreversion; DestName: ChangeLog.txt
Source: ..\NEWS; DestDir: {app}; Flags: ignoreversion; DestName: NEWS.txt
Source: ..\README; DestDir: {app}; Flags: ignoreversion isreadme; DestName: README.txt
Source: ..\COPYING; DestDir: {app}; Flags: ignoreversion; DestName: COPYING.txt
Source: ..\COPYING-MUSEPACK; DestDir: {app}; Flags: ignoreversion; DestName: COPYING-MUSEPACK.txt
Source: ..\COPYING-OGG; DestDir: {app}; Flags: ignoreversion; DestName: COPYING-OGG.txt
Source: ..\COPYING-VORBIS; DestDir: {app}; Flags: ignoreversion; DestName: COPYING-VORBIS.txt
Source: ..\COPYING-FLAC; DestDir: {app}; Flags: ignoreversion; DestName: COPYING-FLAC.txt

[Icons]
Name: {group}\FUPPES; Filename: {app}\fuppes.exe; IconIndex: 0
Name: {group}\{cm:UninstallProgram,FUPPES}; Filename: {uninstallexe}
Name: {userdesktop}\FUPPES; Filename: {app}\fuppes.exe; Tasks: desktopicon; IconIndex: 0
Name: {userappdata}\Microsoft\Internet Explorer\Quick Launch\FUPPES; Filename: {app}\fuppes.exe; Tasks: quicklaunchicon; Languages: ; IconIndex: 0
Name: {group}\Edit configuration; Filename: {userappdata}\FUPPES\fuppes.cfg; Tasks: ; Languages: 
Name: {group}\FUPPES (command line); Filename: {app}\fuppescmd.exe; IconIndex: 0

[Run]
Filename: {app}\fuppes.exe; Description: {cm:LaunchProgram,Free UPnP Entertainment Service}; Flags: nowait postinstall skipifsilent unchecked; Tasks: ; Languages: 
