TARGET=libQDecodeStream.dylib

CXX=g++
CXXFLAGS = -pipe -O2 -Wall -W -fPIC
LFLAGS = -dynamiclib
INCLUDEPATH += -I../QSlidingWindow -I../QSlidingWindowConsume
INCLUDEPATH += -I/usr/local/include

LIBS += -L/usr/local/lib -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_objdetect -lavutil -lsdl -lSDL_image -lx265 -lpthread
LIBS += -L/usr/local/lib -lQSlidingWindow -lQSlidingWindowConsume


$(TARGET):QDecodeStream.o GetRAWData.o
	$(CXX) $(LFLAGS)  -o $(TARGET) QDecodeStream.o GetRAWData.o  $(LIBS)


QDecodeStream.o:qdecodestream.cpp qdecodestream.h decode_param.h
	$(CXX) -c $(CXXFLAGS) $(INCLUDEPATH) -o QDecodeStream.o qdecodestream.cpp

GetRAWData.o:getrawdata.cpp getrawdata.h
	$(CXX) -c $(CXXFLAGS) $(INCLUDEPATH) -o GetRAWData.o getrawdata.cpp

clean:
	rm -f *.o *.dylib

install:
	cp -f $(TARGET) /usr/local/lib
