QT += sql

SQLBUILDER_DIR = $$PWD/sqlbuilder

HEADERS += \
        $$SQLBUILDER_DIR/Config.h \
        $$SQLBUILDER_DIR/Query.h

INCLUDEPATH *= $$SQLBUILDER_DIR

LIBS += \
     -L$$DESTDIR \
     -lsqlbuilder
