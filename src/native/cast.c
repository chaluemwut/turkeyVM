/*
 * Copyright (C) 2003 Robert Lougher <rob@lougher.demon.co.uk>.
 *
 * This file is part of JamVM.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <stdio.h>
#include "../main/turkey.h"
#include "cast.h"

#define C Class_t

int implements(C class, C test)
{
    ClassBlock_t *test_cb = CLASS_CB(test);
    int i;

    for (i = 0; i < test_cb->interface_count; i++)
        if ((class == test_cb->interfaces[i]) ||
            implements(class, test_cb->interfaces[i]))
            return TRUE;

    if (test_cb->super)
        return implements(class, test_cb->super);

    return FALSE;
}

int isSubClassOf(C class, C test)
{
    //for(; test != NULL && test != class; test = ((CLASS_CB(test))->super));

    while ((test != NULL) && (test != class)) {
        ClassBlock_t *cb = CLASS_CB(test);
        test = cb->super;
    }
    return test != NULL;
}

int isInstOfArray(C class, C test)
{
    if (isSubClassOf(class, test))
        return TRUE;
    else {
        ClassBlock_t *class_cb = CLASS_CB(class);
        ClassBlock_t *test_cb = CLASS_CB(test);

        if (IS_ARRAY(class_cb) && (test_cb->element != NULL) &&
            (class_cb->element != NULL) && (class_cb->dim == test_cb->dim))
            return isInstanceOf(class_cb->element, test_cb->element);
        else
            return FALSE;
    }
}

/*two args, class is the constants_pool, test is the obj->class*/
int isInstanceOf(C class, C test)
{
    if (class == test)
        return TRUE;

    ClassBlock_t *cb = CLASS_CB(class);
    ClassBlock_t *testcb = CLASS_CB(test);
    if (IS_INTERFACE(cb))
        return implements(class, test);
    else if (IS_ARRAY(cb))
        return isInstOfArray(class, test);
    else
        return isSubClassOf(class, test);
}
