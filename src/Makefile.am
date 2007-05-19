## Process this file with automake to produce Makefile.in
## $Id: $

INCLUDES = ${FUPPES_CFLAGS} ${LIBXML_CFLAGS} ${PCRE_CFLAGS} ${VORBISFILE_CFLAGS} ${TAGLIB_CFLAGS}

lib_LTLIBRARIES = libfuppes.la

bin_PROGRAMS = fuppes fuppesd

fuppes_SOURCES = \
  console_main.cpp

fuppes_LDADD = ./libfuppes.la
fuppes_LDFLAGS = \
  $(FUPPES_LIBS)\
  ${PTHREAD_LIBS}\
  ${UUID_LIBS}\
  ${LIBXML_LIBS}\
	${PCRE_LIBS}\
  ${SQLITE3_LIBS}\
  ${TAGLIB_LIBS}


fuppesd_SOURCES = \
  daemon_main.cpp
  
fuppesd_LDADD = ./libfuppes.la
fuppesd_LDFLAGS = \
  $(FUPPES_LIBS)\
  ${PTHREAD_LIBS}\
  ${UUID_LIBS}\
  ${LIBXML_LIBS}\
	${PCRE_LIBS}\
  ${SQLITE3_LIBS}\
  ${TAGLIB_LIBS}




libfuppes_la_SOURCES = \
  lib/Common/Common.h\
  lib/Common/md5.h\
  lib/Common/UUID.h\
  lib/Common/Timer.h\
  lib/MessageBase.h\
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
  lib/HTTP/CommonFunctions.h\
  lib/UPnPBase.h\
	lib/UPnPDevice.h\
	lib/UPnPService.h\
  lib/UPnPActions/UPnPAction.h\
  lib/UPnPActions/UPnPAction.cpp\
  lib/UPnPActions/UPnPActionFactory.h\
  lib/UPnPActions/UPnPBrowse.h\
  lib/UPnPActions/UPnPSearch.h\
	lib/MediaServer.h\
  lib/ContentDirectory/FileDetails.h\
  lib/ContentDirectory/PlaylistFactory.h\
  lib/ContentDirectory/ContentDatabase.h\
  lib/ContentDirectory/ContentDirectory.h\
  lib/ConnectionManager/ConnectionManager.h\
  lib/GENA/SubscriptionMgr.h\
  lib/GENA/EventNotification.h\
  lib/Presentation/PresentationHandler.h\
  lib/Presentation/Stylesheet.h\
  lib/Transcoding/WrapperBase.h\
  lib/Transcoding/TranscodingMgr.h\
  lib/Transcoding/TranscodingCache.h\
  lib/Transcoding/LameWrapper.h\
  lib/Transcoding/TwoLameEncoder.h\
  lib/Transcoding/VorbisWrapper.h\
  lib/Transcoding/MpcWrapper.h\
  lib/Transcoding/FlacWrapper.h\
	lib/DeviceSettings/DeviceSettings.h\
	lib/DeviceSettings/DeviceIdentificationMgr.h\
	lib/XMSMediaReceiverRegistrar.h\
	lib/SharedConfig.h\
  lib/SharedLog.h\
	lib/Fuppes.h\
	lib/Common/RegEx.h\
  lib/Common/Common.cpp\
  lib/Common/md5.c\
  lib/Common/UUID.cpp\
  lib/Common/Timer.cpp\
  lib/MessageBase.cpp\
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
  lib/HTTP/CommonFunctions.cpp\
  lib/UPnPBase.cpp\
	lib/UPnPDevice.cpp\
	lib/UPnPService.cpp\
  lib/UPnPActions/UPnPActionFactory.cpp\
  lib/UPnPActions/UPnPBrowse.cpp\
  lib/UPnPActions/UPnPSearch.cpp\
	lib/MediaServer.cpp\
  lib/ContentDirectory/FileDetails.cpp\
  lib/ContentDirectory/PlaylistFactory.cpp\
  lib/ContentDirectory/ContentDatabase.cpp\
	lib/ContentDirectory/ContentDirectory.cpp\
  lib/ContentDirectory/ContentDirectoryDescription.cpp\
  lib/ConnectionManager/ConnectionManager.cpp\
  lib/ConnectionManager/ConnectionManagerDescription.cpp\
  lib/GENA/SubscriptionMgr.cpp\
  lib/GENA/EventNotification.cpp\
  lib/Presentation/PresentationHandler.cpp\
  lib/Presentation/Images/fuppes_small_png.cpp\
  lib/Presentation/Images/header_gradient_png.cpp\
  lib/Presentation/Images/header_gradient_small_png.cpp\
  lib/Presentation/Images/device_type_unknown_png.cpp\
  lib/Presentation/Images/device_type_media_server_png.cpp\
  lib/Transcoding/TranscodingMgr.cpp\
  lib/Transcoding/TranscodingCache.cpp\
  lib/Transcoding/LameWrapper.cpp\
  lib/Transcoding/TwoLameEncoder.cpp\
  lib/Transcoding/VorbisWrapper.cpp\
  lib/Transcoding/MpcWrapper.cpp\
  lib/Transcoding/FlacWrapper.cpp\
	lib/DeviceSettings/DeviceSettings.cpp\
	lib/DeviceSettings/DeviceIdentificationMgr.cpp\
	lib/XMSMediaReceiverRegistrar.cpp\
	lib/SharedConfig.cpp\
  lib/SharedLog.cpp\
	lib/Fuppes.cpp\
	lib/libmain.cpp\
	lib/ContentDirectory/FileAlterationMonitor.cpp\
	lib/ContentDirectory/FileAlterationMonitor.h\
	lib/ContentDirectory/VirtualContainerMgr.cpp\
	lib/ContentDirectory/VirtualContainerMgr.h\
	lib/Common/XMLParser.cpp\
	lib/Common/XMLParser.h\
	lib/ContentDirectory/iTunesImporter.cpp\
	lib/ContentDirectory/iTunesImporter.h\
	lib/Configuration/ConfigFile.cpp\
	lib/Configuration/ConfigFile.h
	

libfuppes_la_LDFLAGS = \
  $(FUPPES_LIBS)\
  ${PTHREAD_LIBS}\
  ${UUID_LIBS}\
  ${LIBXML_LIBS}\
	${PCRE_LIBS}\
  ${SQLITE3_LIBS}\
  ${TAGLIB_LIBS}


