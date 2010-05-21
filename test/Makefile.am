
if BUILD_TESTS

lib_LTLIBRARIES = libfuppestest.la
libfuppestest_la_LDFLAGS = \
	$(FUPPES_LIBS)
libfuppestest_la_SOURCES = \
  ../src/lib/Log.h \
  ../src/lib/Log.cpp \
  ../src/lib/Common/Thread.h \
  ../src/lib/Common/Thread.cpp \
  ../src/lib/Common/Socket.h \
  ../src/lib/Common/Socket.cpp \
  ../src/lib/Common/Exception.h \
  ../src/lib/Common/Exception.cpp


bin_PROGRAMS = http-test
http_test_LDADD = ./libfuppestest.la
http_test_DEPENDENCIES = ./libfuppestest.la
http_test_LDFLAGS = \
	$(FUPPES_LIBS)
http_test_SOURCES = \
  http/http-test.cpp


bin_PROGRAMS += socket-test
socket_test_LDADD = ./libfuppestest.la
socket_test_DEPENDENCIES = ./libfuppestest.la
socket_test_LDFLAGS = \
	$(FUPPES_LIBS)
socket_test_SOURCES = \
  socket/socket-test.cpp

endif
