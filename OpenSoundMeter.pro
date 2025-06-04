TEMPLATE = app

QT += qml quick quickcontrols2 core
CONFIG += c++1z
CONFIG += sdk_no_version_check
CONFIG += qmltypes
QML_IMPORT_NAME = OpenSoundMeterModule
QML_IMPORT_MAJOR_VERSION = 1

SOURCES += src/main.cpp \
    src/abstract/data.cpp \
    src/abstract/levelsdata.cpp \
    src/abstract/source.cpp \
    src/audio/client.cpp \
    src/audio/deviceinfo.cpp \
    src/audio/devicemodel.cpp \
    src/audio/format.cpp \
    src/audio/plugin.cpp \
    src/audio/stream.cpp \
    \
    src/chart/crestfactorplot.cpp \
    src/chart/cursorhelper.cpp \
    src/chart/frequencybasedplot.cpp \
    src/chart/frequencybasedserieshelper.cpp \
    src/chart/groupdelayplot.cpp \
    src/chart/levelobject.cpp \
    src/chart/levelplot.cpp \
    src/chart/meterplot.cpp \
    src/chart/nyquistplot.cpp \
    src/chart/palette.cpp \
    src/chart/phasedelayplot.cpp \
    src/chart/seriesesitem.cpp \
    src/chart/spectrogramplot.cpp \
    src/chart/stepplot.cpp \
    src/chart/coherenceplot.cpp \
    src/chart/axis.cpp \
    src/chart/painteditem.cpp \
    src/chart/variablechart.cpp \
    src/chart/plot.cpp \
    src/chart/rtaplot.cpp \
    src/chart/impulseplot.cpp \
    src/chart/phaseplot.cpp \
    src/chart/magnitudeplot.cpp \
    src/chart/xyplot.cpp \
    \
    src/common/autosaver.cpp \
    src/common/recentfilesmodel.cpp \
    src/common/wavfile.cpp \
    src/common/workingfolder.cpp \
    src/common/appearance.cpp \
    src/common/logger.cpp \
    src/common/notifier.cpp \
    src/common/profiler.cpp \
    src/common/settings.cpp \
    \
    src/filesystem/dialog.cpp \
    src/filesystem/dialogPlugin.cpp \
    src/filesystem/plugins/widgetdialogplugin.cpp \
    \
    src/generator/generator.cpp \
    src/generator/generatorthread.cpp \
    src/generator/pinknoise.cpp \
    src/generator/outputdevice.cpp \
    src/generator/whitenoise.cpp \
    src/generator/sinnoise.cpp \
    src/generator/sinsweep.cpp \
    src/generator/brownnoise.cpp \
    src/generator/burstnoise.cpp \
    src/generator/channelmodel.cpp \
    src/generator/mlsplus.cpp \
    src/generator/musicnoise.cpp \
    src/generator/sinburst.cpp \
    src/generator/wav.cpp \
    src/generator/mnoise.cpp \
    \
    src/inputdevice.cpp \
    src/sourcelist.cpp \
    src/targettrace.cpp \
    \
    src/math/bandpass.cpp \
    src/math/biquad.cpp \
    src/math/equalloudnesscontour.cpp \
    src/math/integration_tree.cpp \
    src/math/leq.cpp \
    src/math/lowpassfilter.cpp \
    src/math/notch.cpp \
    src/math/weighting.cpp \
    src/math/meter.cpp \
    src/math/coherence.cpp \
    src/math/averaging.cpp \
    src/math/complex.cpp \
    src/math/fouriertransform.cpp \
    src/math/windowfunction.cpp \
    src/math/deconvolution.cpp \
    \
    src/meta/metabase.cpp \
    src/meta/metafilter.cpp \
    src/meta/metameasurement.cpp \
    src/meta/metastored.cpp \
    src/meta/metawindowing.cpp \
    \
    src/model/metertablemodel.cpp \
    src/model/sourcemodel.cpp \
    \
    src/remote/generatorremote.cpp \
    src/remote/item.cpp \
    src/remote/items/groupitem.cpp \
    src/remote/items/measurementitem.cpp \
    src/remote/items/storeditem.cpp \
    src/remote/network.cpp \
    src/remote/remoteclient.cpp \
    src/remote/server.cpp \
    src/remote/tcpreciever.cpp \
    \
    src/shared/source_shared.cpp \
    src/source/filtersource.cpp \
    src/source/group.cpp \
    src/source/sourcewindowing.cpp \
    src/source/standardline.cpp \
    src/source/union.cpp \
    src/source/stored.cpp \
    src/source/measurement.cpp \
    \

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

