dnl Process this file with autoconf to produce a configure script.
dnl $Id: $

AC_INIT([fuppes], [0.633], [fuppes@ulrich-voelkel.de])
AM_CONFIG_HEADER(src/config.h)
AM_INIT_AUTOMAKE([1.9])

AC_PROG_CC
AC_PROG_CXX
AC_DISABLE_STATIC
#AC_DISABLE_SHARED
AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL
AC_PROG_INSTALL
AM_PROG_INSTALL_STRIP

AC_FUNC_FSEEKO
AC_TYPE_OFF_T
AC_SYS_LARGEFILE
AC_C_BIGENDIAN

PKG_PROG_PKG_CONFIG

FUPPES_CFLAGS=""
FUPPES_LIBS=""

# CHECK HOST
is_windows=false
build_daemon=true
AC_MSG_CHECKING([whether we are building with mingw])
case $host in 
  *-*-mingw32*)
    AC_MSG_RESULT([yes])

    FUPPES_LIBS="$FUPPES_LIBS -lws2_32 -lole32"
		LDFLAGS="$LDFLAGS -no-undefined"

    AC_CHECK_TOOL(WINDRES, windres)
		build_daemon=false
		is_windows=true
	;;
	*)
    AC_MSG_RESULT([no])
	;;
esac
AM_CONDITIONAL([BUILD_DAEMON], [test x$build_daemon = xtrue])
AM_CONDITIONAL([WINDOWS_BUILD], [test x$is_windows = xtrue])
AM_CONDITIONAL([WIN32], [test x$is_windows = xtrue])


dnl Checks for header files.

AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netdb.h netinet/in.h string.h sys/ioctl.h sys/param.h sys/socket.h unistd.h sys/utsname.h syslog.h])
dnl AC_CHECK_FUNC(select)
AC_CHECK_FUNCS(select)

dnl Checks for typedefs, structures, and compiler characteristics.

AC_CHECK_SIZEOF(off_t)
AC_CHECK_SIZEOF(long long int)
AC_CHECK_SIZEOF(unsigned long long int)
AC_CHECK_SIZEOF(long int)
AC_CHECK_SIZEOF(unsigned long int)
AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(unsigned int)
AC_CHECK_SIZEOF(short)
AC_CHECK_SIZEOF(unsigned short)



dnl Checks for libraries.

echo ""
echo "check minimum dependencies"
echo ""

PKG_CHECK_MODULES(PCRE, libpcre >= 5.0)
PKG_CHECK_MODULES(LIBXML, libxml-2.0 >= 2.6.1)
PKG_CHECK_MODULES(SQLITE3, sqlite3 >= 3.2)
AC_CHECK_LIB(pthread, pthread_create, FUPPES_LIBS="$FUPPES_LIBS -lpthread")


echo ""
echo "check recommended dependencies"
echo ""

# uuid
have_uuid=no
AH_TEMPLATE([HAVE_UUID], [])
PKG_CHECK_MODULES(UUID, uuid >= 0, have_uuid=yes, have_uuid=no)
if test "x$have_uuid" = "xyes"; then
  AC_DEFINE([HAVE_UUID], [1], [])
fi
AC_SUBST(UUID_CFLAGS)
AC_SUBST(UUID_LIBS)


AM_ICONV
if test x$is_windows = xtrue; then
# the above iconv macro executes some tests that require a shared iconv lib.
# if we are crosscompiling we can use  wine to execute the tests (wine-bin on debian)
# or we just set the LDFLAGS if at least the  iconv function declaration is found
	if test "x$am_cv_func_iconv" = "xyes"; then
		am_func_iconv=yes
		LIBICONV="-liconv"
		AC_SUBST(LIBICONV)
		AC_DEFINE(HAVE_ICONV, 1)
	fi
fi



echo ""
echo "check optional stuff"
echo ""


# TAGLIB	
have_taglib=no
AC_ARG_ENABLE([taglib], [AC_HELP_STRING([--enable-taglib],
    [enable taglib metadata plugin [default=yes]])],
    [enable_taglib=$enableval], [enable_taglib='yes'])
