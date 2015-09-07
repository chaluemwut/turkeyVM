TURKEY-VM
=================
[![Build Status](https://drone.io/github.com/qc1iu/turkeyVM/status.png)](https://drone.io/github.com/qc1iu/turkeyVM/latest)
[![Coverage Status](https://coveralls.io/repos/qc1iu/turkeyVM/badge.svg?branch=master&service=github)](https://coveralls.io/github/qc1iu/turkeyVM?branch=master)
[![Licence](http://img.shields.io/badge/Licence-MIT-brightgreen.svg)](LICENSE)
![Version](https://img.shields.io/badge/version-0.1.1-blue.svg)

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
	-classpath   {path}         set classpath
	-dump        {hash|class}   dump turkey status
	-opcode      <NULL>         statistic instrctions
	-log         {name}         log method
	-verbose     {0|1|2|3}      verbose turkey
	-trace       {name}         trace specific method
	-dr          <NULL>         disable resolve
	-help        <NULL>         commandline list
	-test        <NULL>         super test!!



###-opcode
Now, turkey can use **`-opcode`** to statistic the frequency for every instrctions and store the result in `instruction.txt`

	INSTRUCTION                   TIMES          PERCENT
	----------------------------------------------------
	nop                           0              0.00%
	aconst_null                   8522           0.16%
	iconst_m1                     13317          0.24%
	iconst_0                      75574          1.39%
	iconst_1                      32521          0.60%
	...
	...
	...
	new                           19986          0.37%
	newarray                      20147          0.37%
	anewarray                     60             0.00%
	arraylength                   107603         1.97%
	athrow                        0              0.00%
	checkcast                     8699           0.16%
	instanceof                    4247           0.08%
	monitorenter                  11535          0.21%
	monitorexit                   0              0.00%
	wide                          0              0.00%
	multianewarray                0              0.00%
	ifnull                        15376          0.28%
	ifnonnull                     26736          0.49%
	goto_w                        0              0.00%
	jsr_w                         0              0.00%
	----------------------------------------------------
	SUM                           5449389        100%

###-dr
-dr can close the resolve, so we can compare the effect of resolve module.
	
The result bellow is open resolve(default), run `tiger0.0.6` to compile LinkedList.java.

	turkey run 0.597374 seconds
	--------------------------------
	|Hash table status             |
	--------------------------------
	|Num of insertions:  180
	|Num of deletes:     0
	|Num of lookups:     31514
	|Num of links:       48
	|Longest chain:      1
	|Max hash size:      4096
	|Max load factor:    0.043945
	|Expanded times:     0
	|Contraction times:  0
	|Averager position:  0.001523
	|Items nums:         180
	|currentLoad:        0.043945

when use -dr to closed

	turkey run 0.881234 seconds
	--------------------------------
	|Hash table status             |
	--------------------------------
	|Num of insertions:  180
	|Num of deletes:     0
	|Num of lookups:     1147440
	|Num of links:       32851
	|Longest chain:      1
	|Max hash size:      4096
	|Max load factor:    0.043945
	|Expanded times:     0
	|Contraction times:  0
	|Averager position:  0.028630
	|Items nums:         180
	|currentLoad:        0.043945
	--------------------------------
