#
# cross compile FUPPES for Netgear ReadyNas
# Copyright (C) 2010 Ulrich Völkel <u-voelkel@users.sourceforge.net>
#

# get the compiler from here:
# http://www.readynas.com/download/development/readynas-cross-3.3.5.tar.gz


# settings

# the build host
HOST="sparc-linux"

# target directory
PREFIX=`pwd`"/readynas"

# make command (e.g. "make -j 2")
MAKE="make -j 2"
MAKE_INSTALL="make install"

# you should not need to change anything below

# exports
export CFLAGS="-I$PREFIX/include $CFLAGS"
export CPPFLAGS="-I$PREFIX/include $CPPFLAGS"
export LDFLAGS="-L$PREFIX/lib"
export PKG_CONFIG_LIBDIR="$PREFIX/lib/pkgconfig"
export WINEDLLPATH="$PREFIX/bin"

FUPPES_DIR=`pwd`


# create target directory
if ! test -d $PREFIX; then
  #echo "enter root password for target directory creation"
  #su -c "mkdir $PREFIX && chmod 777 $PREFIX"
  mkdir $PREFIX
fi

# create source directory
if ! test -d $PREFIX"/src"; then
  mkdir $PREFIX"/src"
fi
cd $PREFIX"/src"/


# $1 = paket name, $2 = file ext, $3 = url
function loadpkt {

  if ! test -d ../downloads; then
    mkdir ../downloads
  fi

  SRC_PKT=$1$2
  if ! test -e ../downloads/$SRC_PKT; then
    wget --directory-prefix=../downloads/ $3$SRC_PKT
  fi

  case $2 in
    ".tar.gz")
      tar -xvzf ../downloads/$1$2
    ;;
    ".tar.bz2")
      tar -xvjf ../downloads/$1$2
    ;;
    ".zip")
      unzip -o ../downloads/$1$2
    ;;
  esac

  cd $1
}



HAVE_ZLIB="yes"
HAVE_ICONV="yes"
HAVE_TAGLIB="yes"


HAVE_JPEG="yes"
HAVE_FFMPEG="yes"
HAVE_FFMPEGTHUMBNAILER="yes"


HAVE_SCREEN="yes"


# zlib 1.2.3 (for taglib, exiv2, imagemagick & co.) 
# strongly recommended

if test "$HAVE_ZLIB" == "yes"; then

echo "start building zlib"

loadpkt "zlib-1.2.3" ".tar.gz" \
        "http://www.zlib.net/"

echo "AC_INIT([zlib], [1.2.3], [fuppes@ulrich-voelkel.de])
AM_INIT_AUTOMAKE([foreign])

AC_PROG_CC
AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL
AC_PROG_INSTALL
AM_PROG_INSTALL_STRIP

AC_OUTPUT([Makefile])" \
> configure.ac

echo "include_HEADERS = zlib.h zconf.h  

lib_LTLIBRARIES = libz.la

libz_la_SOURCES = \
  adler32.h adler32.c \
  compress.h compress.c \
  crc32.h crc32.c \
  gzio.h gzio.c \
  uncompr.h uncompr.c \
  deflate.h deflate.c \
  trees.h trees.c \
  zutil.h zutil.c \
  inflate.h inflate.c \
  infback.h infback.c \
  inftrees.h inftrees.c \
  inffast.h inffast.c

libz_la_LDFLAGS = -no-undefined" \
> Makefile.am

autoreconf -vfi
./configure --host=$HOST --prefix=$PREFIX
$MAKE
$MAKE_INSTALL
cd ..

else
  echo "skipped zlib"
fi


# libiconv 1.13 (recommended)
if test "$HAVE_ICONV" == "yes"; then

  echo "start building libiconv"

  loadpkt "libiconv-1.13" ".tar.gz" \
          "http://ftp.gnu.org/pub/gnu/libiconv/"
  ./configure --host=$HOST --prefix=$PREFIX
  $MAKE
  $MAKE_INSTALL
  cd ..

else
  echo "skipped libiconv"
fi



# PCRE 7.9 (required)
echo "start building pcre"
loadpkt "pcre-7.9" ".zip" \
        "ftp://ftp.csx.cam.ac.uk/pub/software/programming/pcre/"
./configure --host=$HOST --prefix=$PREFIX \
--enable-utf8 --enable-newline-is-anycrlf --disable-cpp
$MAKE
$MAKE_INSTALL
cd ..


# SQLite 3.6.21 (required)
echo "start building sqlite"
loadpkt "sqlite-amalgamation-3.6.21" ".tar.gz" \
        "http://www.sqlite.org/"
