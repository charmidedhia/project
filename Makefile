SHELL			= /bin/bash
CC				= g++
CFLAGS			= -pedantic -Wall -D __STDC_LIMIT_MACROS -D __STDC_FORMAT_MACROS

DEBUGFLAGS		= -g -O0 -D _DEBUG -ggdb
RELEASEFLAGS	= -O2 -D NDEBUG

TARGET	= panda
SOURCES	= $(shell echo src/*.cpp)
HEADERS	= $(shell echo include/*.h)
OBJECTS	= $(SOURCES:.cpp=.o)


all: $(TARGET)
 
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(DEBUGFLAGS) $(OBJECTS) -o $(TARGET) 

.cpp.o:
	$(CC) $(CFLAGS) $(DEBUGFLAGS) -c $< -o $@  

clean:
	rm -f src/*.o $(TARGET) 	
