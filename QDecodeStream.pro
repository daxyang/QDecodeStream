#-------------------------------------------------
#
# Project created by QtCreator 2016-05-06T12:35:49
#
#-------------------------------------------------

QT       += core

TARGET = QDecodeStream
TEMPLATE = lib

DEFINES += QDECODESTREAM_LIBRARY

SOURCES += qdecodestream.cpp \
    getrawdata.cpp

HEADERS += qdecodestream.h \
       qdecodestream_global.h \
    decode_param.h \
    getrawdata.h


#include opencv ffmpeg library
#windows
win32{
INCLUDEPATH += e:/ffmpeg-win32-dev/include/
INCLUDEPATH += e:/pthread/Pre-built2/include/
INCLUDEPATH += e:/opencv/opencv_build/include
INCLUDEPATH += e:/opencv/opencv_build/include/opencv
INCLUDEPATH += e:/opencv/opencv_build/include/opencv2

LIBS     += -LE:/pthread/Pre-built2/lib/pthreadVC2
LIBS     += -LE:/ffmpeg-win32-dev/lib/ -lavcodec -lavutil -lswscale
LIBS     += -LE:/opencv/opencv_build/lib/ -lopencv_highgui300 -lopencv_core300 -lopencv_imgproc300 -lopencv_core300 -lopencv_objdetect300
LIBS     += -lwsock32  #windows下的网络函数相关的库
}
#Mac
unix:macx{
INCLUDEPATH += /usr/local/include/opencv
INCLUDEPATH += /usr/local/include/opencv2
INCLUDEPATH += /usr/local/include
INCLUDEPATH += /usr/local/include/SDL/

LIBS        += -L/usr/local/lib/  -lopencv_core -lopencv_highgui  -lopencv_imgproc  -lopencv_objdetect -lavutil
LIBS        += -L/usr/local/lib/ -lsdl -lSDL_image -lx265
LIBS        += -lpthread
}


#ubuntu
unix:!macx{
INCLUDEPATH  += /usr/local/linux_lib/opencv/include
INCLUDEPATH  += /usr/local/linux_lib/ffmpeg/include
LIBS += -L/usr/local/linux_lib/opencv/lib -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_objdetect
LIBS += -L/usr/local/linux_lib/ffmpeg/lib -lavutil -lavcodec -lswresample -lswscale
}
#arm
#unix:!macx{
##INCLUDEPATH  += /opt/opencv-3.1/include
#INCLUDEPATH  += /opt/ffmpeg-2.8.5/include
##LIBS += -L/opt/opencv-3.1/lib -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_objdetect
#LIBS += -L/opt/ffmpeg-2.8.5/lib -lavutil -lavcodec -lswresample -lswscale
#LIBS += -L/opt/iconv/lib -liconv
#}

#include usr library
#mac
unix:macx{
LIBS        += -L/usr/local/lib  -lQSlidingWindow -lQSlidingWindowConsume
INCLUDEPATH += ../QSlidingWindow
INCLUDEPATH += ../QSlidingWindowConsume
}
#ubuntu
unix:!macx{
LIBS        += -L/usr/local/linux_lib/lib -lQSlidingWindow -lQSlidingWindowConsume
INCLUDEPATH += ../QSlidingWindow
INCLUDEPATH += ../QSlidingWindowConsume
}
#arm
#unix:!macx {
#LIBS        += -L/usr/local/arm_lib -lQSlidingWindow -lQSlidingWindowConsume
#INCLUDEPATH += ../QSlidingWindow
#INCLUDEPATH += ../QSlidingWindowConsume
#}
#install
#ubuntu
unix:!macx {
    target.path = /usr/local/linux_lib/lib
    INSTALLS += target
}
#arm
#unix:!macx {
#    target.path = /usr/local/arm_lib
#    INSTALLS += target
#}
#mac
unix:macx{
    target.path = /usr/local/lib
    INSTALLS += target
}

