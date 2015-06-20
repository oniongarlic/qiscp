#-------------------------------------------------
#
# Project created by QtCreator 2014-08-15T16:37:33
#
#-------------------------------------------------

QT       += core
QT       += network
# Needed for QImage
QT       += gui
QT       += declarative

TARGET = qiscp
CONFIG   -= app_bundle

TEMPLATE = lib


SOURCES += qiscp.cpp \
    iscpmsg.cpp \
    deviceinforparser.cpp \
    artworkimageprovider.cpp \
    artworkparser.cpp

HEADERS += \
    qiscp.h \
    iscpmsg.h \
    deviceinforparser.h \
    artworkimageprovider.h \
    artworkparser.h \
    qiscpinputs.h
