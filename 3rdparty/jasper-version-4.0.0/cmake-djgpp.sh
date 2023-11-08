BUILD_DIR=djgpp/
SOURCE_DIR=.

mkdir $BUILD_DIR

cmake -H$SOURCE_DIR -B$BUILD_DIR \
	-DCMAKE_TOOLCHAIN_FILE=../../../CMake-DJGPP-Toolchain.txt \
	-DJAS_STDC_VERSION=c99 \
	-DCMAKE_BUILD_TYPE=Release \
	-DBUILD_SHARED_LIBS:bool=off \
	-DJAS_ENABLE_PIC=false \
	-DJAS_ENABLE_MULTITHREADING_SUPPORT=false \
	-DJAS_ENABLE_PROGRAMS=false \
	-DJAS_ENABLE_DOC=false \
	-DJAS_ENABLE_LIBJPEG=false \
	-DJAS_ENABLE_OPENGL=false \
	-DJAS_ENABLE_SHARED=false \
	-DJAS_ENABLE_HIDDEN=false \
	-DJAS_ENABLE_32BIT=true \
	-DJAS_INCLUDE_BMP_CODEC=false \
	-DJAS_INCLUDE_JP2_CODEC=true \
	-DJAS_INCLUDE_JPC_CODEC=true \
	-DJAS_INCLUDE_JPG_CODEC=false \
	-DJAS_INCLUDE_MIF_CODEC=false \
	-DJAS_INCLUDE_PGX_CODEC=false \
	-DJAS_INCLUDE_PNM_CODEC=true \
	-DJAS_INCLUDE_RAS_CODEC=true \
	-DJAS_ENABLE_BMP_CODEC=false \
	-DJAS_ENABLE_JP2_CODEC=true \
	-DJAS_ENABLE_JPC_CODEC=true \
	-DJAS_ENABLE_JPG_CODEC=false \
	-DJAS_ENABLE_MIF_CODEC=false \
	-DJAS_ENABLE_PGX_CODEC=false \
	-DJAS_ENABLE_PNM_CODEC=true \
	-DJAS_ENABLE_RAS_CODEC=true \
	-DJAS_ENABLE_ASAN=false \
	-DJAS_ENABLE_UBSAN=false \
	-DJAS_ENABLE_LSAN=false \
	-DJAS_ENABLE_MSAN=false \
	-DJAS_HAVE_MKOSTEMP=false

make -C $BUILD_DIR
