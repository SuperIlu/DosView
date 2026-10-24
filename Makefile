###
# Makefile for cross compiling
# All compilation was done with DJGPP 12.2.0 built from https://github.com/jwt27/build-gcc
# make sure the DJGPP toolchain is in your path (i586-pc-msdosdjgpp-XXX)!
###

# temp directory for building FreeDOS archive
TMP=/tmp/FDOS

THIRDPARTY	= 3rdparty
ALLEGRO		= $(THIRDPARTY)/DOSlegro
ZLIB		= $(THIRDPARTY)/zlib-1.3.2
ALPNG		= $(THIRDPARTY)/alpng13
WEBP		= $(THIRDPARTY)/libwebp-1.6.0
JPEG		= $(THIRDPARTY)/jpeg-10
TIFF		= $(THIRDPARTY)/tiff-4.7.2
JASPER		= $(THIRDPARTY)/jasper-4.2.9
ALGIF		= $(THIRDPARTY)/algif_1.3
STB			= $(THIRDPARTY)/stb

JASPER_BUILD=$(THIRDPARTY)/djgpp-jasper

LIB_Z		= $(ZLIB)/msdos/libz.a
LIB_ALPNG	= $(ALPNG)/libalpng.a
LIB_WEBP 	= $(WEBP)/src/libwebp.a
LIB_JPEG 	= $(JPEG)/libjpeg.a
LIB_TIFF 	= $(TIFF)/libtiff/.libs/libtiff.a
LIB_JASPER 	= $(JASPER_BUILD)/src/libjasper/libjasper.a
LIB_ALGIF 	= $(ALGIF)/libalgif.a

# compiler
CDEF     = #-DDEBUG_ENABLED
CFLAGS   = -MMD -Wall -std=gnu99 -Os -march=i386 -mtune=i586 -ffast-math -fomit-frame-pointer $(INCLUDES) -fgnu89-inline -Wmissing-prototypes $(CDEF)
INCLUDES = \
	-I$(realpath ./src) \
	-I$(realpath $(ALLEGRO))/include \
	-I$(realpath $(ZLIB)) \
	-I$(realpath $(ALPNG))/src \
	-I$(realpath $(ALGIF))/src \
	-I$(realpath $(JPEG)) \
	-I$(realpath $(STB)) \
	-I$(realpath $(TIFF))/libtiff \
	-I$(realpath $(JASPER))/src/libjasper/include \
	-I$(realpath $(JASPER_BUILD))/src/libjasper/include \
	-I$(realpath $(WEBP))/src

# linker
LIBS     = -ljpeg -lwebp -lsharpyuv -lalpng -lalgif -ltiff -ljasper -lz -lalleg -lm -lemu 
LDFLAGS  = -s \
	-L$(DOJSPATH)/$(ALLEGRO)/lib/ \
	-L$(DOJSPATH)/$(ALPNG) \
	-L$(DOJSPATH)/$(ALGIF) \
	-L$(DOJSPATH)/$(WEBP)/src \
	-L$(DOJSPATH)/$(WEBP)/sharpyuv \
	-L$(DOJSPATH)/$(JPEG) \
	-L$(DOJSPATH)/$(TIFF)/libtiff/.libs \
	-L$(DOJSPATH)/$(JASPER_BUILD)/src/libjasper \
	-L$(DOJSPATH)/$(ZLIB)

# output
EXE      = dosview.exe
UPXEXE   = upxview.exe
RELZIP   = dosview-X.Y.zip
FDZIP    = $(shell pwd)/FreeDOS_dosview-X.Y.zip

# dirs/files
DOJSPATH		= $(realpath .)
BUILDDIR		= build

CROSS=i586-pc-msdosdjgpp
CROSS_PLATFORM=i586-pc-msdosdjgpp-
CC=$(CROSS_PLATFORM)gcc
CXX=$(CROSS_PLATFORM)g++
AR=$(CROSS_PLATFORM)ar
LD=$(CROSS_PLATFORM)ld
STRIP=$(CROSS_PLATFORM)strip
RANLIB=$(CROSS_PLATFORM)ranlib
export

MKDIRPRG=mkdir
UPXPRG=./upx/upx
ZIPPRG=zip
SHPRG=bash
RMPRG=rm
FINDPRG=find
CPPRG=cp
GITPRG=git

MPARA=-j8

PARTS= \
	$(BUILDDIR)/format-stb.o \
	$(BUILDDIR)/format-jasper.o \
	$(BUILDDIR)/format-qoi.o \
	$(BUILDDIR)/format-webp.o \
	$(BUILDDIR)/format-jpeg.o \
	$(BUILDDIR)/format-tiff.o \
	$(BUILDDIR)/util.o \
	$(BUILDDIR)/main.o

all: dosview

dosview: $(EXE)

libz: $(LIB_Z)
$(LIB_Z):
	$(MAKE) $(MPARA) -C $(ZLIB) -f Makefile.dojs

