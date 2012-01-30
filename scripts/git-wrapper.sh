#!/bin/bash

#
# Git wrapper for memoizing cloned repositories and implementing
# "pull-all" to update memoized repositories from origin.
#

GIT_MEMO_FILE=~/.git-pulls
GIT_BINARY=/usr/bin/git

if [ "pull-all" = "$1" ]
then
    echo "Pulling all memoized repositories."
    while read REPO ; do
	pushd ${REPO}
	${GIT_BINARY} pull
	popd
    done < ${GIT_MEMO_FILE}
    exit 0
fi

${GIT_BINARY} $*
if [ $? = 0 ]    
then

    EXTRACT_DIR='s/.*\/\([^\.]*\)\.git.*/\1/'
    if [ "clone" = "$1" ]
    then
	if [ -n "${3}" ]
	then
	    memoize_dir="${3}"
	else
	    memoize_dir=`echo ${2} | sed ${EXTRACT_DIR}`
	fi
	memoize_dir=`pwd`/${memoize_dir}
	echo "Memoizing clone ${memoize_dir}."
	echo ${memoize_dir} >> ${GIT_MEMO_FILE}
    fi

fi
