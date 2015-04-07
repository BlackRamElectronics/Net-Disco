# InnoPrint
# FindHubs-Server

include Sources.mk
include Includes.mk

TOOLCHAIN_PREFIX = arm-linux-
CC = $(TOOLCHAIN_PREFIX)g++

OUTPUT = FindHubs-Server

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
        