if test "x$enable_taglib" = "xyes" ; then

	TAGLIB_CFLAGS=""
	TAGLIB_LIBS=""

	if test -z "$TAGLIB_CONFIG"; then
		AC_PATH_PROG(TAGLIB_CONFIG, taglib-config, no, $prefix/bin/)
	fi

	if test "$TAGLIB_CONFIG" != "no" ; then   

		AC_MSG_CHECKING(TAGLIB_CFLAGS)
		TAGLIB_CFLAGS=`$TAGLIB_CONFIG --cflags`
		AC_MSG_RESULT($TAGLIB_CFLAGS)
	
		AC_MSG_CHECKING(TAGLIB_LIBS)
		TAGLIB_LIBS=`$TAGLIB_CONFIG --libs`
		AC_MSG_RESULT($TAGLIB_LIBS)
	
		have_taglib=yes
	fi

fi
AM_CONDITIONAL(TAGLIB_METADATA_PLUGIN, test "x$have_taglib" = "xyes")
AC_SUBST(TAGLIB_CFLAGS)
AC_SUBST(TAGLIB_LIBS)
# END TAGLIB


#
# ImageMagick (MagickWand)
#
have_magickwand=no
AC_ARG_ENABLE([magickwand], [AC_HELP_STRING([--enable-magickwand],
    [enable ImageMagick metadata/transcoder plugin [default=yes]])],
    [enable_magickwand=$enableval], [enable_magickwand='yes'])
if test "x$enable_magickwand" = "xyes" ; then
  PKG_CHECK_MODULES(IMAGEMAGICK_WAND, Wand >= 6.2.4, have_magickwand=yes, have_magickwand=no)
fi
AM_CONDITIONAL(MAGICK_WAND_METADATA_PLUGIN, test "x$have_magickwand" = "xyes")
AM_CONDITIONAL(MAGICK_WAND_TRANSCODER_PLUGIN, test "x$have_magickwand" = "xyes")
#end ImageMagick




#
# Exiv2
#
have_exiv2=no
AC_ARG_ENABLE([exiv2], [AC_HELP_STRING([--enable-exiv2],
    [enable exiv2 metadata plugin [default=yes]])],
    [enable_exiv2=$enableval], [enable_exiv2='yes'])
if test "x$enable_exiv2" = "xyes" ; then
  PKG_CHECK_MODULES(EXIV2, exiv2 >= 0.16, have_exiv2=yes, have_exiv2=no)
fi
AM_CONDITIONAL(EXIV2_METADATA_PLUGIN, test "x$have_exiv2" = "xyes")
AC_SUBST(EXIV2_CFLAGS)
AC_SUBST(EXIV2_LIBS)




#
# libavformat
#
AH_TEMPLATE([HAVE_LIBAVFORMAT], [])
AH_TEMPLATE([HAVE_AVSTRING_H], [])
AH_TEMPLATE([HAVE_LIBSWSCALE], [])
AH_TEMPLATE([HAVE_AV_PARSE_VIDEO_FUNCTS], [])
AH_TEMPLATE([FFMPEG_VERSION], [])
use_libavformat=yes
have_avstring_h=no
AC_ARG_ENABLE(libavformat, 
            [  --disable-libavformat  Do not use libavformat (from the FFmpeg package)],
            [ use_libavformat=no; ] )


have_libavformat=no
have_libavformat52=no
have_libavformat51=no
have_libavformat50=no
ffmpeg_new_includes=no

