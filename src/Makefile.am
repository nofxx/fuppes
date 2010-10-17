## Process this file with automake to produce Makefile.in
## $Id: $

include $(top_srcdir)/PreRules.am

BUILT_SOURCES =
CLEANFILES=


INCLUDES = \
  ${FUPPES_CFLAGS}\
  ${UUID_CFLAGS}\
  ${LIBXML_CFLAGS}\
  ${PCRE_CFLAGS}\
	-DFUPPES_DATADIR=\"$(datadir)/fuppes\" \
	-DFUPPES_PLUGINDIR=\"$(libdir)/fuppes\" \
	-DFUPPES_LOCALSTATEDIR=\"$(localstatedir)/lib/fuppes\"

lib_LTLIBRARIES = libfuppes.la

bin_PROGRAMS = fuppes

fuppes_SOURCES = console_main.cpp
fuppes_LDADD = ./libfuppes.la $(LIBADD_DL)
fuppes_DEPENDENCIES = ./libfuppes.la
fuppes_LDFLAGS = \
	$(FUPPES_LIBS)\
	${UUID_LIBS}\
	${LIBXML_LIBS}\
	${PCRE_LIBS}\
	${MAD_LIBS}\
	${LIBICONV}


if WIN32

fuppes_LDADD += \
	fuppes.res

fuppes.res: windows/fuppes.rc
	$(WINDRES) -i windows/fuppes.rc --input-format=rc -o fuppes.res -O coff 

BUILT_SOURCES += \
	fuppes.res

CLEANFILES += \
	fuppes.res

endif


if BUILD_DAEMON
bin_PROGRAMS += fuppesd
fuppesd_SOURCES = daemon_main.cpp
fuppesd_LDADD = ./libfuppes.la $(LIBADD_DL)
fuppesd_DEPENDENCIES = ./libfuppes.la
fuppesd_LDFLAGS = \
	$(FUPPES_LIBS)\
	${UUID_LIBS}\
	${LIBXML_LIBS}\
	${PCRE_LIBS}\
	${MAD_LIBS}\
	${LIBICONV}

endif

