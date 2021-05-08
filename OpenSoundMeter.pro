TEMPLATE = app

QT += qml quick quickcontrols2 core
CONFIG += c++1z
CONFIG+=sdk_no_version_check
QML_IMPORT_NAME = OpenSoundMeter
QML_IMPORT_MAJOR_VERSION = 1

SOURCES += src/main.cpp \
    src/appearance.cpp \
    src/audio/client.cpp \
    src/audio/deviceinfo.cpp \
    src/audio/devicemodel.cpp \
    src/audio/format.cpp \
    src/audio/plugin.cpp \
    src/audio/stream.cpp \
    src/chart/cursorhelper.cpp \
    src/chart/frequencybasedplot.cpp \
    src/chart/frequencybasedserieshelper.cpp \
    src/chart/groupdelayplot.cpp \
    src/chart/palette.cpp \
    src/chart/spectrogramplot.cpp \
    src/chart/stepplot.cpp \
    src/elc.cpp \
    src/filesystem/dialog.cpp \
    src/filesystem/dialogPlugin.cpp \
    src/filesystem/plugins/widgetdialogplugin.cpp \
    src/generator.cpp \
    src/inputdevice.cpp \
    src/mnoise.cpp \
    src/pinknoise.cpp \
    src/outputdevice.cpp \
    src/profiler.cpp \
    src/sinsweep.cpp \
    src/sourcelist.cpp \
    src/sourcemodel.cpp \
    src/union.cpp \
    src/wavfile.cpp \
    src/whitenoise.cpp \
    src/sinnoise.cpp \
    src/complex.cpp \
    src/stored.cpp \
    src/fouriertransform.cpp \
    src/deconvolution.cpp \
    src/windowfunction.cpp \
    src/chart/axis.cpp \
    src/chart/painteditem.cpp \
    src/chart/source.cpp \
    src/meter.cpp \
    src/measurement.cpp \
    src/chart/variablechart.cpp \
    src/chart/plot.cpp \
    src/chart/rtaplot.cpp \
    src/chart/impulseplot.cpp \
    src/chart/phaseplot.cpp \
    src/chart/magnitudeplot.cpp \
    src/chart/xyplot.cpp \
    src/generatorthread.cpp \
    src/averaging.cpp \
    src/coherence.cpp \
    src/chart/coherenceplot.cpp \
    src/settings.cpp

RESOURCES += qml.qrc \
    audio/noises.qrc \
    fonts/fonts.qrc \
    images.qrc \
    shaders/shaders.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH = qml

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

INCLUDEPATH += src
HEADERS += \
    src/appearance.h \
    src/audio/client.h \
    src/audio/deviceinfo.h \
    src/audio/devicemodel.h \
    src/audio/format.h \
    src/audio/plugin.h \
    src/audio/stream.h \
    src/chart/cursorhelper.h \
    src/chart/frequencybasedplot.h \
    src/chart/frequencybasedserieshelper.h \
    src/chart/groupdelayplot.h \
    src/chart/palette.h \
    src/chart/spectrogramplot.h \
    src/chart/stepplot.h \
    src/elc.h \
    src/filesystem/dialog.h \
    src/filesystem/dialogPlugin.h \
    src/filesystem/plugins/widgetdialogplugin.h \
    src/generator.h \
    src/inputdevice.h \
    src/mnoise.h \
    src/pinknoise.h \
    src/outputdevice.h \
    src/profiler.h \
    src/sinsweep.h \
    src/sourcelist.h \
    src/sourcemodel.h \
    src/union.h \
    src/wavfile.h \
    src/whitenoise.h \
    src/sinnoise.h \
    src/sample.h \
    src/complex.h \
    src/stored.h \
    src/fouriertransform.h \
    src/deconvolution.h \
    src/windowfunction.h \
    src/chart/axis.h \
    src/chart/painteditem.h \
    src/chart/type.h \
    src/chart/source.h \
    src/filter.h \
    src/meter.h \
    src/measurement.h \
    src/chart/variablechart.h \
    src/chart/plot.h \
    src/chart/rtaplot.h \
    src/chart/impulseplot.h \
    src/chart/phaseplot.h \
    src/chart/magnitudeplot.h \
    src/chart/xyplot.h \
    src/generatorthread.h \
    src/averaging.h \
    src/container/fifo.h \
    src/container/array.h \
    src/coherence.h \
    src/chart/coherenceplot.h \
    src/settings.h

#dialogs
ios: {
    QMAKE_IOS_DEPLOYMENT_TARGET = 12.0
    QMAKE_APPLE_TARGETED_DEVICE_FAMILY = 2 #iPad
    QMAKE_IOS_DEVICE_ARCHS = "arm64"
    DEFINES += SUPPORT_64_BIT_IOS

    HEADERS += src/filesystem/plugins/iosdialogplugin.h
    SOURCES += src/filesystem/plugins/iosdialogplugin.mm
    HEADERS += src/armmath.h
} else {
    HEADERS += src/ssemath.h
    SOURCES += src/ssemath.cpp
}

