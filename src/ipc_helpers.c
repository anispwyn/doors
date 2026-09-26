#include "ipc_helpers.h"
#include "server.h"
#include "transaction.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// from ipc.c
void send_success(int client_fd, const char *msg);
void send_failure(int client_fd, const char *msg);

bool streq(const char *a, const char *b) {
	return strcmp(a, b) == 0;
}

// parse a hex-digit character
static int ipc_parse_hex_digit(char c) {
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return 10 + (c - 'a');
	if (c >= 'A' && c <= 'F')
		return 10 + (c - 'A');
	return 0;
}

// parse a single RRGGBBAA or RRGGBB color into a float[4]
bool ipc_parse_color(const char *hex, float *rgba) {
	if (!hex)
		return false;
	if (*hex == '#')
		hex++;
	size_t len = strlen(hex);
	if (len != 6 && len != 8)
		return false;
	rgba[0] = (float)(ipc_parse_hex_digit(hex[0]) * 16 + ipc_parse_hex_digit(hex[1])) / 255.0f;
	rgba[1] = (float)(ipc_parse_hex_digit(hex[2]) * 16 + ipc_parse_hex_digit(hex[3])) / 255.0f;
	rgba[2] = (float)(ipc_parse_hex_digit(hex[4]) * 16 + ipc_parse_hex_digit(hex[5])) / 255.0f;
	rgba[3] = (len == 8) ? (float)(ipc_parse_hex_digit(hex[6]) * 16 + ipc_parse_hex_digit(hex[7])) /
		255.0f : 1.0f;
	return true;
}

bool ipc_parse_color_float(const char *str, float rgba[4]) {
	if (!str)
		return false;
	rgba[3] = 1.0f;
	int n = sscanf(str, "%f %f %f %f", &rgba[0], &rgba[1], &rgba[2], &rgba[3]);
	return n >= 3;
}

void ipc_format_color_float(char *buf, size_t bufsz, const float rgba[4]) {
	snprintf(buf, bufsz, "%.3f %.3f %.3f %.3f\n", rgba[0], rgba[1], rgba[2], rgba[3]);
}

bool ipc_parse_gradient(const char *str, float out[BORDER_GRADIENT_MAX_STOPS * 4], int *count,
		float *angle_out) {
	if (!str)
		return false;
	char buf[512];
	strncpy(buf, str, sizeof(buf) - 1);
	buf[sizeof(buf) - 1] = '\0';

	*count = 0;
	*angle_out = 0.0f;

	char *tok = strtok(buf, " \t");
	while (tok) {
		size_t tlen = strlen(tok);
		if (tlen > 3 && strcmp(tok + tlen - 3, "deg") == 0) {
			char tmp[16];
			strncpy(tmp, tok, sizeof(tmp) - 1);
			tmp[tlen - 3] = '\0';
			*angle_out = (float)atof(tmp) * 3.14159265f / 180.0f;
		} else if (tlen >= 6) {
			if (*count >= BORDER_GRADIENT_MAX_STOPS)
				break;
			float rgba[4];
			if (ipc_parse_color(tok, rgba)) {
				memcpy(out + (*count) * 4, rgba, 4 * sizeof(float));
				(*count)++;
			}
		}
		tok = strtok(NULL, " \t");
	}
	return (*count >= 1);
}

// serialise a gradient back to a string for IPC get queries.
void ipc_format_gradient(char *buf, size_t bufsz, const float *colors, int count, float angle) {
	buf[0] = '\0';
	for (int i = 0; i < count; i++) {
		char tmp[20];
		unsigned r = (unsigned)(colors[i * 4 + 0] * 255.0f + 0.5f);
		unsigned g = (unsigned)(colors[i * 4 + 1] * 255.0f + 0.5f);
		unsigned b = (unsigned)(colors[i * 4 + 2] * 255.0f + 0.5f);
		unsigned a = (unsigned)(colors[i * 4 + 3] * 255.0f + 0.5f);
		snprintf(tmp, sizeof(tmp), "%02x%02x%02x%02x ", r, g, b, a);
		strncat(buf, tmp, bufsz - strlen(buf) - 1);
	}
	char tmp[20];
	snprintf(tmp, sizeof(tmp), "%ddeg", (int)(angle * 180.0f / 3.14159265f));
	strncat(buf, tmp, bufsz - strlen(buf) - 1);
}

