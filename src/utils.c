#include "utils.h"

gboolean trash_delete_file(const gchar *file_path, gint is_directory, GError **err) {
    gboolean success = TRUE;
    g_autoptr(GFile) file = g_file_new_for_path(file_path);
    if (is_directory) {
        GFileInfo *file_info;
        g_autoptr(GFileEnumerator) enumerator = g_file_enumerate_children(file,
                                                                          FILE_ATTRIBUTES_STANDARD_NAME_AND_TYPE,
                                                                          G_FILE_QUERY_INFO_NONE,
                                                                          NULL,
                                                                          NULL);

        // Iterate over all of the children and delete them
        while ((file_info = g_file_enumerator_next_file(enumerator, NULL, NULL))) {
            g_autofree gchar *child_path = g_build_path(G_DIR_SEPARATOR_S, file_path, g_file_info_get_name(file_info), NULL);

            if (g_file_info_get_file_type(file_info) == G_FILE_TYPE_DIRECTORY) {
                // Directories must be empty to be deleted, so recursively delete all children first
                success = trash_delete_file(child_path, TRUE, err);
            } else {
                // Not a directory, just delete the file
                g_autoptr(GFile) child_file = g_file_new_for_path(child_path);
                success = g_file_delete(child_file, NULL, err);
            }

            if (!success) {
                break;
            }

            g_object_unref(file_info);
        }

        g_file_enumerator_close(enumerator, NULL, NULL);

        // Return early if there was a problem deleting children in a directory
        if (!success) {
            return success;
        }
    }

    // Delete the current file
    return g_file_delete(file, NULL, err);
}

gchar *substring(gchar *source, gint offset, size_t length) {
    if ((offset + length > strlen(source)) && length != strlen(source)) {
        return NULL;
    }

    if (length == strlen(source)) {
        length = length - offset;
    }

    gchar *dest = malloc(sizeof(gchar) * length + 1);

    strncpy(dest, source + offset, length);
    dest[length] = '\0';
    return dest;
}
