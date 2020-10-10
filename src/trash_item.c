#include "trash_item.h"

TrashItem *trash_item_new(const char *name, const char *path)
{
    g_return_val_if_fail(name != NULL, NULL);
    g_return_val_if_fail(path != NULL, NULL);

    struct TrashItem *trash_item = (struct TrashItem *)malloc(sizeof(struct TrashItem));
    if (!trash_item)
    {
        return NULL;
    }

    trash_item->name = name;
    trash_item->path = path;
    trash_item->is_directory = g_file_test(trash_item->path, G_FILE_TEST_IS_DIR);

    return trash_item;
}

TrashItem *trash_item_new_with_info(const char *name, const char *path, TrashInfo *trash_info)
{
    g_return_val_if_fail(trash_info != NULL, NULL);

    struct TrashItem *trash_item = trash_item_new(name, path);
    g_return_val_if_fail(trash_item != NULL, NULL);

    trash_item->trash_info = trash_info;

    return trash_item;
}

TrashInfo *trash_info_new(char *restore_path, GDateTime *deletion_date)
{
    g_return_val_if_fail(restore_path != NULL, NULL);
    g_return_val_if_fail(deletion_date != NULL, NULL);

    TrashInfo *trash_info = (TrashInfo *)malloc(sizeof(TrashInfo));
    if (!trash_info)
    {
        return NULL;
    }

    trash_info->restore_path = restore_path;
    trash_info->deletion_date = deletion_date;

    return trash_info;
}

void trash_info_free(TrashInfo *trash_info)
{
    g_return_if_fail(trash_info != NULL);

    free(trash_info->restore_path);
    g_date_time_unref(trash_info->deletion_date);
    free(trash_info);
}

void trash_item_free(TrashItem *trash_item)
{
    g_return_if_fail(trash_item != NULL);

    free((char *)trash_item->name);
    free((char *)trash_item->path);
    if (trash_item->trash_info)
    {
        trash_info_free(trash_item->trash_info);
    }
    free(trash_item);
}
