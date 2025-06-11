QT       += core gui sql network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    customcalendarwidget.cpp \
    dragtreewidget.cpp \
    folder.cpp \
    foldereditwindow.cpp \
    main.cpp \
    mainwindow.cpp \
    task.cpp \
    taskcreationwindow.cpp \
    taskeditwindow.cpp

HEADERS += \
    customcalendarwidget.h \
    dragtreewidget.h \
    folder.h \
    foldereditwindow.h \
    mainwindow.h \
    task.h \
    taskcreationwindow.h \
    taskeditwindow.h

FORMS += \
    foldereditwindow.ui \
    mainwindow.ui \
    taskcreationwindow.ui \
    taskeditwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RC_FILE = icon.rc

RESOURCES += \
    resources.qrc
