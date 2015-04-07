# InnoPrint
# FindServer

include Sources.mk
include Includes.mk

TOOLCHAIN_PREFIX = 
CC = $(TOOLCHAIN_PREFIX)g++

OUTPUT = FindHubs-Client

LIBS =
LIBS_DIR =

CFLAGS= -Wall $(INCLUDES) $(LIBS) $(LIBS_DIR)
CPPSOURCES= 

all: $(OUTPUT)
        
$(OUTPUT): $(SOURCES)
	echo $(CFLAGS)
	$(CC) -o $@ $^ $(CFLAGS) $(LFLAGS)

clean:
	rm $(OUTPUT)
        
