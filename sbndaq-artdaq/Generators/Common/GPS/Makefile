
all: gps bcast

gps:	GPSReceiver.cc NetworkReceiver.cc GPSReceiver.hh GPSInfo.cc GPSInfo.hh NetworkReceiver.hh
	g++ -g -std=c++14 GPSReceiver.cc GPSInfo.cc NetworkReceiver.cc -I. -o gps -lboost_system -lboost_thread -lpthread -lrt

bcast:	GPSBroadcast.cc GPSBroadcast.hh GPSInfo.cc GPSInfo.hh
#	g++ -g -Wno-write-strings -std=c++11 GPSBroadcast.cc GPSInfo.cc -I. -o bcast -lpqxx -L$(EPICS_LIB) -lca -lboost_regex -lboost_system -lrt
	g++ -g -std=c++14 GPSBroadcast.cc GPSInfo.cc -o bcast -I. -I$(EPICS_INC) -I$(EPICS_INC)/os/Linux -I$(EPICS_INC)/compiler/gcc -I$(PQXX_INC) -L$(EPICS_LIB) -L$(PQXX_LIB) -lca -lpqxx -lboost_system -lrt


clean:
	rm -f gps
	rm -f bcast
	rm -f *.o
