#include "utils.h"

struct _FileDeleteData {
    int _ref_count;

    GFile *file;

    gboolean is_directory;
    gboolean delete_children;
};

FileDeleteData *file_delete_data_ref(FileDeleteData *data) {
    g_atomic_int_inc(&data->_ref_count);
    return data;
}

void file_delete_data_unref(gpointer user_data) {
    FileDeleteData *data = (FileDeleteData *) user_data;
    if (g_atomic_int_dec_and_test(&data->_ref_count)) {
        g_slice_free(FileDeleteData, data);
    }
}

FileDeleteData *file_delete_data_new(GFile *file, gboolean is_directory, gboolean delete_children) {
    FileDeleteData *data = g_slice_new0(FileDeleteData);
    data->_ref_count = 1;
    data->file = file;
    data->is_directory = is_directory;
    data->delete_children = delete_children;

    return data;
}

gpointer trash_utils_delete_file(FileDeleteData *data) {
    g_autoptr(GError) err = NULL;
    gboolean success = TRUE;

    if (data->delete_children) {
        /* 
        * The `g_file_delete` operation works differently for locations provided
        * by the trash backend as it prevents modifications of trashed items.
        * For that reason, it is enough to call `g_file_delete` on top-level
        * items only.
        * 
        * Source: https://github.com/GNOME/nautilus/blob/master/src/nautilus-file-operations.c#L8037-L8041
        */
        gboolean should_recurse = !g_file_has_uri_scheme(data->file, "trash");

        g_autoptr(GFileEnumerator) enumerator = g_file_enumerate_children(data->file,
                                                                          FILE_ATTRIBUTES_STANDARD_NAME_AND_TYPE,
                                                                          G_FILE_QUERY_INFO_NONE,
                                                                          NULL,
                                                                          NULL);

        if (enumerator) {
            GFileInfo *file_info;
            GFile *child;
            while ((file_info = g_file_enumerator_next_file(enumerator, NULL, NULL))) {
                child = g_file_get_child(data->file, g_file_info_get_name(file_info));
                gboolean is_dir = (g_file_info_get_file_type(file_info) == G_FILE_TYPE_DIRECTORY);
                FileDeleteData *child_data = file_delete_data_new(child, is_dir, should_recurse && is_dir);
                trash_utils_delete_file(child_data);

                g_object_unref(child);
                g_object_unref(file_info);
                file_delete_data_unref(child_data);
            }

            g_file_enumerator_close(enumerator, NULL, NULL);
        }
    }

    success = g_file_delete(data->file, NULL, &err);

    if (!success) {
        trash_notify_try_send("Trash Bin Error", err->message, "dialog-error-symbolic");
        g_critical("%s:%d: Error deleting item at '%s': %s", __BASE_FILE__, __LINE__, g_file_get_uri(data->file), err->message);
    }

    file_delete_data_unref(data);

    return NULL;
}

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
