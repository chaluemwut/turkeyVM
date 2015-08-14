TURKEY VM
=================

Turkey is a Java Virtual Mechine for [GNU classpath 0.0.6](http://savannah.gnu.org/forum/forum.php?forum_id=2466).

**need: 32bit linux**

## Getting Started
first, you need add `GNUclasspath0.0.6/` into the `CLASSPATH` like this:

	export CLASSPATH=$CLASSPATH:/mnt/hgfs/share/turkey/classpath0.0.6

then, youo need add `dll/` into the `LD_LIBRARY_PATH` like this:

	export LD_LIBRARY_PATH=/mnt/hgfs/share/turkey/dll

finally, 

	cd src
	cmake CMakeList.txt
	make

This will retrieve the library and compile the turkey into src/.


We supply some file in `test/` for test.

	./turkey -cp ../test/bin/ Test

will get the result like this

	test case1: hello, world
	hello,world

	test case2: sum
	1+2+...+100=5050

	test case3: args

	VM run 0.054671 seconds

also, you can give static main args 

	./turkey -cp ../test/bin/ Test arg1 arg2 arg3


you can find all source of test case in `test/`. All test case(except Test.java) are in miniJava lanuage, and they can compile by [tiger-comp](https://github.com/qc1iu/tiger-comp#tiger)**^_^**

##More About Turkey

Besides run this simple test case, turkey now can run  [tiger0.0.6](https://github.com/qc1iu/tiger0.0.6).

	

##Command-line

	-cp          {path}         set class search path
	-log         {name}         log method
	-verbose     {0|1|2|3}      verbose turkey
	-trace       {name}         trace specific method
	-help        <NULL>         commandline list
 	-test        <NULL>         super test!!
