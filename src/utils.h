#pragma once

#include "notify.h"
#include <gio/gio.h>
#include <math.h>
#include <string.h>

#define FILE_ATTRIBUTES_STANDARD_NAME_AND_TYPE "standard::name,standard::type"

/**
 * Checks if a string is not `NULL` and not empty.
 */
gboolean trash_utils_is_string_valid(gchar *string);
