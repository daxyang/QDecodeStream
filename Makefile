
ifeq ($(os),macx)
CXX=g++
TARGET=libQDecodeStream.dylib
else
CXX=arm-none-linux-gnueabi-g++
TARGET=libQDecodeStream.so
endif

CXXFLAGS = -pipe -O2 -Wall -W -fPIC
ifeq ($(os),macx)
LFLAGS = -dynamiclib
else
LFLAGS = -shared
endif
INCLUDEPATH += -I../QSlidingWindow -I../QSlidingWindowConsume
INCLUDEPATH += -I/usr/local/include

ifeq ($(os),macx)
LIBS += -L/usr/local/lib -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_objdetect -lavutil -lsdl -lSDL_image -lx265 -lpthread
else
INCLUDEPATH  += -I/opt/ffmpeg-2.8.5/include
LIBS += -L/opt/ffmpeg-2.8.5/lib -lavutil -lavcodec -lswresample -lswscale
endif

LIBS += -L/usr/local/lib -lQSlidingWindow -lQSlidingWindowConsume -lpthread


$(TARGET):QDecodeStream.o GetRAWData.o
	$(CXX) $(LFLAGS)  -o $(TARGET) QDecodeStream.o GetRAWData.o  $(LIBS)


QDecodeStream.o:qdecodestream.cpp qdecodestream.h decode_param.h
	$(CXX) -c $(CXXFLAGS) $(INCLUDEPATH) -o QDecodeStream.o qdecodestream.cpp

GetRAWData.o:getrawdata.cpp getrawdata.h
	$(CXX) -c $(CXXFLAGS) $(INCLUDEPATH) -o GetRAWData.o getrawdata.cpp

clean:
	rm -f *.o $(TARGET)

install:
	cp -f $(TARGET) /usr/local/lib
