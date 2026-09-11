#include "storage.h"

#include "util.h"

#include "cJSON.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <direct.h>
#include <io.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

static int read_file(const char *path, char **content, size_t *length) {
    FILE *file;
    long file_size;
    char *buffer;
    size_t read_count;

    file = fopen(path, "rb");
    if (file == NULL) {
        return -1;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return -1;
    }

    file_size = ftell(file);
    if (file_size < 0) {
        fclose(file);
        return -1;
    }

    if (fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return -1;
    }

    buffer = (char *)malloc((size_t)file_size + 1);
    if (buffer == NULL) {
        fclose(file);
        return -1;
    }

    read_count = fread(buffer, 1, (size_t)file_size, file);
    fclose(file);

    if (read_count != (size_t)file_size) {
        free(buffer);
        return -1;
    }

    buffer[file_size] = '\0';
    *content = buffer;
    *length = (size_t)file_size;
    return 0;
}

static int write_file(const char *path, const char *content) {
    FILE *file;

    file = fopen(path, "wb");
    if (file == NULL) {
        return -1;
    }

    if (fputs(content, file) == EOF) {
        fclose(file);
        return -1;
    }

    if (fclose(file) != 0) {
        return -1;
    }

    return 0;
}

static int parse_entry(const cJSON *item, MediaEntry *entry, char *error, size_t error_size) {
    const cJSON *field;
    MediaType type;
    WatchStatus status;

    if (!cJSON_IsObject(item)) {
        if (error != NULL && error_size > 0) {
            snprintf(error, error_size, "Entry must be a JSON object.");
        }
        return -1;
    }

    memset(entry, 0, sizeof(*entry));

    field = cJSON_GetObjectItemCaseSensitive(item, "id");
    if (!cJSON_IsNumber(field)) {
        if (error != NULL && error_size > 0) {
            snprintf(error, error_size, "Entry missing numeric id.");
        }
        return -1;
    }
    entry->id = field->valueint;

    field = cJSON_GetObjectItemCaseSensitive(item, "title");
    if (!cJSON_IsString(field) || field->valuestring == NULL) {
        if (error != NULL && error_size > 0) {
            snprintf(error, error_size, "Entry missing title.");
        }
        return -1;
    }
    util_copy_string(entry->title, sizeof(entry->title), field->valuestring);
    util_trim(entry->title);

    field = cJSON_GetObjectItemCaseSensitive(item, "type");
    if (!cJSON_IsString(field) || media_type_from_string(field->valuestring, &type) != 0) {
        if (error != NULL && error_size > 0) {
            snprintf(error, error_size, "Entry has invalid type.");
        }
        return -1;
    }
    entry->type = type;

    field = cJSON_GetObjectItemCaseSensitive(item, "status");
    if (!cJSON_IsString(field) || watch_status_from_string(field->valuestring, &status) != 0) {
        if (error != NULL && error_size > 0) {
            snprintf(error, error_size, "Entry has invalid status.");
        }
        return -1;
    }
    entry->status = status;

    field = cJSON_GetObjectItemCaseSensitive(item, "rating");
    if (!cJSON_IsNumber(field)) {
        if (error != NULL && error_size > 0) {
            snprintf(error, error_size, "Entry missing rating.");
        }
        return -1;
    }
    entry->rating = field->valueint;

    field = cJSON_GetObjectItemCaseSensitive(item, "notes");
    if (cJSON_IsString(field) && field->valuestring != NULL) {
        util_copy_string(entry->notes, sizeof(entry->notes), field->valuestring);
    }

    field = cJSON_GetObjectItemCaseSensitive(item, "added_at");
    if (cJSON_IsString(field) && field->valuestring != NULL) {
        util_copy_string(entry->added_at, sizeof(entry->added_at), field->valuestring);
    }

    field = cJSON_GetObjectItemCaseSensitive(item, "updated_at");
    if (cJSON_IsString(field) && field->valuestring != NULL) {
        util_copy_string(entry->updated_at, sizeof(entry->updated_at), field->valuestring);
    }

    return media_entry_validate(entry, error, error_size);
}

static cJSON *entry_to_json(const MediaEntry *entry) {
    cJSON *object = cJSON_CreateObject();
    if (object == NULL) {
        return NULL;
    }

    cJSON_AddNumberToObject(object, "id", entry->id);
    cJSON_AddStringToObject(object, "title", entry->title);
    cJSON_AddStringToObject(object, "type", media_type_to_string(entry->type));
    cJSON_AddStringToObject(object, "status", watch_status_to_string(entry->status));
    cJSON_AddNumberToObject(object, "rating", entry->rating);
    cJSON_AddStringToObject(object, "notes", entry->notes);
    cJSON_AddStringToObject(object, "added_at", entry->added_at);
    cJSON_AddStringToObject(object, "updated_at", entry->updated_at);

    return object;
}

int storage_get_default_path(char *path, size_t path_size) {
    const char *home;

#if defined(_WIN32)
    home = getenv("USERPROFILE");
#else
    home = getenv("HOME");
#endif

    if (home == NULL || home[0] == '\0') {
        return -1;
    }

    if (snprintf(path, path_size, "%s/.media-tracker/library.json", home) >= (int)path_size) {
        return -1;
    }

#if defined(_WIN32)
    {
        size_t i;
        for (i = 0; path[i] != '\0'; i++) {
            if (path[i] == '/') {
                path[i] = '\\';
            }
        }
    }
#endif

    return 0;
}

