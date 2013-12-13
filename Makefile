CXX=$(shell root-config --cxx)
CFLAGS=-c -g -Wall $(shell root-config --cflags) -I./src -I ./include -I/user/lipschut/Introspective/ -I/user/lipschut/DDASEvent/ -O3
LDLIBS=$(shell root-config --glibs)
LDFLAGS=$(shell root-config --ldflags)
#SOURCES=./src/SL_Event.cc ./src/FileManager.cc ./src/Filter.cc
SOURCES=$(shell ls ./src/*.cc)
TEMP=$(shell ls ./src/*.cc~)
TEMP2=$(shell ls ./include/*.hh~)
OBJECTS=$(SOURCES:.cc=.o) 
HEADERS=$(shell ls ./include/*h*)

EXE= LendaAnalysis
EXECUTABLE= $(EXE)
MAIN=$(addsuffix .C,$(EXE))
MAINO=./src/$(addsuffix .o,$(EXE))


INCLUDEPATH=include
SRCPATH=src
ROOTCINT=rootcint

## .so libraries
#EVENTLIBPATH=/mnt/daqtesting/lenda/sam_analysis/LendaEvent/
EVENTLIBPATH=/user/lipschut/Introspective
LIB=LendaEvent
EVENTLIB=$(addsuffix $(LDFLAGS),$(LIB))
DDASCHANNELPATH=/user/lipschut/DDASEvent/
CHLIB=DDASEvent
CHEVENTLIB=$(addsuffix $(LDFLAGS),$(CHLIB))

.PHONY: clean get put all test sclean

all: $(EXECUTABLE) 

$(EXECUTABLE) : $(OBJECTS) $(MAINO)
	@echo "Building target" $@ "..." 
	@$(CXX) $(LDFLAGS) -o $@ $(OBJECTS) $(MAINO) $(LDLIBS) -L$(EVENTLIBPATH) -l$(EVENTLIB) -L$(DDASCHANNELPATH) -l$(CHEVENTLIB)
	@echo
	@echo "Build succeed"


.cc.o:
	@echo "Compiling" $< "..."
	@$(CXX) $(CFLAGS) $< -o $@ 

$(MAINO): $(MAIN) $(HEADERS)
	@echo "Compiling" $< "..."
	@$(CXX) $(CFLAGS) $< -o $@  

%.hh: 
	@

clean:
	-rm -f ./$(OBJECTS)
	-rm -f ./$(EXECUTABLE)
	-rm -f ./$(MAINO)

test:
	@echo $(EVENTLIBPATH)
	@echo $(HEADERS)
sclean:
	-rm  ./$(TEMP)
	-rm  ./$(TEMP2)
	-rm  ./$(OBJECTS)
	-rm  ./$(EXECUTABLE)
	-rm  ./$(MAINO)

