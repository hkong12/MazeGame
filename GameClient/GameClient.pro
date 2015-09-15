#-------------------------------------------------
#
# Project created by QtCreator 2015-09-14T17:10:05
#
#-------------------------------------------------

QT       += network widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GameClient
TEMPLATE = app

HEADERS = client.h \
    gamestate.h

SOURCES += client.cpp \
            main.cpp \
    gamestate.cpp
