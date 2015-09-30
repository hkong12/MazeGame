#-------------------------------------------------
#
# Project created by QtCreator 2015-09-14T17:10:05
#
#-------------------------------------------------

QT       += network widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GameClient
TEMPLATE = app

HEADERS = chessboard.h \
          gamestate.h \
          gameclient.h \
          gameui.h \
          connection.h

SOURCES += main.cpp \
           chessboard.cpp \
           gamestate.cpp \
           gameclient.cpp \
           gameui.cpp \
           connection.cpp
