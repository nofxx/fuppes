## Process this file with automake to produce Makefile.in
## $Id: $

if WIN32

bin_PROGRAMS = fuppes-win

fuppes_win_SOURCES = \
	Common.cpp \
	Common.h \
	WinMainForm.cpp \
	WinMainForm.h \
	WinMain.cpp \
	fuppes.rc

fuppes_win_LDFLAGS = \
	-mwindows

fuppes_win_LDADD = \
	./../libfuppes.la \
	fuppes.res


fuppes.res: fuppes.rc
	$(WINDRES) -i fuppes.rc --input-format=rc -o fuppes.res -O coff

BUILT_SOURCES = \
	fuppes.res

CLEANFILES = \
	fuppes.res

endif # WIN32

