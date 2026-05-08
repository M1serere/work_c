QT += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PhoneBook
TEMPLATE = app

SOURCES += main.cpp \
           mainwindow.cpp \
           database.cpp

HEADERS += mainwindow.h \
           database.h