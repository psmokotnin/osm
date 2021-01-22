TEMPLATE = app

QT += qml quick quickcontrols2 multimedia core opengl
CONFIG += c++1z
SOURCES += src/main.cpp \
    src/chart/frequencybasedplot.cpp \
    src/chart/groupdelayplot.cpp \
    src/chart/groupdelayseriesrenderer.cpp \
    src/chart/palette.cpp \
    src/chart/spectrogramplot.cpp \
    src/chart/spectrogramseriesrenderer.cpp \
    src/chart/stepplot.cpp \
    src/chart/stepseriesrenderer.cpp \
    src/elc.cpp \
    src/generator.cpp \
    src/pinknoise.cpp \
    src/outputdevice.cpp \
    src/sinsweep.cpp \
    src/sourcelist.cpp \
    src/sourcemodel.cpp \
    src/union.cpp \
    src/whitenoise.cpp \
    src/sinnoise.cpp \
    src/complex.cpp \
    src/stored.cpp \
    src/fouriertransform.cpp \
    src/deconvolution.cpp \
    src/windowfunction.cpp \
    src/chart/axis.cpp \
    src/chart/painteditem.cpp \
    src/ssemath.cpp \
    src/chart/source.cpp \
    src/meter.cpp \
    src/measurement.cpp \
    src/chart/variablechart.cpp \
    src/chart/plot.cpp \
    src/chart/rtaplot.cpp \
    src/chart/seriesfbo.cpp \
    src/chart/rtaseriesrenderer.cpp \
    src/chart/seriesrenderer.cpp \
    src/chart/impulseplot.cpp \
    src/chart/impulseseriesrenderer.cpp \
    src/chart/phaseplot.cpp \
    src/chart/phaseseriesrenderer.cpp \
    src/chart/magnitudeplot.cpp \
    src/chart/magnitudeseriesrenderer.cpp \
    src/chart/frequencybasedseriesrenderer.cpp \
    src/chart/xyplot.cpp \
    src/generatorthread.cpp \
    src/measurementaudiothread.cpp \
    src/averaging.cpp \
    src/chart/xyseriesrenderer.cpp \
    src/coherence.cpp \
    src/chart/coherenceplot.cpp \
    src/chart/coherenceseriesrenderer.cpp \
    src/settings.cpp

RESOURCES += qml.qrc \
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

HEADERS += \
    src/chart/frequencybasedplot.h \
    src/chart/groupdelayplot.h \
    src/chart/groupdelayseriesrenderer.h \
    src/chart/palette.h \
    src/chart/spectrogramplot.h \
    src/chart/spectrogramseriesrenderer.h \
    src/chart/stepplot.h \
    src/chart/stepseriesrenderer.h \
    src/elc.h \
    src/generator.h \
    src/pinknoise.h \
    src/outputdevice.h \
    src/sinsweep.h \
    src/sourcelist.h \
    src/sourcemodel.h \
    src/union.h \
    src/whitenoise.h \
    src/sinnoise.h \
    src/sample.h \
    src/complex.h \
    src/stored.h \
    src/fouriertransform.h \
    src/ssemath.h \
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
    src/chart/seriesfbo.h \
    src/chart/rtaseriesrenderer.h \
    src/chart/seriesrenderer.h \
    src/chart/impulseplot.h \
    src/chart/impulseseriesrenderer.h \
    src/chart/phaseplot.h \
    src/chart/phaseseriesrenderer.h \
    src/chart/magnitudeplot.h \
    src/chart/magnitudeseriesrenderer.h \
    src/chart/frequencybasedseriesrenderer.h \
    src/chart/xyplot.h \
    src/generatorthread.h \
    src/measurementaudiothread.h \
    src/averaging.h \
    src/chart/xyseriesrenderer.h \
    src/container/fifo.h \
    src/container/array.h \
    src/coherence.h \
    src/chart/coherenceplot.h \
    src/chart/coherenceseriesrenderer.h \
    src/settings.h

APP_GIT_VERSION = $$system(git --git-dir $$_PRO_FILE_PWD_/.git --work-tree $$_PRO_FILE_PWD_ describe --tags $$system(git --git-dir $$_PRO_FILE_PWD_/.git --work-tree $$_PRO_FILE_PWD_ rev-list --tags --max-count=1))
DEFINES += APP_GIT_VERSION=\\\"$$APP_GIT_VERSION\\\"

# Special rules for deployment on Linux for AppImage

unix:!macx:CONFIG(release, debug|release) {
    QMAKE_POST_LINK += $$QMAKE_COPY $$PWD/OpenSoundMeter.desktop $$OUT_PWD/OpenSoundMeter_\\"$$APP_GIT_VERSION\\".desktop
    QMAKE_POST_LINK +=&& $$QMAKE_COPY $$PWD/icons/white.png $$OUT_PWD
    QMAKE_POST_LINK +=&& $$QMAKE_COPY $$PWD/linuxdeployosm.sh $$OUT_PWD
    QMAKE_POST_LINK +=  && chmod u+x $$OUT_PWD/linuxdeployosm.sh && $$OUT_PWD/linuxdeployosm.sh $$APP_GIT_VERSION $$PWD $$[QT_INSTALL_BINS]
}

!isEqual(QT_MAJOR_VERSION, 5):lessThan(QT_MINOR_VERSION, 15) {
    error(OpenSoundMeter $$APP_GIT_VERSION  requires Qt 5.15 or newer but Qt $$[QT_VERSION] was detected.)
}

FORMS +=

win32:QMAKE_CXXFLAGS += -m64 -msse2
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -Ofast

!ios {
    QMAKE_INFO_PLIST = $$PWD/Info.plist
}

DISTFILES += \
    OpenSoundMeter.desktop \
    README.md \
    future.tasks \
    linuxdeployosm.sh \
    list.tasks \
    shaders/coherence.frag \
    shaders/fragment.frag \
    shaders/interpolateColor.frag \
    shaders/logx.vert \
    shaders/phase.geom \
    shaders/color.frag \
    shaders/magnitude.frag \
    shaders/pos.vert \
    shaders/spectrogram.vert \
    style.astylerc

ICON = icons/white.icns
win32:RC_ICONS = icons/white.ico
