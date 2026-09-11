#ifndef MODEL_H
#define MODEL_H

#include <stddef.h>

typedef enum {
    MEDIA_ANIME,
    MEDIA_MOVIE,
    MEDIA_TYPE_COUNT
} MediaType;

typedef enum {
    STATUS_PLANNED,
    STATUS_WATCHING,
    STATUS_COMPLETED,
    STATUS_DROPPED,
    STATUS_COUNT
} WatchStatus;

typedef struct {
    int id;
    char title[256];
    MediaType type;
    WatchStatus status;
    int rating;
    char notes[512];
    char added_at[32];
    char updated_at[32];
} MediaEntry;

typedef struct {
    MediaEntry *entries;
    size_t count;
    size_t capacity;
} MediaLibrary;

const char *media_type_to_string(MediaType type);
const char *watch_status_to_string(WatchStatus status);

int media_type_from_string(const char *value, MediaType *out);
int watch_status_from_string(const char *value, WatchStatus *out);

int media_entry_validate(const MediaEntry *entry, char *error, size_t error_size);
int media_title_is_valid(const char *title, char *error, size_t error_size);
int media_rating_is_valid(int rating, char *error, size_t error_size);

void media_library_init(MediaLibrary *library);
void media_library_free(MediaLibrary *library);
int media_library_reserve(MediaLibrary *library, size_t capacity);

#endif
