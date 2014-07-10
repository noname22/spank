#!/bin/bash

run() # [directory] [expected return code rebuild] [expected return code  build] [expected return code  clean]
{
	echo -n "  $1 ... "
	spank=`realpath ../spank`

	cd "$1"
	
	$spank --verbosity 4 rebuild > /dev/null 2> /dev/null
	out=$?
	if [ "$out" != "$2" ]; then
		echo ""
		echo "expected $2 but got $out"
		exit 1;
	fi

	
	if [ "$3" != "" ]; then
		$spank --verbosity 4 build > /dev/null 2> /dev/null 
		out=$?
		if [ "$out" != "$3" ]; then
			echo ""
			echo "expected $3 but got $out"
			exit 1;
		fi
	fi
	
	if [ "$4" != "" ]; then
		$spank --verbosity 4 clean > /dev/null 2> /dev/null
		out=$?
		if [ "$out" != "$4" ]; then
			echo ""
			echo "expected $4 but got $out"
			exit 1;
		fi
	fi
		
	cd - > /dev/null

	echo "ok"
}

run 'directory with space' 0 0 0
run 'gcc' 0 0 0
run 'gcc-static-lib/lib' 0
run 'gcc-static-lib/app' 0 0 0
run 'gcc-static-lib/lib' 0 0 0
run 'installer' 0 0 0
run 'mcs' 0 0 0
run 'mixed_sources' 0 0 0
run 'newfile' 0 0 0
run 'sections' 0 0 0
run 'vala' 0 0 0
run 'compilation-failed' 1 1 0
run 'unknown-option' 1 1 1
run 'skipstep' 0 0 0

echo "all tests passed"