libfuppes_la_SOURCES = \
  lib/SSDP/UDPSocket.h\
  lib/SSDP/SSDPCtrl.h\
	lib/SSDP/SSDPMessage.h\
  lib/SSDP/MSearchSession.h\
  lib/SSDP/NotifyMsgFactory.h\
  lib/HTTP/HTTPParser.h\
	lib/HTTP/HTTPMessage.h\
	lib/HTTP/HTTPServer.h\
  lib/HTTP/HTTPClient.h\
  lib/HTTP/HTTPRequestHandler.h\
  lib/UPnPBase.h\
	lib/UPnPDevice.h\
	lib/UPnPService.h\
  lib/UPnPActions/UPnPAction.h\
  lib/UPnPActions/UPnPAction.cpp\
  lib/UPnPActions/UPnPActionFactory.h\
  lib/UPnPActions/UPnPBrowse.h\
  lib/UPnPActions/UPnPSearch.h\
	lib/MediaServer.h\
  lib/ContentDirectory/UPnPObjectTypes.h\
  lib/ContentDirectory/FileDetails.h\
  lib/ContentDirectory/FileDetails.cpp\
  lib/ContentDirectory/PlaylistFactory.h\
  lib/ContentDirectory/PlaylistFactory.cpp\
  lib/ContentDirectory/PlaylistParser.h\
  lib/ContentDirectory/PlaylistParser.cpp\
  lib/ContentDirectory/DatabaseConnection.h\
  lib/ContentDirectory/DatabaseConnection.cpp\
  lib/ContentDirectory/ContentDatabase.h\
  lib/ContentDirectory/ContentDatabase.cpp\
  lib/ContentDirectory/DatabaseObject.h\
  lib/ContentDirectory/DatabaseObject.cpp\
  lib/ContentDirectory/UpdateThread.h\
  lib/ContentDirectory/UpdateThread.cpp\
  lib/ContentDirectory/ContentDirectory.h\
  lib/ContentDirectory/ContentDirectory.cpp\
  lib/ContentDirectory/ContentDirectoryDescription.cpp\
  lib/ConnectionManager/ConnectionManager.h\
  lib/GENA/SubscriptionMgr.h\
  lib/GENA/EventNotification.h\
  lib/Transcoding/WrapperBase.h\
  lib/Transcoding/TranscodingMgr.h\
  lib/Transcoding/TranscodingCache.h\
	lib/SharedConfig.h\
  lib/SharedLog.h\
	lib/Fuppes.h\
	lib/Common/RegEx.h\
  lib/Common/Common.cpp\
	lib/Common/Common.h\
  lib/Common/Exception.cpp\
	lib/Common/Exception.h\
  lib/Common/md5.c\
	lib/Common/md5.h\
  lib/Common/UUID.cpp\
	lib/Common/UUID.h\
  lib/Common/Timer.cpp\
	lib/Common/Timer.h\
  lib/Common/XMLParser.cpp\
	lib/Common/XMLParser.h\
	lib/Common/Process.cpp\
	lib/Common/Process.h\
	lib/Common/Thread.cpp\
	lib/Common/Thread.h\
  lib/Common/File.cpp\
	lib/Common/File.h\
  lib/Common/Directory.cpp\
	lib/Common/Directory.h\
	lib/Common/Socket.cpp\
	lib/Common/Socket.h\
	lib/Plugins/Plugin.cpp\
	lib/Plugins/Plugin.h\
  lib/SSDP/UDPSocket.cpp\
	lib/SSDP/SSDPCtrl.cpp\
	lib/SSDP/SSDPMessage.cpp\
  lib/SSDP/MSearchSession.cpp\
	lib/SSDP/NotifyMsgFactory.cpp\
	lib/HTTP/HTTPParser.cpp\
  lib/HTTP/HTTPMessage.cpp\
	lib/HTTP/HTTPServer.cpp\
  lib/HTTP/HTTPClient.cpp\
  lib/HTTP/HTTPRequestHandler.cpp\
  lib/ControlInterface/ErrorCodes.h\
  lib/ControlInterface/ControlActions.h\
  lib/ControlInterface/ControlInterface.cpp\
  lib/ControlInterface/ControlInterface.h\
  lib/ControlInterface/SoapControl.cpp\
  lib/ControlInterface/SoapControl.h\
  lib/ControlInterface/CmdlineInterface.cpp\
  lib/ControlInterface/CmdlineInterface.h\
  lib/UPnPBase.cpp\
	lib/UPnPDevice.cpp\
	lib/UPnPService.cpp\
  lib/UPnPActions/UPnPActionFactory.cpp\
  lib/UPnPActions/UPnPBrowse.cpp\
  lib/UPnPActions/UPnPSearch.cpp\
	lib/MediaServer.cpp\
	lib/ContentDirectory/FileAlterationMonitor.cpp\
	lib/ContentDirectory/FileAlterationMonitor.h\
	lib/ContentDirectory/FileAlterationHandler.cpp\
	lib/ContentDirectory/FileAlterationHandler.h\
	lib/ContentDirectory/HotPlug.cpp\
	lib/ContentDirectory/HotPlug.h\
	lib/ContentDirectory/iTunesImporter.cpp\
	lib/ContentDirectory/iTunesImporter.h\
 	lib/ContentDirectory/VirtualContainerMgr.h\
  lib/ContentDirectory/VirtualContainerMgr.cpp\
  lib/ConnectionManager/ConnectionManager.cpp\
  lib/ConnectionManager/ConnectionManagerDescription.cpp\
  lib/GENA/SubscriptionMgr.cpp\
  lib/GENA/EventNotification.cpp\
  lib/Presentation/PresentationHandler.cpp \
  lib/Presentation/PresentationHandler.h \
  lib/Presentation/PresentationPage.h \
  lib/Presentation/PageStart.h \
  lib/Presentation/PageStart.cpp \
  lib/Presentation/PageDevice.h \
  lib/Presentation/PageDevice.cpp \
  lib/Presentation/PageBrowse.h \
  lib/Presentation/PageBrowse.cpp \
  lib/Presentation/PageConfig.h \
  lib/Presentation/PageConfig.cpp \
  lib/Presentation/PageJsTest.h \
  lib/Presentation/PageJsTest.cpp \
  lib/Transcoding/TranscodingMgr.cpp\
  lib/Transcoding/TranscodingCache.cpp\
  lib/Transcoding/LameWrapper.h\
  lib/Transcoding/LameWrapper.cpp\
  lib/Transcoding/TwoLameEncoder.h\
  lib/Transcoding/TwoLameEncoder.cpp\
  lib/Transcoding/FaadWrapper.h\
  lib/Transcoding/FaadWrapper.cpp\
  lib/Transcoding/MadDecoder.h\
  lib/Transcoding/MadDecoder.cpp\
  lib/Transcoding/ExternalCmdWrapper.h\
  lib/Transcoding/ExternalCmdWrapper.cpp\
  lib/DeviceSettings/DeviceSettings.h\
  lib/DeviceSettings/DeviceSettings.cpp\
  lib/DeviceSettings/DeviceIdentificationMgr.h\
  lib/DeviceSettings/DeviceIdentificationMgr.cpp\
  lib/DeviceSettings/MacAddressTable.h\
  lib/DeviceSettings/MacAddressTable.cpp\
 	lib/XMSMediaReceiverRegistrar/XMSMediaReceiverRegistrar.h\
  lib/XMSMediaReceiverRegistrar/XMSMediaReceiverRegistrar.cpp\
  lib/XMSMediaReceiverRegistrar/XMSMediaReceiverRegistrarDescription.cpp\
  lib/Configuration/DeviceConfigFile.h\
  lib/Configuration/DeviceConfigFile.cpp\
  lib/Configuration/ConfigSettings.h\
  lib/Configuration/ConfigSettings.cpp\
  lib/Configuration/DefaultConfig.h\
	lib/Configuration/SharedObjects.h\
	lib/Configuration/SharedObjects.cpp\
	lib/Configuration/NetworkSettings.h\
	lib/Configuration/NetworkSettings.cpp\
	lib/Configuration/GlobalSettings.h\
	lib/Configuration/GlobalSettings.cpp\
	lib/Configuration/DeviceMapping.h\
	lib/Configuration/DeviceMapping.cpp\
	lib/Configuration/ContentDirectoryConfig.h\
	lib/Configuration/ContentDirectoryConfig.cpp\
	lib/Configuration/DatabaseSettings.h\
	lib/Configuration/DatabaseSettings.cpp\
	lib/Configuration/TranscodingSettings.h\
	lib/Configuration/TranscodingSettings.cpp\
	lib/Configuration/PathFinder.h\
	lib/Configuration/PathFinder.cpp\
	lib/DLNA/DLNA.h\
	lib/DLNA/DLNA.cpp\
	lib/DLNA/dlna_audio_profiles.h\
	lib/DLNA/dlna_image_profiles.h\
	lib/DLNA/dlna_video_profiles.h\
  lib/SharedConfig.cpp\
  lib/SharedLog.cpp\
  lib/Log.h\
  lib/Log.cpp\
  lib/Fuppes.cpp\
  lib/libmain.cpp

if HAVE_INOTIFY
libfuppes_la_SOURCES += \
  lib/ContentDirectory/inotify-cxx-0.7.3/inotify-cxx.h \
  lib/ContentDirectory/inotify-cxx-0.7.3/inotify-cxx.cpp
endif

libfuppes_la_LIBADD = $(LIBADD_DL)
libfuppes_la_LDFLAGS = \
	$(FUPPES_LIBS)\
	${UUID_LIBS}\
	${LIBXML_LIBS}\
	${PCRE_LIBS}\
	${MAD_LIBS}\
	${LIBICONV}

# pixmapsdir = $(datadir)$(datadir)/fuppes
# dist_pixmaps_DATA = \
#  ../resources/style.css \
#  ../resources/fuppes-small.jpg \
#  ../resources/fuppes-small.png \
#  ../resources/header-gradient.png \
#  ../resources/header-gradient-small.png

