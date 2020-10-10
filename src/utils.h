#ifndef __BTA_UTILS_H
#define __BTA_UTILS_H

#include <gio/gio.h>
#include <string.h>

#define FILE_ATTRIBUTES_STANDARD_NAME_AND_TYPE "standard::name,standard::type"

gboolean delete_trashed_file(const char *file_path, int is_directory, GError *err);

char *substring(char *source, char *dest, int offset, int length);

#endif
