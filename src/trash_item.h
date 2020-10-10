#ifndef _BTA_TRASH_ITEM_H
#define _BTA_TRASH_ITEM_H

#include <gio/gio.h>

/**
 * Holds the deletion date and restore path for a
 * trashed file.
 */
struct TrashInfo;
typedef struct TrashInfo
{
    char *restore_path;
    GDateTime *deletion_date;
} TrashInfo;

/**
 * Represents an item in the trash bin.
 */
struct TrashItem;
typedef struct TrashItem
{
    const char *name;
    const char *path;
    TrashInfo *trash_info;
    int is_directory;
} TrashItem;

/**
 * Creates and allocates a new TrashItem.
 * 
 * The returned pointer should be freed with `trash_item_free()`.
 */
TrashItem *trash_item_new(const char *name, const char *path);

/**
 * Creates and allocates a new TrashItem with a given TrashInfo
 * struct.
 * 
 * The returned pointer should be freed with `trash_item_free()`.
 */
TrashItem *trash_item_new_with_info(const char *name, const char *path, TrashInfo *trash_info);

/**
 * Creates and allocates a new TrashInfo.
 * 
 * The result of this should be freed with `trash_info_free()`.
 */
TrashInfo *trash_info_new(char *restore_path, GDateTime *deletion_date);

/**
 * Frees all resources for a TrashInfo struct.
 */
void trash_item_free(TrashItem *trash_item);

/**
 * Frees all resources for a TrashItem.
 */
void trash_item_free(struct TrashItem *item);

#endif
