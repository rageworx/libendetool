# Makefile by Raphael Kim
# ------------------------
GCC=gcc
GPP=g++

TARGET  = endecmd
LFLAGS += -O2
LFLAGS += -mtune=native -lpthread

DIRSRC=src
DIRINC=src
DIROBJ=obj
DIRBIN=bin
DIRLEDT=../lib

CFLAGS+=-I${DIRINC}
CFLAGS+=-I${DIRLEDT}
LFLAGS+=-L${DIRLEDT}
LFLAGS+=-lendetool

# Automatic detecting architecture.
KRNL := $(shell uname -s)
KVER := $(shell uname -r | cut -d . -f1)
ARCH := $(shell uname -m)

ifeq ($(KRNL),Darwin)
	ifeq ($(shell test $(KVER) -gt 19; echo $$?),0)
		CFLAGS += -arch x86_64 -arch arm64
	endif
else
	SUBSYS := $(shell uname -s | cut -d _ -f1)
	ifeq ($(SUBSYS),MINGW64)
		LFLAGS += -static -s
	else
		LFLAGS += -s
	endif
endif

SRCS=$(wildcard ${DIRSRC}/*.cpp)
OBJS=$(SRCS:${DIRSRC}/%.cpp=${DIROBJ}/%.o)

all: prepare clean ${DIRBIN}/${TARGET}

prepare:
	@mkdir -p ./${DIROBJ}
	@mkdir -p ./${DIRBIN}

clean:
	@echo "Cleaning ..."
	@rm -rf ./${DIROBJ}/*
	@rm -rf ./${DIRBIN}/${TARGET}

${OBJS}: ${DIROBJ}/%.o: ${DIRSRC}/%.cpp
	@echo "Compiling $< ... "
	@$(GPP) ${CFLAGS} -c $< -o $@

${DIRBIN}/${TARGET}: ${OBJS}
	@echo "Generating $@ ..."
	@$(GPP) ${OBJS} ${LFLAGS} -o $@
	
