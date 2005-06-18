## Process this file with automake to produce Makefile.in
## $Id: $

FUPPES_CXXFLAGS = \
	-Wall\
	-g

INCLUDES = ${LIBXML_CFLAGS}\
	$(FUPPES_CXXFLAGS)	

bin_PROGRAMS = fuppes

h_sources = \
  UDPSocket.h\
	SSDPCtrl.h\
	SSDPSession.h\
	SSDPMessage.h\
	NotifyMsgFactory.h\
	HTTPMessage.h\
	HTTPServer.h\
	MessageParser.h\
	UPnPDevice.h\
	UPnPService.h\
	MediaServer.h\
	ContentDirectory.h\
	SharedConfig.h\
	Fuppes.h\
	win32.h

cpp_sources = \
  UDPSocket.cpp\
	SSDPCtrl.cpp\
	SSDPSession.cpp\
	SSDPMessage.cpp\
	NotifyMsgFactory.cpp\
	HTTPMessage.cpp\
	HTTPServer.cpp\
	MessageParser.cpp\
	UPnPDevice.cpp\
	UPnPService.cpp\
	MediaServer.cpp\
	ContentDirectory.cpp\
	SharedConfig.cpp\
	Fuppes.cpp\
	main.cpp
	
fuppes_SOURCES = ${h_sources} ${cpp_sources}

fuppes_LDFLAGS = \
	-lpcre\
	${LIBXML_LIBS}

fuppes_LDADD = $(FUPPES_LIBS)
