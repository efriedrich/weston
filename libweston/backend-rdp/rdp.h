/*
 * Copyright © 2013 Hardening <rdp.effort@gmail.com>
 * Copyright © 2020 Microsoft
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef RDP_H
#define RDP_H

#include <freerdp/version.h>

#include <freerdp/freerdp.h>
#include <freerdp/listener.h>
#include <freerdp/update.h>
#include <freerdp/input.h>
#include <freerdp/codec/color.h>
#include <freerdp/codec/rfx.h>
#include <freerdp/codec/nsc.h>
#include <freerdp/locale/keyboard.h>

#include <libweston/libweston.h>
#include <libweston/backend-rdp.h>
#include <libweston/weston-log.h>

#include "backend.h"

#include "shared/helpers.h"
#include "shared/string-helpers.h"

#define MAX_FREERDP_FDS 32
#define DEFAULT_AXIS_STEP_DISTANCE 10
#define DEFAULT_PIXEL_FORMAT PIXEL_FORMAT_BGRA32

/* https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getkeyboardtype
 * defines a keyboard type that isn't currently defined in FreeRDP, but is
 * available for RDP connections */
#ifndef KBD_TYPE_KOREAN
#define KBD_TYPE_KOREAN 8
#endif

/* WinPR's GetVirtualKeyCodeFromVirtualScanCode() can't handle hangul/hanja keys */
/* 0x1f1 and 0x1f2 keys are only exists on Korean 103 keyboard (Type 8:SubType 6) */

/* From Linux's keyboard driver at drivers/input/keyboard/atkbd.c */
#define ATKBD_RET_HANJA 0xf1
#define ATKBD_RET_HANGEUL 0xf2

struct rdp_output;

struct rdp_backend {
	struct weston_backend base;
	struct weston_compositor *compositor;

	freerdp_listener *listener;
	struct wl_event_source *listener_events[MAX_FREERDP_FDS];
	struct rdp_output *output;
	struct weston_log_scope *debug;
	struct weston_log_scope *verbose;

	char *server_cert;
	char *server_key;
	char *rdp_key;
	int tls_enabled;
	int no_clients_resize;
	int force_no_compression;
	bool remotefx_codec;
	int external_listener_fd;
	int rdp_monitor_refresh_rate;
};

enum peer_item_flags {
	RDP_PEER_ACTIVATED      = (1 << 0),
	RDP_PEER_OUTPUT_ENABLED = (1 << 1),
};

struct rdp_peers_item {
	int flags;
	freerdp_peer *peer;
	struct weston_seat *seat;

	struct wl_list link;
};

struct rdp_head {
	struct weston_head base;
};

struct rdp_output {
	struct weston_output base;
	struct wl_event_source *finish_frame_timer;
	pixman_image_t *shadow_surface;

	struct wl_list peers;
};

struct rdp_peer_context {
	rdpContext _p;

	struct rdp_backend *rdpBackend;
	struct wl_event_source *events[MAX_FREERDP_FDS];
	RFX_CONTEXT *rfx_context;
	wStream *encode_stream;
	RFX_RECT *rfx_rects;
	NSC_CONTEXT *nsc_context;

	struct rdp_peers_item item;

	bool button_state[5];

	int verticalAccumWheelRotationPrecise;
	int verticalAccumWheelRotationDiscrete;
	int horizontalAccumWheelRotationPrecise;
	int horizontalAccumWheelRotationDiscrete;

};
typedef struct rdp_peer_context RdpPeerContext;

#define rdp_debug_verbose(b, ...) \
	rdp_debug_print(b->verbose, false, __VA_ARGS__)
#define rdp_debug_verbose_continue(b, ...) \
	rdp_debug_print(b->verbose, true,  __VA_ARGS__)
#define rdp_debug(b, ...) \
	rdp_debug_print(b->debug, false, __VA_ARGS__)
#define rdp_debug_continue(b, ...) \
	rdp_debug_print(b->debug, true,  __VA_ARGS__)

/* rdputil.c */
void
rdp_debug_print(struct weston_log_scope *log_scope, bool cont, char *fmt, ...);

void
convert_rdp_keyboard_to_xkb_rule_names(UINT32 KeyboardType, UINT32 KeyboardSubType, UINT32 KeyboardLayout, struct xkb_rule_names *xkbRuleNames);

static inline struct rdp_head *
to_rdp_head(struct weston_head *base)
{
	return container_of(base, struct rdp_head, base);
}

static inline struct rdp_output *
to_rdp_output(struct weston_output *base)
{
	return container_of(base, struct rdp_output, base);
}

static inline struct rdp_backend *
to_rdp_backend(struct weston_compositor *base)
{
	return container_of(base->backend, struct rdp_backend, base);
}

#endif
