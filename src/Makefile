UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Linux)
	CC := g++
endif

ifeq ($(UNAME_S),Darwin)
	CC := cc
endif

TARGET := dispenserController
OBJS := main.o
DEPS := INIReader.h
LIBS := 
CFLAGS += -I.

%.o: %.cpp $(DEPS)

${TARGET} : ${OBJS}
	$(CC) -o $@ $^ ${CFLAGS} ${LIBS}

.PHONY: clean

clean:
	rm -f ${TARGET} ${OBJS} 
