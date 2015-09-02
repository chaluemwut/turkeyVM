#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../main/turkey.h"
#include "resolve.h"
#include "class.h"
#include "../control/control.h"
#include "../util/exception.h"
#include "../interp/stackmanager.h"
#include "../lib/error.h"
#include "../lib/assert.h"

#define C Class_t
#define JF JFrame_t

/*
 * Find field in current class.
 */
static FieldBlock_t *findFieldinCurrent(C class, char *name, char *type)
{
    ClassBlock_t *cb = CLASS_CB(class); /*{{{ */
    int i;

    for (i = 0; i < cb->fields_count; i++) {
        FieldBlock_t *fb = &cb->fields[i];
        if ((strcmp(fb->name, name) == 0) && (strcmp(fb->type, type) == 0))
            return fb;
    }
    return 0;                   /*}}} */
}

/*
 * Find field in current, if not, find in super recursivly.
 */
FieldBlock_t *findField(C class, char *name, char *type)
{
    if (class == NULL)          /*{{{ */
        DEBUG("NULLPointer error");

    ClassBlock_t *cb = CLASS_CB(class);
    FieldBlock_t *fb = NULL;
    fb = findFieldinCurrent(class, name, type);
    if (fb != NULL)
        return fb;
    if (!cb->super)
        return fb;
    return findField(cb->super, name, type); /*}}} */
}

/*
 * Given the Fieldinfo_idx, and find the field from the given
 * class resursivly. Finally, return the offset of the field
 * for a certain object. The objectref has been in the stack.
 *
 * @parm class current class
 * @parm CONSTANT_Fieldref_info index in current constant pool
 *
 * note: the offset start with 1. Take 0 as failure case. So
 *       before use the offset, remember offset-=1.
 * @qcliu 2015/01/30
 */
FieldBlock_t *resolveField(C class, u2 index)
{
    /*{{{ */
    FieldBlock_t *fb;
    ConstantPool_t *const_pool;
    u2 name_type_idx;
    u2 symclass_idx;
    u2 name_idx;
    u2 type_idx;
    u4 fieldref_info;
    u4 name_type_info;
    char *name;
    char *type;
    C sym_class;

    const_pool = &(CLASS_CB(class))->constant_pool;
    switch (CP_TYPE(const_pool, index)) {
    case CONSTANT_Fieldref:
        fieldref_info = CP_INFO(const_pool, index);
        symclass_idx = fieldref_info;
        sym_class = (C) resolveClass(class, symclass_idx);
        name_type_idx = fieldref_info >> 16;
        name_type_info = CP_INFO(const_pool, name_type_idx);
        name_idx = name_type_info;
        type_idx = name_type_info >> 16;
        name = CP_UTF8(const_pool, name_idx);
        type = CP_UTF8(const_pool, type_idx);
        fb = findField(sym_class, name, type);
        if (Control_resolve) {
            *(FieldBlock_t **) & CP_INFO(const_pool, index) = fb;
            CP_TYPE(const_pool, index) = RESOLVED;
        }
        break;
    case RESOLVED:
        fb = (FieldBlock_t *) CP_INFO(const_pool, index);
        break;
    default:
        ERROR("impossible");
    }
    return fb;                  /*}}} */
}

/*
 * Find method in current class.
 * If not found, return NULL.
 */
MethodBlock_t *findMethodinCurrent(C class, char *name, char *type)
{
    ClassBlock_t *cb = CLASS_CB(class); /*{{{ */
    int i;

    for (i = 0; i < cb->methods_count; i++) {
        MethodBlock_t *mb = &cb->methods[i];
        if ((strcmp(mb->name, name) == 0) && (strcmp(mb->type, type) == 0))
            return mb;
    }
    return NULL;                /*}}} */
}

/*
 * find the method in current class, if not ,find in super recursivly.
 * note: find the method in ClassBlock_t, not the vtable.
 */
MethodBlock_t *findMethod(C class, char *name, char *type)
{
    ClassBlock_t *cb = CLASS_CB(class); /*{{{ */
    MethodBlock_t *mb = findMethodinCurrent(class, name, type);

    if (mb != NULL)
        return mb;
    if (!cb->super)
        return mb;
    return findMethod(cb->super, name, type); /*}}} */
}

/**
 * Find method in vtable.
 * @see exe_OPC_INVOKEVIRTUAL()
 */
MethodBlock_t *quickSearch(C class, char *name, char *type)
{
    /*{{{ */
    ClassBlock_t *cb;
    MethodBlock_t **methods_table;
    u2 methods_table_size;

    cb = CLASS_CB(class);
    methods_table = cb->methods_table;
    methods_table_size = cb->methods_table_size;
    Assert_ASSERT(methods_table);
    int i;
    for (i = 0; i < methods_table_size; i++) {
        MethodBlock_t *mb = *(methods_table + i);
        if (!(0 == strcmp(name, mb->name) && 0 == strcmp(type, mb->type)))
            continue;

        return mb;
    }
    return NULL;
    /*}}} */
}

