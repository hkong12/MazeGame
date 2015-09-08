#-------------------------------------------------
#
# Project created by QtCreator 2015-09-06T16:38:51
#
#-------------------------------------------------

QT       += core gui \
        network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MazeGame
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    playerthreadclient.cpp \
    playerthreadserver.cpp \
    gamestate.cpp \
    player.cpp \
    gameserver.cpp \
    socketthreadbase.cpp

HEADERS  += mainwindow.h \
    gamestate.h \
    player.h \
    gameserver.h \
    socketthreadbase.h \
    socketthreadserver.h \
    socketthreadclient.h

FORMS    += mainwindow.ui
