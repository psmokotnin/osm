TEMPLATE = app

QT += charts qml quick quickcontrols2 multimedia core opengl
CONFIG += c++1z
SOURCES += src/main.cpp \
    src/generator.cpp \
    src/pinknoise.cpp \
    src/outputdevice.cpp \
    src/whitenoise.cpp \
    src/sinnoise.cpp \
    src/complex.cpp \
    src/audiostack.cpp \
    src/stored.cpp \
    src/fouriertransform.cpp \
    src/deconvolution.cpp \
    src/windowfunction.cpp \
    src/chart/axis.cpp \
    src/chart/painteditem.cpp \
    src/chart/series.cpp \
    src/chart/chart.cpp \
    src/ssemath.cpp \
    src/chart/source.cpp \
    src/inputdevice.cpp \
    src/filter.cpp \
    src/meter.cpp \
    src/measurement.cpp

RESOURCES += qml.qrc

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
    src/generator.h \
    src/pinknoise.h \
    src/outputdevice.h \
    src/whitenoise.h \
    src/sinnoise.h \
    src/sample.h \
    src/complex.h \
    src/audiostack.h \
    src/stored.h \
    src/fouriertransform.h \
    src/ssemath.h \
    src/deconvolution.h \
    src/windowfunction.h \
    src/chart/axis.h \
    src/chart/painteditem.h \
    src/chart/chart.h \
    src/chart/series.h \
    src/chart/type.h \
    src/chart/source.h \
    src/inputdevice.h \
    src/filter.h \
    src/meter.h \
    src/measurement.h

APP_GIT_VERSION = $$system(git --git-dir $$PWD/.git --work-tree $$PWD describe --always --tags)
DEFINES += APP_GIT_VERSION=\\\"$$APP_GIT_VERSION\\\"


FORMS +=

QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -Ofast

DISTFILES += \
    README.md

ICON = icons/osm.icns
