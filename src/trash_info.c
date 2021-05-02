#include "trash_info.h"

TrashInfo *trash_info_new_from_file(GFile *file) {
    return trash_info_new_from_file_with_prefix(file, NULL);
}

TrashInfo *trash_info_new_from_file_with_prefix(GFile *file, gchar *prefix) {
    g_autoptr(GError) err = NULL;
    g_autoptr(GFileInputStream) input_stream = g_file_read(file, NULL, &err);
    if (!input_stream) {
        return NULL;
    }

    // Seek to the Path line
    g_seekable_seek(G_SEEKABLE(input_stream), TRASH_INFO_PATH_OFFSET, G_SEEK_SET, NULL, NULL);

    // Read the file contents and extract the line containing the restore path
    g_autofree gchar *buffer = (gchar *) malloc(1024 * sizeof(gchar));
    gssize read;
    while ((read = g_input_stream_read(G_INPUT_STREAM(input_stream), buffer, 1024, NULL, &err))) {
        buffer[read] = '\0';
    }

    g_input_stream_close(G_INPUT_STREAM(input_stream), NULL, NULL);

    gchar **lines = g_strsplit(buffer, "\n", 2);
    gchar *restore_path = substring(lines[0], TRASH_INFO_PATH_PREFIX_OFFSET, strlen(lines[0]));
    if (prefix) {
        restore_path = g_strconcat(prefix, G_DIR_SEPARATOR_S, restore_path, NULL);
    }
    g_autofree gchar *deletion_time_str = g_strstrip(substring(lines[1], TRASH_INFO_DELETION_DATE_PREFIX_OFFSET, strlen(lines[1])));
    g_autoptr(GTimeZone) tz = g_time_zone_new_local();
    GDateTime *deletion_time = g_date_time_new_from_iso8601((const gchar *) deletion_time_str, tz);
    g_strfreev(lines);

    TrashInfo *self = g_slice_new(TrashInfo);
    self->restore_path = restore_path;
    self->deleted_time = deletion_time;

    return self;
}
