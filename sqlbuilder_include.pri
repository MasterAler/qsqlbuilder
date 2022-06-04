QT += sql

SQLBUILDER_DIR = $$PWD/sqlbuilder

HEADERS += \
        $$SQLBUILDER_DIR/Config.h \
        $$SQLBUILDER_DIR/Query.h \
        $$SQLBUILDER_DIR/Where.h \
        $$SQLBUILDER_DIR/Selector.h \
        $$SQLBUILDER_DIR/Inserter.h \
        $$SQLBUILDER_DIR/Deleter.h

INCLUDEPATH *= $$SQLBUILDER_DIR

LIBS += \
     -L$$DESTDIR \
     -lsqlbuilder
