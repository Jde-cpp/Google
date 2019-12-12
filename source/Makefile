CCACHE_EXISTS := $(shell ccache -V)
ifdef CCACHE_EXISTS
    CC := ccache $(CC)
    CXX := ccache g++-8
endif
DEBUG ?= 1
CXXFLAGS_BASE = -c -g -pthread -fPIC -std=c++17 -Wall -Wno-unknown-pragmas -DJDE_GOOGLE_EXPORTS
BIN_DIR_BASE:=../../bin/
OUT_DIR_BASE := .obj/
LIB_BASE = -lJde.Ssl -lssl -lstdc++fs
#
ifeq ($(DEBUG), 1)
	OUT_DIR=$(OUT_DIR_BASE)debug2
	CXXFLAGS= $(CXXFLAGS_BASE) -O0 -fsanitize=address -fno-omit-frame-pointer
	BIN_DIR=$(BIN_DIR_BASE)debug2
	LIB = -llsan $(LIB_BASE)
else
	OUT_DIR=$(OUT_DIR_BASE)release2
	CXXFLAGS= $(CXXFLAGS_BASE) -march=native -DNDEBUG -O3
	BIN_DIR=$(BIN_DIR_BASE)release2
	LIB=$(LIB_BASE)
endif

OUTPUT = $(BIN_DIR)/libJde.Google.so

# test:
# 	@echo $(DEBUG)
# 	@echo $(OUTPUT)

#CXX=g++-8

OBJECTS = $(OUT_DIR)/GoogleApi.o $(OUT_DIR)/GoogleDrive.o $(OUT_DIR)/DriveTypes.o
LDFLAGS = -pthread -shared -Wl,-z,origin -Wl,-rpath='$$ORIGIN'

all: $(OUTPUT)
	$(NOECHO) $(NOOP)

$(OUTPUT): $(OUT_DIR)/stdafx.h.gch $(OBJECTS)
	g++-8 -L$(BIN_DIR) $(LDFLAGS) $(OBJECTS) -o$(OUTPUT) $(LIB)

$(OUT_DIR)/stdafx.h.gch: pc.h
	$(CXX) $(CXXFLAGS) ./pc.h -o $(OUT_DIR)/stdafx.h.gch -I/home/duffyj/code/libraries/json/include -I/home/duffyj/code/libraries/spdlog/include -I$$BOOST_ROOT -I../../Framework/source
######################################################################################
$(OUT_DIR)/%.o: %.cpp %.h pc.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -include stdafx.h  -I$(OUT_DIR) ./$< -o$@
	#-I/home/duffyj/code/libraries/spdlog/include -I/home/duffyj/code/libraries/json/include 

clean:
	rm -rf -d $(OUT_DIR)/*.*
	rm -rf -d $(OUTPUT)