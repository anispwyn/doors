#include "client.h"
#include "toplevel.h"
#include "types.h"
#include "wlr/types/wlr_xdg_shell.h"
#include "wlr/xwayland.h"
#include "xwayland.h"
#include <assert.h>

static inline void client_check_view(const client_t *c) {
	if (c == NULL)
		return;
	assert((c->type == VIEW_XDG) == (c->toplevel != NULL));
	assert((c->type == VIEW_XWAYLAND) == (c->xwayland_view != NULL));
}


#define CLIENT_DISPATCH(client, field) \
	(client_check_view(client), \
		(client)->type == VIEW_XDG ? (client)->toplevel->field : \
		(client)->type == VIEW_XWAYLAND ? (client)->xwayland_view->field : NULL)

surface_blur_t **client_blur_slot(client_t *c) {
	client_check_view(c);
	if (c == NULL)
		return NULL;
	if (c->type == VIEW_XDG)
		return &c->toplevel->blur;
	if (c->type == VIEW_XWAYLAND)
		return &c->xwayland_view->blur;
	return NULL;
}

surface_rounded_t **client_rounded_slot(client_t *c) {
	client_check_view(c);
	if (c == NULL)
		return NULL;
	if (c->type == VIEW_XDG)
		return &c->toplevel->rounded;
	if (c->type == VIEW_XWAYLAND)
		return &c->xwayland_view->rounded;
	return NULL;
}

surface_shadow_t **client_shadow_slot(client_t *c) {
	client_check_view(c);
	if (c == NULL)
		return NULL;
	if (c->type == VIEW_XDG)
		return &c->toplevel->shadow;
	if (c->type == VIEW_XWAYLAND)
		return &c->xwayland_view->shadow;
	return NULL;
}

// Whether the client's view is currently mapped.
bool client_is_mapped(const client_t *c) {
	client_check_view(c);
	if (c == NULL)
		return false;
	if (c->type == VIEW_XDG)
		return c->toplevel->mapped;
	if (c->type == VIEW_XWAYLAND)
		return c->xwayland_view->mapped;
	return false;
}

struct wlr_surface *client_wlr_surface(client_t *c) {
	client_check_view(c);
	if (c == NULL)
		return NULL;
	if (c->type == VIEW_XDG)
		return c->toplevel->xdg_toplevel ? c->toplevel->xdg_toplevel->base->surface : NULL;
	if (c->type == VIEW_XWAYLAND)
		return c->xwayland_view->xwayland_surface ? c->xwayland_view->xwayland_surface->surface : NULL;
	return NULL;
}

struct wlr_ext_foreign_toplevel_handle_v1 *client_get_ext_foreign_toplevel(const client_t *c) {
	if (c == NULL)
		return NULL;
	if (c->type == VIEW_XDG)
		return c->toplevel->ext_foreign_toplevel;
	if (c->type == VIEW_XWAYLAND)
		return c->xwayland_view->ext_foreign_toplevel;
	return NULL;
}

struct wlr_foreign_toplevel_handle_v1 *client_get_foreign_toplevel(const client_t *c) {
	if (c == NULL)
		return NULL;
	if (c->type == VIEW_XDG)
		return c->toplevel->foreign_toplevel;
	if (c->type == VIEW_XWAYLAND)
		return c->xwayland_view->foreign_toplevel;
	return NULL;
}

struct wlr_scene_tree *client_get_scene_tree(client_t *client) {
	if (!client)
		return NULL;
	return CLIENT_DISPATCH(client, scene_tree);
}

struct wlr_scene_tree *client_get_content_tree(client_t *client) {
	if (!client)
		return NULL;
	return CLIENT_DISPATCH(client, content_tree);
}

struct wlr_scene_tree *client_border_tree(client_t *client) {
	if (!client)
		return NULL;
	return CLIENT_DISPATCH(client, border_tree);
}

struct wlr_scene_rect **client_border_rects(client_t *client) {
	if (!client)
		return NULL;
	return CLIENT_DISPATCH(client, border_rects);
}

surface_rounded_t *client_get_rounded(client_t *client) {
	if (!client)
		return NULL;
	return CLIENT_DISPATCH(client, rounded);
}

void client_set_visible(client_t *client, bool show) {
	if (client == NULL)
		return;

	if (show && client->flags.minimized)
		return;

	client->flags.shown = show;

	struct wlr_scene_tree *st = client_get_scene_tree(client);
	if (st)
		wlr_scene_node_set_enabled(&st->node, show);
}

node_t *client_get_node(client_t *client) {
	if (!client)
		return NULL;
	return CLIENT_DISPATCH(client, node);
}

output_t *client_get_output(client_t *client) {
	node_t *n = client_get_node(client);
	return n ? n->output : NULL;
}
