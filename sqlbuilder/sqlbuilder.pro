DESTDIR = $$PWD/../bin

QT       += core sql

TARGET = sqlbuilder
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    Config.cpp \
    Query.cpp \
    Selector.cpp

HEADERS += \
    Config.h \
    Query.h \
    Selector.h

DEFINES *= QT_USE_QSTRINGBUILDER
