QT += widgets
CONFIG += c++17 console
CONFIG -= app_bundle
TARGET = ColorSwitcher
TEMPLATE = app

SOURCES += main.cpp colorwidget.cpp palettebutton.cpp mainwindow.cpp
HEADERS += colorwidget.h palettebutton.h mainwindow.h

msvc:QMAKE_CXXFLAGS += /utf-8
