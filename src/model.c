#include "model.h"

#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *MEDIA_TYPE_STRINGS[MEDIA_TYPE_COUNT] = {
    "anime",
    "movie"
};

static const char *WATCH_STATUS_STRINGS[STATUS_COUNT] = {
    "planned",
    "watching",
    "completed",
    "dropped"
};

const char *media_type_to_string(MediaType type) {
    if (type < 0 || type >= MEDIA_TYPE_COUNT) {
        return "unknown";
    }
    return MEDIA_TYPE_STRINGS[type];
}

const char *watch_status_to_string(WatchStatus status) {
    if (status < 0 || status >= STATUS_COUNT) {
        return "unknown";
    }
    return WATCH_STATUS_STRINGS[status];
}

int media_type_from_string(const char *value, MediaType *out) {
    int i;

    if (value == NULL || out == NULL) {
        return -1;
    }

    for (i = 0; i < MEDIA_TYPE_COUNT; i++) {
        if (util_streq_ignore_case(value, MEDIA_TYPE_STRINGS[i])) {
            *out = (MediaType)i;
            return 0;
        }
    }

    return -1;
}

int watch_status_from_string(const char *value, WatchStatus *out) {
    int i;

    if (value == NULL || out == NULL) {
        return -1;
    }

    for (i = 0; i < STATUS_COUNT; i++) {
        if (util_streq_ignore_case(value, WATCH_STATUS_STRINGS[i])) {
            *out = (WatchStatus)i;
            return 0;
        }
    }

    return -1;
}

int media_title_is_valid(const char *title, char *error, size_t error_size) {
    char trimmed[256];

    if (title == NULL) {
        if (error != NULL && error_size > 0) {
            snprintf(error, error_size, "Title is required.");
        }
        return -1;
    }

    util_copy_string(trimmed, sizeof(trimmed), title);
    util_trim(trimmed);

    if (trimmed[0] == '\0') {
        if (error != NULL && error_size > 0) {
            snprintf(error, error_size, "Title is required.");
        }
        return -1;
    }

    if (strlen(trimmed) >= sizeof(((MediaEntry *)0)->title)) {
        if (error != NULL && error_size > 0) {
            snprintf(error, error_size, "Title is too long (max 255 characters).");
        }
        return -1;
    }

    return 0;
}

int media_rating_is_valid(int rating, char *error, size_t error_size) {
    if (rating < 0 || rating > 10) {
        if (error != NULL && error_size > 0) {
            snprintf(error, error_size, "Rating must be between 0 and 10.");
        }
        return -1;
    }

    return 0;
}

int media_entry_validate(const MediaEntry *entry, char *error, size_t error_size) {
    if (entry == NULL) {
        if (error != NULL && error_size > 0) {
            snprintf(error, error_size, "Entry is null.");
        }
        return -1;
    }

    if (media_title_is_valid(entry->title, error, error_size) != 0) {
        return -1;
    }

    if (entry->type < 0 || entry->type >= MEDIA_TYPE_COUNT) {
        if (error != NULL && error_size > 0) {
            snprintf(error, error_size, "Invalid media type.");
        }
        return -1;
    }

    if (entry->status < 0 || entry->status >= STATUS_COUNT) {
        if (error != NULL && error_size > 0) {
            snprintf(error, error_size, "Invalid watch status.");
        }
        return -1;
    }

    return media_rating_is_valid(entry->rating, error, error_size);
}

void media_library_init(MediaLibrary *library) {
    if (library == NULL) {
        return;
    }

    library->entries = NULL;
    library->count = 0;
    library->capacity = 0;
}

void media_library_free(MediaLibrary *library) {
    if (library == NULL) {
        return;
    }

    free(library->entries);
    library->entries = NULL;
    library->count = 0;
    library->capacity = 0;
}

int media_library_reserve(MediaLibrary *library, size_t capacity) {
    MediaEntry *new_entries;

    if (library == NULL) {
        return -1;
    }

    if (capacity <= library->capacity) {
        return 0;
    }

    new_entries = (MediaEntry *)realloc(library->entries, capacity * sizeof(MediaEntry));
    if (new_entries == NULL) {
        return -1;
    }

    library->entries = new_entries;
    library->capacity = capacity;
    return 0;
}
