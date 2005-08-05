## Process this file with automake to produce Makefile.in
## $Id: $

FUPPES_CXXFLAGS = \
	-Wall\
	-g

INCLUDES = ${LIBXML_CFLAGS}\
	$(FUPPES_CXXFLAGS)	

bin_PROGRAMS = fuppes

h_sources = \
  Common.h\
  UUID.h\
  UDPSocket.h\
  MessageBase.h\
  SSDPCtrl.h\
	SSDPSession.h\
	SSDPMessage.h\
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
  ContentDirectory/ContentDirectory.h\
  Presentation/PresentationHandler.h\
  Presentation/Stylesheet.h\
	SharedConfig.h\
  SharedLog.h\
	Fuppes.h\
	RegEx.h
	
cpp_sources = \
  Common.cpp\
  UUID.cpp\
  UDPSocket.cpp\
  MessageBase.cpp\
	SSDPCtrl.cpp\
	SSDPSession.cpp\
	SSDPMessage.cpp\
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
	ContentDirectory/ContentDirectory.cpp\
  Presentation/PresentationHandler.cpp\
	SharedConfig.cpp\
  SharedLog.cpp\
	Fuppes.cpp\
	main.cpp
	
fuppes_SOURCES = ${h_sources} ${cpp_sources}

fuppes_LDFLAGS = \
	-lpcre\
	${LIBXML_LIBS}

fuppes_LDADD = $(FUPPES_LIBS)
