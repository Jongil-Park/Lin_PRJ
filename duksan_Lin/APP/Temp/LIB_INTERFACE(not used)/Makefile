#Makefile to build BACnet Library with GCC

# tools
CC = arm-linux-gcc
AR = arm-linux-ar

# target
TARGET = libinterface
LIBRARY = $(TARGET).a

#PFLAGS = -lpthread
CORE_SRC = libinterface.c

#SRCS = ${CORE_SRC} ${PORT_SRC} ${HANDLER_SRC} ${OBJECT_SRC}
#SRCS = ${CORE_SRC}
SRCS = libinterface.c \
libqueue.c

OBJS = ${SRCS:.c=.o}

all: $(LIBRARY)

lib: $(LIBRARY)

$(LIBRARY): $(OBJS) Makefile
	$(AR) rcs $@ $(OBJS)

.c.o:
	${CC} $(PFLAGS) -c  $*.c -o $@

depend:
	rm -f .depend
	${CC} -MM *.c >> .depend

clean:
	rm -rf core $(OBJS) $(LIBRARY)

include: .depend
