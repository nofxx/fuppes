[Setup]
AppName=Free UPnP Entertainment Service
AppVerName=Free UPnP Entertainment Service SVN-640
AppPublisher=Ulrich Völkel
AppPublisherURL=http://sourceforge.net/projects/fuppes
AppSupportURL=http://sourceforge.net/projects/fuppes
AppUpdatesURL=http://sourceforge.net/projects/fuppes
DefaultDirName={pf}\Free UPnP Entertainment Service\
DefaultGroupName=Free UPnP Entertainment Service
LicenseFile=..\setup\license-eng.rtf
OutputDir=..\setup
OutputBaseFilename=fuppes-SVN-640-win32-setup
SolidCompression=true
ShowTasksTreeLines=true
SourceDir=..\setup
AppCopyright=Ulrich Völkel
WizardImageFile=compiler:wizmodernimage-is.bmp
WizardSmallImageFile=compiler:wizmodernsmallimage-is.bmp
ShowLanguageDialog=auto
SetupIconFile=..\fuppes.ico

[Languages]
Name: eng; Messagesfile: compiler:Default.isl; LicenseFile: ..\setup\license-eng.rtf
Name: ger; Messagesfile: compiler:Languages\German.isl; LicenseFile: ..\setup\license-eng.rtf

[Tasks]
Name: desktopicon; Description: {cm:CreateDesktopIcon}; GroupDescription: {cm:AdditionalIcons}; Flags: unchecked
Name: quicklaunchicon; Description: {cm:CreateQuickLaunchIcon}; GroupDescription: {cm:AdditionalIcons}; Flags: unchecked
Name: presentation; Description: install webinterface plugin; GroupDescription: {cm:MiscPlugins}; Languages: ; Flags: unchecked
Name: metadata_dlna; Description: install DLNA profiles plugin; GroupDescription: {cm:MetadataPlugins}
Name: metadata_taglib; Description: install taglib plugin; GroupDescription: {cm:MetadataPlugins}
Name: metadata_exiv2; Description: install exiv2 plugin; GroupDescription: {cm:MetadataPlugins}
Name: metadata_simage; Description: install simage plugin; GroupDescription: {cm:MetadataPlugins}
Name: metadata_magickwand; Description: install ImageMagick plugin; GroupDescription: {cm:MetadataPlugins}; Flags: unchecked
Name: metadata_libavformat; Description: install libavformat plugin; GroupDescription: {cm:MetadataPlugins}
Name: decoder_vorbis; Description: install ogg/vorbis plugin; GroupDescription: {cm:AudioDecoderPlugins}
Name: decoder_flac; Description: install FLAC plugin; GroupDescription: {cm:AudioDecoderPlugins}
Name: decoder_musepack; Description: install MusePack/mpc plugin; GroupDescription: {cm:AudioDecoderPlugins}
Name: transcoder_ffmpeg; Description: install ffmpeg plugin; GroupDescription: {cm:TranscoderPlugins}
Name: transcoder_magickwand; Description: install ImageMagick plugin; GroupDescription: {cm:TranscoderPlugins}; Flags: unchecked

