#!/bin/bash

run() # [directory] [expected output rebuild] [expected output build] [expected output clean]
{
	echo -n "  $1 ... "
	spank=`realpath ../spank`

	cd "$1"
	
	$spank --verbosity 4 rebuild
	out=$?
	if [ "$out" != "$2" ]; then
		echo ""
		echo "expected $2 but got $out"
		exit 1;
	fi

	
	if [ "$3" != "" ]; then
		$spank --verbosity 4 build 
		out=$?
		if [ "$out" != "$3" ]; then
			echo ""
			echo "expected $3 but got $out"
			exit 1;
		fi
	fi
	
	if [ "$4" != "" ]; then
		$spank --verbosity 4 clean
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

echo "all tests passed"
