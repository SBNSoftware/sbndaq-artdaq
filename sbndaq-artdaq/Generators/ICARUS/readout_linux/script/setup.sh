#!/usr/bin/bash
SCRIPT=`realpath ${BASH_SOURCE}`
PATH_TO_SCRIPT=`dirname $SCRIPT`
export PERL5LIB=$PATH_TO_SCRIPT/../macro:$PERL5LIB
export PATH=$PATH_TO_SCRIPT:$PATH_TO_SCRIPT/../macro:$PATH
