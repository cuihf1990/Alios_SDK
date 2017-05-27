#include <stdlib.h>
#include <yts.h>

int application_start(int argc, char **argv)
{
    yts_run(argc, argv);
    yos_kv_deinit();
    exit(0);
}

