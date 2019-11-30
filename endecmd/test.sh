#!/bin/bash
KEY="12345678901234567890123456789012"
SRC="Testing_source_words_for_libendetool_and_endecmd."
ENDECMD=bin/endecmd
#TESTRUN=$(shell ${ENDECMD} --help)
TESTRUN=$(${ENDECMD} --help)

# test is it compiled.
if [ "${TESTRUN}" == "" ];then
    echo "Build ${ENDECMD} first"
    echo "${TESTRUN}"
    exit 0
fi

ENCDATA=$(${ENDECMD} --key=${KEY} --verboseoff ${SRC})
DECDATA=$(${ENDECMD} --key=${KEY} --decode --verboseoff ${ENCDATA})
COMENCDATA=$(${ENDECMD} --key=${KEY} --verboseoff --compress ${SRC})
COMDECDATA=$(${ENDECMD} --key=${KEY} --verboseoff --decode --compress ${COMENCDATA})


echo "source : ${SRC}"
echo "key : ${KEY}"
echo "encoded : ${ENCDATA}"
echo "decoded : ${DECDATA}"
echo "compressing encoded : ${COMENCDATA}"
echo "compressing decoded : ${COMDECDATA}"