INCLUDEPATH += \
    src \
    src/meta \
    src/chart \
    src/common \
    src/remote \
    src/generator \
    src/source \
    src/model

HEADERS += \
    src/abstract/data.h \
    src/abstract/levelsdata.h \
    src/abstract/source.h \
    src/audio/client.h \
    src/audio/deviceinfo.h \
    src/audio/devicemodel.h \
    src/audio/format.h \
    src/audio/plugin.h \
    src/audio/stream.h \
    src/chart/crestfactorplot.h \
    src/chart/cursorhelper.h \
    src/chart/frequencybasedplot.h \
    src/chart/frequencybasedserieshelper.h \
    src/chart/groupdelayplot.h \
    src/chart/levelobject.h \
    src/chart/levelplot.h \
    src/chart/meterplot.h \
    src/chart/nyquistplot.h \
    src/chart/palette.h \
    src/chart/phasedelayplot.h \
    src/chart/plotpadding.h \
    src/chart/seriesesitem.h \
    src/chart/seriesitem.h \
    src/chart/spectrogramplot.h \
    src/chart/stepplot.h \
    src/chart/coherenceplot.h \
    src/common/atomic.h \
    src/common/autosaver.h \
    src/common/recentfilesmodel.h \
    src/common/wavfile.h \
    src/common/workingfolder.h \
    src/filesystem/dialog.h \
    src/filesystem/dialogPlugin.h \
    src/filesystem/plugins/widgetdialogplugin.h \
    src/source/filtersource.h \
    src/generator/brownnoise.h \
    src/generator/burstnoise.h \
    src/generator/channelmodel.h \
    src/generator/mlsplus.h \
    src/generator/musicnoise.h \
    src/generator/sinburst.h \
    src/generator/wav.h \
    src/inputdevice.h \
    src/common/appearance.h \
    src/common/logger.h \
    src/generator/mnoise.h \
    src/common/notifier.h \
    src/common/profiler.h \
    src/math/bandpass.h \
    src/math/bessellpf.h \
    src/math/biquad.h \
    src/math/equalloudnesscontour.h \
    src/math/filter.h \
    src/math/integration_tree.h \
    src/math/leq.h \
    src/math/lowpassfilter.h \
    src/math/notch.h \
    src/math/weighting.h \
    src/meta/metabase.h \
    src/meta/metafilter.h \
    src/meta/metagroup.h \
    src/meta/metameasurement.h \
    src/meta/metastored.h \
    src/meta/metawindowing.h \
    src/remote/generatorremote.h \
    src/remote/item.h \
    src/remote/items/groupitem.h \
    src/remote/items/measurementitem.h \
    src/remote/items/storeditem.h \
    src/remote/network.h \
    src/remote/remoteclient.h \
    src/remote/server.h \
    src/remote/tcpreciever.h \
    src/source/group.h \
    src/source/sourcewindowing.h \
    src/shared/source_shared.h \
    src/sourcelist.h \
    src/model/metertablemodel.h \
    src/model/sourcemodel.h \
    src/source/standardline.h \
    src/targettrace.h \
    src/source/union.h \
    src/generator/generator.h \
    src/generator/generatorthread.h \
    src/generator/pinknoise.h \
    src/generator/outputdevice.h \
    src/generator/whitenoise.h \
    src/generator/sinnoise.h \
    src/generator/sinsweep.h \
    src/generator/sample.h \
    src/source/stored.h \
    src/chart/axis.h \
    src/chart/painteditem.h \
    src/chart/type.h \
    src/source/measurement.h \
    src/common/settings.h \
    src/chart/variablechart.h \
    src/chart/plot.h \
    src/chart/rtaplot.h \
    src/chart/impulseplot.h \
    src/chart/phaseplot.h \
    src/chart/magnitudeplot.h \
    src/chart/xyplot.h \
    src/math/meter.h \
    src/math/coherence.h \
    src/math/averaging.h \
    src/math/complex.h \
    src/math/fouriertransform.h \
    src/math/deconvolution.h \
    src/math/windowfunction.h \
    src/math/deconvolution.h \
    src/container/fifo.h \
    src/container/circular.h \
    src/container/array.h

