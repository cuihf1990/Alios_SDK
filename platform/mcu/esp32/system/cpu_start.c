#include "esp_attr.h"
#include <rom/ets_sys.h>

void IRAM_ATTR call_start_cpu0()
{
    ets_printf("hello esp32\n");
    while(1);
}

