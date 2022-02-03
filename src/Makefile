
YOCTO_HOME := ../extlib/yoctolib_cpp
YOCTO_API_SRC := ${YOCTO_HOME}/Sources

UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Linux)
	CC := g++
	YOCTO_LIB_PATH = ${YOCTO_HOME}/Binaries/linux/64bits
endif

ifeq ($(UNAME_S),Darwin)
	CC := cc
endif

YOCTO_LIB := ${YOCTO_LIB_PATH}/libyocto-static.a


TARGET := dispenserController
OBJS := main.o
DEPS := INIReader.h
LIBS := ${YOCTO_LIB} -lm -lpthread -lusb-1.0
CFLAGS += -I. -I${YOCTO_API_SRC} 


${TARGET}: ${OBJS} ${YOCTO_LIB}
	$(CC) -o $@ $^ ${CFLAGS} ${LIBS}

%.o: %.cpp $(DEPS)
	$(CC) ${CFLAGS} -c -o $@ $<

${YOCTO_HOME}/Binaries/%/libyocto-static.a:
	@echo compiling Yoctopuce C++ lib for $*
	@make -C ${YOCTO_HOME}/Binaries $*/libyocto-static.a


.PHONY: clean

clean:
	rm -f ${TARGET} ${OBJS} 
