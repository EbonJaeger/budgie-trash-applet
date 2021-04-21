#include "utils.h"

gboolean trash_delete_file(const gchar *file_path, gint is_directory, GError **err) {
    gboolean success = FALSE;
    GFile *file = g_file_new_for_path(file_path);
    if (is_directory) {
        GFileInfo *file_info;
        GFileEnumerator *enumerator = g_file_enumerate_children(file,
                                                                FILE_ATTRIBUTES_STANDARD_NAME_AND_TYPE,
                                                                G_FILE_QUERY_INFO_NONE,
                                                                NULL,
                                                                NULL);

        while ((file_info = g_file_enumerator_next_file(enumerator, NULL, NULL))) {
            g_autofree gchar *child_path = g_build_path(G_DIR_SEPARATOR_S, file_path, g_file_info_get_name(file_info), NULL);

            if (g_file_info_get_file_type(file_info) == G_FILE_TYPE_DIRECTORY) {
                // Directories must be empty to be deleted, so recursively delete all children first
                success = trash_delete_file(child_path, TRUE, err);
                if (!success) {
                    break;
                }
            } else {
                // Not a directory, just delete the file
                GFile *child_file = g_file_new_for_path(child_path);
                success = g_file_delete(child_file, NULL, err);
                g_object_unref(child_file);
                if (!success) {
                    break;
                }
            }

            g_object_unref(file_info);
        }

        g_file_enumerator_close(enumerator, NULL, NULL);
        g_object_unref(enumerator);

        if (!success) {
            g_object_unref(file);
            return success;
        }
    }

    // Delete the current file
    success = g_file_delete(file, NULL, err);
    g_object_unref(file);
    return success;
}

gchar *substring(gchar *source, gchar *dest, gint offset, gint length) {
    gint input_length = strlen(source);
    if (offset + length > input_length) {
        return '\0';
    }

    strncpy(dest, source + offset, length);
    return dest;
}

GDateTime *trash_get_deletion_date(gchar *data) {
    gint substr_start = (gint)(strchr(data, '\n') - data + TRASH_INFO_DELETION_DATE_PREFIX_OFFSET);
    gint length = strlen(data) - substr_start - 1;

    g_autofree gchar *deletion_date_str = (gchar *) malloc(length + 1);

    deletion_date_str = substring(data, deletion_date_str, substr_start, length);
    deletion_date_str[length] = '\0';

    GTimeZone *tz = g_time_zone_new_local();
    GDateTime *deletion_date = g_date_time_new_from_iso8601((const gchar *) deletion_date_str, tz);
    g_time_zone_unref(tz);

    return deletion_date;
}

GString *trash_get_restore_path(gchar *data) {
    gint end_of_line = (gint)(strchr(data, '\n') - data);
    gint length = end_of_line - TRASH_INFO_PATH_PREFIX_OFFSET;

    gchar *tmp = (gchar *) malloc(length + 1);
    tmp = substring(data, tmp, TRASH_INFO_PATH_PREFIX_OFFSET, length);
    tmp[length] = '\0';

    // TODO: Make this suck less
    GString *restore_path = g_string_new(tmp);
    g_string_replace(restore_path, "%20", " ", 0);
    g_string_replace(restore_path, "%28", "(", 0);
    g_string_replace(restore_path, "%29", ")", 0);
    return restore_path;
}