#math
equals(QT_ARCH, "arm64") {
    message("Building for: ARM64")
    HEADERS += src/armmath.h
} else {
    message("Building for: x64")
    HEADERS += src/math/ssemath.h
    SOURCES += src/math/ssemath.cpp
}

#dialogs
ios: {
    QMAKE_IOS_DEPLOYMENT_TARGET = 12.0
    QMAKE_APPLE_TARGETED_DEVICE_FAMILY = 1,2 #iPhone,iPad
    QMAKE_IOS_DEVICE_ARCHS = "arm64"
    DEFINES += SUPPORT_64_BIT_IOS

    HEADERS += src/filesystem/plugins/iosdialogplugin.h
    SOURCES += src/filesystem/plugins/iosdialogplugin.mm
}

APP_GIT_VERSION = $$system(git --git-dir $$_PRO_FILE_PWD_/.git --work-tree $$_PRO_FILE_PWD_ describe --tags $$system(git --git-dir $$_PRO_FILE_PWD_/.git --work-tree $$_PRO_FILE_PWD_ rev-list --tags --max-count=1))
DEFINES += APP_GIT_VERSION=\\\"$$APP_GIT_VERSION\\\"

#logger
DEFINES += QT_MESSAGELOGCONTEXT

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
        src/chart/metal/phasedelayseriesnode.h \
        src/chart/metal/impulseseriesnode.h \
        src/chart/metal/magnitudeseriesnode.h \
        src/chart/metal/phaseseriesnode.h \
        src/chart/metal/seriesnode.h \
        src/chart/metal/spectrogramseriesnode.h \
        src/chart/metal/crestfactorseriesnode.h \
        src/chart/metal/levelseriesnode.h \
        src/chart/metal/nyquistseriesnode.h \
        src/chart/metal/stepseriesnode.h \
        src/chart/metal/xyseriesnode.h \
        src/chart/metal/rtaseriesnode.h \
        src/chart/metal/seriesitem.h \

    SOURCES += \
        src/chart/metal/rtaseriesnode.mm \
        src/chart/metal/coherenceseriesnode.mm \
        src/chart/metal/groupdelayseriesnode.mm \
        src/chart/metal/phasedelayseriesnode.mm \
        src/chart/metal/impulseseriesnode.mm \
        src/chart/metal/magnitudeseriesnode.mm \
        src/chart/metal/phaseseriesnode.mm \
        src/chart/metal/seriesnode.mm \
        src/chart/metal/spectrogramseriesnode.mm \
        src/chart/metal/crestfactorseriesnode.mm \
        src/chart/metal/levelseriesnode.mm \
        src/chart/metal/nyquistseriesnode.mm \
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
        METAL_TARGET = "-mmacosx-version-min=10.13"
    }
    ios {
        METAL_SDK = "iphoneos"
        METAL_STD = "ios-metal1.0"
        METAL_TARGET = "-mios-version-min=12.0"
    }

    metal_command = echo "build metal"
    for (METAL_SOURCE, METAL_SOURCES) {
        AIR_FILE = $$basename(METAL_SOURCE)
        AIR_FILE = $$OUT_PWD/$$replace(AIR_FILE, .metal, .air)
        AIR_FILES += $$AIR_FILE
        metal_command += && xcrun -sdk $$METAL_SDK metal $$METAL_TARGET -std=$$METAL_STD -c $$PWD/$$METAL_SOURCE -o $$AIR_FILE
    }
    metal_command += && xcrun -sdk $$METAL_SDK metallib $$AIR_FILES -o $$OUT_PWD/lib.metallib

    metal_target.target = metal
    metal_target.commands = $$metal_command
    QMAKE_EXTRA_TARGETS += metal_target
    PRE_TARGETDEPS += metal

    metal_install.target = metal
    metal_install.files += $$OUT_PWD/lib.metallib
    metal_install.path = Contents/Resources
    metal_install.CONFIG += no_check_exist

    #runs before build
    #QMAKE_BUNDLE_DATA += metal_install

    #QMAKE_POST_LINK and PRE_TARGETDEPS - takes no effect on Xcode projects
    QMAKE_POST_LINK = cp $$OUT_PWD/lib.metallib $$OUT_PWD/OpenSoundMeter.app/Contents/Resources/lib.metallib

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
        src/chart/opengl/phasedelayseriesrenderer.h \
        src/chart/opengl/spectrogramseriesrenderer.h \
        src/chart/opengl/crestfactorseriesrenderer.h \
        src/chart/opengl/nyquistseriesrenderer.h \
        src/chart/opengl/levelseriesrenderer.h \
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
        src/chart/opengl/phasedelayseriesrenderer.cpp \
        src/chart/opengl/spectrogramseriesrenderer.cpp \
        src/chart/opengl/crestfactorseriesrenderer.cpp \
        src/chart/opengl/nyquistseriesrenderer.cpp \
        src/chart/opengl/levelseriesrenderer.cpp \
        src/chart/opengl/stepseriesrenderer.cpp
}

