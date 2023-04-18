#pragma once

#include <libnotify/notification.h>

G_BEGIN_DECLS

void trash_notify_try_send(gchar *summary, gchar *body, gchar *icon_name);

G_END_DECLS
