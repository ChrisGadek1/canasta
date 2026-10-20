#-------------------------------------------------
#
# Project created by QtCreator 2025-08-10T23:07:40
#
#-------------------------------------------------

QT       += core gui \
            opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = canasta
TEMPLATE = app

LIBS += -lopengl32


SOURCES += main.cpp\
        mainwindow.cpp \
    card.cpp \
    player.cpp \
    game.cpp \
    mainglwidget.cpp \
    cardsdeck.cpp \
    position.cpp \
    meld.cpp

HEADERS  += mainwindow.h \
    card.h \
    player.h \
    game.h \
    cardsuit.h \
    cardvalue.h \
    mainglwidget.h \
    cardsdeck.h \
    cardsdecktype.h \
    position.h \
    meld.h \
    roundresult.h \
    teamroundscore.h

FORMS    += mainwindow.ui

RESOURCES += \
    resources.qrc

DISTFILES += \
    results.json
