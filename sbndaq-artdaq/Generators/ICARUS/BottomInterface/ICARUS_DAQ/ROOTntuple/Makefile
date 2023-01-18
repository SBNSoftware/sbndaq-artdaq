CXX	= g++

CXXFLAGS	+= -Wall

%:%.cxx
	$(CXX) $(CXXFLAGS) `root-config --cflags --libs` $< -o $@

TARGETS	= ChipProgram

all: $(TARGETS)

clean:
	rm -rf $(TARGETS)
