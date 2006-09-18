## Process this file with automake to produce Makefile.in
## $Id: $

INCLUDES = ${LIBXML_CFLAGS} ${PCRE_CFLAGS} ${VORBISFILE_CFLAGS} ${TAGLIB_CFLAGS}

bin_PROGRAMS = fuppes

h_sources = \
  Common.h\
  md5.h\
  UUID.h\
  Timer.h\
  MessageBase.h\
  SSDP/UDPSocket.h\
  SSDP/SSDPCtrl.h\
	SSDP/SSDPMessage.h\
  SSDP/MSearchSession.h\
  SSDP/NotifyMsgFactory.h\
	HTTP/HTTPMessage.h\
	HTTP/HTTPServer.h\
  HTTP/HTTPClient.h\
  HTTP/HTTPRequestHandler.h\
  UPnPBase.h\
	UPnPDevice.h\
	UPnPService.h\
  UPnPAction.h\
  UPnPActionFactory.h\
  UPnPActions/UPnPBrowse.h\
	MediaServer.h\
  ContentDirectory/UPnPObject.h\
  ContentDirectory/UPnPContainer.h\
  ContentDirectory/UPnPItem.h\
  ContentDirectory/AudioItem.h\
  ContentDirectory/ImageItem.h\
  ContentDirectory/VideoItem.h\
  ContentDirectory/FileDetails.h\
  ContentDirectory/UPnPObjectFactory.h\
  ContentDirectory/ContentDatabase.h\
  ContentDirectory/ContentDirectory.h\
  ConnectionManager/ConnectionManager.h\
  Presentation/PresentationHandler.h\
  Presentation/Stylesheet.h\
  Transcoding/WrapperBase.h\
  Transcoding/LameWrapper.h\
  Transcoding/VorbisWrapper.h\
  Transcoding/MpcWrapper.h\
  Transcoding/FlacWrapper.h\
	SharedConfig.h\
  SharedLog.h\
	Fuppes.h\
	RegEx.h	

cpp_sources = \
  Common.cpp\
  md5.c\
  UUID.cpp\
  Timer.cpp\
  MessageBase.cpp\
  SSDP/UDPSocket.cpp\
	SSDP/SSDPCtrl.cpp\
	SSDP/SSDPMessage.cpp\
  SSDP/MSearchSession.cpp\
	SSDP/NotifyMsgFactory.cpp\
	HTTP/HTTPMessage.cpp\
	HTTP/HTTPServer.cpp\
  HTTP/HTTPClient.cpp\
  HTTP/HTTPRequestHandler.cpp\
  UPnPBase.cpp\
	UPnPDevice.cpp\
	UPnPService.cpp\
  UPnPAction.cpp\
  UPnPActionFactory.cpp\
  UPnPActions/UPnPBrowse.cpp\
	MediaServer.cpp\
  ContentDirectory/UPnPContainer.cpp\
  ContentDirectory/AudioItem.cpp\
  ContentDirectory/ImageItem.cpp\
  ContentDirectory/VideoItem.cpp\
  ContentDirectory/FileDetails.cpp\
  ContentDirectory/UPnPObjectFactory.cpp\
  ContentDirectory/ContentDatabase.cpp\
	ContentDirectory/ContentDirectory.cpp\
  ContentDirectory/ContentDirectoryDescription.cpp\
  ConnectionManager/ConnectionManager.cpp\
  ConnectionManager/ConnectionManagerDescription.cpp\
  Presentation/PresentationHandler.cpp\
  Presentation/Images/fuppes_png.cpp\
  Presentation/Images/fuppes_small_png.cpp\
  Presentation/Images/header_gradient_png.cpp\
  Presentation/Images/header_gradient_small_png.cpp\
  Transcoding/LameWrapper.cpp\
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
  ${LIBXML_LIBS}\
	${PCRE_LIBS}\
  ${SQLITE3_LIBS}\
  ${TAGLIB_LIBS}
