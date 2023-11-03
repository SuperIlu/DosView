#!/bin/sh
#
# This is a helper script for using a cross compiler to build and install
# the Allegro library. It is currently set up to use the MinGW
# cross-compiler out of the box but you can edit XC_PATH and INSTALL_BASE
# to use it with for example a djgpp cross-compiler.

# 1. Put here the path on which the cross compiler and other tools
# for the target will be found with standard names.

# Note: these paths are Mac OS X specific.
XC_PATH=/home/ilu/djgpp/bin
XPREFIX=i586-pc-msdosdjgpp-

# 2. Put here the path for where things are to be installed.
# You should have created the lib, info and include directories
# in this directory.

# Note: unused. I don't recommend using install, it probably
# won't work correctly. (msikma, 2016)
INSTALL_BASE=/usr/local/djgpp/

# Set up some environment variables and export them to GNU make.

CROSSCOMPILE=1
MINGDIR=$INSTALL_BASE
DJDIR=$INSTALL_BASE
NATIVEPATH=$PATH
PATH=$XC_PATH:$NATIVEPATH
DEBUGMODE=1

export CROSSCOMPILE MINGDIR DJDIR NATIVEPATH PATH XPREFIX

# Then run make and pass through all command line parameters to it.

make $*
