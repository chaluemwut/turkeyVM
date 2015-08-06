/*------------------------------------------------------------------*//*{{{*/
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
#include "../interp/execute.h"
#include "../interp/interp.h"
#include "../dll/dll.h"
#include "../heapManager/alloc.h"
#include "../main/turkey.h"
#include "../control/control.h"
#include "../util/string.h"
#include "../classloader/resolve.h"
#include "../classloader/class.h"
#include "../util/exception.h"
#include "../util/testvm.h"
#include "../util/string.h"
#include "../interp/stackmanager.h"
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
void getClassLoader0();
void desiredAssertionStatus0();
void fillInStackTrace();
void getClass();
void getName0();
void registerNatives();
void floatToRawIntBits();
void doubleToRawLongBits();
void longBitsToDouble();

void insertSystemProperties();
void arrayCopy();
void currentClassLoader();
void nativeGetLibname();
void nativeLoad();
void isWordsBidEndian();
void turkeyCopy();
void nativeInit();
void nativeValid();


/* Binding method name with function*/
Binding nativeMethods[] =
{
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


/*FileDescriptor*/
void nativeWriteBuf()
{
    JF current_frame = getCurrentFrame();
    NF nframe = getNativeFrame();

    long long fd = *(long long*)&nframe->locals[1];
    O buf = *(O*)&nframe->locals[3];
    int _offset = nframe->locals[4];
    int len = nframe->locals[5];

    if (!buf->isArray)
        throwException("not array");
    char* p = (char*)buf->data;
    int i;
    for (i=_offset; i<len; i++)
    {
        printf("%c", *p);
        p++;
    }

    current_frame->ostack++;
    *(long long*)current_frame->ostack = 1;
}

/*VMClass.Class */
void identityHashCode()
{

    JF current_frame = getCurrentFrame();
    NF nframe = getNativeFrame();

    O this = *(O*)&nframe->locals[0];

    current_frame->ostack++;
    *(int*)current_frame->ostack = (int)this;
}




/**
 * @see java/lang/Constructor.class
 *
 */
void constructNative()
{

    JF current_frame = getCurrentFrame();
    NF nframe = getNativeFrame();

    O this = *(O*)&nframe->locals[0];

    //a array
    O args = *(O*)&nframe->locals[1];
    //
    O declaringClass = *(O*)&nframe->locals[2];
    //
    int slot = nframe->locals[3];

    C declclass = declaringClass->binding;
    ClassBlock* cb = CLASS_CB(declclass);
    MethodBlock* mb = &cb->methods[slot];
    //char* method_name = mb->name;
    //char* method_type = mb->type;

    O newobj = (O)allocObject(declclass);

    //2015/07/01
    //DEBUG("TODO");
    //exit(0);

    //TODO automatically unwrapped and widened, if needed

    //@TEST
    //printObjectWrapper(args);
    //Object* inputstream = ARRAY_DATA(args, 0, Object*);

    //executeMethodArgs(NULL, mb, newobj,inputstream);

    invoke(mb, args, newobj);
    current_frame->ostack++;
    *(O*)current_frame->ostack = newobj;



}

void getDeclaredConstructors()
{

    JF current_frame = getCurrentFrame();
    NF nframe = getNativeFrame();

    O vmClass = *(O*)&nframe->locals[0];
    int isPublic = nframe->locals[1];

    O cons = getClassConstructors(vmClass, isPublic);

    current_frame->ostack++;
    *(O*)current_frame->ostack = cons;


}

//lang/lang/VMClass.class
void forName()
{
    JF current_frame = getCurrentFrame();
    NF nframe = getNativeFrame();

    O name = *(O*)&nframe->locals[0];

    //printString0(name);

    char* classname = String2Char(name);
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
    //C c = cobj->class;
    //cb = CLASS_CB(c);

    current_frame->ostack++;
    *(O*)current_frame->ostack = cobj;

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

//java/lang/Class
/*void getClass()
  {
  Object* this = *(Object**)&nframe->locals[0];
  C class = this->class;
  Object* class_obj = class->class;

  current_frame->ostack++;
 *(Object**)current_frame->ostack = class_obj;


 }*/

//java/io/FileDescriptor

void nativeValid()
{
    JF current_frame = getCurrentFrame();
    NF nframe = getNativeFrame();

    int ret = TRUE;
    long long nativeFd = *(long long*)&nframe->locals[1];//not static
    if (nativeFd >= 0)
        ret = TRUE;
    else
        ret = FALSE;

    // long needs 2 slot in stack
    current_frame->ostack++;
    *(int*)current_frame->ostack = ret;

}

void nativeInit()
{
    //printf("nativeInit\n");
    //C c = (C)findClassInTable(head, "java/io/FileDescriptor");
    C c = findClass("java/io/FileDescriptor");
    FieldBlock* fb = (FieldBlock*)findField(c, "out", "Ljava/io/FileDescriptor;");
    FieldBlock* fb_err = (FieldBlock*)findField(c, "err", "Ljava/io/FileDescriptor;");
    O err = (O)fb_err->static_value;
    O out = (O)fb->static_value;

    //C class = out->class;
    //ClassBlock* cb = CLASS_CB(class);
    MethodBlock* mb = (MethodBlock*)findMethod(out->class, "<init>", "(J)V");
    if (mb == NULL)
        throwException("no such method!");
    /*hack*/
    executeMethodArgs(out->class, mb, out, 0, 0);

    MethodBlock* mb_err = (MethodBlock*)findMethod(err->class, "<init>", "(J)V");
    if (mb_err == NULL)
        throwException("no such method!");
    executeMethodArgs(err->class, mb_err, err, 0, 0);
}


//(Ljava/lang/Cloneable;)Ljava/lang/Object;   VMObject
void turkeyCopy()
{
    JF current_frame = getCurrentFrame();
    NF nframe = getNativeFrame();

    O cloneable = *(O*)&nframe->locals[0];
    int copy_size = cloneable->copy_size;
    O obj = (O)sysMalloc(copy_size);
    memset(obj, 0, copy_size);
    memcpy(obj, cloneable, copy_size);

    /*NOTE: !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
    obj->data = (unsigned int*)(obj+1);

    current_frame->ostack++;
    *(O*)current_frame->ostack = obj;

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
    char* s = String2Char(string);
    int i = resolveDll(s);
    current_frame->ostack++;
    *(int*)current_frame->ostack = i;
}

void nativeGetLibname()
{
    JF current_frame = getCurrentFrame();
    NF nframe = getNativeFrame();

    O pathname = *(O*)&nframe->locals[0];
    O libname = *(O*)&nframe->locals[1];
    char* path = String2Char(pathname);
    char* name = String2Char(libname);


    char* lib = getDllName(path, name);
    free(path);
    free(name);

    O s = createString(lib);

    current_frame->ostack++;
    *(O*)current_frame->ostack = s;
    free(lib);
}

void currentClassLoader()
{
    JF current_frame = getCurrentFrame();
    current_frame->ostack++;
    *current_frame->ostack = 0;
}



/**
 *
 *
 */
void setProperty(O this, char* key, char* value)
{
    JF current_frame = getCurrentFrame();
    O k = createString(key);
    O v = createString(value);
    // char* s = String2Char(k);
    // char* t = String2Char(v);

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

void arrayCopy()
{
    //printf("native arrayCopy----------------------------------------\n");
    NF nframe = getNativeFrame();

    int size;
    O src = *(O*)&nframe->locals[0];
    int srcStart = nframe->locals[1];
    O dest = *(O*)&nframe->locals[2];
    int destStart = nframe->locals[3];
    int length = nframe->locals[4];

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

/* java/lang/Float */
void floatToRawIntBits()
{
    JF current_frame = getCurrentFrame();
    NF nframe = getNativeFrame();

    //static
    float value = *(float*)&nframe->locals[0];
    current_frame->ostack++;
    *(float*)current_frame->ostack = value;

}

/* java/lang/Class */
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
    O string = createString(cb->this_classname);
    if (string == NULL)
        throwException("getName0 error. string is NULL");

    current_frame->ostack++;
    *(O*)current_frame->ostack = string;
}

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
/*(Ljava/lang?Class;)Z
 * This method is belong to java/lang/Class
 * assume that is alway return true;
 * @qcliu 2015/03/21
 */
//TODO
void desiredAssertionStatus0()
{
    JF current_frame = getCurrentFrame();
    current_frame->ostack++;
    *current_frame->ostack = 1;
}

/* ()Ljava/lang/ClassLoader;
 * This method is belong to java/lang/Class
 * assume that is alway return 0.
 *
 * @qcliu 2015/03/21
 */
void getClassLoader0()
{
    JF current_frame = getCurrentFrame();
    //TODO
    current_frame->ostack++;
    *current_frame->ostack = 0;
}

void registerNatives()
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



void getPrimitiveClass()
{
    JF current_frame = getCurrentFrame();
    NF nframe = getNativeFrame();

    /*In the frame is a String*/
    O obj = (O)nframe->locals[0];
    O array = String_getValue(obj);
    char* ty = String2Char(obj);

    //printf("getPrimitiveClass:%s\n", (char*)array->data);
    char primtype = getPrimType(ty);

    C class = (C)findPrimitiveClass(primtype);
    if (class != NULL)
    {
        current_frame->ostack++;
        *(O*)current_frame->ostack = class->class;
    }
    else
        throwException("getPrimitiveClass");
}

#undef C
#undef O
#undef JF
#undef NF
