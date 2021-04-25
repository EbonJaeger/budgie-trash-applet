#pragma once

#include <gio/gio.h>
#include <string.h>

#define FILE_ATTRIBUTES_STANDARD_NAME_AND_TYPE "standard::name,standard::type"

/**
 * The offset from the beginning of a line to the start of
 * the restore path in a trash info file.
 */
#define TRASH_INFO_PATH_PREFIX_OFFSET 5

/**
 * The offset to the beginning of the date from the start
 * of a line.
 */
#define TRASH_INFO_DELETION_DATE_PREFIX_OFFSET 14

/**
 * Get the number of digits in a number. E.g., 3 => 1, 25 => 2, etc.
 */
int get_num_digits(int number);

gboolean trash_delete_file(const gchar *file_path, gint is_directory, GError **err);

gchar *substring(gchar *source, gint offset, size_t length);

GDateTime *trash_get_deletion_date(gchar *data);
GString *trash_get_restore_path(gchar *data);
