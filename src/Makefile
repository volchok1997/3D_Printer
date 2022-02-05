
YOCTO_HOME := ../extlib/yoctolib_cpp
YOCTO_API_SRC := ${YOCTO_HOME}/Sources

UNAME_S := $(shell uname -s)
ARCH  := $(shell uname -m| sed -e s/i.86/i386/ -e s/arm.*/arm/)

ifeq ($(UNAME_S),Linux)
	CC := g++
ifeq (${ARCH}, x86_64)
	YOCTO_LIB_PATH = ${YOCTO_HOME}/Binaries/linux/64bits
else ifeq (${ARCH}, aarch64)
	YOCTO_LIB_PATH = ${YOCTO_HOME}/Binaries/linux/aarch64
endif
else ifeq ($(UNAME_S),Darwin)
	CC := cc
endif

YOCTO_LIB := ${YOCTO_LIB_PATH}/libyocto-static.a


TARGET := dispenserController
OBJS := main.o ml808gx.o yocto_pwm.o
DEPS := INIReader.hpp serial.h ml808gx.hpp yocto_pwm.hpp
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
