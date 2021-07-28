#include "trash_info.h"
#include "utils.h"

TrashInfo *trash_info_new(gchar *file_name, const gchar *file_path, gboolean is_directory, goffset size) {
    TrashInfo *self = g_slice_new(TrashInfo);
    self->file_name = g_strdup(file_name);
    self->file_path = g_strdup(file_path);
    self->is_directory = is_directory;
    self->size = size;

    return self;
}