int storage_ensure_directory(const char *file_path) {
    char directory[512];
    const char *slash;
    size_t length;

    if (file_path == NULL) {
        return -1;
    }

    util_copy_string(directory, sizeof(directory), file_path);

#if defined(_WIN32)
    slash = strrchr(directory, '\\');
    if (slash == NULL) {
        slash = strrchr(directory, '/');
    }
#else
    slash = strrchr(directory, '/');
#endif

    if (slash == NULL) {
        return -1;
    }

    length = (size_t)(slash - directory);
    directory[length] = '\0';

#if defined(_WIN32)
    return _mkdir(directory) == 0 || errno == EEXIST ? 0 : -1;
#else
    return mkdir(directory, 0755) == 0 || errno == EEXIST ? 0 : -1;
#endif
}

int storage_load(MediaLibrary *library, const char *path, char *error, size_t error_size) {
    char *content = NULL;
    size_t length = 0;
    cJSON *root = NULL;
    cJSON *entries = NULL;
    cJSON *item = NULL;
    FILE *probe;

    if (library == NULL || path == NULL) {
        if (error != NULL && error_size > 0) {
            snprintf(error, error_size, "Invalid arguments to storage_load.");
        }
        return -1;
    }

    media_library_init(library);

    probe = fopen(path, "rb");
    if (probe == NULL) {
        return 0;
    }
    fclose(probe);

    if (read_file(path, &content, &length) != 0) {
        if (error != NULL && error_size > 0) {
            snprintf(error, error_size, "Failed to read library file: %s", path);
        }
        return -1;
    }

    root = cJSON_ParseWithLength(content, length);
    free(content);

    if (root == NULL) {
        if (error != NULL && error_size > 0) {
            snprintf(error, error_size, "Failed to parse library JSON.");
        }
        return -1;
    }

    entries = cJSON_GetObjectItemCaseSensitive(root, "entries");
    if (!cJSON_IsArray(entries)) {
        cJSON_Delete(root);
        if (error != NULL && error_size > 0) {
            snprintf(error, error_size, "Library JSON missing entries array.");
        }
        return -1;
    }

    cJSON_ArrayForEach(item, entries) {
        MediaEntry entry;

        if (parse_entry(item, &entry, error, error_size) != 0) {
            cJSON_Delete(root);
            media_library_free(library);
            return -1;
        }

        if (media_library_reserve(library, library->count + 1) != 0) {
            cJSON_Delete(root);
            media_library_free(library);
            if (error != NULL && error_size > 0) {
                snprintf(error, error_size, "Out of memory while loading library.");
            }
            return -1;
        }

        library->entries[library->count++] = entry;
    }

    cJSON_Delete(root);
    return 0;
}

int storage_save(const MediaLibrary *library, const char *path, char *error, size_t error_size) {
    cJSON *root = NULL;
    cJSON *entries = NULL;
    char *json = NULL;
    char temp_path[560];
    size_t i;

    if (library == NULL || path == NULL) {
        if (error != NULL && error_size > 0) {
            snprintf(error, error_size, "Invalid arguments to storage_save.");
        }
        return -1;
    }

    if (storage_ensure_directory(path) != 0) {
        if (error != NULL && error_size > 0) {
            snprintf(error, error_size, "Failed to create data directory.");
        }
        return -1;
    }

    root = cJSON_CreateObject();
    if (root == NULL) {
        if (error != NULL && error_size > 0) {
            snprintf(error, error_size, "Out of memory while saving library.");
        }
        return -1;
    }

    cJSON_AddNumberToObject(root, "version", 1);
    entries = cJSON_AddArrayToObject(root, "entries");
    if (entries == NULL) {
        cJSON_Delete(root);
        if (error != NULL && error_size > 0) {
            snprintf(error, error_size, "Out of memory while saving library.");
        }
        return -1;
    }

    for (i = 0; i < library->count; i++) {
        cJSON *entry_json = entry_to_json(&library->entries[i]);
        if (entry_json == NULL) {
            cJSON_Delete(root);
            if (error != NULL && error_size > 0) {
                snprintf(error, error_size, "Out of memory while saving library.");
            }
            return -1;
        }
        cJSON_AddItemToArray(entries, entry_json);
    }

    json = cJSON_Print(root);
    cJSON_Delete(root);

    if (json == NULL) {
        if (error != NULL && error_size > 0) {
            snprintf(error, error_size, "Failed to serialize library JSON.");
        }
        return -1;
    }

    if (snprintf(temp_path, sizeof(temp_path), "%s.tmp", path) >= (int)sizeof(temp_path)) {
        cJSON_free(json);
        if (error != NULL && error_size > 0) {
            snprintf(error, error_size, "Temporary path is too long.");
        }
        return -1;
    }

    if (write_file(temp_path, json) != 0) {
        cJSON_free(json);
        if (error != NULL && error_size > 0) {
            snprintf(error, error_size, "Failed to write temporary library file.");
        }
        return -1;
    }

    cJSON_free(json);

#if defined(_WIN32)
    if (remove(path) != 0 && errno != ENOENT) {
        remove(temp_path);
        if (error != NULL && error_size > 0) {
            snprintf(error, error_size, "Failed to replace library file.");
        }
        return -1;
    }
#endif

    if (rename(temp_path, path) != 0) {
        remove(temp_path);
        if (error != NULL && error_size > 0) {
            snprintf(error, error_size, "Failed to rename library file.");
        }
        return -1;
    }

    return 0;
}
