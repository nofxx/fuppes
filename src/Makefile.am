## Process this file with automake to produce Makefile.in
## $Id: $

INCLUDES = ${LIBXML_CFLAGS} ${PCRE_CFLAGS} ${VORBISFILE_CFLAGS} ${TAGLIB_CFLAGS}

bin_PROGRAMS = fuppes

h_sources = \
  Common/Common.h\
  Common/md5.h\
  Common/UUID.h\
  Common/Timer.h\
  MessageBase.h\
  SSDP/UDPSocket.h\
  SSDP/SSDPCtrl.h\
	SSDP/SSDPMessage.h\
  SSDP/MSearchSession.h\
  SSDP/NotifyMsgFactory.h\
  HTTP/HTTPParser.h\
	HTTP/HTTPMessage.h\
	HTTP/HTTPServer.h\
  HTTP/HTTPClient.h\
  HTTP/HTTPRequestHandler.h\
  UPnPBase.h\
	UPnPDevice.h\
	UPnPService.h\
  UPnPActions/UPnPAction.h\
  UPnPActions/UPnPActionFactory.h\
  UPnPActions/UPnPBrowse.h\
	MediaServer.h\
  ContentDirectory/FileDetails.h\
  ContentDirectory/PlaylistFactory.h\
  ContentDirectory/ContentDatabase.h\
  ContentDirectory/ContentDirectory.h\
  ConnectionManager/ConnectionManager.h\
  GENA/SubscriptionMgr.h\
  Presentation/PresentationHandler.h\
  Presentation/Stylesheet.h\
  Transcoding/WrapperBase.h\
  Transcoding/TranscodingMgr.h\
  Transcoding/TranscodingCache.h\
  Transcoding/LameWrapper.h\
  Transcoding/TwoLameEncoder.h\
  Transcoding/VorbisWrapper.h\
  Transcoding/MpcWrapper.h\
  Transcoding/FlacWrapper.h\
	SharedConfig.h\
  SharedLog.h\
	Fuppes.h\
	Common/RegEx.h	

cpp_sources = \
  Common/Common.cpp\
  Common/md5.c\
  Common/UUID.cpp\
  Common/Timer.cpp\
  MessageBase.cpp\
  SSDP/UDPSocket.cpp\
	SSDP/SSDPCtrl.cpp\
	SSDP/SSDPMessage.cpp\
  SSDP/MSearchSession.cpp\
	SSDP/NotifyMsgFactory.cpp\
	HTTP/HTTPParser.cpp\
  HTTP/HTTPMessage.cpp\
	HTTP/HTTPServer.cpp\
  HTTP/HTTPClient.cpp\
  HTTP/HTTPRequestHandler.cpp\
  UPnPBase.cpp\
	UPnPDevice.cpp\
	UPnPService.cpp\
  UPnPActions/UPnPAction.cpp\
  UPnPActions/UPnPActionFactory.cpp\
  UPnPActions/UPnPBrowse.cpp\
	MediaServer.cpp\
  ContentDirectory/FileDetails.cpp\
  ContentDirectory/PlaylistFactory.cpp\
  ContentDirectory/ContentDatabase.cpp\
	ContentDirectory/ContentDirectory.cpp\
  ContentDirectory/ContentDirectoryDescription.cpp\
  ConnectionManager/ConnectionManager.cpp\
  ConnectionManager/ConnectionManagerDescription.cpp\
  GENA/SubscriptionMgr.cpp\
  Presentation/PresentationHandler.cpp\
  Presentation/Images/fuppes_small_png.cpp\
  Presentation/Images/header_gradient_png.cpp\
  Presentation/Images/header_gradient_small_png.cpp\
  Transcoding/TranscodingMgr.cpp\
  Transcoding/TranscodingCache.cpp\
  Transcoding/LameWrapper.cpp\
  Transcoding/TwoLameEncoder.cpp\
  Transcoding/VorbisWrapper.cpp\
  Transcoding/MpcWrapper.cpp\
  Transcoding/FlacWrapper.cpp\
	SharedConfig.cpp\
  SharedLog.cpp\
	Fuppes.cpp\
	main.cpp

fuppes_SOURCES = ${h_sources} ${cpp_sources}

fuppes_LDFLAGS = \
  $(FUPPES_LIBS)\
  ${PTHREAD_LIBS}\
  ${UUID_LIBS}\
  ${LIBXML_LIBS}\
	${PCRE_LIBS}\
  ${SQLITE3_LIBS}\
  ${TAGLIB_LIBS}
