mkdir build
cd build
cmake .. \
	-DCMAKE_TOOLCHAIN_FILE=../../../CMake-DJGPP-Toolchain.txt \
	-DCMAKE_BUILD_TYPE=Release \
	-DBUILD_SHARED_LIBS:bool=off \
	-DBUILD_PKGCONFIG_FILES=off \
  	-DBUILD_CODEC:bool=off \
	-DOPJ_USE_THREAD=off
make
