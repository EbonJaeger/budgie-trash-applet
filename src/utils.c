#include "utils.h"

gboolean trash_utils_is_string_valid(gchar *string) {
    return (string != NULL && g_strcmp0(string, "") != 0);
}

gdouble trash_utils_logn(gdouble n, gdouble base) {
    return log(n) / log(base);
}

gchar *trash_utils_substring(gchar *source, gint offset, size_t length) {
    size_t source_len = strlen(source);

    // Make sure we aren't trying to copy past the length of the source string
    if ((offset + length > source_len) && length != source_len) {
        return NULL;
    }

    if (length == source_len) {
        // Set the length to the number of chars being copied
        length = length - offset;
    }

    gchar *dest = malloc(sizeof(gchar) * length + 1);
    g_return_val_if_fail(dest != NULL, NULL);

    strncpy(dest, source + offset, length);
    dest[length] = '\0';
    return dest;
}