have_libswscale=no
if test "x$use_libavformat" = "xyes"; then
  LIBAVFORMAT_CFLAGS=""
  LIBAVFORMAT_LIBS=""

  PKG_CHECK_MODULES(LIBAVFORMAT, libavformat >= 52, [have_libavformat52=yes have_libavformat=yes], have_libavformat52=no)
  
	if test "x$have_libavformat52" = "xno"; then
		PKG_CHECK_MODULES(LIBAVFORMAT, libavformat >= 51, [have_libavformat51=yes have_libavformat=yes], have_libavformat51=no)
  
		if test "x$have_libavformat51" = "xno"; then
			PKG_CHECK_MODULES(LIBAVFORMAT, libavformat, [have_libavformat50=yes have_libavformat=yes], have_libavformat50=no)
		fi
		
	else
		AC_CHECK_HEADERS(libavformat/avformat.h, ffmpeg_new_includes=yes, ffmpeg_new_includes=no)
		if test "x$ffmpeg_new_includes" = "xno"; then
			LIBAVFORMAT_CFLAGS="$LIBAVFORMAT_CFLAGS -DOLD_INCLUDES_PATH"
		fi
	fi

  if test "x$have_libavformat" = "xyes"; then
		
		AC_DEFINE([HAVE_LIBAVFORMAT], [1], [])
		PKG_CHECK_MODULES(LIBSWSCALE, libswscale, have_libswscale=yes, have_libswscale=no)
		if test "x$have_libswscale" = "xyes"; then
      AC_DEFINE([HAVE_LIBSWSCALE], [1], [])
    fi
		
		if test "x$ffmpeg_new_includes" = "xyes"; then		
	    AC_CHECK_HEADERS(libavutil/avstring.h, have_avstring_h=yes, have_avstring_h=no)
    else
	    AC_CHECK_HEADERS(ffmpeg/avstring.h, have_avstring_h=yes, have_avstring_h=no)    
    fi
    if test "x$have_avstring_h" = "xyes"; then
      AC_DEFINE([HAVE_AVSTRING_H], [1], [])
    fi

		if test "x$have_libavformat52" = "xyes"; then
      LIBAVFORMAT_CFLAGS="$LIBAVFORMAT_CFLAGS -DFFMPEG_VERSION=52"
			AC_DEFINE([FFMPEG_VERSION], [52], [])
			AC_DEFINE([HAVE_AV_PARSE_VIDEO_FUNCTS], [1], [])
		elif test "x$have_libavformat51" = "xyes"; then
			AC_DEFINE([FFMPEG_VERSION], [51], [])
			#AC_DEFINE([HAVE_AV_PARSE_VIDEO_FUNCTS], [1], [])
		elif test "x$have_libavformat50" = "xyes"; then
			AC_DEFINE([FFMPEG_VERSION], [50], [])
		fi

  else
    AC_MSG_NOTICE([*** libavformat (FFmpeg) support disabled ***]);
  fi 
fi
AM_CONDITIONAL(LIBAVFORMAT_METADATA_PLUGIN, test "x$have_libavformat" = "xyes")
AM_CONDITIONAL(FFMPEG_TRANSCODER_PLUGIN, test "x$have_libavformat" = "xyes")
# end libavformat



AC_ARG_ENABLE([mp4v2], [AC_HELP_STRING([--enable-mp4v2],
    [enable mp4v2 metadata plugin [default=yes]])],
    [enable_mp4v2=$enableval], [enable_mp4v2='yes'])

have_mpeg4ip=no
if test "x$enable_mp4v2" = "xyes" ; then

	
	if test -z "$MPEG4IP_CONFIG"; then
		AC_PATH_PROG(MPEG4IP_CONFIG, mpeg4ip-config, no, $prefix/bin/)
	fi

	if test "$MPEG4IP_CONFIG" = "no" ; then   
		 AC_MSG_NOTICE([*** mpeg4ip/libmp4v2 support disabled ***]);
	else

		AC_MSG_CHECKING(MP4V2_CFLAGS)
		MP4V2_CFLAGS=`$MPEG4IP_CONFIG --cflags`
		MP4V2_LIBS="-lmp4v2"
		AC_MSG_RESULT($MP4V2_CFLAGS)
		
		have_mpeg4ip=yes
	fi

	AC_SUBST(MP4V2_CFLAGS)
	AC_SUBST(MP4V2_LIBS)
fi
AM_CONDITIONAL(MP4V2_METADATA_PLUGIN, test "x$have_mpeg4ip" = "xyes")





              

# check for LAME
have_lame=no
LAME_LIBS=""
AC_ARG_ENABLE([lame], [AC_HELP_STRING([--enable-lame],
    [enable lame encoder plugin [default=no]])],
    [enable_lame=$enableval], [enable_lame='no'])

AC_ARG_WITH(lame-dir, AC_HELP_STRING([--with-lame-dir=path],
            [include path of lame/lame.h]),
            [
	      	if test "x$withval" != "x"; then
	          LAME_H_DIR="$withval/"
		  CXXFLAGS="$CXXFLAGS -I$withval"
		fi
	    ],
	    [LAME_H_DIR=""])
AC_SUBST(LAME_H_DIR)

