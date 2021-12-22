include /usr/local/etc/PcapPlusPlus.mk
LOCAL_INCLUDES = -I/usr/local/include
MYSQL_INCLUDES = -I/usr/local/include/cppconn
INCLUDES = ./include

all: project
	sudo -S ./project

project: main.o TCPSniffer.o Widget.o
	g++ $(PCAPPP_LIBS_DIR) -o project -Wl,-Bdynamic main.o TCPSniffer.o Widget.o $(PCAPPP_LIBS) -g -lmysqlcppconn `wx-config --libs`

main.o: main.cpp TCPSniffer.h Widget.h
	g++ $(PCAPPP_INCLUDES) `wx-config --cxxflags` -c -o main.o main.cpp $(INCLUDES) $(LOCAL_INCLUDES) $(MYSQL_INCLUDES)

TCPSniffer.o: TCPSniffer.cpp TCPSniffer.h Widget.h
	g++ $(PCAPPP_INCLUDES) `wx-config --cxxflags` -c -o TCPSniffer.o TCPSniffer.cpp  $(LOCAL_INCLUDES) $(MYSQL_INCLUDES) $(INCLUDES)

Widget.o: Widget.cpp TCPSniffer.h Widget.h
	g++ $(PCAPPP_INCLUDES) `wx-config --cxxflags` -c -o Widget.o Widget.cpp $(LOCAL_INCLUDES) $(MYSQL_INCLUDES) $(INCLUDES)

clean:
	rm main.o TCPSniffer.o Widget.o application widgetTest