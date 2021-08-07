#include "utils.h"

gboolean trash_utils_is_string_valid(gchar *string) {
    return (string != NULL && g_strcmp0(string, "") != 0);
}
