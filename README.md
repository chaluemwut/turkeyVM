TURKEY VM
=================

Turkey is a Java virtual mechine. 

Turkey is base on [GNU classpath 0.0.6](http://savannah.gnu.org/forum/forum.php?forum_id=2466).

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

This will retrieve the library and compile the turkey into src/.Then run
	
	./turkey ../test/Test

will get the result like this

	hello,world
	1+2+...+100=5050

	VM run 0.054671 seconds

also, you can run 

	./turkey/LinkedList

magic!!!

###Command-line

	-verbose     {0|1|2|3}      verbose turkey
  	-trace       {name}         trace specific method
  	-help        <NULL>         commandline list
  	-disv        <NULL>         display vtable
  	-dish        <NULL>         display list
  	-disb        <NULL>         display bytecode
  	-test        <NULL>         super test!!


