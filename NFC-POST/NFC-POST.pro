#-------------------------------------------------
#
# Project created by QtCreator 2013-06-13T10:02:13
#
#-------------------------------------------------

QT       += core gui
QT       += xml

TARGET = NFC-POST
TEMPLATE = app


SOURCES += \
    uart.c \
    uart_posix.c \
    nfc-utils.c \
    nfc/nfc.c \
    pn532_uart.c \
    mirror-subr.c \
    pn53x.c \
    nfc-device.c \
    iso14443-subr.c \
    rsa.c \
    nfc-main.c \
    nfc-ndef.c \
    nfc-emulation.c \
    nfc-emulate-forum-tag4.c \
    main.cpp \
    video.cpp \
    map.cpp \
    dynamic-load.cpp \
    log.c \
    pwm.c \
    nfcthread.cpp \
    xmlconfigfile.cpp \
    webtransaction.cpp

HEADERS += \
    include/nfc-utils.h \
    include/pn532_uart.h \
    pn532_uart.h \
    nfc-utils.h \
    nfc/nfc.h \
    chips/pn53x-internal.h \
    chips/pn53x.h \
    nfc-internal.h \
    drivers.h \
    uart.h \
    pn53x-internal.h \
    mirror-subr.h \
    nfc/nfc-types.h \
    rsa.h \
    nfc-ndef.h \
    nfc/nfc-emulation.h \
    nfc_emulate_card.h \
    iso7816.h \
    nfc-main.h \
    video.h \
    ndeffile.h \
    map.h \
    curl/easy.h \
    curl/curl.h \
    dynamic-load.h \
    nfc-emulation.h \
    main.h \
    myconf.h \
    log.h \
    pwm.h \
    nfcthread.h \
    xmlconfigfile.h \
    webtransaction.h

LIBS += -L/usr/local/curl/lib -lcurl
#LIBS += -L/usr/local/libcurl-arm/lib -lcurl

FORMS +=