#TODO: finish this
#unix:!macx:!ios {
#    #Add Glibc wrapper for older platforms
#    #./LibcWrapGenerator --target 2.14 --libdir /lib --output libcwrap.h

##    CONFIG += cmdline precompile_header
##    PRECOMPILED_HEADER = ../libcwrap.h
#    QMAKE_CXXFLAGS += -U_FORTIFY_SOURCE -include ../libcwrap.h
#    HEADERS += ../libcwrap.h
#}

# Special rules for deployment on Linux for AppImage
unix:!macx:!ios:CONFIG(release, debug|release) {
    QMAKE_POST_LINK += $$QMAKE_COPY $$PWD/OpenSoundMeter.desktop $$OUT_PWD/OpenSoundMeter_\\"$$APP_GIT_VERSION\\".desktop
    QMAKE_POST_LINK += && $$QMAKE_COPY $$PWD/icons/white.png $$OUT_PWD
    QMAKE_POST_LINK += && $$QMAKE_COPY $$PWD/linuxdeployosm.sh $$OUT_PWD
    QMAKE_POST_LINK += && chmod u+x $$OUT_PWD/linuxdeployosm.sh && $$OUT_PWD/linuxdeployosm.sh $$APP_GIT_VERSION $$PWD $$[QT_INSTALL_BINS]

    QMAKE_CXXFLAGS_RELEASE += -fno-fast-math
}

!isEqual(QT_MAJOR_VERSION, 5):lessThan(QT_MINOR_VERSION, 15) {
    error(OpenSoundMeter $$APP_GIT_VERSION  requires Qt 5.15 or newer but Qt $$[QT_VERSION] was detected.)
}

FORMS +=

#win32:QMAKE_CXXFLAGS += -m64 -msse2
#QMAKE_CXXFLAGS_RELEASE -= -O2
#QMAKE_CXXFLAGS_RELEASE += -Ofast

QMAKE_INFO_PLIST = $$PWD/Info.plist
macx {
    APP_ENTITLEMENTS.files = info.entitlements
    APP_ENTITLEMENTS.path = Contents/Resources
    QMAKE_BUNDLE_DATA += APP_ENTITLEMENTS
}

macx {
    isEqual(GRAPH, "METAL") {
        QMAKE_POST_LINK += &&
    }

    QMAKE_POST_LINK += plutil -replace NSMicrophoneUsageDescription -string \"Audio measurement\" $${TARGET}.app/Contents/Info.plist
}
ios {
    QMAKE_POST_LINK += plutil -replace NSMicrophoneUsageDescription -string \"Audio measurement.\" Info.plist
    ios_icon.files = $$files($$PWD/icons/ios/*.png)
    QMAKE_BUNDLE_DATA += ios_icon
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


pvs_studio.target = pvs
pvs_studio.output = true
pvs_studio.args = --dump-files
pvs_studio.cxxflags = -std=c++1z
pvs_studio.sources = $${SOURCES}
include(PVS-Studio.pri)
