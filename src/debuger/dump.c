#include <stdio.h>
#include <stdio.h>
#include "dump.h"
#include "../util/jstring.h"
#include "../util/testvm.h"
#include "../lib/assert.h"
#include "../lib/error.h"

#define C Class_t
#define O Object_t



void dumpField(FILE* fp, FieldBlock* fb)
{
    fprintf(fp, "%s ", fb->type);
    fprintf(fp, "%s = ", fb->name);
    switch (fb->type[0])
    {
        case 'J':
            fprintf(fp, "%lld\n", *(long long*)&fb->static_value);
            break;
        case 'D':
            fprintf(fp, "%lf\n", *(double*)&fb->static_value);
            break;
        case 'F':
            fprintf(fp, "%f\n", *(float*)&fb->static_value);
            break;
        default:
            fprintf(fp, "%d\n", fb->static_value);
            break;
    }
}

void dumpClass(FILE* fp, char* classname, C class)
{
#define INDENT(x)                       \
    do{                                 \
        int indent = x;                 \
        while(indent--)                 \
            fprintf(fp, "%s", " ");     \
    }while(0)

    Assert_ASSERT(classname||class);
    if (class == NULL)
        class  = findClass(classname);
    if (class == NULL)
      return;
    ClassBlock* cb = CLASS_CB(class);
    fprintf(fp, "\ndumpClass>%s\n", cb->this_classname);
    fprintf(fp, "super class:%s\n", cb->super_classname);

    int i = 0;
    for (; i<cb->fields_count; i++)
    {
        FieldBlock* fb = &cb->fields[i];
        if (fb->access_flags&ACC_STATIC)
        {
            INDENT(2);
            dumpField(fp, fb);
        }
    }

    fprintf(fp, "Methods count:%d\n", cb->methods_count);
    i = 0;
    for (; i<cb->methods_count; i++)
    {
        INDENT(2);
        MethodBlock* mb = &cb->methods[i];
        fprintf(fp, "%s|%s\n", mb->name, mb->type);
    }

    fprintf(fp, "Object size:%d\n", cb->obj_size);

#undef INDENT
}


void dumpObject(O obj)
{
    Assert_ASSERT(obj);
    Assert_ASSERT(obj->type != TYPE_ARRAY);

    switch (obj->type)
    {
        case TYPE_OBJECT:
            printObject(obj);
            break;
        case TYPE_STRING:
            dumpJstring(obj);
            break;
        default:
            ERROR("wrong type");
            break;
    }
}


#undef C
#undef O