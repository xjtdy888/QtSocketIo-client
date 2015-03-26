#-------------------------------------------------
#
# Project created by QtCreator 2015-03-24T12:17:17
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = example
TEMPLATE = app

CONFIG += c++11


SOURCES += main.cpp\
        mainwindow.cpp \
    clienttest.cpp

HEADERS  += mainwindow.h \
    clienttest.h

FORMS    += mainwindow.ui




win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../socketio/release/ -lQtSocketIo
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../socketio/debug/ -lQtSocketIo
else:unix: LIBS += -L$$OUT_PWD/../socketio/ -lQtSocketIo

INCLUDEPATH += $$PWD/../socketio
DEPENDPATH += $$PWD/../socketio