if test "x$enable_lame" = "xyes" ; then

	AH_TEMPLATE([HAVE_LAME], [])
	AC_CHECK_HEADERS([${LAME_H_DIR}lame/lame.h], have_lame=yes, have_lame=no)
	if test "x$have_lame" = "xyes"; then    
		LAME_LIBS="-lmp3lame"
		AC_DEFINE([HAVE_LAME], [1], [])  
	fi
fi

# check for TwoLAME
have_twolame=no
TWOLAME_LIBS=""
AC_ARG_ENABLE([twolame], [AC_HELP_STRING([--enable-twolame],
    [enable twolame encoder plugin [default=no]])],
    [enable_twolame=$enableval], [enable_twolame='no'])
    
if test "x$enable_twolame" = "xyes" ; then
	AH_TEMPLATE([HAVE_TWOLAME], [])
	PKG_CHECK_MODULES(TWOLAME, twolame >= 0.3.8, have_twolame=yes, have_twolame=no)
	if test "x$have_twolame" = "xyes"; then
		AC_DEFINE([HAVE_TWOLAME], [1], [])
	fi
fi		




# vorbis decoder plugin
have_vorbis=no
have_tremor=no
AC_ARG_ENABLE([vorbis], [AC_HELP_STRING([--enable-vorbis],
    [enable vorbis decoder plugin [default=yes]])],
    [enable_vorbis=$enableval], [enable_vorbis='yes'])
AC_ARG_ENABLE([tremor], [AC_HELP_STRING([--enable-tremor],
    [use tremor instead of libvorbisfile [default=no]])],
    [enable_tremor=$enableval], [enable_tremor='no'])
    
if test "x$enable_vorbis" = "xyes" ; then

  TREMOR_H_DIR=""
  if test "x$enable_tremor" = "xyes"; then
    AC_CHECK_HEADERS(${TREMOR_H_DIR}tremor/ivorbisfile.h, have_tremor=yes, have_tremor=no)
    if test "x$have_tremor" = "xyes"; then
      VORBISFILE_CFLAGS="-DTREMOR"
		else
			echo "Tremor not found. Checking for libvorbisfile instead."
		fi
  fi

  if test "x$have_tremor" = "xno"; then
    PKG_CHECK_MODULES(VORBISFILE, vorbisfile >= 1.0, have_vorbis=yes, have_vorbis=no)    
  fi

fi


# musepack decoder plugin
MUSEPACK_LIBS=""
have_mpc=no
AC_ARG_ENABLE([musepack], [AC_HELP_STRING([--enable-musepack],
    [enable musepack decoder plugin [default=yes]])],
    [enable_musepack=$enableval], [enable_musepack='yes'])

AC_ARG_WITH(musepack-dir, AC_HELP_STRING([--with-musepack-dir=path],
        [include path of mpcdec/mpcdec.h]),
        [
            if test "x$withval" != "x"; then
              MPC_H_DIR="$withval/"
              CXXFLAGS="$CXXFLAGS -I$withval"
            fi
        ],
        [MPC_H_DIR=""])
      AC_SUBST(MPC_H_DIR)

if test "x$enable_musepack" = "xyes" ; then
    AC_CHECK_HEADERS(${MPC_H_DIR}mpcdec/config_types.h, have_mpc=yes, have_mpc=no)
    if test "x$have_mpc" = "xyes"; then      
      MUSEPACK_LIBS="-lmpcdec"      
		fi
fi



# flac decoder plugin
have_flac=no
FLAC_LIBS=""
AC_ARG_ENABLE([flac], [AC_HELP_STRING([--enable-flac],
    [enable flac decoder plugin [default=yes]])],
    [enable_flac=$enableval], [enable_flac='yes'])

AC_ARG_WITH(flac-dir, AC_HELP_STRING([--with-flac-dir=path],
        [include path of FLAC/file_decoder.h (flac <= 1.1.2) or FLAC/stream_decoder.h (flac >= 1.1.3)]),
        [
            if test "x$withval" != "x"; then
              FLAC_H_DIR="$withval/"
              CXXFLAGS="$CXXFLAGS -I$withval"
            fi
        ],
        [FLAC_H_DIR=""])
  AC_SUBST(FLAC_H_DIR)

