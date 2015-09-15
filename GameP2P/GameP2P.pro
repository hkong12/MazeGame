#-------------------------------------------------
#
# Project created by QtCreator 2015-09-15T15:35:50
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GameP2P
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    peermanager.cpp \
    client.cpp \
    server.cpp \
    connection.cpp

HEADERS  += mainwindow.h \
    peermanager.h \
    client.h \
    server.h \
    connection.h

FORMS    += mainwindow.ui