alpng: libz $(LIB_ALPNG)
$(LIB_ALPNG):
	$(MAKE) -C $(ALPNG) -f Makefile.zlib

algif: $(LIB_ALGIF)
$(LIB_ALGIF):
	$(MAKE) -C $(ALGIF) -f Makefile.dj

libwebp: $(LIB_WEBP)
$(LIB_WEBP):
	$(MAKE) $(MPARA) -C $(WEBP) -f makefile.djgpp src/libwebp.a sharpyuv/libsharpyuv.a

libjpeg: $(LIB_JPEG)
$(LIB_JPEG):
	$(MAKE) $(MPARA) -C $(JPEG) -f makefile.dj libjpeg.a

libtiff: $(LIB_TIFF)
$(LIB_TIFF):
	$(MAKE) $(MPARA) -C $(TIFF)

libjasper: $(LIB_JASPER)
$(LIB_JASPER):
	(cd $(JASPER) && $(SHPRG) ./cmake-djgpp.sh)

$(EXE): init libz alpng algif libwebp libjpeg libtiff libjasper $(PARTS) 
	$(CC) $(LDFLAGS) -o $@ $(PARTS) $(LIBS)

$(BUILDDIR)/%.o: src/%.c Makefile
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/loadpng/%.o: $(LOADPNG)/%.c Makefile
	$(CC) $(CFLAGS) -c $< -o $@

zip: all
	$(RMPRG) -f $(RELZIP) $(UPXEXE)
	$(UPXPRG) -9 -o$(UPXEXE) $(EXE)
	$(ZIPPRG) -9 $(RELZIP) $(EXE) $(UPXEXE) CWSDPMI.EXE LICENSE *.md

init: configure_tiff
	$(MKDIRPRG) -p $(BUILDDIR) $(BUILDDIR)/loadpng

clean:
	$(RMPRG) -rf $(BUILDDIR)/
	$(RMPRG) -f $(EXE) $(RELZIP) upxview.exe UPXVIEW.EXE

distclean: clean zclean webpclean jpegclean distclean_tiff jasperclean alpngclean algifclean
	$(RMPRG) -f OUT.* LOW.*

zclean:
	$(MAKE) -C $(ZLIB) -f Makefile.dojs clean

jpegclean:
	$(MAKE) -C $(JPEG) -f makefile.dj clean

webpclean:
	$(MAKE) -C $(WEBP) -f makefile.djgpp clean

tiffclean:
	$(MAKE) -C $(TIFF) clean

alpngclean:
	$(MAKE) -C $(ALPNG) -f Makefile.zlib clean

algifclean:
	$(MAKE) -C $(ALGIF) -f Makefile.dj clean

jasperclean:
	$(RMPRG) -rf $(JASPER_BUILD)

fixnewlines:
	$(FINDPRG) . -iname "*.sh" -exec dos2unix -v \{\} \;

configure_tiff: $(TIFF)/Makefile
$(TIFF)/Makefile:
	(cd $(TIFF) && HOST=$(CROSS) CFLAGS="$(CFLAGS)" LDFLAGS="" LIBS="" $(SHPRG) ./djgpp-config.sh)

distclean_tiff:
	-(cd $(TIFF) && make distclean)

fdos: zip
	# clean and re-create  working directories
	$(RMPRG) -rf $(TMP) $(FDZIP)
	$(MKDIRPRG) -p $(TMP)/APPINFO
	$(MKDIRPRG) -p $(TMP)/UTIL/DOSVIEW
	$(MKDIRPRG) -p $(TMP)/SOURCE/DOSVIEW
	$(MKDIRPRG) -p $(TMP)/tmp
	
	# copy LSMs
	$(CPPRG) FDOS/* $(TMP)/APPINFO

	# copy distribution files
	$(CPPRG) -R \
		$(EXE) CWSDPMI.EXE LICENSE *.md \
		$(TMP)/UTIL/DOSVIEW

	# get sources
	$(GITPRG) clone https://github.com/SuperIlu/DosView.git $(TMP)/tmp
	$(RMPRG) -rf $(TMP)/tmp/.git
	$(RMPRG) -rf $(TMP)/tmp/images

	# zip up sources and remove tmp
	(cd $(TMP)/tmp && $(ZIPPRG) -9 -r ../SOURCE/DOSVIEW/SOURCES.ZIP * && $(RMPRG) -rf $(TMP)/tmp)

	# ZIP up everything as DOS ZIP and clean afterwards
	(cd $(TMP) && $(ZIPPRG) -k -9 -r $(FDZIP) *)
	$(RMPRG) -rf $(TMP)

.PHONY: clean distclean init distclean_tiff fdos

DEPS := $(wildcard $(BUILDDIR)/*.d)
ifneq ($(DEPS),)
include $(DEPS)
endif