if test "x$enable_flac" = "xyes" ; then

	# flac <= 1.1.2
  AC_CHECK_HEADERS(${FLAC_H_DIR}FLAC/file_decoder.h, have_flac=yes, have_flac=no)
  if test "x$have_flac" = "xyes"; then      
    FLAC_LIBS="-lFLAC"
		FLAC_CFLAGS="-DHAVE_FLAC_FILEDECODER"
  fi

	# flac >= 1.1.3
  if test "x$have_flac" = "xno"; then      
  AC_CHECK_HEADERS(${FLAC_H_DIR}FLAC/stream_decoder.h, have_flac=yes, have_flac=no)
    if test "x$have_flac" = "xyes"; then      
      FLAC_LIBS="-lFLAC"
    fi
  fi
fi



# libmad
have_mad=no
AC_ARG_ENABLE([mad], [AC_HELP_STRING([--enable-mad],
    [enable mad support [default=no]])],
    [enable_mad=$enableval], [enable_mad='no'])
if test "x$enable_mad" = "xyes" ; then
	AH_TEMPLATE([HAVE_MAD], [])
	PKG_CHECK_MODULES(MAD, mad >= 0.15, have_mad=yes, have_mad=no)    
	if test "x$have_mad" = "xyes"; then      
		have_decoder=yes
		AC_DEFINE([HAVE_MAD], [1], [])
	fi
fi



# faad
have_faad=no
have_mp4ff=no
AC_ARG_ENABLE([faad], [AC_HELP_STRING([--enable-faad],
    [enable faad support [default=no]])],
    [enable_faad=$enableval], [enable_faad='no'])

AC_ARG_WITH(faad-dir, AC_HELP_STRING([--with-faad-dir=path],
	      [include path of faad.h]),
	      [if test "x$withval" != "x"; then
	         FAAD_H_DIR="$withval/"
	         CXXFLAGS="$CXXFLAGS -I$withval"
	       fi], [FAAD_H_DIR=""])

