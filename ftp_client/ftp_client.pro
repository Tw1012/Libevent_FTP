QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
DEFINES += NOLFS
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
#INCLUDEPATH += $$PWD/openssl/include

SOURCES += \
    ftp.cpp \
    ftpconnectdlg.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    ftp.h \
    ftpconnectdlg.h \
    mainwindow.h

FORMS += \
    ftpconnectdlg.ui \
    mainwindow.ui

RESOURCES += \
    rsc.qrc

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/ftplib/lib/ -lftplib
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/ftplib/lib/ -lftplibd

INCLUDEPATH += $$PWD/ftplib/include
DEPENDPATH += $$PWD/ftplib/include
