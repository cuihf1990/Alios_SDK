#include <k_api.h>

void k_err_proc(kstat_t err)
{
    if (g_err_proc != NULL) {
        g_err_proc(err);
    }
}

