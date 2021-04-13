#include "utils.h"

gboolean delete_trashed_file(const gchar *file_path, gint is_directory, GError *err) {
    GFile *file = g_file_new_for_path(file_path);
    if (is_directory) {
        GFileInfo *file_info;
        GFileEnumerator *enumerator = g_file_enumerate_children(file,
                                                                FILE_ATTRIBUTES_STANDARD_NAME_AND_TYPE,
                                                                G_FILE_QUERY_INFO_NONE,
                                                                NULL,
                                                                NULL);

        while ((file_info = g_file_enumerator_next_file(enumerator, NULL, NULL))) {
            const gchar *child_path = g_build_path("/", file_path, g_file_info_get_name(file_info), NULL);

            if (g_file_info_get_file_type(file_info) == G_FILE_TYPE_DIRECTORY) {
                // Directories must be empty to be deleted, so recursively delete all children first
                gboolean success = delete_trashed_file(child_path, 1, err);
                if (!success) {
                    g_warning("Error while deleting a child directory: %s\n", err->message);
                    break;
                }
            } else {
                // Not a directory, just delete the file
                GFile *child_file = g_file_new_for_path(child_path);
                gboolean success = g_file_delete(child_file, NULL, &err);
                g_object_unref(child_file);
                if (!success) {
                    g_warning("Error while deleting child: %s\n", err->message);
                    break;
                }
            }

            g_free((gchar *) child_path);
            g_object_unref(file_info);
        }

        g_file_enumerator_close(enumerator, NULL, NULL);
        g_object_unref(enumerator);
    }

    // Delete the current file
    gboolean success = g_file_delete(file, NULL, &err);
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

    gchar *deletion_date_str = (gchar *) malloc(length + 1);

    deletion_date_str = substring(data, deletion_date_str, substr_start, length);
    deletion_date_str[length] = '\0';

    GTimeZone *tz = g_time_zone_new_local();
    GDateTime *deletion_date = g_date_time_new_from_iso8601((const gchar *) deletion_date_str, tz);
    g_time_zone_unref(tz);
    free(deletion_date_str);

    return deletion_date;
}

gchar *trash_get_restore_path(gchar *data) {
    gint end_of_line = (gint)(strchr(data, '\n') - data);
    gint length = end_of_line - TRASH_INFO_PATH_PREFIX_OFFSET;

    gchar *restore_path = (gchar *) malloc(length + 1);

    restore_path = substring(data, restore_path, TRASH_INFO_PATH_PREFIX_OFFSET, length);
    restore_path[length] = '\0';

    return restore_path;
}
