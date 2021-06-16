# Makefile for libendetool
# (C)2016 ~ 2021 Raphael Kim / rageworx

# To enable build for embedded linux, you may encomment next 2 lines.
# CCPREPATH = ${ARM_LINUX_GCC_PATH}
# CCPREFIX  = arm-linux-

# To enable build for embedded linux, change following line.
# CCPATH    = ${CCPREPATH}/${CCPREFIX}
CCPATH = ${CCPREFIX}

# Compiler configure.
GCC = ${CCPATH}gcc
GPP = ${CCPATH}g++
AR  = ${CCPATH}ar
RL  = ${CCPATH}ranlib

SOURCEDIR = ./src
OUTDIR    = ./lib
AES256DIR = ${SOURCEDIR}/aes256
BASE64DIR = ${SOURCEDIR}/base64
# LZMA may removed for next version -
LZMATDIR  = ${SOURCEDIR}/lzmat
OBJDIR    = ./obj
OUTBIN    = libendetool.a
DEFINEOPT = -D_GNU_SOURCE
OPTIMOPT  = -O2 -fomit-frame-pointer
#OPTIMOPT  =  -g3  -DDEBUG
OPTADD    = 
OPTARCH   =

# Automatic detecting architecture.
KRNL := $(shell uname -s)
KVER := $(shell uname -r | cut -d . -f1)
ARCH := $(shell uname -m)

ifeq ($(KRNL),Darwin)
	ifeq ($(shell test $(KVER) -gt 19; echo $$?),0)
		OPTARCH += -arch x86_64 -arch arm64
	endif
else
	SUBSYS := $(shell uname -s | cut -d _ -f1)
	ifeq ($(SUBSYS),MINGW64)
		OPTARCH += -mconsole -static
	endif
endif

CFLAGS += -I$(SOURCEDIR) -I$(AES256DIR) -I$(BASE64DIR) -I$(LZMATDIR)
CFLAGS += $(DEFINEOPT)
CFLAGS += $(OPTARCH) $(OPTIMOPT)
LFLAGS += $(OPTADD)

.PHONY: prepare clean test

all: prepare ${OUTDIR}/${OUTBIN}
cleanall: prepare clean ${OUTDIR}/${OUTBIN}

prepare:
	@mkdir -p ${OBJDIR}
	@mkdir -p ${OUTDIR}

${OBJDIR}/aes256.o:
	@echo "Compiling AES256 ..."
	@$(GCC) -c ${CFLAGS} ${AES256DIR}/aes.c -o $@

${OBJDIR}/base64.o:
	@echo "Compiling BASE64 ..."
	@$(GCC) -c ${CFLAGS} ${BASE64DIR}/base64.c -o $@

${OBJDIR}/lzmat_dec.o:
	@echo "Compiling LZMAT decoder ..."
	@$(GCC) -c ${CFLAGS} ${LZMATDIR}/lzmat_dec.c -o $@

${OBJDIR}/lzmat_enc.o:
	@echo "Compiling LZMAT encoder ..."
	@$(GCC) -c ${CFLAGS} ${LZMATDIR}/lzmat_enc.c -o $@
	
${OBJDIR}/endetool.o:
	@echo "Compiling library exports ..."
	@$(GPP) -c ${CFLAGS} ${SOURCEDIR}/endetool.cpp -I$(AES256DIR) -I$(BASE64DIR) -I$(LZMATDIR) $(OPTIMIZEOPT) -o $@

${OUTDIR}/${OUTBIN}: ${OBJDIR}/aes256.o ${OBJDIR}/base64.o ${OBJDIR}/lzmat_dec.o ${OBJDIR}/lzmat_enc.o ${OBJDIR}/endetool.o
	@echo "Generating library ..."
	@$(AR) -cr $@ $^
	@$(RL) $@
	@cp -rf ${SOURCEDIR}/endetool.h ${OUTDIR}

test: ${OUTDIR}/${OUTBIN} test/test.cpp
	@echo "Compiling test ..."
	@$(GPP) ${CFLAGS} test/test.cpp -Ilib -Llib -lendetool ${LFLAGS} -o test/endetest

clean:
	@echo "Cleaning ...."
	@rm -rf ${OBJDIR}/*
	@rm -rf ${OUTDIR}/${OUTBIN}
	@rm -rf ${OUTDIR}/endetool.h
	@rm -rf test/endetest.*
