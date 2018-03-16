#include <stdio.h>
#include "new.h"
#include "interface/abstract_class.h"

static const char string_ctor_dtor_pattern[] = "%s %s @%p\n";
static const char string_ctor[] = "constructed";
static const char string_dtor[] = "destructed";

void* new_object(const void* _class, ...)
{
    const abstract_class_t* ab_class = _class;

    void* p = calloc(1, ab_class->_size);

    if (p == NULL) return NULL;

    *(const abstract_class_t**)p = ab_class;

    if (ab_class->ctor) {
        va_list params;

        va_start(params, _class);
        p = ab_class->ctor(p, &params);
        va_end(params);
    }

    printf(string_ctor_dtor_pattern, ab_class->_class_name, string_ctor, p);

    return p;
}

void delete_object(void* _object)
{
    const abstract_class_t** ab_class = _object;

    if (_object && *ab_class && (*ab_class)->dtor) {
        _object = (*ab_class)->dtor(_object);

        printf(string_ctor_dtor_pattern, (*ab_class)->_class_name, string_dtor, _object);

        free(_object);
    }
}
