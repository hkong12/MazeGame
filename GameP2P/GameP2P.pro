#-------------------------------------------------
#
# Project created by QtCreator 2015-09-15T15:35:50
#
#-------------------------------------------------

QT       += core gui \
            network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GameP2P
TEMPLATE = app


SOURCES += main.cpp \
           connection.cpp \
           gameui.cpp \
           gameclient.cpp \
           gameserver.cpp \
           gameserverthread.cpp \
           chessboard.cpp \
           gamestate.cpp


HEADERS  += connection.h \
            gameui.h \
            gameclient.h \
            gameserver.h \
            gameserverthread.h \
            chessboard.h \
            gamestate.h

