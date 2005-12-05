## Process this file with automake to produce Makefile.in
## $Id: $

INCLUDES = ${LIBXML_CFLAGS} ${PCRE_CFLAGS} ${VORBISFILE_CFLAGS} ${TAGLIB_CFLAGS}

bin_PROGRAMS = fuppes

h_sources = \
  Common.h\
  UUID.h\
  Timer.h\
  UDPSocket.h\
  MessageBase.h\
  SSDPCtrl.h\
	SSDPMessage.h\
  MSearchSession.h\
  NotifyMsgFactory.h\
	HTTPMessage.h\
	HTTPServer.h\
  HTTPClient.h\
  UPnPBase.h\
	UPnPDevice.h\
	UPnPService.h\
  UPnPAction.h\
  UPnPActionFactory.h\
  UPnPActions/UPnPBrowse.h\
	MediaServer.h\
  ContentDirectory/UPnPObject.h\
  ContentDirectory/StorageFolder.h\
  ContentDirectory/UPnPItem.h\
  ContentDirectory/AudioItem.h\
  ContentDirectory/ImageItem.h\
  ContentDirectory/VideoItem.h\
  ContentDirectory/FileDetails.h\
  ContentDirectory/ContentDatabase.h\
  ContentDirectory/ContentDirectory.h\
  Presentation/PresentationHandler.h\
  Presentation/Stylesheet.h\
  Transcoding/WrapperBase.h\
  Transcoding/LameWrapper.h\
  Transcoding/VorbisWrapper.h\
  Transcoding/MpcWrapper.h\
	SharedConfig.h\
  SharedLog.h\
	Fuppes.h\
	RegEx.h	

cpp_sources = \
  Common.cpp\
  UUID.cpp\
  Timer.cpp\
  UDPSocket.cpp\
  MessageBase.cpp\
	SSDPCtrl.cpp\
	SSDPMessage.cpp\
  MSearchSession.cpp\
	NotifyMsgFactory.cpp\
	HTTPMessage.cpp\
	HTTPServer.cpp\
  HTTPClient.cpp\
  UPnPBase.cpp\
	UPnPDevice.cpp\
	UPnPService.cpp\
  UPnPAction.cpp\
  UPnPActionFactory.cpp\
	MediaServer.cpp\
  ContentDirectory/StorageFolder.cpp\
  ContentDirectory/AudioItem.cpp\
  ContentDirectory/ImageItem.cpp\
  ContentDirectory/VideoItem.cpp\
  ContentDirectory/FileDetails.cpp\
  ContentDirectory/ContentDatabase.cpp\
	ContentDirectory/ContentDirectory.cpp\
  Presentation/PresentationHandler.cpp\
  Presentation/Images/fuppes_png.cpp\
  Presentation/Images/fuppes_small_png.cpp\
  Transcoding/LameWrapper.cpp\
  Transcoding/VorbisWrapper.cpp\
  Transcoding/MpcWrapper.cpp\
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
