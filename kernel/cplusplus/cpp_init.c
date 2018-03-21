#if defined(__GNUC__)
typedef void (*cpp_func) ();
extern pfunc __ctors_start__[];
extern pfunc __ctors_end__[];

void cpp_init(void)
{
    cpp_func *f;

    for (f = __ctors_start__; f < __ctors_end__; f++) {
        (*f)();
    }
}
#endif