if test "x$enable_faad" = "xyes" ; then

	AH_TEMPLATE([HAVE_FAAD], [])
	AH_TEMPLATE([HAVE_MP4FF_H], [])

	AC_CHECK_HEADERS(${FAAD_H_DIR}faad.h, have_faad=yes, have_faad=no)
	if test "x$have_faad" = "xyes"; then
		AC_RUN_IFELSE(
		  [AC_LANG_PROGRAM(
		    [[#include <faad.h>]],
		    [[(void) faacDecOpen]])],
		  [have_decoder=yes
		   AC_DEFINE([HAVE_FAAD], [1], [])],
		  [have_faad=no])

		AC_SUBST(FAAD_H_DIR)

		AC_CHECK_HEADERS(${FAAD_H_DIR}mp4ff.h, have_mp4ff=yes, have_mp4ff=no)
		if test "x$have_mp4ff" = "xyes"; then
		  AC_DEFINE([HAVE_MP4FF_H], [1], [])
		else
		  echo "mp4ff.h not found. AAC decoder is build without MP4 support"
		fi
	fi

fi


# audio encoder

AM_CONDITIONAL(LAME_ENCODER_PLUGIN, test "x$have_lame" = "xyes")
#AC_SUBST(LAME_CFLAGS)
AC_SUBST(LAME_LIBS)

AM_CONDITIONAL(TWOLAME_ENCODER_PLUGIN, test "x$have_twolame" = "xyes")
AC_SUBST(TWOLAME_CFLAGS)
AC_SUBST(TWOLAME_LIBS)

# audio decoder

AM_CONDITIONAL(VORBIS_DECODER_PLUGIN, test "x$have_vorbis" = "xyes")
AC_SUBST(VORBISFILE_CFLAGS)
AC_SUBST(VORBISFILE_LIBS)

AM_CONDITIONAL(MUSEPACK_DECODER_PLUGIN, test "x$have_mpc" = "xyes")
AC_SUBST(MUSEPACK_CFLAGS)
AC_SUBST(MUSEPACK_LIBS)

AM_CONDITIONAL(FLAC_DECODER_PLUGIN, test "x$have_flac" = "xyes")
AC_SUBST(FLAC_CFLAGS)
AC_SUBST(FLAC_LIBS)

AM_CONDITIONAL(FAAD_DECODER_PLUGIN, test "x$have_faad" = "xyes")
AC_SUBST(FAAD_CFLAGS)
AC_SUBST(FAAD_LIBS)

AM_CONDITIONAL(MAD_DECODER_PLUGIN, test "x$have_mad" = "xyes")
AC_SUBST(MAD_CFLAGS)
AC_SUBST(MAD_LIBS)




if test "x$transcoding" = "xno"; then
  ENABLE_TRANSCODING="false"
  CXXFLAGS="$CXXFLAGS -DDISABLE_TRANSCODING"
  AC_MSG_NOTICE(Transcoding disabled");     
fi
AM_CONDITIONAL(DISABLE_TRANSCODING, test x$ENABLE_TRANSCODING = xfalse)




SIMAGE_CFLAGS=""
SIMAGE_LIBS=""
have_simage=no
if test -z "$SIMAGE_CONFIG"; then
  AC_PATH_PROG(SIMAGE_CONFIG, simage-config, no, $prefix/bin/)
fi

if test "$SIMAGE_CONFIG" != "no" ; then   
  AC_MSG_CHECKING(SIMAGE_CFLAGS)
  SIMAGE_CFLAGS=`$SIMAGE_CONFIG --cflags`
  AC_MSG_RESULT($SIMAGE_CFLAGS)
  
	AC_MSG_CHECKING(SIMAGE_LIBS)
	SIMAGE_LIBS=`$SIMAGE_CONFIG --libs`
  AC_MSG_RESULT($SIMAGE_LIBS)

  have_simage=yes
fi
AM_CONDITIONAL(SIMAGE_METADATA_PLUGIN, test "x$have_simage" = "xyes")
AC_SUBST(SIMAGE_CFLAGS)
AC_SUBST(SIMAGE_LIBS)



MYSQL_CFLAGS=""
MYSQL_LIBS=""
have_mysql=no
if test -z "$MYSQL_CONFIG"; then
  AC_PATH_PROG(MYSQL_CONFIG, mysql_config, no, $prefix/bin/)
fi

if test "$MYSQL_CONFIG" != "no" ; then   
  AC_MSG_CHECKING(MYSQL_CFLAGS)
  MYSQL_CFLAGS=`$MYSQL_CONFIG --cflags`
  AC_MSG_RESULT($MYSQL_CFLAGS)
  
	AC_MSG_CHECKING(MYSQL_LIBS)
	MYSQL_LIBS=`$MYSQL_CONFIG --libs`
  AC_MSG_RESULT($MYSQL_LIBS)
  
  have_mysql=yes
fi
AM_CONDITIONAL(MYSQL_DATABASE_PLUGIN, test "x$have_mysql" = "xyes")
AC_SUBST(MYSQL_CFLAGS)
AC_SUBST(MYSQL_LIBS)





dnl Checks for Additional stuffs.

# DLNA
AC_ARG_ENABLE([dlna], [AC_HELP_STRING([--enable-dlna],
    [enable dlna support [default=yes]])],
    [enable_dlna=$enableval], [enable_dlna='yes'])
AM_CONDITIONAL(DLNA_SUPPORT, test "x$enable_dlna" = "xyes")


# HTTP Port
AC_ARG_WITH([http-port],
    [AC_HELP_STRING([--with-http-port=PORT],
                    [set the default HTTP port])],
    [with_http_port=$enableval], [with_http_port=''])
if test "$with_http_port" != ''; then
	CXXFLAGS="$CXXFLAGS -DDEFAULT_HTTP_PORT=$with_http_port"
fi


# inotify
have_inotify=no
AH_TEMPLATE([HAVE_INOTIFY], [inotify support])
AC_ARG_ENABLE([inotify], [AC_HELP_STRING([--enable-inotify],
    [enable inotify support (Linux only) [default=yes]])],
    [enable_inotify=$enableval], [enable_inotify='yes'])
if test "x$enable_inotify" = "xyes" ; then	
	AC_CHECK_HEADERS(sys/inotify.h, 
			have_inotify=yes
			AC_DEFINE([HAVE_INOTIFY], [1], []), 
			have_inotify=no)
fi
AM_CONDITIONAL([HAVE_INOTIFY], [test "x$have_inotify" = "xyes"])




AC_SUBST(FUPPES_LIBS)
AC_SUBST(FUPPES_CFLAGS)

AC_OUTPUT([
Makefile
m4/Makefile
src/Makefile
src/plugins/Makefile
src/windows/Makefile
])

dnl summary

echo ""
echo "  SUMMARY"
echo ""


echo "  audio transcoding plugins"  
echo "    encoder:"
if test "x$have_lame" = "xyes"; then
  echo "      lame       : yes"
else
  echo "      lame       : no"
fi

if test "x$have_twolame" = "xyes"; then
  echo "      twolame    : yes"
else  
  echo "      twolame    : no"
fi

echo "      pcm/wav    : yes"
  
echo "    decoder:"
if test "x$have_vorbis" = "xyes"; then
  echo "      vorbis     : yes (libvorbisfile)"
else
  if test "x$have_tremor" = "xyes"; then
    echo "      vorbis     : yes (tremor)"
  else
    echo "      vorbis     : no"      
  fi
fi

if test "x$have_mpc" = "xyes"; then
  echo "      mpc        : yes"
else
  echo "      mpc        : no"      
fi     

if test "x$have_flac" = "xyes"; then
  echo "      flac       : yes"
else
  echo "      flac       : no"      
fi 

if test "x$have_faad" = "xyes"; then
  if test "x$have_mp4ff" = "xyes"; then
    echo "      faad       : yes (aac/mp4/m4a)"
  else
    echo "      faad       : yes (aac/NO mp4/m4a)"
  fi
else
  echo "      faad       : no (aac/mp4/m4a)"
fi 

if test "x$have_mad" = "xyes"; then
  echo "      mad        : yes (mpeg Layer I, II & III)"
else
  echo "      mad        : no (mpeg Layer I, II & III)"      
fi 

echo ""
#end audio transcoding

echo "  video transcoding plugins"
if test "x$have_libavformat" = "xyes"; then
  echo "    ffmpeg     : enabled"
else
  echo "    ffmpeg     : disabled"
fi
echo ""


echo "  image conversion/rescaling plugins"
if test "x$have_magickwand" = "xyes"; then
  echo "    ImageMagick: enabled  (Wand C-API)"
else
  echo "    ImageMagick: disabled (Wand C-API)"
fi
echo ""
#end image conversion/rescaling


echo "  audio metadata extraction plugins"
# taglib
if test "x$have_taglib" = "xyes"; then
  echo "    taglib        : enabled  (mp3, ogg, flac & mpc)"
else
  echo "    taglib        : disabled (mp3, ogg, flac & mpc)"
fi

# mpeg4ip
if test "x$have_mpeg4ip" = "xyes"; then
  echo "    mpeg4ip/mp4v2 : enabled  (mp4/m4a)"
else
  echo "    mpeg4ip/mp4v2 : disabled (mp4/m4a)"
fi
echo ""


echo "  image metadata extraction plugins"
# Exiv2
if test "x$have_exiv2" = "xyes"; then
  echo "    Exiv2         : enabled"
else
  echo "    Exiv2         : disabled"
fi

# ImageMagick
if test "x$have_magickwand" = "xyes"; then
  echo "    ImageMagick   : enabled  (Wand C-API)"
else
  echo "    ImageMagick   : disabled (Wand C-API)"
fi

# simage
if test "x$have_simage" = "xyes"; then
  echo "    simage        : enabled  (jpeg, png, gif, tiff, rgb, pic, tga, eps)"
else
  echo "    simage        : disabled (jpeg, png, gif, tiff, rgb, pic, tga, eps)"
fi
echo ""


echo "  video metadata extraction plugins"
# libavformat
if test "x$have_libavformat" = "xyes"; then
  echo "    libavformat   : enabled"
else
  echo "    libavformat   : disabled"
fi
echo ""
#end metadata


echo "  miscellaneous"

# iconv
if test "x$am_func_iconv" = "xyes"; then
  echo "    iconv      : enabled (charset conversion)"
else
  echo "    iconv      : disabled (charset conversion)"
fi

# uuid
if test x$is_windows = xfalse; then
if test "x$have_uuid" = "xyes"; then
  echo "    uuid       : enabled"
else
  echo "    uuid       : disabled"
fi
fi

# mysql
#if test "x$have_mysql" = "xyes"; then
#  echo "    mysql      : enabled"
#else
#  echo "    mysql      : disabled"
#fi

# inotify
if test "x$have_inotify" = "xyes"; then
  echo "    inotify    : enabled"
else
  echo "    inotify    : disabled"
fi


echo ""
echo "Thanks for using fuppes"
echo "please report bugs"
echo ""

