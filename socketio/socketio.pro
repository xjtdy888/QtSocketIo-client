

TARGET = QtSocketIo

QT = core network websockets
CONFIG += c++11

TEMPLATE = lib

DEFINES += QTSOCKETIO_LIBRARY QT_USE_STRINGBUILDER

#QMAKE_DOCS = $$PWD/doc/qtsocketio.qdocconfig
OTHER_FILES += doc/src/*.qdoc    show .qdoc files in Qt Creator
OTHER_FILES += doc/snippets/*.cpp

PRIVATE_HEADERS +=

SOURCES += qsocketioclient.cpp \
    qtransport.cpp \
    qtransportwebsocket.cpp \
    qtransportxhrpolling.cpp

HEADERS += qcallback.h \
    qsocketio_global.h \
    qsocketioclient.h \
    qtransport.h \
    qtransportwebsocket.h \
    qtransportxhrpolling.h



