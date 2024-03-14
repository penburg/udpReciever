QT -= gui
QT += network

CONFIG += c++17 console
CONFIG -= app_bundle

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        bytetools.cpp \
        chacha20.cpp \
        dhcpmessage.cpp \
        main.cpp \
        messagerebuilder.cpp \
        micromessage.cpp \
        udpreciever.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    bytetools.h \
    chacha20.h \
    dhcpmessage.h \
    messagerebuilder.h \
    micromessage.h \
    udpreciever.h

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += libgcrypt
