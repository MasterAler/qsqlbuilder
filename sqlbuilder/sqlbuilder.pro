DESTDIR = $$PWD/../bin

QT       += core sql

TARGET = sqlbuilder
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    Config.cpp \
    Query.cpp \
    Selector.cpp \
    Where.cpp

HEADERS += \
    Config.h \
    Query.h \
    Selector.h \
    Where.h

DEFINES *= QT_USE_QSTRINGBUILDER