cd sqlite-3.6.21
./configure --host=$HOST --prefix=$PREFIX \
--enable-tempstore=yes --enable-threadsafe
$MAKE
$MAKE_INSTALL
cd ..


# libxml2 2.6.32 (required)
echo "start building libxml2"
loadpkt "libxml2-2.6.32" ".tar.gz" \
        "ftp://xmlsoft.org/libxml2/old/"

#loadpkt "libxml2-2.7.6" ".tar.gz" \
#        "ftp://xmlsoft.org/libxml2/"

./configure --host=$HOST --prefix=$PREFIX \
--enable-ipv6=no --with-debug=no --with-http=no \
--with-ftp=no --with-docbook=no --with-schemas=yes --with-schematron=no \
--with-catalog=no --with-html=no --with-legacy=no --with-pattern=no \
--with-push=yes --with-python=no --with-readline=no --with-regexps=no \
--with-sax1=no --with-xinclude=no --with-xpath=no --with-xptr=no --with-modules=no \
--with-valid=no --with-reader=yes --with-writer=yes
$MAKE
$MAKE_INSTALL
cd ..





#
# OPTIONAL
#

# taglib
if test "$HAVE_TAGLIB" == "yes"; then

  echo "start building taglib"

  loadpkt "taglib-1.6.1" ".tar.gz" \
          "http://developer.kde.org/~wheeler/files/src/"
  CXXFLAGS="$CXXFLAGS -DMAKE_TAGLIB_LIB" \
  LDFLAGS="$LDFLAGS" \
  ./configure --host=$HOST --prefix=$PREFIX
  $MAKE
  $MAKE_INSTALL
  cd ..
  
else
  echo "skipped taglib"
fi


# LAME
if test "$HAVE_LAME" == "yes"; then

  echo "start building lame"

  loadpkt "lame-398-2" ".tar.gz" \
          "http://downloads.sourceforge.net/lame/"

  # we need to reconfigure the sources because of this bug
  # http://git.savannah.gnu.org/gitweb/?p=autoconf.git;a=commitdiff;h=78dc34

  sed -i -e 's/AM_PATH_GTK(1.2.0, HAVE_GTK="yes", HAVE_GTK="no")/HAVE_GTK="no"\nGTK_CFLAGS=""\nAC_SUBST(GTK_CFLAGS)/' configure.in
  autoreconf -vfi
          
  ./configure --host=$HOST --prefix=$PREFIX
  $MAKE
  $MAKE_INSTALL
  cd ..

else
  echo "skipped LAME"
fi


# ogg/vorbis
if test "$HAVE_OGGVORBIS" == "yes"; then

# ogg
echo "start building ogg"
loadpkt "libogg-1.1.4" ".tar.gz" \
        "http://downloads.xiph.org/releases/ogg/"
autoreconf -vfi
./configure --host=$HOST --prefix=$PREFIX
$MAKE
$MAKE_INSTALL
cd ..

# vorbis
echo "start building vorbis"
loadpkt "libvorbis-1.2.3" ".tar.gz" \
        "http://downloads.xiph.org/releases/vorbis/"
./configure --host=$HOST --prefix=$PREFIX
$MAKE
$MAKE_INSTALL
cd ..

else
  echo "skipped ogg/vorbis"
fi


# FLAC
if test "$HAVE_FLAC" == "yes"; then

#http://sourceforge.net/tracker/index.php?func=detail&aid=2000973&group_id=13478&atid=313478

echo "start building flac"

loadpkt "flac-1.2.1" ".tar.gz" \
        "http://downloads.sourceforge.net/flac/"

sed -i -e 's/!defined __MINGW32__ &&/ /' include/share/alloc.h

sed -i -e 's/AM_MAINTAINER_MODE/AM_MAINTAINER_MODE\nAC_LIBTOOL_WIN32_DLL/' configure.in
sed -i -e 's/AM_PATH_XMMS/echo "xmms"\n# AM_PATH_XMMS/' configure.in

sed -i -e 's/@OGG_LIBS@/@OGG_LIBS@ -lws2_32/' src/libFLAC/Makefile.am 
sed -i -e 's/libFLAC_la_LDFLAGS =/libFLAC_la_LDFLAGS = -no-undefined/' src/libFLAC/Makefile.am 

autoreconf -vfi
./configure --host=$HOST --prefix=$PREFIX \
--disable-xmms-plugin --disable-cpplibs
$MAKE
$MAKE_INSTALL
cd ..

