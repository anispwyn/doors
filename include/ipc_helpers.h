#pragma once

#include "output.h"
#include "types.h"

#include <stdbool.h>

#define IPC_FLAG_NONE 0
#define IPC_FLAG_COMMIT (1 << 0)

bool streq(const char *a, const char *b);

bool ipc_parse_color(const char *hex, float *rgba);
bool ipc_parse_color_float(const char *str, float rgba[4]);
void ipc_format_color_float(char *buf, size_t bufsz, const float rgba[4]);
bool ipc_parse_gradient(const char *str, float out[BORDER_GRADIENT_MAX_STOPS * 4], int *count,
	float *angle_out);
void ipc_format_gradient(char *buf, size_t bufsz, const float *colors, int count, float angle);

bool ipc_handle_bool(char **args, int num, int client_fd, bool *var, int flags);
bool ipc_handle_int(char **args, int num, int client_fd, int *var, int flags, int min, int max,
	const char *errmsg);
bool ipc_handle_float(char **args, int num, int client_fd, float *var, int flags, float min,
	float max, const char *fmt, const char *errmsg);

typedef struct {
	const char *name;
	long value;
} cfg_enum_value_t;

bool ipc_handle_enum(char **args, int num, int client_fd, void *var, size_t varsize,
	const cfg_enum_value_t *values, size_t nvalues, const char *errmsg);

bool ipc_handle_rgba(char **args, int num, int client_fd, float rgba[4], const char *errmsg);

node_t *ipc_focused_node(int client_fd, const char *ctx, output_t **mon, desktop_t **desk);
