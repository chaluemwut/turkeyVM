TURKEY VM
=================

Turkey is a Java virtual mechine. 


## Getting Started

	cd src
	export LD_LIBRARY_PATH=./
	cmake CMakeList.txt
	make

This will retrieve the library and compile the turkey into src/.Then run
	
	./turkey ../test/Test

will get the result like this

	hello,world

	VM run 0.054671 seconds

`-dish` can print all class that be loaded.
	
	./turkey ../test/Test -dish 


