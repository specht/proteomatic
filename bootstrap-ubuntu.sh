#!/bin/bash
if [ "$1" != "go" ]; then
	echo "This script downloads, compiles and installs the md5-rfc and yaml-cpp libs which are required for Proteomatic to compile."
	echo "Call with 'go' to proceed at your own risk."
	exit 1
fi
sudo apt-get install qmake libqt4-dev g++ cmake
rm -rf bootstrap
mkdir bootstrap
cd bootstrap

# download and install md5-rfc
mkdir md5
cd md5
wget http://downloads.sourceforge.net/project/libmd5-rfc/libmd5-rfc/2002-04-13/md5.tar.gz
tar xzf md5.tar.gz
echo "TEMPLATE = lib
CONFIG += staticlib
TARGET = 
DEPENDPATH += .
INCLUDEPATH += .
HEADERS += md5.h
SOURCES += md5.c" > md5.pro
qmake
make
sudo cp md5.h /usr/local/include
sudo cp libmd5.a /usr/local/lib
cd ..
rm -rf md5

# download and install yaml-cpp

mkdir yaml-cpp
cd yaml-cpp/
wget http://yaml-cpp.googlecode.com/files/yaml-cpp-0.2.6.tar.gz
tar xzf yaml-cpp-0.2.6.tar.gz
cd yaml-cpp/
cmake .
make
sudo make install
cd ..
cd ..
rm -rf yaml-cpp

# clean up

cd ..
rm -rf bootstrap
