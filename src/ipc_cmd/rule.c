#include "ipc.h"
#include "ipc_cmd.h"
#include "ipc_helpers.h"
#include "rule.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

static const struct {
	const char *key;
	unsigned flag;
} rule_flag_names[] = {
	{"follow", RULE_TYPE_FOLLOW},
	{"focus", RULE_TYPE_FOCUS},
	{"manage", RULE_TYPE_MANAGE},
	{"locked", RULE_TYPE_LOCKED},
	{"hidden", RULE_TYPE_HIDDEN},
	{"maximized", RULE_TYPE_MAXIMIZED},
	{"minimized", RULE_TYPE_MINIMIZED},
	{"sticky", RULE_TYPE_STICKY},
	{"blur", RULE_TYPE_BLUR},
	{"mica", RULE_TYPE_MICA},
	{"acrylic", RULE_TYPE_ACRYLIC},
	{"shadow", RULE_TYPE_SHADOW},
	{"block_out_from_screenshare", RULE_TYPE_BLOCK_OUT_FROM_SCREENSHARE},
	{"allow_tearing", RULE_TYPE_ALLOW_TEARING},
	{"shortcuts_inhibitor", RULE_TYPE_SHORTCUTS_INHIBITOR},
	{"animations_disable", RULE_TYPE_ANIM_DISABLE},
};

static bool rule_flag_apply(rule_t *r, const char *arg) {
	for (size_t i = 0; i < sizeof(rule_flag_names) / sizeof(rule_flag_names[0]); i++) {
		size_t klen = strlen(rule_flag_names[i].key);
		if (strncmp(arg, rule_flag_names[i].key, klen) != 0 || arg[klen] != '=')
			continue;
		const char *val = arg + klen + 1;
		if (streq(val, "on")) {
			r->consequence.flags |= rule_flag_names[i].flag;
			r->consequence.has |= rule_flag_names[i].flag;
		} else if (streq(val, "off")) {
			r->consequence.flags &= ~rule_flag_names[i].flag;
			r->consequence.has |= rule_flag_names[i].flag;
		} else {
			return false;
		}
		return true;
	}
	return false;
}

