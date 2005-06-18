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
	NotifyMsgFactory.h\
	Message.h\
	HTTPMessage.h\
	MessageParser.h\
	HTTPServer.h\
	UPnPDevice.h\
	UPnPService.h\
	MediaServer.h\
	ContentDirectory.h\
	SharedConfig.h\
	Fuppes.h

cpp_sources = \
  UDPSocket.cpp\
	SSDPCtrl.cpp\
	SSDPSession.cpp\
	NotifyMsgFactory.cpp\
	Message.cpp\
	HTTPMessage.cpp\
	MessageParser.cpp\
	HTTPServer.cpp\
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
