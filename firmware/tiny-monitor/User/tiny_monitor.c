#include "tiny_monitor.h"
#include "board.h"
#include "commands.h"
#include "console.h"

#define LINE_SIZE 128U

void tiny_monitor_init(void)
{
    board_init();
    console_init();
    console_puts("\r\nAkatsuki I Tiny Monitor\r\nRev0.3\r\n");
    commands_init();
    console_puts("Type 'help' for commands.\r\n\r\ntiny> ");
}

void tiny_monitor_run(void)
{
    char line[LINE_SIZE]; uint32_t heartbeat = 0;
    for (;;) {
        if (console_poll_line(line, sizeof(line))) {
            commands_execute(line);
            console_puts("tiny> ");
        }
        if (++heartbeat >= 300000U) {
            heartbeat = 0;
            if (commands_heartbeat_enabled()) board_led0_toggle();
        }
    }
}
