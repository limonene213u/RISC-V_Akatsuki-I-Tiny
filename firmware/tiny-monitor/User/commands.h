#ifndef TINY_COMMANDS_H
#define TINY_COMMANDS_H
void commands_init(void);
void commands_execute(char *line);
int commands_heartbeat_enabled(void);
#endif
