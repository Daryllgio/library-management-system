QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    datastore.cpp \
    main.cpp \
    mainwindow.cpp \
    patronwindow.cpp \
    rolewindows.cpp \
    startupdialog.cpp

HEADERS += \
    datastore.hpp \
    mainwindow.h \
    models.hpp \
    patronwindow.hpp \
    rolewindows.hpp \
    startupdialog.hpp

FORMS += \
    mainwindow.ui

TRANSLATIONS += \
    hinlibs_d1_en_CA.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    D1.pro.user \
    hinlibs_d1.pro.user

SUBDIRS += \
    D1.pro \
    D1.pro
