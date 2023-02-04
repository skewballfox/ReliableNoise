# Thanks to 
# https://stackoverflow.com/questions/68428103/how-to-create-a-makefile-for-a-c-project-with-multiple-directories
# https://stackoverflow.com/questions/12605051/how-to-check-if-a-directory-doesnt-exist-in-make-and-create-it
# https://www.gnu.org/software/make/manual/html_node/Catalogue-of-Rules.html#Catalogue-of-Rules

CXX 		= g++
CXXFLAGS 	= -std=c++17 -c -g -w -O3
LFLAGS		= -lpthread -lboost_system

SRC_DIR 	:= src
OBJ_DIR		:= obj

SRCEXTS     := .c .C .cc .cpp .CPP .c++ .cxx .cp
HDREXTS     := .h .H .hh .hpp .HPP .h++ .hxx .hp

TARGETS := client server

SOURCES := $(wildcard $(addprefix $(SRC_DIR)/*,$(SRCEXTS)))
OBJECTS := $(patsubst src/%.cpp,obj/%.o,$(SOURCES))


HEADERS := $(shell find inc -name "*.h")

HEADERDIRS := $(sort $(dir $(HEADERS)))
INCLUDEFLAGS := $(addprefix -I,$(HEADERDIRS))
#I'd rather handle this automatically but this will work for now
INCLUDEFLAGS += -I/usr/include/boost -I/usr/include/boost/asio -I/usr/include/boost/system

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(shell mkdir -p $(OBJ_DIR))
	$(CXX) $(CXXFLAGS) $< -c $(INCLUDEFLAGS) -o $@

#couldn't figure out how to get $(TARGETS):$(OBJECTS)
#to build more than one target

all: client server

client: $(OBJ_DIR)/client.o
	$(CXX)  $(LFLAGS) $(OBJ_DIR)/client.o -o $@

server: $(OBJ_DIR)/server.o
	$(CXX) $(LFLAGS) $(OBJ_DIR)/server.o -o $@


clean:
	rm -f $(OBJECTS) $(TARGETS)
	rmdir $(OBJ_DIR)
