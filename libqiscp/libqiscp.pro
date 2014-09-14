#-------------------------------------------------
#
# Project created by QtCreator 2014-08-15T16:37:33
#
#-------------------------------------------------

QT       += core
QT       += network
QT       -= gui

TARGET = qiscp
CONFIG   -= app_bundle

TEMPLATE = lib


SOURCES += qiscp.cpp \
    iscpmsg.cpp \
    deviceinforparser.cpp

HEADERS += \
    qiscp.h \
    iscpmsg.h \
    deviceinforparser.h
