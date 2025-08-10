#-------------------------------------------------
#
# Project created by QtCreator 2025-08-10T23:07:40
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = canasta
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    card.cpp \
    player.cpp \
    game.cpp

HEADERS  += mainwindow.h \
    card.h \
    player.h \
    game.h \
    cardsuit.h \
    cardvalue.h

FORMS    += mainwindow.ui

RESOURCES += \
    resources.qrc

DISTFILES +=
