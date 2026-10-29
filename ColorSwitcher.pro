QT += widgets

CONFIG += c++17 console
CONFIG -= app_bundle

TARGET = ColorSwitcher
TEMPLATE = app

CODECFORTR = UTF-8
QMAKE_CXXFLAGS += -finput-charset=UTF-8 -fexec-charset=UTF-8

SOURCES += \
    main.cpp \
    colorwidget.cpp \
    palettebutton.cpp \
    mainwindow.cpp

HEADERS += \
    colorwidget.h \
    palettebutton.h \
    mainwindow.h