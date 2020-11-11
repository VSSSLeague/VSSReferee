QT += core \
        gui \
        widgets \
        network \
        opengl

CONFIG += c++14 console
CONFIG -= app_bundle

VERSION  = 1.0.0
DESTDIR  = ../bin
TARGET   = VSS-Referee

# Temporary dirs
OBJECTS_DIR = tmp/obj
MOC_DIR = tmp/moc
UI_DIR = tmp/moc
RCC_DIR = tmp/rc

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += PROJECT_PATH=\\\"$${PWD}\\\"
LIBS *= -lprotobuf -lGLU -pthread -lQt5Core -lpthread

# Compiling protobuf
system(echo "Compiling firaSim protobufs" && cd include/protobuf/firasim && protoc --cpp_out=../../ *.proto && cd ../../..)
system(echo "Compiling referee protobufs" && cd include/protobuf/vssreferee && protoc --cpp_out=../../ *.proto && cd ../../..)

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        constants/constants.cpp \
        exithandler.cpp \
        include/command.pb.cc \
        include/common.pb.cc \
        include/packet.pb.cc \
        include/replacement.pb.cc \
        include/vssclient/netraw.cpp \
        include/vssclient/vssclient.cpp \
        include/vssref_command.pb.cc \
        include/vssref_placement.pb.cc \
        include/vssref_common.pb.cc \
        main.cpp \
        src/entity/entity.cpp \
        src/entity/refereeview/mainwindow/mainwindow.cpp \
        src/entity/refereeview/refereeview.cpp \
        src/entity/refereeview/soccerview/soccerview.cpp \
        src/entity/refereeview/soccerview/util/field.cpp \
        src/entity/refereeview/soccerview/util/gltext.cpp \
        src/entity/vssreferee/vssreferee.cpp \
        src/entity/vssreplacer/vssreplacer.cpp \
        src/entity/vssvisionclient/vssvisionclient.cpp \
        utils/utils.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    constants/constants.h \
    exithandler.h \
    include/command.pb.h \
    include/common.pb.h \
    include/packet.pb.h \
    include/replacement.pb.h \
    include/timer.h \
    include/vssclient/netraw.h \
    include/vssclient/util.h \
    include/vssclient/vssclient.h \
    include/vssref_command.pb.h \
    include/vssref_placement.pb.h \
    include/vssref_common.pb.h \
    src/entity/entity.h \
    src/entity/refereeview/mainwindow/mainwindow.h \
    src/entity/refereeview/refereeview.h \
    src/entity/refereeview/soccerview/soccerview.h \
    src/entity/refereeview/soccerview/util/field.h \
    src/entity/refereeview/soccerview/util/field_default_constants.h \
    src/entity/refereeview/soccerview/util/geometry.h \
    src/entity/refereeview/soccerview/util/gltext.h \
    src/entity/refereeview/soccerview/util/gvector.h \
    src/entity/refereeview/soccerview/util/timer.h \
    src/entity/refereeview/soccerview/util/util.h \
    src/entity/vssreferee/vssreferee.h \
    src/entity/vssreplacer/vssreplacer.h \
    build/tmp/moc/ui_mainwindow.h \
    src/entity/vssvisionclient/vssvisionclient.h \
    utils/utils.h

FORMS += \
    src/entity/refereeview/mainwindow/mainwindow.ui

RESOURCES += \
    constrsrc.qrc \
    resources/resources.qrc
