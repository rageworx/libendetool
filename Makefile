# Makefile for libendetool
# (C)2016 Raphael Kim / rageworx
#

# To enable build for embedded linux, you may encomment next 2 lines.
# CCPREPATH = ${ARM_LINUX_GCC_PATH}
# CCPREFIX  = arm-linux-

# To enable build for embedded linux, change following line.
# CCPATH    = ${CCPREPATH}/${CCPREFIX}
CCPATH =

# Compiler configure.
GCC = gcc
GPP = g++
AR  = ar

SOURCEDIR = ./src
AES256DIR = ${SOURCEDIR}/aes256
BASE64DIR = ${SOURCEDIR}/base64
LZMATDIR  = ${SOURCEDIR}/lzmat
OBJDIR    = ./obj/Release
OUTBIN    = libendetool.a
OUTDIR    = .
DEFINEOPT = -D_GNU_SOURCE
OPTIMIZEOPT = -O2 -fexpensive-optimizations
CFLAGS    = -I$(SOURCEDIR) -I$(AES256DIR) -I$(BASE64DIR) -I$(LZMATDIR) -D$(DEFINEOPT) $(OPTIMIZEOPT) -mwindows

all: prepare clean ${OUTDIR}/${OUTBIN}

prepare:
	@mkdir -p ${OBJDIR}

${OBJDIR}/aes256.o:
	$(GPP) -c ${AES256DIR}/aes256.c -o $@

${OBJDIR}/base64.o:
	$(GPP) -c ${BASE64DIR}/base64.c -o $@

${OBJDIR}/lzmat_dec.o:
	$(GPP) -c ${LZMATDIR}/lzmat_dec.c -o $@

${OBJDIR}/lzmat_enc.o:
	$(GPP) -c ${LZMATDIR}/lzmat_enc.c -o $@
	
${OBJDIR}/endetool.o:
	$(GPP) -c ${SOURCEDIR}/endetool.cpp -I$(AES256DIR) -I$(BASE64DIR) -I$(LZMATDIR) -o $@

${OUTDIR}/${OUTBIN}: ${OBJDIR}/aes256.o ${OBJDIR}/base64.o ${OBJDIR}/lzmat_dec.o ${OBJDIR}/lzmat_enc.o ${OBJDIR}/endetool.o
	$(AR) -q $@ ${OBJDIR}/*.o

clean:
	@rm -rf ${OBJDIR}/*
	@rm -rf ${OUTDIR}/${OUTBIN}