void ipc_cmd_rule(char **args, int num, int client_fd) {
	if (num < 1) {
		send_failure(client_fd, "rule: missing arguments\n");
		return;
	}

	char *subcmd = *args;

	if (streq("-a", subcmd) || streq("--add", subcmd)) {
		if (num < 2) {
			send_failure(client_fd, "rule -a: missing app_id\n");
			return;
		}

		args++;
		num--;

		rule_t *r = make_rule();
		if (!r) {
			send_failure(client_fd, "rule: failed to create rule\n");
			return;
		}

		char *app_id = NULL;
		char *title = NULL;
		r->consequence.has = RULE_TYPE_FOLLOW | RULE_TYPE_FOCUS | RULE_TYPE_MANAGE;
		r->consequence.flags = RULE_TYPE_FOLLOW | RULE_TYPE_FOCUS | RULE_TYPE_MANAGE;

		while (num > 0) {
			char *arg = *args;

			if (rule_flag_apply(r, arg)) {
				// handled in function
			} else if (strncmp(arg, "title=", 6) == 0) {
				title = arg + 6;
				strncpy(r->match.title, title, MAXLEN - 1);
				r->match.title[MAXLEN - 1] = '\0';
			} else if (strncmp(arg, "tag=", 4) == 0) {
				strncpy(r->match.tag, arg + 4, MAXLEN - 1);
				r->match.tag[MAXLEN - 1] = '\0';
			} else if (strncmp(arg, "app_id=", 7) == 0) {
				app_id = arg + 7;
				strncpy(r->match.app_id, app_id, MAXLEN - 1);
				r->match.app_id[MAXLEN - 1] = '\0';
			} else if (arg[0] != '-' && app_id == NULL && strchr(arg, '=') == NULL) {
				app_id = arg;
				strncpy(r->match.app_id, app_id, MAXLEN - 1);
				r->match.app_id[MAXLEN - 1] = '\0';
			} else if (streq("state=tiled", arg)) {
				r->consequence.state = STATE_TILED;
				r->consequence.has |= RULE_TYPE_STATE;
			} else if (streq("state=floating", arg)) {
				r->consequence.state = STATE_FLOATING;
				r->consequence.has |= RULE_TYPE_STATE;
			} else if (streq("state=fullscreen", arg)) {
				r->consequence.state = STATE_FULLSCREEN;
				r->consequence.has |= RULE_TYPE_STATE;
			} else if (streq("state=pseudo_tiled", arg)) {
				r->consequence.state = STATE_PSEUDO_TILED;
				r->consequence.has |= RULE_TYPE_STATE;
			} else if (streq("desktop=^", arg) || (strlen(arg) > 8 && strncmp(arg, "desktop=", 8) == 0)) {
				char *desk = arg + 8;
				strncpy(r->consequence.desktop, desk, SMALEN - 1);
				r->consequence.desktop[SMALEN - 1] = '\0';
				r->consequence.has |= RULE_TYPE_DESKTOP;
			} else if (streq("one_shot", arg)) {
				r->match.one_shot = true;
			} else if (strncmp(arg, "scroller_proportion=", 20) == 0) {
				float val = atof(arg + 20);
				if (val > 0.0f && val <= 1.0f) {
					r->consequence.scroller_proportion = val;
					r->consequence.has |= RULE_TYPE_SCROLLER_PROPORTION;
				} else {
					send_failure(client_fd, "scroller_proportion must be between 0.0 and 1.0");
					return;
				}
			} else if (strncmp(arg, "scroller_proportion_single=", 27) == 0) {
				float val = atof(arg + 27);
				if (val > 0.0f && val <= 1.0f) {
					r->consequence.scroller_proportion_single = val;
					r->consequence.has |= RULE_TYPE_SCROLLER_PROPORTION_SINGLE;
				} else {
					send_failure(client_fd, "scroller_proportion_single must be between 0.0 and 1.0");
					return;
				}
			} else if (strncmp("border_radius=", arg, 14) == 0) {
				r->consequence.border_radius = atof(arg + 14);
				r->consequence.has |= RULE_TYPE_BORDER_RADIUS;
			} else if (strncmp("render_unfocused_fps=", arg, 21) == 0) {
				int val = atoi(arg + 21);
				if (val >= 0 && val <= 1000) {
					r->consequence.render_unfocused_fps = val;
					r->consequence.has |= RULE_TYPE_RENDER_UNFOCUSED_FPS;
				} else {
					send_failure(client_fd, "render_unfocused_fps must be between 0 and 1000");
					return;
				}
			} else if (strncmp("opacity=", arg, 8) == 0) {
				float val = atof(arg + 8);
				if (val >= 0.0f && val <= 1.0f) {
					r->consequence.has |= RULE_TYPE_OPACITY;
					r->consequence.opacity = val;
				} else {
					send_failure(client_fd, "opacity must be between 0.0 and 1.0 inclusive");
					return;
				}
			}

			args++;
			num--;
		}

		if (!app_id && !title && r->match.tag[0] == '\0') {
			free(r);
			send_failure(client_fd, "rule -a: must specify app_id, title, or tag\n");
			return;
		}

		add_rule(r);
		send_success(client_fd, "rule added\n");
	} else if (streq("-r", subcmd) || streq("--remove", subcmd)) {
		if (num < 2) {
			send_failure(client_fd, "rule -r: missing index\n");
			return;
		}
		args++;
		int idx = atoi(*args);
		if (remove_rule_by_index(idx))
			send_success(client_fd, "rule removed\n");
		else
			send_failure(client_fd, "rule -r: invalid index\n");
	} else if (streq("-l", subcmd) || streq("--list", subcmd)) {
		char buf[DOORS_BUFSIZ];
		list_rules(buf, sizeof(buf));
		send_success(client_fd, buf);
	} else {
		send_failure(client_fd, "rule: unknown subcommand (use -a, -r, or -l)\n");
	}
}
