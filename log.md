Reversion Log(GNU Version)
=====================================
####commit date: 2015/06/13
- Object类在加载之后并不紧接着初始化。
- 每个Class类对象里面必须包含一个VMClass对象。

[详细日志](http://note.youdao.com/share/?id=7cd0c22f1f2d4bfbcac7880d74e426a2&type=note)
####commit date: 2015/06/05
- `非常重要的提交！`
- OBJECT_DATA并没有修改，而是修改了turkeyCopy(),在memcpy时候，加入了`obj->data = obj+1`
- 尽管做了修正，但是依然没有解决clone时候property的查找异常问题。通过测试，在clone之前还是正确的。
- `暂时`对System.class做轻微改动，让`property = Runtime.defaultProperty`, 避免clone。在这种情况下，Turkey可以执行到一个未定义的native方法`getClass()`。
- 总的来说，在clone时还是出现了bug，不是到是不是invokespecial不完善引起的。
- 本次提交暂时将hack过的System.class上传。<br>

- 完成了getClass()，未完成forName()方法。

[详细日志](http://note.youdao.com/share/?id=066ba89456b0350b274829a5dd8fccc7&type=note)


####commit date: 2015/06/04
- `非常重要的提交！本次commit是为下一步修改OBJECT_DATA存档`
- 在turkeycopy()方法之所以出错，是`因为对象里面使用了obj->data来寻找对象的filed`，导致hashtable在copy后，putAllinternal还是在之前obj的data段上修改。
<br>
、-----------------------------------------<br>
|header  *data | data1| data2| data3|<br>
`-------------------------------------------
<br>
由于data指针指向data1， 所以copy之后，对新的obj的操作并没有操作到新obj的data，而还是操作的之前的obj的data。

修改方案：

- 从struct Objcet中删除data指针。
- 修改vm.h里面OBJECT_DATA宏的定义，让访问对象field都根据objref+1的方式寻找data段。

NOTE:

- 本次提交多了两个文件`VMSystem.class`和`System.class`,在System.clss中加入了put()语句，调用VMSystem.class里面自己添加的native方法`testObject()`， 用于测试property是否插入正确。最终测试得到的结果就是在clone()之后，property里面的key-value出现的错误。原因就是上面所说，clone的时候执行的hashTable的`putAllinternal()`,但是并没有操纵到新的obj。

####commit date: 2015/06/03
- 完成了`nativeInit`和`nativeValid`，在nativeInit中将nativeFd初始化成0，通过调用out的privete的私有构造方法，也可以调用其setNativeFd()，效果一样。
- 怀疑invokespecial指令的解释有问题。

####commit date: 2015/05/29
- clang重编译！

####commit date: 2015/05/11
- `VMSystem.makeStandardInputStream();`通过。在`VMSystem.makeStandardOutputStream();`中出现致命bug。
- OPC_GETFIELD里面没有对类型做讨论。
- 下一步要更改OPC_GETFIELD，让对象中longlong与double可以以8字节方式入栈。
<br>------------<br>
- 修改了ARRAY_DATA与OBJECT_DATA。对数组和对象data的访问方式不同。在OPC_GETFIELD和OPC_PUTFIELD中使用OBJECT_DATA。在AALOAD与AASTORE系列中使用ARRAY_DATA。

		ARRAY_DATA(obj, index, type) = *(type*)obj->data+index
		OBJECT_DATA(obj, index, type) = *(type*)(obj->data+index) 
- 在FileDescriptor中nativeFd的值目前不知道如何改变。

		public boolean valid()
		{
			if (nativeFd == -1L)
			return(false);

    		return nativeValid(nativeFd);
		}

####commit date: 2015/05/10
- 在`testvm.c`中加入了`printObjectWrapper()`用于打印对象和数组。从打印出的结果看，buckets在clone之前并不是NULL，而是一个length=191的数组。
- 重写了OPC_INVOKEINTERFACE， `resolveInterfaceMethod`， 和`turkeyCopy()`，System.class中的defaultProperties.clone()通过。
- 目前在`VMSystem.makeStandardInputStream();`中抛出了异常。

####commit date: 2015/05/09
- 修改了class.c的`defineClass()`，之前没有记录interface_count。
- 下一步进行testvm.c的修改。让printObjectWrapper可以分别打印对象和数组。在Objcet里面加入一个el_size字段。

####commit date: 2015/04/22
- 加入cast.c，用于支持instanceof指令。之前没有加入interface_table，为了支持instanceof，要加在class.c中进行修改。
- 每次运行时必须export LD_LIBRARY_PATH=/mnt/hgfs/shared/vm/，不然会segment Fault，原因出在properties的哈希表。详情未知。

####commit date: 2015/04/21
- 遇到了名为在System里面需要加载名为`javalang`的库，为满足执行要求，写了一个`libjavalang.so`。
- 加入了新的LinkedList,用于存放已经加载的dll。
- 对于executeMethod的返回值问题，暂时不做处理。让`setProperty`函数特殊的进行pop操作。

####commit date: 2015/04/19
- 改用`GNU`的classpath，版本为最老的版本0.06.
- 加入了`executeMethodArg()`,用于手动调用方法。
<br>存在问题：手动调用方法不能有返回值，否则会破环先前的栈， 可能会导致栈溢出。 解决方案可以让返回值延迟返回。在`executeMethod`中将ret写回栈，在`executeMethodArg`中不写回栈。