#!/bin/bash

export VERSION=$1
SOURCE_DIR=$2
QT_BINS=$3

make clean -j1
OSM_DESKTOP=OpenSoundMeter_$VERSION.desktop
LINUXDEPLOYQT=linuxdeployqt-6-x86_64.AppImage

if test -f "$LINUXDEPLOYQT"; then
  echo "$LINUXDEPLOYQT exist"
else
   wget -c -nv "https://github.com/probonopd/linuxdeployqt/releases/download/6/linuxdeployqt-6-x86_64.AppImage"
   chmod a+x linuxdeployqt-6-x86_64.AppImage
fi
    ./linuxdeployqt-6-x86_64.AppImage $OSM_DESKTOP -appimage -unsupported-allow-new-glibc -qmake=$QT_BINS/qmake -qmldir=$SOURCE_DIR/qml
exit 0
