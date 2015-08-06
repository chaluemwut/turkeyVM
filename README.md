TURKEY VM
=================

Turkey is a Java virtual mechine. 

Turkey is base on [GNU classpath 0.0.6](http://savannah.gnu.org/forum/forum.php?forum_id=2466).


## Getting Started
first, you need add lib/ into the CLASSPATH.

	export CLASSPATH=$CLASSPATH:/mnt/hgfs/share/vm/lib

then, youo need set the LD_LIBRARY_PATH

	export LD_LIBRARY_PATH=/mnt/hgfs/share/vm/dll

finally, 

	cd src
	cmake CMakeList.txt
	make

This will retrieve the library and compile the turkey into src/.Then run
	
	./turkey ../test/Test

will get the result like this

	hello,world

	VM run 0.054671 seconds

###Command-line

	-verbose     {0|1|2|3}      verbose turkey
  	-trace       {name}         trace specific method
  	-help        <NULL>         commandline list
  	-disv        <NULL>         display vtable
  	-dish        <NULL>         display list
  	-disb        <NULL>         display bytecode
  	-test        <NULL>         super test!!


