###
# Makefile for cross compiling
# All compilation was done with DJGPP 12.2.0 built from https://github.com/jwt27/build-gcc
# make sure the DJGPP toolchain is in your path (i586-pc-msdosdjgpp-XXX)!
###

THIRDPARTY	= 3rdparty
ALLEGRO		= $(THIRDPARTY)/allegro-4.2.2-xc-master
ZLIB		= $(THIRDPARTY)/zlib-1.2.12
ALPNG		= $(THIRDPARTY)/alpng13
WEBP		= $(THIRDPARTY)/libwebp-1.3.2
JPEG		= $(THIRDPARTY)/jpeg-9e
TIFF		= $(THIRDPARTY)/tiff-4.6.0
JASPER		= $(THIRDPARTY)/jasper-version-4.0.0
ALGIF		= $(THIRDPARTY)/algif_1.3
STB			= $(THIRDPARTY)/stb

LIB_ALLEGRO	= $(ALLEGRO)/lib/djgpp/liballeg.a
LIB_Z		= $(ZLIB)/msdos/libz.a
LIB_ALPNG	= $(ALPNG)/libalpng.a
LIB_WEBP 	= $(WEBP)/src/libwebp.a
LIB_JPEG 	= $(JPEG)/libjpeg.a
LIB_TIFF 	= $(TIFF)/libtiff/.libs/libtiff.a
LIB_JASPER 	= $(JASPER)/djgpp/src/libjasper/libjasper.a
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
	-I$(realpath $(JASPER))/djgpp/src/libjasper/include/ \
	-I$(realpath $(WEBP))/src

# linker
LIBS     = -ljpeg -lwebp -lsharpyuv -lalpng -lalgif -ltiff -ljasper -lz -lalleg -lm -lemu 
LDFLAGS  = -s \
	-L$(DOJSPATH)/$(ALLEGRO)/lib/djgpp \
	-L$(DOJSPATH)/$(ALPNG) \
	-L$(DOJSPATH)/$(ALGIF) \
	-L$(DOJSPATH)/$(WEBP)/src \
	-L$(DOJSPATH)/$(WEBP)/sharpyuv \
	-L$(DOJSPATH)/$(JPEG) \
	-L$(DOJSPATH)/$(TIFF)/libtiff/.libs \
	-L$(DOJSPATH)/$(JASPER)/djgpp/src/libjasper/ \
	-L$(DOJSPATH)/$(ZLIB)

# output
EXE      = dosview.exe
RELZIP   = dosview-X.Y.zip

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

liballegro: $(LIB_ALLEGRO)
$(LIB_ALLEGRO):
	cd $(ALLEGRO) && bash ./xmake.sh lib

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
	(cd $(JASPER) && bash ./cmake-djgpp.sh)

$(EXE): init liballegro libz alpng algif libwebp libjpeg libtiff libjasper $(PARTS) 
	$(CC) $(LDFLAGS) -o $@ $(PARTS) $(LIBS)

$(BUILDDIR)/%.o: src/%.c Makefile
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/loadpng/%.o: $(LOADPNG)/%.c Makefile
	$(CC) $(CFLAGS) -c $< -o $@

zip: all
	rm -f $(RELZIP)
	zip -9 -r $(RELZIP) $(EXE) CWSDPMI.EXE LICENSE *.md

init: configure_tiff
	mkdir -p $(BUILDDIR) $(BUILDDIR)/loadpng

clean:
	rm -rf $(BUILDDIR)/
	rm -f $(EXE) $(ZIP)

distclean: clean zclean alclean webpclean jpegclean distclean_tiff jasperclean alpngclean algifclean
	rm -f OUT.* LOW.*

zclean:
	$(MAKE) -C $(ZLIB) -f Makefile.dojs clean

jpegclean:
	$(MAKE) -C $(JPEG) -f makefile.dj clean

alclean:
	cd $(ALLEGRO) && bash ./xmake.sh clean

webpclean:
	$(MAKE) -C $(WEBP) -f makefile.djgpp clean

tiffclean:
	$(MAKE) -C $(TIFF) clean

alpngclean:
	$(MAKE) -C $(ALPNG) -f Makefile.zlib clean

algifclean:
	$(MAKE) -C $(ALGIF) -f Makefile.dj clean

jasperclean:
	rm -rf $(JASPER)/djgpp

fixnewlines:
	find . -iname "*.sh" -exec dos2unix -v \{\} \;

configure_tiff: $(TIFF)/Makefile
$(TIFF)/Makefile:
	(cd $(TIFF) && HOST=$(CROSS) CFLAGS="$(CFLAGS)" LDFLAGS="" LIBS="" bash ./djgpp-config.sh)

distclean_tiff:
	-(cd $(TIFF) && make distclean)

.PHONY: clean distclean init distclean_tiff

DEPS := $(wildcard $(BUILDDIR)/*.d)
ifneq ($(DEPS),)
include $(DEPS)
endif
