#-------------------------------------------------
#
# Project created by QtCreator 2019-10-18T14:02:23
#
#-------------------------------------------------

QT       -= gui
QT       += core network

TARGET = qvk
TEMPLATE = lib
CONFIG += staticlib
QMAKE_CXXFLAGS += -std=c++1y

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        src/api.cpp \
    src/attachment.cpp \
    src/messages.cpp

HEADERS += \
        src/api.h \
    src/attachment.h \
    src/messages.h
unix {
    target.path = /usr/lib
    INSTALLS += target
}
