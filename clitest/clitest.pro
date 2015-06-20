#-------------------------------------------------
#
# Project created by QtCreator 2014-08-15T16:37:33
#
#-------------------------------------------------

QT       += core
QT       += network
QT       -= gui

TARGET = qiscp
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += main.cpp

HEADERS += 

unix:!macx: LIBS += -L$$OUT_PWD/../libqiscp/ -lqiscp
win32: LIBS += -L$$OUT_PWD/../libqiscp/ -lqiscp

INCLUDEPATH += $$PWD/../libqiscp
DEPENDPATH += $$PWD/../libqiscp