else
  echo "skipped FLAC"
fi


# musepack
if test "$HAVE_MUSEPACK" == "yes"; then

echo "start building musepack"

loadpkt "libmpcdec-1.2.6" ".tar.bz2" \
        "http://files.musepack.net/source/"

sed -i -e 's/AC_MSG_ERROR(\[working memcmp is not available.\])/echo "no memcmp"/' configure.ac
sed -i -e 's/AM_PROG_LIBTOOL/AM_PROG_LIBTOOL\nAC_PROG_CXX/' configure.ac

#loadpkt "musepack_src_r435" ".tar.gz" \
#        "http://files.musepack.net/source/"

#sed -i -e 's/\\/\n/' Makefile.am
#sed -i -e 's/\tmpcchap/#/' Makefile.am


autoreconf -vfi
./configure --host=$HOST --prefix=$PREFIX
$MAKE
$MAKE_INSTALL
cp include/mpcdec/config_win32.h $PREFIX/include/mpcdec/
cd ..

else
  echo "skipped MusePack"
fi


# MAD
if test "$HAVE_MAD" == "yes"; then

#http://downloads.sourceforge.net/mad/libid3tag-0.15.1b.tar.gz

  echo "start building mad"

  loadpkt "libmad-0.15.1b" ".tar.gz" \
          "http://downloads.sourceforge.net/mad/"
  sed -i -e 's/libmad_la_LDFLAGS =/libmad_la_LDFLAGS = -no-undefined/' Makefile.am
  aclocal
  autoconf
  automake --foreign
  ./configure --host=$HOST --prefix=$PREFIX
  $MAKE
  $MAKE_INSTALL
  mv $PREFIX/lib/libmad-0 $PREFIX/bin/libmad-0.dll
  cd ..

echo "# Package Information for pkg-config

prefix=$PREFIX
exec_prefix=\${prefix}
libdir=\${exec_prefix}/lib
includedir=\${prefix}/include

Name: mad
Description: MPEG Audio Decoder
Requires:
Version: 0.15.0b
Libs: -L\${libdir} -lmad
Cflags: -I\${includedir}" \
> $PREFIX/lib/pkgconfig/mad.pc

else
  echo "skipped libmad"
fi


# exiv2
if test "$HAVE_EXIV2" == "yes"; then

  echo "start building exiv2"
  loadpkt "exiv2-0.18.2" ".tar.gz" \
          "http://www.exiv2.org/"

  PARAMS="--disable-xmp --disable-visibility"
  if test "$HAVE_ZLIB" != "yes"; then
    PARAMS="$PARAMS --without-zlib"
  fi
  
  ./configure --host=$HOST --prefix=$PREFIX $PARAMS

  sed -i -e 's/LDFLAGS = /LDFLAGS = -no-undefined /' config/config.mk
  sed -i -e 's/-lm//' config/config.mk
  $MAKE
  mv src/.libs/exiv2 src/.libs/exiv2.exe
  $MAKE_INSTALL
  cd ..

else
  echo "skipped exiv2"
fi

# todo libtiff & libungif




#libjpeg (for ImageMagick, simage and ffmpegthumbnailer)
if test "$HAVE_JPEG" == "yes"; then
  echo "start building jpeg"  
  loadpkt "jpegsrc.v7" ".tar.gz" \
          "http://www.ijg.org/files/"
  cd jpeg-7
  ./configure --host=$HOST --prefix=$PREFIX
  $MAKE
  $MAKE_INSTALL
  cd ..

else
  echo "skipped libjpeg"
fi

#libpng (for ImageMagick, simage and ffmpegthumbnailer)
if test "$HAVE_PNG" == "yes"; then
  echo "start building png"
  loadpkt "libpng-1.2.41" ".tar.bz2" \
          "http://prdownloads.sourceforge.net/libpng/"
  ./configure --host=$HOST --prefix=$PREFIX \
  --with-libpng-compat=no
  $MAKE
  $MAKE_INSTALL
  cd ..

else
  echo "skipped libpng"
fi


#simage
if test "$HAVE_SIMAGE" == "yes"; then

  echo "start building simage"
  loadpkt "simage-1.6.1" ".tar.gz" \
          "http://ftp.coin3d.org/coin/src/all/"

  ./configure --host=$HOST --prefix=$PREFIX
  $MAKE
  $MAKE_INSTALL
  cd ..

else
  echo "skipped simage"
fi


