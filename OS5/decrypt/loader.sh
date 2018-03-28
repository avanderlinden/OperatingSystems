#!/bin/bash

EXECUTABLE=$1
KEYFILE=$2

DEBUG=0

function log_info () {
	if [[ $DEBUG -eq 1 ]]; then
		echo "INFO: $@"
	fi
}

function log_error () {
	echo "ERROR: $@"
}



# check executable exists and is executable
if [ -f $EXECUTABLE ]; then
	if [[ -x "$EXECUTABLE" ]]; then
		log_info "found executable '$EXECUTABLE'"
	else
		log_error "file '$EXECUTABLE' is not executable"
		exit 1
	fi
else 
	log_error "can't find file '$EXECUTABLE'"
	exit 1
fi

# check key file exists and is executable
if [ -f $KEYFILE ]; then
	openssl pkey -inform PEM -pubin -in $KEYFILE -noout &> /dev/null
	if [ $? != 0 ] ; then
		log_error "'$KEYFILE' is not a valid public key"
		exit 1
	else
		log_info "found public key '$KEYFILE'"
	fi
else
	log_error "can't find file '$KEYFILE'"
	exit 1
fi


# check if executable has .sha section
readelf -t $EXECUTABLE | grep '.sha'  &> /dev/null
if [ $? != 0 ] ; then
	log_error "executable '$EXECUTABLE' does not have a .sha section"
	exit 1
fi


# dump digest from .sha section to a file called digest
objcopy --dump-section .sha=digest $EXECUTABLE
if [ $? != 0 ] ; then
	log_error "dumping .sha section failed"
	exit 1
fi

# remove .sha section from executable
objcopy --remove-section .sha $EXECUTABLE "$EXECUTABLE"_unsigned
if [ $? != 0 ] ; then
	log_error "removing .sha section failed"
	exit 1
fi

# verify digest with the public key
openssl dgst -sha256 -verify $KEYFILE -signature ./digest "$EXECUTABLE"_unsigned &> /dev/null
if [ $? != 0 ] ; then
	log_error "verification of digest failed"
	exit 1
fi

# execute the executable
$EXECUTABLE


exit 0
