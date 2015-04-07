# Black Ram Electronics
# Net-Disco daemon

include Sources.mk
include Includes.mk

TOOLCHAIN_PREFIX = arm-linux-
CC = $(TOOLCHAIN_PREFIX)g++

OUTPUT = Net-Disco-daemon

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
        