# ImageMagick
if test "$HAVE_IMAGEMAGICK" == "yes"; then

  echo "start building ImageMagick"
  loadpkt "ImageMagick-6.5.8-5" ".tar.gz" \
          "ftp://ftp.imagemagick.org/pub/ImageMagick/"

  sed -i -e 's/"\*.la"/"\*.dll"/' magick/module.c
  sed -i -e 's/"%s.la"/"%s.dll"/' magick/module.c
  sed -i -e 's/GetEnvironmentValue("MAGICK_CODER_MODULE_PATH")/ConstantString(".\\\\magick-modules")/' magick/module.c
  sed -i -e 's/GetEnvironmentValue("MAGICK_CODER_FILTER_PATH")/ConstantString(".\\\\magick-filters")/' magick/module.c


  ./configure --host=$HOST --prefix=$PREFIX \
  --disable-deprecated --without-perl --with-modules \
  --without-x --without-gslib --with-magick-plus-plus=no \
  --with-quantum-depth=8 --enable-embeddable
  #--disable-installed 
  
  $MAKE CFLAGS="-DHAVE_BOOLEAN $CFLAGS"
  $MAKE_INSTALL
  cd ..

else
  echo "skipped ImageMagick"
fi


# So. 24 Jan :: rev 21414

if test "$HAVE_FFMPEG" == "yes"; then

echo "start building ffmpeg"

if ! test -d ffmpeg-svn; then
  svn co svn://svn.mplayerhq.hu/ffmpeg/trunk ffmpeg-svn
  cd ffmpeg-svn
else
  cd ffmpeg-svn
  svn update
fi

#sed -i -e 's/__MINGW32_MINOR_VERSION >= 15/__MINGW32_MINOR_VERSION >= 13/' configure
#sed -i -e 's/usleep/Sleep/' ffmpeg.c
 
./configure --prefix=$PREFIX --enable-cross-compile --target-os=linux --arch=sparc \
--cross-prefix=$HOST- --enable-shared \
--enable-gpl --disable-ffmpeg --disable-ffplay --disable-ffserver \
--disable-demuxer=dv1394 --disable-indevs \
--disable-decoder=flv --disable-encoder=flv --disable-demuxer=flv \
--disable-muxer=flv \
--as=/usr/bin/sparc-linux-as


#./configure --prefix=$PREFIX --enable-cross-compile  \
# --cross-prefix=$HOST- --enable-shared \
#--disable-static --disable-ffmpeg --disable-ffplay --disable-ffserver \
#--disable-demuxer=dv1394 --disable-indevs --arch=sparc  \
#--disable-decoder=flv --disable-encoder=flv --disable-demuxer=flv \
#--disable-muxer=flv



$MAKE
$MAKE_INSTALL
cd ..

else
  echo "skipped ffmpeg"
fi


if test "$HAVE_FFMPEGTHUMBNAILER" == "yes"; then

  echo "start building ffmpegthumbnailer"
  loadpkt "ffmpegthumbnailer-1.5.6" ".tar.gz" \
          "http://ffmpegthumbnailer.googlecode.com/files/"

  LIBS="$LIBS -lz" ./configure --host=$HOST --prefix=$PREFIX 
  $MAKE
  $MAKE_INSTALL
  cd ..

else
  echo "skipped ffmpegthumbnailer"
fi





# FUPPES
cd $FUPPES_DIR

# add /usr/include for inotify support
#export CFLAGS="$CFLAGS -I/usr/include"
#export CPPFLAGS="$CPPFLAGS -I/usr/include"

#mkdir readynas/include/sys
#cp /usr/include/sys/inotify.h readynas/include/sys/

./configure --host=$HOST --prefix=$PREFIX
$MAKE
$MAKE_INSTALL


# strip libraries and executables
if test "$DO_STRIP" == "yes"; then
  $HOST-strip -s $PREFIX/bin/*
  $HOST-strip -s $PREFIX/lib/bin/*
  touch $PREFIX/bin/*
  touch $PREFIX/lib/bin/*
  touch $PREFIX/share/fuppes/*
fi




if test "$HAVE_SCREEN" == "yes"; then


  cd $PREFIX
  cd src

  loadpkt "screen-4.0.3" ".tar.gz" \
          "http://ftp.gnu.org/gnu/screen/"
  

  # patch from http://bugs.gentoo.org/attachment.cgi?id=173051&action=view
  wget -O crosscompile.patch "http://bugs.gentoo.org/attachment.cgi?id=173051&action=view" 
  patch -p0 -i crosscompile.patch 


  ./configure --host=$HOST --prefix=$PREFIX
  $MAKE
  $MAKE_INSTALL
  cd ..

fi



