#pragma once

#include "view.h"

#include <wlr/types/wlr_xdg_dialog_v1.h>

typedef struct xdg_toplevel_t {
	view_t view;

	struct wlr_xdg_toplevel *xdg_toplevel;

	char *tag;
	bool is_dialog;

	// xdg-decoration
	struct wlr_xdg_toplevel_decoration_v1 *xdg_decoration;
	struct wl_listener decoration_destroy;
	struct wl_listener decoration_request_mode;

	// listeners
	struct wl_listener map;
	struct wl_listener unmap;
	struct wl_listener commit;
	struct wl_listener destroy;
	struct wl_listener request_move;
	struct wl_listener request_resize;
	struct wl_listener request_maximize;
	struct wl_listener request_fullscreen;
	struct wl_listener request_minimize;
	struct wl_listener set_title;
	struct wl_listener set_app_id;
	struct wl_listener new_xdg_popup;
} xdg_toplevel_t;

// helper functions
void client_update_foreign_toplevel_state(client_t *client);
void client_update_ext_foreign_toplevel(client_t *client);
void client_connect_foreign_toplevel(client_t *client,
	struct wlr_foreign_toplevel_handle_v1 *handle);
void client_disconnect_foreign_toplevel(client_t *client);
void xdg_toplevel_apply_decoration_mode(xdg_toplevel_t *tl);

xdg_toplevel_t *xdg_toplevel_create(struct wlr_xdg_toplevel *xdg_toplevel);
void xdg_toplevel_adopt(xdg_toplevel_t *toplevel);
