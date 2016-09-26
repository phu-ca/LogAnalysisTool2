#-------------------------------------------------
#
# Project created by QtCreator 2016-09-13T13:10:50
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = LogAnalysisTool
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    controller.cpp \
    workingspace.cpp \
    logfile.cpp

HEADERS  += mainwindow.h \
    controller.h \
    workingspace.h \
    logfile.h

FORMS    += mainwindow.ui

RESOURCES += \
    logat.qrc

RC_FILE += \
    loganalysistool.rc

DISTFILES += \
    templates.json
