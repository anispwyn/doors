#pragma once

#include "surface.h"

#include <wayland-server-core.h>
#include <wayland-server.h>
#include <wlr/types/wlr_ext_foreign_toplevel_list_v1.h>
#include <wlr/types/wlr_foreign_toplevel_management_v1.h>
#include <wlr/types/wlr_tearing_control_v1.h>
#include <wlr/types/wlr_xdg_dialog_v1.h>

typedef struct toplevel_t {
	struct wl_list link;
	struct wlr_xdg_toplevel *xdg_toplevel;
	struct wlr_scene_tree *scene_tree; // Parent container
	struct wlr_scene_tree *content_tree; // XDG surface content
	struct wlr_scene_tree *saved_surface_tree; // Saved buffer snapshot
	struct wlr_scene_buffer *output_handler; // For tracking output enter/leave events

	surface_blur_t *blur;
	surface_rounded_t *rounded;
	surface_shadow_t *shadow;

	struct wlr_ext_foreign_toplevel_handle_v1 *ext_foreign_toplevel;
	struct wlr_foreign_toplevel_handle_v1 *foreign_toplevel;

	char *foreign_identifier;
	char *tag;
	bool is_dialog;

	struct wlr_ext_image_capture_source_v1 *image_capture_source;
	struct wlr_scene_surface *image_capture_surface;
	struct wlr_scene *image_capture;
	struct wlr_scene_tree *image_capture_tree;
	void *capture_renderer;

	struct wlr_scene_tree *border_tree;
	struct wlr_scene_rect *border_rects[4];

	node_t *node;
	client_t *client;

	struct wlr_box geometry, last_requested;

	bool mapped, configured, wants_fade;
	int max_render_time;

	// tearing control
	enum wp_tearing_control_v1_presentation_hint tearing_hint;

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
} toplevel_t;

// helper functions
void focus_toplevel(toplevel_t *toplevel);
bool toplevel_is_ready(toplevel_t *toplevel);
void client_update_foreign_toplevel_state(client_t *client);
void client_update_ext_foreign_toplevel(client_t *client);
void client_connect_foreign_toplevel(client_t *client,
	struct wlr_foreign_toplevel_handle_v1 *handle);
void client_disconnect_foreign_toplevel(client_t *client);
void client_handle_outputs_update(struct wl_listener *listener, void *data);
void client_disconnect_outputs_update(client_t *client);
bool client_output_handler_point_accepts_input(struct wlr_scene_buffer *buffer, double *x,
	double *y);
void toplevel_center_and_clip_surface(toplevel_t *toplevel);

// buffer saving for transactions
void toplevel_save_buffer(toplevel_t *toplevel);
void toplevel_remove_saved_buffer(toplevel_t *toplevel);
void toplevel_send_frame_done(toplevel_t *toplevel);

void toplevel_freeze_sibling_buffers(desktop_t *d, node_t *n);

bool toplevel_get_surface_offset(toplevel_t *tl, int *ox, int *oy);
void toplevel_apply_decoration_mode(toplevel_t *tl);

bool toplevel_can_tear(toplevel_t *toplevel);

void toplevel_send_frame_done_iterator(struct wlr_scene_buffer *scene_buffer, int x, int y,
	void *data);

toplevel_t *toplevel_create(struct wlr_xdg_toplevel *xdg_toplevel);
