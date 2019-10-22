#!/bin/bash
#
# This script builds a make command line in a convenient way.  Terse
# options are expanded into a valid make command line.
#
# This script is ABSOLUTELY NOT NECESSARY to build the project.  All of
# the essential work is done in the makefile.  Sorry for shouting, but I
# have had some people misunderstand the purpose of this script and I
# want to be clear about it.  I agree: the makefile should be all you
# need to build a project.  And it is.
#
# Not only does this build command lines for make, it also automatically
# runs the make command piped through tee to save the output of the
# build.  By default this output file is called "make.m".  I use a text
# editor to examine the errors in make.m, and I have a macro that parses
# an error message, opening the source file and jumping to the line of
# the error.  I find this very convenient indeed.
#
# Examples of use:
#
# mm Default build, does the same thing as "make DEBUG=1"
#
# mm -f Does the same thing as "make clean; make DEBUG=1"
#
# mm -s Does the same thing as "make NDEBUG=1"


ssArgs=""
ssErr="mm: Error: build failed"

fBuildInShip=0
fClean=0
fCleanOnly=0
fDebug=1
fEchoOnly=0
fIgnoreErrs=0
fProfile=0
fTimeStamp=0
fVerbose=0

ffLog=make.m



while true; do
	# -d: Debug build
	if test "($1)" == "(-d)"; then
		fDebug=1
		fBuildInShip=0
		shift
		continue
	fi

	# -e: Echo only (don't actually build)
	if test "($1)" == "(-e)"; then
		fEchoOnly=1
		shift
		continue
	fi

	# -f: Force build, forces every file to be re-built
	if test "($1)" == "(-f)"; then
		echo "Force build: delete all files, then build"
		fClean=1
		shift
		continue
	fi

	# -i: Ignore errors, tells make to ignore errors
	if test "($1)" == "(-i)"; then
		fIgnoreErrs=1
		shift
		continue
	fi

	# -l: Log file (specify a different output log than "make.m")
	if test "($1)" == "(-l)"; then
		ffLog=$2
		shift
		shift
		continue
	fi

	# -p: Profile build; sets PROFILE=1; it's up to the makefile whether
	# this will do anything or not.
	if test "($1)" == "(-p)"; then
		fProfile=1
		shift
		continue
	fi

	# -s: Ship build; sets NDEBUG=1 and does not set DEBUG=1.
	if test "($1)" == "(-s)"; then
		fDebug=0
		fBuildInShip=1
		shift
		continue
	fi

	# -ts: TimeStamp; includes start time and end time in build log
	if test "($1)" == "(-ts)"; then
		fTimeStamp=1
		shift
		continue
	fi

	# -v: Verbose; echoes commands before running them
	if test "($1)" == "(-v)"; then
		fVerbose=1
		shift
		continue
	fi

	# clean: just clean, don't build anything
	if test "($1)" == "(clean)"; then
		fClean=1
		fCleanOnly=1
		shift
		continue
	fi

	break
done



if test "($fDebug)" == "(1)"; then
	ssArgs="$ssArgs DEBUG=1"
else
	ssArgs="$ssArgs NDEBUG=1"
fi

if test "($fIgnoreErrs)" == "(1)"; then
	ssArgs="-i $ssArgs"
fi

if test "($fProfile)" == "(1)"; then
	ssArgs="$ssArgs PROFILE=1"
fi

MAKE="make"
ssMakeArgs="$ssArgs $*"
ssClean="$MAKE $ssMakeArgs clean"
ssMakeCmd="$MAKE $ssMakeArgs 2>&1 | tee -a $ffLog"


if test "$fEchoOnly" == "1"; then
	echo $sMakeCmd
	exit 0
fi



if test "$fClean" == "1"; then
	if test "$fBuildInShip" == "1"; then
		echo "Cleaning SHIP directory..."
	else
		echo "Cleaning directory..."
	fi
	if test "$fVerbose" == "0"; then
		$MAKE $ssMakeArgs clean 2>&1 > /dev/null
	else
		echo $MAKE $ssMakeArgs clean
		$MAKE $ssMakeArgs clean
	fi
	test -f make.m && rm -f make.m
	test -f $ffLog && rm -f $ffLog
fi

if test "$fCleanOnly" == "1"; then
	exit 0
fi

echo $ssMakeCmd


if test "$fTimeStamp" == "1"; then
	echo "Build started:" `date` > $ffLog
else
	rm -f $ffLog
	touch $ffLog
fi

if test "$fVerbose" == "1"; then
	echo "$MAKE $ssMakeArgs 2>&1 | tee -a $ffLog || echo $ssErr | tee -a $ffLog"
fi
$MAKE $ssMakeArgs 2>&1 | tee -a $ffLog || echo $ssErr | tee -a $ffLog

nStatus=$?


if test "$fTimeStamp" == "1"; then
	echo "Build finished:" `date` >> $ffLog
fi

exit $nStatus
