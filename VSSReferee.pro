# Qt libs to import
QT += core    \
      gui     \
      widgets \
      network \
      opengl

# Project configs
TEMPLATE = app
DESTDIR  = ../bin
TARGET   = VSSReferee
VERSION  = 2.0.0

CONFIG += c++14 console
CONFIG -= app_bundle

# Temporary dirs
OBJECTS_DIR = tmp/obj
MOC_DIR = tmp/moc
UI_DIR = tmp/moc
RCC_DIR = tmp/rc

# Project libs
LIBS *= -lprotobuf -lQt5Core

# Compiling .proto files
system(echo "Compiling protobuf files" && cd include/proto && protoc --cpp_out=../ *.proto && cd ../..)

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += APP_VERSION=\\\"$$VERSION\\\"
DEFINES += PROJECT_PATH=\\\"$${PWD}\\\"

SOURCES += \
        include/command.pb.cc \
        include/common.pb.cc \
        include/packet.pb.cc \
        include/replacement.pb.cc \
        include/vssref_command.pb.cc \
        include/vssref_common.pb.cc \
        include/vssref_placement.pb.cc \
        main.cpp \
        src/constants/constants.cpp \
        src/refereecore.cpp \
        src/soccerview/soccerview.cpp \
        src/utils/types/angle/angle.cpp \
        src/utils/types/field/field.cpp \
        src/utils/types/object/object.cpp \
        src/utils/types/position/position.cpp \
        src/utils/types/velocity/velocity.cpp \
        src/utils/utils.cpp \
        src/world/entities/entity.cpp \
        src/utils/exithandler/exithandler.cpp \
        src/utils/text/text.cpp \
        src/utils/timer/timer.cpp \
        src/world/entities/referee/checkers/ballplay/checker_ballplay.cpp \
        src/world/entities/referee/checkers/checker.cpp \
        src/world/entities/referee/checkers/goalie/checker_goalie.cpp \
        src/world/entities/referee/checkers/halftime/checker_halftime.cpp \
        src/world/entities/referee/checkers/stoppedball/checker_stuckedball.cpp \
        src/world/entities/referee/checkers/twoattackers/checker_twoattackers.cpp \
        src/world/entities/referee/checkers/twodefenders/checker_twodefenders.cpp \
        src/world/entities/referee/referee.cpp \
        src/world/entities/replacer/replacer.cpp \
        src/world/entities/vision/filters/loss/lossfilter.cpp \
        src/world/entities/vision/filters/noise/noisefilter.cpp \
        src/world/entities/vision/filters/kalman/kalmanfilter.cpp \
        src/world/entities/vision/filters/kalman/matrix/matrix.cpp \
        src/world/entities/vision/filters/kalman/state/kalmanstate.cpp \
        src/world/entities/vision/vision.cpp \
        src/world/world.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    include/command.pb.h \
    include/common.pb.h \
    include/packet.pb.h \
    include/replacement.pb.h \
    include/vssref_command.pb.h \
    include/vssref_common.pb.h \
    include/vssref_placement.pb.h \
    src/constants/constants.h \
    src/refereecore.h \
    src/soccerview/soccerview.h \
    src/utils/types/angle/angle.h \
    src/utils/types/field/field.h \
    src/utils/types/field/field_default_3v3.h \
    src/utils/types/object/object.h \
    src/utils/types/position/position.h \
    src/utils/types/velocity/velocity.h \
    src/utils/utils.h \
    src/world/entities/entity.h \
    src/utils/exithandler/exithandler.h \
    src/utils/text/text.h \
    src/utils/timer/timer.h \
    src/world/entities/referee/checkers/ballplay/checker_ballplay.h \
    src/world/entities/referee/checkers/checker.h \
    src/world/entities/referee/checkers/checkers.h \
    src/world/entities/referee/checkers/goalie/checker_goalie.h \
    src/world/entities/referee/checkers/halftime/checker_halftime.h \
    src/world/entities/referee/checkers/stoppedball/checker_stuckedball.h \
    src/world/entities/referee/checkers/twoattackers/checker_twoattackers.h \
    src/world/entities/referee/checkers/twodefenders/checker_twodefenders.h \
    src/world/entities/referee/referee.h \
    src/world/entities/replacer/replacer.h \
    src/world/entities/vision/filters/loss/lossfilter.h \
    src/world/entities/vision/filters/noise/noisefilter.h \
    src/world/entities/vision/filters/kalman/kalmanfilter.h \
    src/world/entities/vision/filters/kalman/matrix/matrix.h \
    src/world/entities/vision/filters/kalman/state/kalmanstate.h \
    src/world/entities/vision/vision.h \
    src/world/world.h

FORMS += \
    src/soccerview/soccerview.ui

RESOURCES += \
    rsc/resources.qrc