/*
 * @parm class current class
 * @parm class_idx CONSTANT_Classinfo index in currnet constant pool
 *
 * note: the args means that the index must belong to the
 *       class's cp. So if you want to get an address of
 *       a class but don't know the CONSTANT_Class belong
 *       to which class, you should use loadClass().
 *
 *  @qcliu 2015/01/25
 **/
C resolveClass(C class, u2 class_idx)
{
    C resolve_class;
    ConstantPool_t *const_pool;
    char *classname;

    const_pool = &CLASS_CB(class)->constant_pool;
    switch (CP_TYPE(const_pool, class_idx)) {
    case CONSTANT_Class:
        classname = CP_UTF8(const_pool, CP_INFO(const_pool, class_idx));
        resolve_class = loadClass(classname);

        if (Control_resolve) {
            *(C *) & CP_INFO(const_pool, class_idx) = resolve_class;
            CP_TYPE(const_pool, class_idx) = RESOLVED;
        }
        break;
    case RESOLVED:
        resolve_class = (C) CP_INFO(const_pool, class_idx);
        break;
    default:
        ERROR("impossible");
    }
    return resolve_class;
}

/*
* The interfacemethod is different with normal method. Because
* an interface may have several implementation. So the InterfaceMethodref
* in the current_cp can not determine. This is why HotSpot use "ITable".
*
* In our turkey, we just find method, don't need rewrite the constants_pool.
*
* @qcliu 2015/01/26
*/
MethodBlock_t *resolveInterfaceMethod(C class, u2 index)
{
    /*{{{ */
    MethodBlock_t *resolved_mtd;
    ConstantPool_t *const_pool;
    C sym_class;
    u4 interface_mtdref_info;
    u4 name_type_info;
    u2 name_type_idx;
    u2 symclass_idx;
    u2 name_idx;
    u2 type_idx;
    char *mtd_name;
    char *mtd_type;

    const_pool = &CLASS_CB(class)->constant_pool;
    switch (CP_TYPE(const_pool, index)) {
    case CONSTANT_InterfaceMethodref:
        interface_mtdref_info = CP_INFO(const_pool, index);
        symclass_idx = interface_mtdref_info;
        sym_class = resolveClass(class, symclass_idx);
        Assert_ASSERT(sym_class);
        name_type_idx = interface_mtdref_info >> 16;
        name_type_info = CP_INFO(const_pool, name_type_idx);
        name_idx = name_type_info;
        type_idx = name_type_info >> 16;
        mtd_name = CP_UTF8(const_pool, name_idx);
        mtd_type = CP_UTF8(const_pool, type_idx);
        resolved_mtd = findMethod(sym_class, mtd_name, mtd_type);
        Assert_ASSERT(resolved_mtd);

        if (Control_resolve) {
            *(MethodBlock_t **) & CP_INFO(const_pool, index) = resolved_mtd;
            CP_TYPE(const_pool, index) = RESOLVED;
        }
        break;
    case RESOLVED:
        resolved_mtd = (MethodBlock_t *) CP_INFO(const_pool, index);
        break;
    default:
        ERROR("impossible");
    }

    return resolved_mtd;
    /*}}}*/
}

/*
 * @parm class current class
 * @index CONSTANT_Methodref_index
 *
 * @qcliu 2015/09/02
 */
MethodBlock_t *resolveMethod(C class, u2 index)
{
    MethodBlock_t *resolved_mtd;
    ConstantPool_t *const_pool;
    C sym_class;
    u4 mtdref_info;
    u4 name_type_info;
    u2 name_type_idx;
    u2 symclass_idx;
    u2 name_idx;
    u2 type_idx;
    char *mtd_name;
    char *mtd_type;

    const_pool = &CLASS_CB(class)->constant_pool;
    switch (CP_TYPE(const_pool, index)) {
        case CONSTANT_Methodref:
            mtdref_info = CP_INFO(const_pool, index);
            symclass_idx = mtdref_info;
            sym_class = resolveClass(class, symclass_idx);
            Assert_ASSERT(sym_class);
            name_type_idx = mtdref_info >> 16;
            name_type_info = CP_INFO(const_pool, name_type_idx);
            name_idx = name_type_info;
            type_idx = name_type_info >> 16;
            mtd_name = CP_UTF8(const_pool, name_idx);
            mtd_type = CP_UTF8(const_pool, type_idx);
            resolved_mtd = findMethod(sym_class, mtd_name, mtd_type);
            Assert_ASSERT(resolved_mtd);

            if (Control_resolve) {
                *(MethodBlock_t **) & CP_INFO(const_pool, index) = resolved_mtd;
                CP_TYPE(const_pool, index) = RESOLVED;
            }
            break;
    case RESOLVED:
            resolved_mtd = (MethodBlock_t *) CP_INFO(const_pool, index);
            break;
    default:
        ERROR("impossible");
    }

    return resolved_mtd;
}

u4 resolveConstant(C class, int cp_index)
{
    Assert_ASSERT(class);
    ClassBlock_t *cb = CLASS_CB(class);
    ConstantPool_t *cp = &cb->constant_pool;

    switch (CP_TYPE(cp, cp_index)) {
    case CONSTANT_String:
        TODO("String");
        break;
    default:
        break;
    }

    return CP_INFO(cp, cp_index);
}

#undef C
#undef JF
