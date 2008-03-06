## Process this file with automake to produce Makefile.in
## $Id: $

if WINDOWS_BUILD

bin_PROGRAMS = fuppes-win

fuppes_win_SOURCES = \
	Common.cpp \
	Common.h \
	WinMainForm.cpp \
	WinMainForm.h \
	WinMain.cpp

fuppes_win_LDFLAGS = -mwindows
fuppes_win_LDADD = \
	./../libfuppes.la

# fuppes_applet_LDFLAGS = \
#  $(LIBPANELAPPLET_LIBS) \
#  ${LIBNOTIFY_LIBS}

endif
