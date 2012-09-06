#!/bin/bash

#
# Git wrapper for memoizing cloned repositories and implementing
# "pull-all" to update memoized repositories from origin.
#
# Also automatically pulls and updates submodules, if present.
#

GIT_MEMO_FILE=~/.git-pulls
GIT_BINARY=/usr/bin/git

GIT_CMD="$1"

if [ "pull-all" = ${GIT_CMD} ]
then
    echo "Pulling all memoized repositories."
    while read REPO ; do
	pushd ${REPO}
	${GIT_BINARY} pull
	if [ -e .gitmodules ]
	then
		git submodule update
	fi
	popd
    done < ${GIT_MEMO_FILE}
    exit 0
fi

ARGS="$@"

if [ "push" = ${GIT_CMD} ]
then
    ARGS="push --recurse-submodules=check ${@:2}"
fi

${GIT_BINARY} ${ARGS}

if [ $? = 0 ]    
then

    EXTRACT_DIR='s/.*\/\([^\.]*\)\.git.*/\1/'
    if [ "clone" = ${GIT_CMD} ]
    then
	if [ -n "${3}" ]
	then
	    repo_dir="${3}"
	else
	    repo_dir=`echo ${2} | sed ${EXTRACT_DIR}`
	fi
	if [ -e "${repo_dir}/.gitmodules" ]
	then
		echo "Pulling submodules."
		(cd "${repo_dir}"; git submodule init; git submodule update)
	fi 
	memoize_dir=`pwd`/${repo_dir}
	echo "Memoizing clone ${memoize_dir}."
	echo ${memoize_dir} >> ${GIT_MEMO_FILE}
    fi

fi