[Files]
Source: ..\ChangeLog; DestDir: "{app}"; DestName: ChangeLog.txt
Source: ..\AUTHORS; DestDir: "{app}"; DestName: AUTHORS.txt
Source: ..\COPYING; DestDir: "{app}"; DestName: COPYING.txt
Source: ..\setup\COPYING-VORBIS.txt; DestDir: "{app}"; Tasks: " decoder_vorbis"
Source: ..\setup\COPYING-OGG.txt; DestDir: "{app}"; Tasks: " decoder_vorbis"
Source: ..\setup\COPYING-FLAC.txt; DestDir: "{app}"; Tasks: " decoder_flac"
Source: ..\setup\COPYING-MUSEPACK.txt; DestDir: "{app}"; Tasks: " decoder_musepack"
Source: ..\setup\ImageMagick-License.txt; DestDir: "{app}"; Tasks: " metadata_magickwand transcoder_magickwand"
Source: ..\..\win32\fuppes.exe; DestDir: "{app}"
Source: ..\..\win32\fuppes-win.exe; DestDir: "{app}"
Source: ..\..\win32\libfuppes-0.dll; DestDir: "{app}"
Source: ..\..\win32\libiconv-2.dll; DestDir: "{app}"
Source: ..\..\win32\libpcre-0.dll; DestDir: "{app}"
Source: ..\..\win32\libxml2-2.dll; DestDir: "{app}"
Source: ..\..\win32\libz-0.dll; DestDir: "{app}"
Source: ..\..\win32\libdatabase_sqlite3-0.dll; DestDir: "{app}"
Source: ..\..\win32\libsqlite3-0.dll; DestDir: "{app}"
Source: ..\..\win32\libcore_presentation-0.dll; DestDir: "{app}"; Tasks: " presentation"
Source: ..\..\win32\data\fuppes-small.png; DestDir: "{app}\data"
Source: ..\..\win32\data\header-gradient.png; DestDir: "{app}\data"
Source: ..\..\win32\data\header-gradient-small.png; DestDir: "{app}\data"
Source: ..\..\win32\data\style.css; DestDir: "{app}\data"
Source: ..\..\win32\libmetadata_dlna_profiles-0.dll; DestDir: "{app}"; Tasks: " metadata_dlna"
Source: ..\..\win32\libmetadata_taglib-0.dll; DestDir: "{app}"; Tasks: " metadata_taglib"
Source: ..\..\win32\libtag-1.dll; DestDir: "{app}"; Tasks: " metadata_taglib"
Source: ..\..\win32\libmetadata_exiv2-0.dll; DestDir: "{app}"; Tasks: " metadata_exiv2"
Source: ..\..\win32\libexiv2-5.dll; DestDir: "{app}"; Tasks: " metadata_exiv2"
Source: ..\..\win32\libmetadata_simage-0.dll; DestDir: "{app}"; Tasks: " metadata_simage"
Source: ..\..\win32\libsimage-20.dll; DestDir: "{app}"; Tasks: " metadata_simage"
Source: ..\..\win32\libmetadata_libavformat-0.dll; DestDir: "{app}"; Tasks: " metadata_libavformat"
Source: ..\..\win32\avformat-52.dll; DestDir: "{app}"; Tasks: " metadata_libavformat transcoder_ffmpeg"
Source: ..\..\win32\avcodec-52.dll; DestDir: "{app}"; Tasks: " metadata_libavformat transcoder_ffmpeg"
Source: ..\..\win32\avutil-50.dll; DestDir: "{app}"; Tasks: " metadata_libavformat transcoder_ffmpeg"
Source: ..\..\win32\libmetadata_magickwand-0.dll; DestDir: "{app}"; Tasks: " metadata_magickwand"
Source: ..\..\win32\libMagickWand-2.dll; DestDir: "{app}"; Tasks: " metadata_magickwand transcoder_magickwand"
Source: ..\..\win32\libMagickCore-2.dll; DestDir: "{app}"; Tasks: " metadata_magickwand transcoder_magickwand"
Source: ..\..\win32\libdecoder_vorbis-0.dll; DestDir: "{app}"; Tasks: " decoder_vorbis"
Source: ..\..\win32\libvorbisfile-3.dll; DestDir: "{app}"; Tasks: " decoder_vorbis"
Source: ..\..\win32\libvorbis-0.dll; DestDir: "{app}"; Tasks: " decoder_vorbis"
Source: ..\..\win32\libogg-0.dll; DestDir: "{app}"; Tasks: " decoder_vorbis"
Source: ..\..\win32\libdecoder_flac-0.dll; DestDir: "{app}"; Tasks: " decoder_flac"
Source: ..\..\win32\libFLAC-8.dll; DestDir: "{app}"; Tasks: " decoder_flac"
Source: ..\..\win32\libdecoder_musepack-0.dll; DestDir: "{app}"; Tasks: " decoder_musepack"
Source: ..\..\win32\libmpcdec-5.dll; DestDir: "{app}"; Tasks: " decoder_musepack"
Source: ..\..\win32\libtranscoder_ffmpeg-0.dll; DestDir: "{app}"; Tasks: " transcoder_ffmpeg"
Source: ..\..\win32\swscale-0.dll; DestDir: "{app}"; Tasks: " transcoder_ffmpeg"
Source: ..\..\win32\libtranscoder_magickwand-0.dll; DestDir: "{app}"; Tasks: " transcoder_magickwand"

[Icons]
Name: {group}\FUPPES; Filename: "{app}\fuppes.exe"; IconIndex: 0
Name: {group}\{cm:UninstallProgram,FUPPES}; Filename: {uninstallexe}
Name: {userdesktop}\FUPPES; Filename: "{app}\fuppes.exe"; Tasks: desktopicon; IconIndex: 0
Name: {userappdata}\Microsoft\Internet Explorer\Quick Launch\FUPPES; Filename: "{app}\fuppes.exe"; Tasks: quicklaunchicon; Languages: ; IconIndex: 0
Name: {group}\Edit configuration; Filename: {userappdata}\FUPPES\fuppes.cfg; Tasks: ; Languages: 
Name: {group}\FUPPES (command line); Filename: "{app}\fuppescmd.exe"; IconIndex: 0

[Run]
Filename: "{app}\fuppes.exe"; Description: {cm:LaunchProgram,Free UPnP Entertainment Service}; Flags: nowait postinstall skipifsilent unchecked; Tasks: ; Languages: 
[CustomMessages]
MiscPlugins=miscellaneous plugins
MetadataPlugins=metadata plugins
AudioDecoderPlugins=audio decoder plugins
AudioEncoderPlugins=audio encoder plugins
TranscoderPlugins=transcoding plugins
