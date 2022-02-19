
UNAME_S := $(shell uname -s)
ARCH  := $(shell uname -m| sed -e s/i.86/i386/ -e s/arm.*/arm/)
CFLAG := -O2

ifeq ($(UNAME_S),Linux)
	CC := g++
else ifeq ($(UNAME_S),Darwin)
	CC := cc
endif


TARGET := dispenserController
OBJS := main.o ml808gx.o signal_dector.o
DEPS := INIReader.hpp serial.h ml808gx.hpp signal_dector.hpp
LIBS := -lpigpio -lm -lpthread
CFLAGS += -I. 


${TARGET}: ${OBJS}
	$(CC) -o $@ $^ ${CFLAGS} ${LIBS}

%.o: %.cpp $(DEPS)
	$(CC) ${CFLAGS} -c -o $@ $<


.PHONY: clean

clean:
	rm -f ${TARGET} ${OBJS} 