bool ipc_handle_bool(char **args, int num, int client_fd, bool *var, int flags) {
	if (num >= 2) {
		*var = streq(args[1], "true") || streq(args[1], "on") || streq(args[1], "1");
		if (flags & IPC_FLAG_COMMIT)
			transaction_commit_dirty();
		char msg[128];
		snprintf(msg, sizeof(msg), "%s set\n", args[0]);
		send_success(client_fd, msg);
		return true;
	}
	send_success(client_fd, *var ? "true\n" : "false\n");
	return false;
}

bool ipc_handle_int(char **args, int num, int client_fd, int *var, int flags, int min, int max,
		const char *errmsg) {
	if (num >= 2) {
		int val = atoi(args[1]);
		if (val < min || val > max) {
			if (errmsg) {
				char buf[128];
				snprintf(buf, sizeof(buf), "config %s: %s\n", args[0], errmsg);
				send_failure(client_fd, buf);
				return false;
			}
			if (val < min)
				val = min;
			if (val > max)
				val = max;
		}
		*var = val;
		if (flags & IPC_FLAG_COMMIT)
			transaction_commit_dirty();
		char msg[128];
		snprintf(msg, sizeof(msg), "%s set\n", args[0]);
		send_success(client_fd, msg);
		return true;
	}
	char buf[64];
	snprintf(buf, sizeof(buf), "%d\n", *var);
	send_success(client_fd, buf);
	return false;
}

bool ipc_handle_float(char **args, int num, int client_fd, float *var, int flags, float min,
		float max, const char *fmt, const char *errmsg) {
	if (num >= 2) {
		float val = (float)atof(args[1]);
		if (val < min || val > max) {
			if (errmsg) {
				char buf[128];
				snprintf(buf, sizeof(buf), "config %s: %s\n", args[0], errmsg);
				send_failure(client_fd, buf);
				return false;
			}
			if (val < min)
				val = min;
			if (val > max)
				val = max;
		}
		*var = val;
		if (flags & IPC_FLAG_COMMIT)
			transaction_commit_dirty();
		char msg[128];
		snprintf(msg, sizeof(msg), "%s set\n", args[0]);
		send_success(client_fd, msg);
		return true;
	}
	char buf[64];
	snprintf(buf, sizeof(buf), fmt, *var);
	send_success(client_fd, buf);
	return false;
}

node_t *ipc_focused_node(int client_fd, const char *ctx, output_t **mon, desktop_t **desk) {
	output_t *m = server.focused_output;
	desktop_t *d = m ? m->desk : NULL;
	node_t *n = d ? d->focus : NULL;
	if (mon)
		*mon = m;
	if (desk)
		*desk = d;

	char msg[128];
	if (!d) {
		snprintf(msg, sizeof(msg), "%s: no focused desktop\n", ctx);
		send_failure(client_fd, msg);
		return NULL;
	}
	if (!n) {
		snprintf(msg, sizeof(msg), "%s: no focused node\n", ctx);
		send_failure(client_fd, msg);
		return NULL;
	}
	return n;
}

bool ipc_handle_enum(char **args, int num, int client_fd, void *var, size_t varsize,
		const cfg_enum_value_t *values, size_t nvalues, const char *errmsg) {
	assert(varsize == sizeof(int));
	int cur;
	memcpy(&cur, var, sizeof(cur));

	char buf[128];
	if (num < 2) {
		const char *name = NULL;
		for (size_t i = 0; i < nvalues; i++) {
			if (values[i].value == cur) {
				name = values[i].name;
				break;
			}
		}
		// fall back to the raw value if it is outside the table
		if (name != NULL)
			snprintf(buf, sizeof(buf), "%s\n", name);
		else
			snprintf(buf, sizeof(buf), "%d\n", cur);
		send_success(client_fd, buf);
		return false;
	}

	for (size_t i = 0; i < nvalues; i++) {
		if (streq(values[i].name, args[1])) {
			int next = (int)values[i].value;
			memcpy(var, &next, sizeof(next));
			snprintf(buf, sizeof(buf), "%s set\n", args[0]);
			send_success(client_fd, buf);
			return true;
		}
	}

	send_failure(client_fd, errmsg);
	return false;
}

bool ipc_handle_rgba(char **args, int num, int client_fd, float rgba[4], const char *errmsg) {
	if (num < 2) {
		char buf[128];
		ipc_format_color_float(buf, sizeof(buf), rgba);
		send_success(client_fd, buf);
		return false;
	}

	float parsed[4];
	if (!ipc_parse_color_float(args[1], parsed)) {
		send_failure(client_fd, errmsg);
		return false;
	}
	memcpy(rgba, parsed, sizeof(parsed));
	char msg[128];
	snprintf(msg, sizeof(msg), "%s set\n", args[0]);
	send_success(client_fd, msg);
	return true;
}
