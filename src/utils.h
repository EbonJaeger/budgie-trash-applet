#pragma once

#include <gio/gio.h>
#include <string.h>

#define FILE_ATTRIBUTES_STANDARD_NAME_AND_TYPE "standard::name,standard::type"

/**
 * Attempt to delete a file from the disk.
 *
 * If the file is a directory, this function will recursively delete
 * the entire file tree inside the directory because directories
 * must be empty in order to be deleted.
 *
 * Returns FALSE and sets `err` on error.
 */
gboolean trash_delete_file(const gchar *file_path, gint is_directory, GError **err);

/**
 * Returns a new string consisting of the substring of the full string
 * starting at `offset` and going until `length`.
 *
 * The returned string should be freed with `g_free()`.
 */
gchar *substring(gchar *source, gint offset, size_t length);
