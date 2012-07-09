#Makefile to build BACnet Library with GCC

# tools
CC = arm-linux-gcc
AR = arm-linux-ar

# target
TARGET = lib_duksan
LIBRARY = $(TARGET).a

#PFLAGS = -lpthread
CORE_SRC = lib_duksan.c

#include
LIB_INCLUDE = ./../INCLUDE

#SRCS = ${CORE_SRC} ${PORT_SRC} ${HANDLER_SRC} ${OBJECT_SRC}
#SRCS = ${CORE_SRC}
SRCS = lib_duksan.c 

OBJS = ${SRCS:.c=.o}

all: $(LIBRARY)

lib: $(LIBRARY)

$(LIBRARY): $(OBJS) Makefile
	$(AR) rcs $@ $(OBJS)
	cp $(LIBRARY) ./../OUTPUT/$(LIBRARY)

.c.o:
	${CC} $(PFLAGS) -I$(LIB_INCLUDE) -c  $*.c -o $@

depend:
	rm -f .depend
	${CC} -MM *.c >> .depend

clean:
	rm -rf core $(OBJS) $(LIBRARY)

include: .depend
