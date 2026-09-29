#include "ipc/cmd.h"
#include "ipc/ipc.h"
#include "protocols/global_shortcuts.h"

void ipc_cmd_globalshortcuts(char **args, int num, int client_fd) {
	(void)args;
	(void)num;
	char buf[DOORS_BUFSIZ];
	global_shortcuts_list(buf, sizeof(buf));
	send_success(client_fd, buf);
}
