#pragma once

#include <gio/gio.h>
#include <string.h>

#define FILE_ATTRIBUTES_STANDARD_NAME_AND_TYPE "standard::name,standard::type"

gboolean trash_delete_file(const gchar *file_path, gint is_directory, GError **err);

gchar *substring(gchar *source, gint offset, size_t length);
