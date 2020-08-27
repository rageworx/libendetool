# Makefile for libendetool
# (C)2016 ~ 2019 Raphael Kim / rageworx

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

SOURCEDIR = ./src
OUTDIR    = ./lib
AES256DIR = ${SOURCEDIR}/aes256
BASE64DIR = ${SOURCEDIR}/base64
LZMATDIR  = ${SOURCEDIR}/lzmat
OBJDIR    = ./obj/static
OUTBIN    = libendetool.a
DEFINEOPT = -D_GNU_SOURCE -DNOLZMAT
OPTIMOPT  = -O2 -s
OPTADD    = 

ifeq (windows,$(firstword $(MAKECMDGOALS)))
OPTADD += -mwindows
endif

CFLAGS    = -I$(SOURCEDIR) -I$(AES256DIR) -I$(BASE64DIR) -I$(LZMATDIR) $(DEFINEOPT)
LFLAGS    = $(OPTIMOPT) $(OPTADD)

all: prepare clean ${OUTDIR}/${OUTBIN}

windows: all

prepare:
	@mkdir -p ${OBJDIR}
	@mkdir -p ${OUTDIR}

${OBJDIR}/aes256.o:
	@echo "Compiling AES256 ..."
	@$(GCC) -c ${CFLAGS} ${AES256DIR}/aes.c -o $@

${OBJDIR}/base64.o:
	@echo "Compiling BASE64 ..."
	@$(GCC) -c ${CFLAGS} ${BASE64DIR}/base64.c -o $@

${OBJDIR}/endetool.o:
	@echo "Compiling library exports ..."
	@$(GPP) -c ${CFLAGS} ${SOURCEDIR}/endetool.cpp -I$(AES256DIR) -I$(BASE64DIR) -I$(LZMATDIR) $(OPTIMIZEOPT) -o $@

${OUTDIR}/${OUTBIN}: ${OBJDIR}/aes256.o ${OBJDIR}/base64.o ${OBJDIR}/endetool.o
	@echo "Generating library ..."
	@$(AR) -q $@ ${OBJDIR}/*.o
	@cp -rf ${SOURCEDIR}/endetool.h ${OUTDIR}

clean:
	@echo "Cleaning ...."
	@rm -rf ${OBJDIR}/*
	@rm -rf ${OUTDIR}/${OUTBIN}
	@rm -rf ${OUTDIR}/endetool.h
