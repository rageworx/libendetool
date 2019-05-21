# Makefile by Raphael Kim
# ------------------------
GCC=gcc
GPP=g++

TARGET=endecmd
CFLAGS+=-O3 -s
LFLAGS+=-static -mtune=native -lpthread

DIRSRC=src
DIRINC=src
DIROBJ=obj
DIRBIN=bin
DIRLEDT=../lib

CFLAGS+=-I${DIRINC}
CFLAGS+=-I${DIRLEDT}
LFLAGS+=-L${DIRLEDT}
LFLAGS+=-lendetool

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
	
