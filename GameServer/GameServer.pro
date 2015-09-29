#-------------------------------------------------
#
# Project created by QtCreator 2015-09-14T15:14:17
#
#-------------------------------------------------

QT       += network widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GameServer
TEMPLATE = app

HEADERS       = dialog.h \
                gameserver.h \
                gameserverthread.h \
                connection.h \
                gamestate.h

SOURCES       = dialog.cpp \
                gameserver.cpp \
                gameserverthread.cpp \
                main.cpp \
                connection.cpp \
                gamestate.cpp