APP_GIT_VERSION = $$system(git --git-dir $$_PRO_FILE_PWD_/.git --work-tree $$_PRO_FILE_PWD_ describe --tags $$system(git --git-dir $$_PRO_FILE_PWD_/.git --work-tree $$_PRO_FILE_PWD_ rev-list --tags --max-count=1))
DEFINES += APP_GIT_VERSION=\\\"$$APP_GIT_VERSION\\\"

#audio plugins:
macx {
    HEADERS += \
        src/audio/plugins/coreaudio.h

    SOURCES += \
        src/audio/plugins/coreaudio.cpp

    LIBS += \
        -framework CoreAudio \
        -framework AudioToolbox
}

ios {
    HEADERS += \
        src/audio/plugins/audiosession.h

    SOURCES += \
    src/audio/plugins/audiosession.mm

    LIBS += \
        -framework AVFoundation
}

win32 {
    HEADERS += \
        src/audio/plugins/wasapi.h

    SOURCES += \
        src/audio/plugins/wasapi.cpp

    LIBS += ole32.lib

    exists($$PWD/../asiosdk/*) {
        DEFINES += USE_ASIO
        message("Add ASIO driver Support")

        INCLUDEPATH += \
            $$PWD/../asiosdk/common \
            $$PWD/../asiosdk/host \
            $$PWD/../asiosdk/host/pc

        HEADERS += \
            src/audio/plugins/asioplugin.h \
            $$PWD/../asiosdk/common/asio.h \
            $$PWD/../asiosdk/common/asiosys.h \
            $$PWD/../asiosdk/host/asiodrivers.h \
            $$PWD/../asiosdk/host/pc/asiolist.h

        SOURCES += \
            src/audio/plugins/asioplugin.cpp \
            $$PWD/../asiosdk/common/asio.cpp \
            $$PWD/../asiosdk/host/asiodrivers.cpp \
            $$PWD/../asiosdk/host/pc/asiolist.cpp

        LIBS += Advapi32.lib
    }
}

unix:!macx:!ios {
    HEADERS += \
        src/audio/plugins/alsa.h

    SOURCES += \
        src/audio/plugins/alsa.cpp

    LIBS += -lasound
}

GRAPH = $$(GRAPH_BACKEND)
isEmpty(GRAPH) {
    GRAPH = "OPENGL"
}

isEqual(GRAPH, "METAL") {
    message("use metal")

    QT += widgets
    DEFINES += GRAPH_METAL

    INCLUDEPATH += src/chart/metal

    HEADERS += \
        src/chart/metal/coherenceseriesnode.h \
        src/chart/metal/groupdelayseriesnode.h \
        src/chart/metal/impulseseriesnode.h \
        src/chart/metal/magnitudeseriesnode.h \
        src/chart/metal/phaseseriesnode.h \
        src/chart/metal/seriesnode.h \
        src/chart/metal/spectrogramseriesnode.h \
        src/chart/metal/stepseriesnode.h \
        src/chart/metal/xyseriesnode.h \
        src/chart/metal/rtaseriesnode.h \
        src/chart/metal/seriesitem.h \

    SOURCES += \
        src/chart/metal/rtaseriesnode.mm \
        src/chart/metal/coherenceseriesnode.mm \
        src/chart/metal/groupdelayseriesnode.mm \
        src/chart/metal/impulseseriesnode.mm \
        src/chart/metal/magnitudeseriesnode.mm \
        src/chart/metal/phaseseriesnode.mm \
        src/chart/metal/seriesnode.mm \
        src/chart/metal/spectrogramseriesnode.mm \
        src/chart/metal/stepseriesnode.mm \
        src/chart/metal/xyseriesnode.mm \
        src/chart/metal/seriesitem.cpp \
        src/chart/metal/plotseriescreator.cpp \

    METAL_SOURCES += \
        src/chart/metal/shaders.metal \

    LIBS += -framework Metal

    macx {
        METAL_SDK = "macosx"
        METAL_STD = "macos-metal1.0"
    }
    ios {
        METAL_SDK = "iphoneos"
        METAL_STD = "ios-metal1.0"
    }
    ios {
        SCRUN_SDK = "iphoneos"
    }

    metal_command = echo "build metal"
    for (METAL_SOURCE, METAL_SOURCES) {
        AIR_FILE = $$basename(METAL_SOURCE)
        AIR_FILE = $$OUT_PWD/$$replace(AIR_FILE, .metal, .air)
        AIR_FILES += $$AIR_FILE
        metal_command += && xcrun -sdk $$METAL_SDK metal -std=$$METAL_STD -c $$PWD/$$METAL_SOURCE -o $$AIR_FILE
    }
    metal_command += && xcrun -sdk $$METAL_SDK metallib $$AIR_FILES -o $$OUT_PWD/lib.metallib

    metal_target.target = metal
    metal_target.commands = $$metal_command
    QMAKE_EXTRA_TARGETS += metal_target
    PRE_TARGETDEPS += metal

    #QMAKE_POST_LINK and PRE_TARGETDEPS - takes no effect on Xcode projects
    macx {
        QMAKE_POST_LINK = cp $$OUT_PWD/lib.metallib $$OUT_PWD/OpenSoundMeter.app/Contents/Resources/lib.metallib
    }
    ios {
        metal_install.target = metal
        metal_install.files += $$OUT_PWD/lib.metallib
        metal_install.CONFIG += no_check_exist
        QMAKE_BUNDLE_DATA += metal_install
    }

    QMAKE_CLEAN += *.air
    QMAKE_CLEAN += *.metallib
}

isEqual(GRAPH, "OPENGL") {
    message("use OpenGL")
    QT += opengl
    DEFINES += GRAPH_OPENGL

    INCLUDEPATH += src/chart/opengl

    HEADERS += \
        src/chart/opengl/seriesfbo.h \
        src/chart/opengl/rtaseriesrenderer.h \
        src/chart/opengl/seriesrenderer.h \
        src/chart/opengl/impulseseriesrenderer.h \
        src/chart/opengl/phaseseriesrenderer.h \
        src/chart/opengl/magnitudeseriesrenderer.h \
        src/chart/opengl/frequencybasedseriesrenderer.h \
        src/chart/opengl/xyseriesrenderer.h \
        src/chart/opengl/coherenceseriesrenderer.h \
        src/chart/opengl/groupdelayseriesrenderer.h \
        src/chart/opengl/spectrogramseriesrenderer.h \
        src/chart/opengl/stepseriesrenderer.h

    SOURCES += \
        src/chart/opengl/plotseriescreator.cpp \
        src/chart/opengl/seriesfbo.cpp \
        src/chart/opengl/rtaseriesrenderer.cpp \
        src/chart/opengl/seriesrenderer.cpp \
        src/chart/opengl/impulseseriesrenderer.cpp \
        src/chart/opengl/phaseseriesrenderer.cpp \
        src/chart/opengl/magnitudeseriesrenderer.cpp \
        src/chart/opengl/frequencybasedseriesrenderer.cpp \
        src/chart/opengl/xyseriesrenderer.cpp \
        src/chart/opengl/coherenceseriesrenderer.cpp \
        src/chart/opengl/groupdelayseriesrenderer.cpp \
        src/chart/opengl/spectrogramseriesrenderer.cpp \
        src/chart/opengl/stepseriesrenderer.cpp
}


# Special rules for deployment on Linux for AppImage
unix:!macx:!ios:CONFIG(release, debug|release) {
    QMAKE_POST_LINK += $$QMAKE_COPY $$PWD/OpenSoundMeter.desktop $$OUT_PWD/OpenSoundMeter_\\"$$APP_GIT_VERSION\\".desktop
    QMAKE_POST_LINK +=&& $$QMAKE_COPY $$PWD/icons/white.png $$OUT_PWD
    QMAKE_POST_LINK +=&& $$QMAKE_COPY $$PWD/linuxdeployosm.sh $$OUT_PWD
    QMAKE_POST_LINK +=  && chmod u+x $$OUT_PWD/linuxdeployosm.sh && $$OUT_PWD/linuxdeployosm.sh $$APP_GIT_VERSION $$PWD $$[QT_INSTALL_BINS]
}

!isEqual(QT_MAJOR_VERSION, 5):lessThan(QT_MINOR_VERSION, 15) {
    error(OpenSoundMeter $$APP_GIT_VERSION  requires Qt 5.15 or newer but Qt $$[QT_VERSION] was detected.)
}

FORMS +=

#win32:QMAKE_CXXFLAGS += -m64 -msse2
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -Ofast

macx {
    QMAKE_INFO_PLIST = $$PWD/Info.plist
    APP_ENTITLEMENTS.files = info.entitlements
    APP_ENTITLEMENTS.path = Contents/Resources
    QMAKE_BUNDLE_DATA += APP_ENTITLEMENTS
}

ios {
    QMAKE_INFO_PLIST = $$PWD/ios.plist
    #QMAKE_POST_LINK += plutil -replace NSMicrophoneUsageDescription -string \"Audio measurement.\" Info.plist

    ios_icon.files = $$files($$PWD/icons/ios/*.png)
    QMAKE_BUNDLE_DATA += ios_icon

    app_launch_images.files = $$PWD/icons/ios/launch/Launch.xib $$files($$PWD/icons/ios/launch/LaunchImage*.png)
    QMAKE_BUNDLE_DATA += app_launch_images
}

DISTFILES += \
    OpenSoundMeter.desktop \
    README.md \
    future.tasks \
    linuxdeployosm.sh \
    list.tasks \
    shaders/coherence.frag \
    shaders/coherence.geom \
    shaders/coherence.vert \
    shaders/fragment.frag \
    shaders/groupdelay.frag \
    shaders/groupdelay.geom \
    shaders/groupdelay.vert \
    shaders/line.geom \
    shaders/logx.vert \
    shaders/magnitude.geom \
    shaders/magnitude.vert \
    shaders/phase.geom \
    shaders/phase.geom \
    shaders/color.frag \
    shaders/magnitude.frag \
    shaders/phase.vert \
    shaders/pos.vert \
    shaders/spectrogram.frag \
    shaders/spectrogram.vert \
    style.astylerc

ICON = icons/white.icns
win32:RC_ICONS = icons/white.ico
