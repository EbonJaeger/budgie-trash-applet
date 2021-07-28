#pragma once

#include "notify.h"
#include <gio/gio.h>
#include <math.h>
#include <string.h>

#define FILE_ATTRIBUTES_STANDARD_NAME_AND_TYPE "standard::name,standard::type"

typedef struct _FileDeleteData FileDeleteData;

FileDeleteData *file_delete_data_new(GFile *file, gboolean is_directory, gboolean delete_children);
FileDeleteData *file_delete_data_ref(FileDeleteData *data);
void file_delete_data_unref(gpointer user_data);

/**
 * Attempt to delete a file from the disk.
 */
gpointer trash_utils_delete_file(FileDeleteData *data);

/**
 * Constructs a string with the given size (in bytes) at the given base
 * (1024 for bytes, KB, MB, etc).
 * 
 * The returned string will consist of the number and the correct unit
 * suffix.
 * 
 * The returned string should be freed with `g_free()`.
 */
gchar *trash_utils_humanize_bytes(goffset size, gint base);

/**
 * Checks if a string is not `NULL` and not empty.
 */
gboolean trash_utils_is_string_valid(gchar *string);

/**
 * Calculate the logarithm of a number for a given base.
 */
gdouble trash_utils_logn(gdouble n, gdouble base);

/**
 * Sanitize a path by replacing certain character codes with the actual
 * character, e.g. '%20' becomes a space.
 *
 * The returned string should be freed with `g_free()`.
 */
gchar *trash_utils_sanitize_path(gchar *path);

/**
 * Returns a new string consisting of the substring of the full string
 * starting at `offset` and going until `length`.
 *
 * The returned string should be freed with `g_free()`.
 */
gchar *trash_utils_substring(gchar *source, gint offset, size_t length);
