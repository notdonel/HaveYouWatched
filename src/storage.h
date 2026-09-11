#ifndef STORAGE_H
#define STORAGE_H

#include "model.h"

int storage_get_default_path(char *path, size_t path_size);
int storage_ensure_directory(const char *file_path);
int storage_load(MediaLibrary *library, const char *path, char *error, size_t error_size);
int storage_save(const MediaLibrary *library, const char *path, char *error, size_t error_size);

#endif
