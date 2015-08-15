/*^_^*--------------------------------------------------------------*//*{{{*/
/* Copyright (C) SSE-USTC, 2014-2015                                */
/*                                                                  */
/*  FILE NAME             :  native.c                               */
/*  LANGUAGE              :  C                                      */
/*  TARGET ENVIRONMENT    :  ANY                                    */
/*  DATE OF FIRST RELEASE :  2015/03/06                             */
/*  DESCRIPTION           :  for the javaVM                         */
/*------------------------------------------------------------------*/

/*
 * Revision log:
 *
 *//*}}}*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../lib/assert.h"
#include "../lib/error.h"
#include "../lib/string.h"
#include "../interp/execute.h"
#include "../interp/interp.h"
#include "../dll/dll.h"
#include "../heapManager/alloc.h"
#include "../main/turkey.h"
#include "../control/control.h"
#include "../util/jstring.h"
#include "../classloader/resolve.h"
#include "../classloader/class.h"
#include "../util/exception.h"
#include "../util/testvm.h"
#include "../interp/stackmanager.h"
#include "javafile.h"
#include "native.h"
#include <sys/utsname.h>
#include "reflect.h"


#define C Class_t
#define O Object_t
#define JF JFrame_t
#define NF NFrame_t

#define THROW throwException("native")
// just for test============
void testObject();
//============================

void nativeWriteBuf();
void identityHashCode();
void constructNative();
void getDeclaredConstructors();
void forName();
void getClass();
void getPrimitiveClass();
void getClass();
void registerNatives();
void insertSystemProperties();
void arrayCopy();
void currentClassLoader();
void nativeGetLibname();
void nativeLoad();
void isWordsBidEndian();
void turkeyCopy();
void nativeInit();
void nativeValid();
void getName(JF retFrame);


/* Binding method name with function*/
Binding nativeMethods[] =
{
    {"getName", "()Ljava/lang/String;", getName},
    {"nativeClose", "(J)J", nativeClose},
    {"nativeReadBuf", "(J[BII)I", nativeReadBuf},
    {"nativeOpen", "(Ljava/lang/String;I)J", nativeOpen},
    {"getPrimitiveClass","(Ljava/lang/String;)Ljava/lang/Class;",getPrimitiveClass},
    {"nativeWriteBuf", "(J[BII)J", nativeWriteBuf},
    {"identityHashCode", "(Ljava/lang/Object;)I", identityHashCode},
    {"constructNative", "([Ljava/lang/Object;Ljava/lang/Class;I)Ljava/lang/Object;", constructNative},
    {"getDeclaredConstructors", "(Z)[Ljava/lang/reflect/Constructor;",getDeclaredConstructors},
    {"forName", "(Ljava/lang/String;)Ljava/lang/Class;", forName},
    {"getClass", "()Ljava/lang/Class;", getClass},
    {"testObject", "(Ljava/lang/Object;)V", testObject},
    {"nativeValid", "(J)Z", nativeValid},
    {"nativeInit", "()V", nativeInit},
    {"clone", "(Ljava/lang/Cloneable;)Ljava/lang/Object;", turkeyCopy},
    {"isWordsBigEndian", "()Z", isWordsBidEndian},
    {"nativeLoad", "(Ljava/lang/String;)I", nativeLoad},
    {"nativeGetLibname", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;", nativeGetLibname},
    {"currentClassLoader", "()Ljava/lang/ClassLoader;", currentClassLoader},
    {"arraycopy", "(Ljava/lang/Object;ILjava/lang/Object;II)V", arrayCopy},
    {"insertSystemProperties", "(Ljava/util/Properties;)V", insertSystemProperties},
    {"registerNatives", "()V", registerNatives},
    {NULL, NULL, NULL},
};


/*VMClass.class*/
/**
 * Get the name of this class, separated by dots for package separators.
 * Primitive types and arrays are encoded as:
 * <pre>
 * boolean             Z
 * byte                B
 * char                C
 * short               S
 * int                 I
 * long                J
 * float               F
 * double              D
 * void                V
 * array type          [<em>element type</em>
 * class or interface, alone: &lt;dotted name&gt;
 * class or interface, as element type: L&lt;dotted name&gt;;
 *
 * @return the name of this class
 * native String getName()
 */
void getName(JF retFrame)
{
    NF n = getNativeFrame();
    O this;//vmClass object
    LOAD(n, this, O, 0);
    Assert_ASSERT(this);
    O classObj = (O)this->binding;
    Assert_ASSERT(classObj);
    C c = classObj->binding;
    Assert_ASSERT(c);

    char* s = CLASSNAME(c);
    Assert_ASSERT(s);
    char* ss = String_new(s);
    int i=0;
    int len = strlen(ss);
    for (i=0; i<len; i++)
    {
        if (ss[i] == '/')
          ss[i] = '.';
    }
    O js = createJstring(ss);
    push(retFrame, &js, TYPE_REFERENCE);
}

/*VMClass.Class */
void identityHashCode(JF retFrame)
{
    NF nframe = getNativeFrame();
    O this;
    LOAD(nframe, this, O, 0);
    Assert_ASSERT(this);
    push(retFrame, &this, TYPE_INT );
}




/**
 * @see java/lang/Constructor.class
 *
 */
void constructNative(JF retFrame)
{
    //JF current_frame = getCurrentFrame();
    NF nframe = getNativeFrame();

    O this;
    O args;//array
    O declaringClass;
    int slot;

    LOAD(nframe, this, O, 0);
    LOAD(nframe, args, O, 1);
    LOAD(nframe, declaringClass, O, 2);
    LOAD(nframe, slot, int , 3);

    Assert_ASSERT(args->type == OBJECT_ARRAY);

    C declclass = declaringClass->binding;
    ClassBlock* cb = CLASS_CB(declclass);
    MethodBlock* mb = &cb->methods[slot];

    O newobj = (O)allocObject(declclass);


    //TODO automatically unwrapped and widened, if needed
    //XXX foucus
    invokeConstructNative(mb, args, newobj);
    push(retFrame, &newobj, TYPE_REFERENCE);
    //current_frame->ostack++;
    //*(O*)current_frame->ostack = newobj;



}

void getDeclaredConstructors(JF retFrame)
{
    //JF current_frame = getCurrentFrame();
    NF nframe = getNativeFrame();

    O vmClass; 
    int isPublic; 
    LOAD(nframe, vmClass, O, 0);
    LOAD(nframe, isPublic, int, 1);

    O cons = getClassConstructors(vmClass, isPublic);

    push(retFrame, &cons, TYPE_REFERENCE);
}

//lang/lang/VMClass.class
void forName(JF retFrame)
{
    //JF current_frame = getCurrentFrame();
    NF nframe = getNativeFrame();

    O name;
    LOAD(nframe, name, O, 0);
    //FIXME: there must be a string Obj, but assert may false
    //Assert_ASSERT(name->type == TYPE_STRING);

    char* classname = Jstring2Char(name);
    //printf("%s\n", classname);

    int len = strlen(classname);
    int i=0;
    for (; i<len; i++)
    {
        if (classname[i] == '.')
            classname[i] = '/';
    }

    C class = (C)loadClass(classname);
    ClassBlock* cb = CLASS_CB(class);

    if (class == NULL)
        throwException("NoSuchClass");

    O cobj = class->class;

    push(retFrame, &cobj, TYPE_REFERENCE);
}

/**
 * @parm classname
 *      like Ljava/io/InputStream
 *
 * @return a class object for this classname
 */
O getClass_name(char* classname)
{
    int len = strlen(classname);
    int i = 0;
    for (; i<len; i++)
    {
        if (classname[i] == '.')
            classname[i] = '/';
    }

    C class = (C)loadClass(classname);
    ClassBlock* cb = CLASS_CB(class);
    if (class == NULL)
        throwException("NoSuchClass");


    O cobj = class->class;

    return cobj;
}

//VMSystem.class
/**
 * this is an extra method, only for test.
 */
void testObject()
{
    NF nframe = getNativeFrame();

    O obj = *(O*)&nframe->locals[0];
    if (obj == NULL)
    {
        printf("obj == null, in testObject");
        throwException("hash false");
    }
    else
    {
        //printString0(obj);
        throwException("hash success");
    }

}




//(Ljava/lang/Cloneable;)Ljava/lang/Object;   VMObject
void turkeyCopy(JF retFrame)
{
    //JF current_frame = getCurrentFrame();
    NF nframe = getNativeFrame();

    O cloneable = *(O*)&nframe->locals[0];
    int copy_size = cloneable->copy_size;
    O obj = (O)sysMalloc(copy_size);
    memset(obj, 0, copy_size);
    memcpy(obj, cloneable, copy_size);

    /*NOTE: !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
    obj->data = (unsigned int*)(obj+1);

    push(retFrame, &obj, TYPE_REFERENCE);
    //current_frame->ostack++;
    //*(O*)current_frame->ostack = obj;
}

void isWordsBidEndian()
{
    JF current_frame = getCurrentFrame();
    current_frame->ostack++;
    *(int*)current_frame->ostack = 0;
}
void nativeLoad()
{
    JF current_frame = getCurrentFrame();
    NF nframe = getNativeFrame();

    O string = *(O*)&nframe->locals[1];
    char* s = Jstring2Char(string);
    int i = resolveDll(s);
    current_frame->ostack++;
    *(int*)current_frame->ostack = i;
}

void nativeGetLibname(JF retFrame)
{
    //JF current_frame = getCurrentFrame();
    NF nframe = getNativeFrame();

    O pathname = *(O*)&nframe->locals[0];
    O libname = *(O*)&nframe->locals[1];
    char* path = Jstring2Char(pathname);
    char* name = Jstring2Char(libname);


    char* lib = getDllName(path, name);
    free(path);
    free(name);

    O s = createJstring(lib);

    push(retFrame, &s, TYPE_REFERENCE);
    //current_frame->ostack++;
    //*(O*)current_frame->ostack = s;
    free(lib);
}

void currentClassLoader(JF retFrame)
{
    //JF current_frame = getCurrentFrame();
    int r = 0;
    push(retFrame, &r, TYPE_INT);
    //current_frame->ostack++;
    //*current_frame->ostack = 0;
}



/**
 *
 *
 */
void setProperty(O this, char* key, char* value)
{
    JF current_frame = getCurrentFrame();
    O k = createJstring(key);
    O v = createJstring(value);
    // char* s = Jstring2Char(k);
    // char* t = Jstring2Char(v);

    MethodBlock* mb = (MethodBlock*)findMethod(this->class, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
    if (mb == NULL)
        throwException("no such method");

    assert_stack = FALSE;
    executeMethodArgs(this->class, mb,this, k, v);

    /*hack*/
    //NOTE: after executeMethod, stack is not correct, so we need revise it.
    current_frame->ostack--;
    assert_stack = TRUE;
}

static int times;
void arrayCopy(JF retFrame)
{
    //printf("native arrayCopy----------------------------------------\n");
    //times++;
    //printf("%d\n", times);
    //if (times == 38)
    //WARNING("arrayCopy");
    NF nf = getNativeFrame();

    int size;
    O src;
    int srcStart;
    O dest;
    int destStart;
    int length;
    LOAD(nf, src, O, 0);
    LOAD(nf, srcStart, int, 1);
    LOAD(nf, dest, O, 2);
    LOAD(nf, destStart, int, 3);
    LOAD(nf, length, int, 4);

    Assert_ASSERT(src);
    Assert_ASSERT(dest);
    Assert_ASSERT(srcStart>=0);
    Assert_ASSERT(destStart>=0);
    Assert_ASSERT(length>=0);
    //printf("strStart:%d\n", srcStart);
    //printf("destStart:%d\n", destStart);
    //printf("length:%d\n", length);

    if ((src == NULL)||(dest == NULL))
        throwException("java/lang/NullPointerException");
    else
    {
        ClassBlock* scb = CLASS_CB(src->class);
        ClassBlock* dcb = CLASS_CB(dest->class);

        unsigned int* sdata = src->data;
        unsigned int* ddata = dest->data;

        if ((scb->this_classname[0] != '[') || (dcb->this_classname[0] != '['))
            goto storeExcep;

        size = 0;
        switch (scb->this_classname[1])
        {
        case 'B':
            size = 1;
            break;
        case 'C':
        case 'S':
            size = 2;
            break;
        case 'I':
        case 'F':
        case 'L':
        case '[':
        case 'Z':
            size = 4;
            break;
        case 'J':
        case 'D':
            size = 8;
            break;
        default:
            throwException("type error");
        }


        memmove(((char*)&ddata[0] + destStart * size),
                ((char*)&sdata[0] + srcStart * size),
                length*size
               );
        //printChar0(dest);
        return;


    }



    goto storeExcep;

    return;

storeExcep:
    throwException("java/lang/ArrayStoreException");
}

void insertSystemProperties()
{
    NF nframe = getNativeFrame();

    O  this = *(O*)&nframe->locals[0];

    struct utsname info;
    uname(&info);


    setProperty(this, "java.vm.name", "TurkeyVM");
    setProperty(this, "java.vm.version", "1.0");
    setProperty(this, "java.vm.vendor", "Robert Lougher");
    setProperty(this, "java.vm.vendor.url", "http://jamvm.sourceforge.net");
    setProperty(this, "java.version", "1.4");
    setProperty(this, "java.vendor", "GNU Classpath");
    setProperty(this, "java.vendor.url", "http://gnu.classpath.org");
    setProperty(this, "java.home", "qcliu");
    setProperty(this, "java.specification.version", "1.4");
    setProperty(this, "java.specification.vendor", "Sun Microsystems, Inc.");
    setProperty(this, "java.specification.name", "Java Platform API Specification");
    setProperty(this, "java.vm.specification.version", "1.0");
    setProperty(this, "java.vm.specification.vendor", "Sun Microsystems, Inc.");
    setProperty(this, "java.vm.specification.name", "Java Virtual Machine Specification");
    setProperty(this, "java.class.version", "48.0");
    setProperty(this, "java.class.path", "");

    setProperty(this, "java.library.path", getDllPath());

    setProperty(this, "java.io.tmpdir", "/tmp");
    setProperty(this, "java.compiler", "tiger");
    setProperty(this, "java.ext.dirs", "");
    setProperty(this, "os.name", info.sysname);
    setProperty(this, "os.arch", info.machine);
    setProperty(this, "os.version", info.release);
    setProperty(this, "file.separator", "/");
    setProperty(this, "path.separator", ":");
    setProperty(this, "line.separator", "\n");
    setProperty(this, "user.name", getenv("USER"));
    setProperty(this, "user.home", getenv("HOME"));
    setProperty(this, "user.dir", getenv("PWD"));
}
/*
void longBitsToDouble()
{
    JF current_frame = getCurrentFrame();
    NF nframe = getNativeFrame();

    long long value = *(long long*)&nframe->locals[0];
    double result;
    memcpy(&result, &value, 8);
    current_frame->ostack++;
    *(double*)current_frame->ostack = result;
    current_frame->ostack++;
}

void doubleToRawLongBits()
{
    JF current_frame = getCurrentFrame();
    NF nframe = getNativeFrame();

    double value = *(double*)&nframe->locals[0];
    current_frame->ostack++;
    *(double*)current_frame->ostack = value;
    current_frame->ostack++;
}
*/
/* java/lang/Float */
/*
void floatToRawIntBits()
{
    JF current_frame = getCurrentFrame();
    NF nframe = getNativeFrame();

    //static
    float value = *(float*)&nframe->locals[0];
    current_frame->ostack++;
    *(float*)current_frame->ostack = value;

}
*/
/* java/lang/Class */
/*
void getName0()
{
    JF current_frame = getCurrentFrame();
    NF nframe = getNativeFrame();

    //This must be a java/lang/Class Object
    O obj = (O)nframe->locals[0];
    if (obj->binding == NULL)
        throwException("getName0 error; binding NULL");

    C class = obj->binding;
    ClassBlock* cb = CLASS_CB(class);
    O string = createJstring(cb->this_classname);
    if (string == NULL)
        throwException("getName0 error. string is NULL");

    current_frame->ostack++;
    *(O*)current_frame->ostack = string;
}
*/
/*java/lang/Object*/
void getClass()
{
    JF current_frame = getCurrentFrame();
    NF nframe = getNativeFrame();

    O obj = (O)nframe->locals[0];
    C class = obj->class;
    //ClassBlock* cb = CLASS_CB(class);
    O class_obj = class->class;

    current_frame->ostack++;
    *(O*)current_frame->ostack = class_obj;
}

/*
void fillInStackTrace()
{
    JF current_frame = getCurrentFrame();
    C class = loadClass("java/lang/Throwable");

    if (class == NULL)
        throwException("no find java/lang/Throwable, in native.c");

    O obj = allocObject(class);

    current_frame->ostack++;
    *(O*)current_frame->ostack = obj;
}
*/
/*(Ljava/lang?Class;)Z
 * This method is belong to java/lang/Class
 * assume that is alway return true;
 * @qcliu 2015/03/21
 */
//TODO
/*
void desiredAssertionStatus0()
{
    JF current_frame = getCurrentFrame();
    current_frame->ostack++;
    *current_frame->ostack = 1;
}
*/

/* ()Ljava/lang/ClassLoader;
 * This method is belong to java/lang/Class
 * assume that is alway return 0.
 *
 * @qcliu 2015/03/21
 */
/*
void getClassLoader0()
{
    JF current_frame = getCurrentFrame();
    //TODO
    current_frame->ostack++;
    *current_frame->ostack = 0;
}
*/

void registerNatives(JF retFrame)
{
    JF current_frame = getCurrentFrame();
    ClassBlock* cb = CLASS_CB(current_frame->class);
    //printf("registerNatives, class:%s\n", cb->this_classname);
}

static char getPrimType(char* s)
{
    char primtype;
    if (strcmp(s, "byte")==0)
        primtype = 'B';
    else if (strcmp(s, "short") == 0)
        primtype = 'S';
    else if (strcmp(s, "int") == 0)
        primtype = 'I';
    else if (strcmp(s, "long") == 0)
        primtype = 'J';
    else if (strcmp(s, "char") == 0 )
        primtype = 'C';
    else if (strcmp(s, "float") == 0)
        primtype = 'F';
    else if (strcmp(s, "double") == 0)
        primtype = 'D';
    else if (strcmp(s, "boolean") == 0)
        primtype = 'Z';
    else
        throwException("getPrimType error");

    return primtype;
}



void getPrimitiveClass(JF retFrame)
{
    //JF current_frame = getCurrentFrame();
    NF nframe = getNativeFrame();

    /*In the frame is a String*/
    O obj = (O)nframe->locals[0];
    O array = String_getValue(obj);
    char* ty = Jstring2Char(obj);

    //printf("getPrimitiveClass:%s\n", ty);
    char primtype = getPrimType(ty);

    C class = (C)findPrimitiveClass(primtype);
    if (class != NULL)
    {
        if (!class->class)
            WARNING("classheader is null");
        push(retFrame, &(class->class), TYPE_REFERENCE);
    }
    else
        throwException("getPrimitiveClass");
}

#undef C
#undef O
#undef JF
#undef NF
