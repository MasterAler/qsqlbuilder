DESTDIR = $$PWD/../bin

QT += core testlib

CONFIG += c++11 qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

SOURCES += \
    tst_builder_test.cpp

include($$PWD/../sqlbuilder_include.pri)

DEFINES *= QT_FORCE_ASSERTS
