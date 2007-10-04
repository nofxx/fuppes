[Setup]
AppName=Free UPnP Entertainment Service
AppVerName=Free UPnP Entertainment Service SVN-549
AppPublisher=Ulrich Völkel
AppPublisherURL=http://sourceforge.net/projects/fuppes
AppSupportURL=http://sourceforge.net/projects/fuppes
AppUpdatesURL=http://sourceforge.net/projects/fuppes
DefaultDirName={pf}\Free UPnP Entertainment Service\
DefaultGroupName=Free UPnP Entertainment Service
LicenseFile=..\setup\license-eng.rtf
OutputDir=..\setup
OutputBaseFilename=fuppes-SVN-549-win32-setup
SolidCompression=true
ShowTasksTreeLines=true
SourceDir=C:\src\fuppes\trunk\setup
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
Source: ..\..\win32\fuppes.exe; DestDir: {app}
Source: ..\..\win32\fuppescmd.exe; DestDir: {app}
Source: ..\..\win32\zlib1.dll; DestDir: {app}
Source: ..\..\win32\swscale-0.dll; DestDir: {app}
Source: ..\..\win32\libxml2-2.dll; DestDir: {app}
Source: ..\..\win32\libWand-10.dll; DestDir: {app}
Source: ..\..\win32\libvorbisfile-3.dll; DestDir: {app}
Source: ..\..\win32\libvorbisenc-2.dll; DestDir: {app}
Source: ..\..\win32\libvorbis-0.dll; DestDir: {app}
Source: ..\..\win32\libtiff-3.dll; DestDir: {app}
Source: ..\..\win32\libtag-1.dll; DestDir: {app}
Source: ..\..\win32\libsqlite3-0.dll; DestDir: {app}
Source: ..\..\win32\libpcre-0.dll; DestDir: {app}
Source: ..\..\win32\libogg-0.dll; DestDir: {app}
Source: ..\..\win32\libmpcdec-5.dll; DestDir: {app}
Source: ..\..\win32\libmp4ff-0.dll; DestDir: {app}
Source: ..\..\win32\libMagick++-10.dll; DestDir: {app}
Source: ..\..\win32\libMagick-10.dll; DestDir: {app}
Source: ..\..\win32\libiconv-2.dll; DestDir: {app}
Source: ..\..\win32\libfuppes-0.dll; DestDir: {app}
Source: ..\..\win32\libFLAC-8.dll; DestDir: {app}
Source: ..\..\win32\libfaad-0.dll; DestDir: {app}
Source: ..\..\win32\avutil-49.dll; DestDir: {app}
Source: ..\..\win32\avformat-51.dll; DestDir: {app}
Source: ..\..\win32\avcodec-51.dll; DestDir: {app}
Source: ..\..\win32\magick-modules\tiff.dll; DestDir: {app}\magick-modules\
Source: ..\..\win32\magick-modules\bmp.dll; DestDir: {app}\magick-modules\
Source: ..\..\win32\magick-modules\gif.dll; DestDir: {app}\magick-modules\
Source: ..\..\win32\magick-modules\jpeg.dll; DestDir: {app}\magick-modules\
Source: ..\..\win32\magick-modules\libjpeg-62.dll; DestDir: {app}\magick-modules\
Source: ..\..\win32\magick-modules\libpng-3.dll; DestDir: {app}\magick-modules\
Source: ..\..\win32\magick-modules\magick.dll; DestDir: {app}\magick-modules\
Source: ..\..\win32\magick-modules\png.dll; DestDir: {app}\magick-modules\
Source: ..\vfolder.cfg; DestDir: {app}
Source: ..\COPYING-FLAC; DestDir: {app}; Flags: ignoreversion; DestName: COPYING-FLAC.txt
Source: ..\COPYING-VORBIS; DestDir: {app}; Flags: ignoreversion; DestName: COPYING-VORBIS.txt
Source: ..\COPYING-OGG; DestDir: {app}; Flags: ignoreversion; DestName: COPYING-OGG.txt
Source: ..\COPYING-MUSEPACK; DestDir: {app}; Flags: ignoreversion; DestName: COPYING-MUSEPACK.txt
Source: ..\COPYING; DestDir: {app}; Flags: ignoreversion; DestName: COPYING.txt
Source: ..\README; DestDir: {app}; Flags: ignoreversion isreadme; DestName: README.txt
Source: ..\NEWS; DestDir: {app}; Flags: ignoreversion; DestName: NEWS.txt
Source: ..\ChangeLog; DestDir: {app}; Flags: ignoreversion; DestName: ChangeLog.txt
Source: ..\AUTHORS; DestDir: {app}; Flags: ignoreversion; DestName: AUTHORS.txt
Source: ..\setup\ImageMagick-License.txt; DestDir: {app}

[Icons]
Name: {group}\FUPPES; Filename: {app}\fuppes.exe; IconIndex: 0
Name: {group}\{cm:UninstallProgram,FUPPES}; Filename: {uninstallexe}
Name: {userdesktop}\FUPPES; Filename: {app}\fuppes.exe; Tasks: desktopicon; IconIndex: 0
Name: {userappdata}\Microsoft\Internet Explorer\Quick Launch\FUPPES; Filename: {app}\fuppes.exe; Tasks: quicklaunchicon; Languages: ; IconIndex: 0
Name: {group}\Edit configuration; Filename: {userappdata}\FUPPES\fuppes.cfg; Tasks: ; Languages: 
Name: {group}\FUPPES (command line); Filename: {app}\fuppescmd.exe; IconIndex: 0

[Run]
Filename: {app}\fuppes.exe; Description: {cm:LaunchProgram,Free UPnP Entertainment Service}; Flags: nowait postinstall skipifsilent unchecked; Tasks: ; Languages: 
