CXX    =   gcc
CXXFLAGS_MYSQL  = -W -std=c++0x #`mysql_config --cflags --libs`
CXXFLAGS  =  -W -std=c++0x
CXX_DIR = g++
CXX_DIR_FLAGS  =  -std=c++0x
LDFLAGS   =  -L/urs/lib -lstdc++ -lm
TARGET   =  startallboards_fcl
TARGET1 = stopallboards_fcl
BASELINE =  ./baselines/baselines
DECODE =  ./decode/decode
HISTOGRAM =  ./histogram/histogram
UNPACKBASELINE =  ./unpackbaseline/unpackbaseline
UNPACKSIGNAL =  ./unpacksignal/unpacksignal
INCLUDE   =  -Iinclude

all: clean install
install: $(TARGET) $(TARGET1) $(BASELINE) $(DECODE) $(HISTOGRAM) $(UNPACKBASELINE) $(UNPACKSIGNAL)

$(TARGET): $(TARGET).o CRT_fcl.o usbreadout.o 
	$(CXX) $(INCLUDE) -o $(TARGET) $(TARGET).o CRT_fcl.o usbreadout.o $(CXXFLAGS) $(LDFLAGS)
$(TARGET).o: $(TARGET).cpp usbreadout.cpp CRT_fcl.cpp
	$(CXX) -c $(TARGET).cpp $(CXXFLAGS) $(INCLUDE)

$(TARGET1): $(TARGET1).o CRT_fcl.o usbreadout.o 
	$(CXX) $(INCLUDE) -o $(TARGET1) $(TARGET1).o CRT_fcl.o usbreadout.o $(CXXFLAGS) $(LDFLAGS)
$(TARGET1).o: $(TARGET1).cpp usbreadout.cpp CRT_fcl.cpp
	$(CXX) -c $(TARGET1).cpp $(CXXFLAGS) $(INCLUDE)

CRT_fcl.o: CRT_fcl.cpp 
	$(CXX) -c CRT_fcl.cpp $(CXXFLAGS) $(INCLUDE)
usbreadout.o: usbreadout.cpp 
	$(CXX) -c usbreadout.cpp $(CXXFLAGS) $(INCLUDE)

$(BASELINE):
	$(CXX_DIR) -o $(BASELINE) $(BASELINE).cpp $(CXX_DIR_FLAGS)

$(DECODE):
	$(CXX_DIR) -o $(DECODE) $(DECODE).cpp $(CXX_DIR_FLAGS)

$(HISTOGRAM):
	$(CXX_DIR) -o $(HISTOGRAM) $(HISTOGRAM).cpp $(CXX_DIR_FLAGS)

$(UNPACKBASELINE):
	$(CXX_DIR) -o $(UNPACKBASELINE) $(UNPACKBASELINE).cpp $(CXX_DIR_FLAGS)

$(UNPACKSIGNAL):
	$(CXX_DIR) -o $(UNPACKSIGNAL) $(UNPACKSIGNAL).cpp $(CXX_DIR_FLAGS)

clean:
	rm -rf CRT_fcl.o usbreadout.o $(TARGET).o $(TARGET)
	rm -rf CRT_fcl.o usbreadout.o $(TARGET1).o $(TARGET1)
	rm -rf $(BASELINE) $(BASELINE).o
	rm -rf $(DECODE) $(DECODE).o
	rm -rf $(HISTOGRAM) $(HISTOGRAM).o
	rm -rf $(UNPACKBASELINE) $(UNPACKBASELINE).o
	rm -rf $(UNPACKSIGNAL) $(UNPACKSIGNAL).o
