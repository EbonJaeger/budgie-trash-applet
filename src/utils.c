#include "utils.h"

gboolean delete_trashed_file(const char *file_path, int is_directory, GError *err)
{
    GFile *file = g_file_new_for_path(file_path);
    if (is_directory)
    {
        GFileInfo *file_info;
        GFileEnumerator *enumerator = g_file_enumerate_children(file,
                                                                FILE_ATTRIBUTES_STANDARD_NAME_AND_TYPE,
                                                                G_FILE_QUERY_INFO_NONE,
                                                                NULL,
                                                                NULL);

        while ((file_info = g_file_enumerator_next_file(enumerator, NULL, NULL)))
        {
            const char *child_path = g_build_path("/", file_path, g_file_info_get_name(file_info), NULL);

            if (g_file_info_get_file_type(file_info) == G_FILE_TYPE_DIRECTORY)
            {
                // Directories must be empty to be deleted, so recursively delete all children first
                gboolean success = delete_trashed_file(child_path, 1, err);
                if (!success)
                {
                    g_warning("Error while deleting a child directory: %s\n", err->message);
                    break;
                }
            }
            else
            {
                // Not a directory, just delete the file
                GFile *child_file = g_file_new_for_path(child_path);
                gboolean success = g_file_delete(child_file, NULL, &err);
                g_object_unref(child_file);
                if (!success)
                {
                    g_warning("Error while deleting child: %s\n", err->message);
                    break;
                }
            }

            g_free((char *)child_path);
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

char *substring(char *source, char *dest, int offset, int length)
{
    int input_length = strlen(source);
    if (offset + length > input_length)
    {
        return '\0';
    }

    strncpy(dest, source + offset, length);
    return dest;
}
