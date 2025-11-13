#!bash
set -e

#
# Create Qt 5.10 Universal binaries for MacOS BigSur by arm64 and x86_64
#
# git clone https://github.com/kishontikft/qtbase.git
# Start this script under qtbase folder. 
# This script will provide you an out folder for architecture arm64 and x86_64
#
# To create UniversalBinary see and use script 'qt_arch_merge'.
#

rm -rf out

./configure -opensource -confirm-license 

make distclean -j 16
./configure -prefix out/x86_64 -platform macx-clang -release -opensource -confirm-license -nomake tests -nomake examples -no-gif -no-cups
make -j 16
make install


make distclean -j 16
./configure -prefix out/arm64 -platform macx-clang -xplatform macx-clang-arm64 -release -opensource -confirm-license -nomake tests -nomake examples -no-gif -no-cups
make -j 16
make install

