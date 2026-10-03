#include "debug.h"
#include "tiny_monitor.h"

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Delay_Init();
    tiny_monitor_init();
    tiny_monitor_run();
    return 0;
}
