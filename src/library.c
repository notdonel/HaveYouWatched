#include "library.h"

#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int compare_title_asc(const void *a, const void *b) {
    const MediaEntry *left = (const MediaEntry *)a;
    const MediaEntry *right = (const MediaEntry *)b;
    return strcmp(left->title, right->title);
}

static int compare_rating_desc(const void *a, const void *b) {
    const MediaEntry *left = (const MediaEntry *)a;
    const MediaEntry *right = (const MediaEntry *)b;

    if (left->rating != right->rating) {
        return right->rating - left->rating;
    }

    return strcmp(left->title, right->title);
}

static int compare_date_desc(const void *a, const void *b) {
    const MediaEntry *left = (const MediaEntry *)a;
    const MediaEntry *right = (const MediaEntry *)b;
    return strcmp(right->updated_at, left->updated_at);
}

int library_next_id(const MediaLibrary *library) {
    size_t i;
    int max_id = 0;

    if (library == NULL) {
        return 1;
    }

    for (i = 0; i < library->count; i++) {
        if (library->entries[i].id > max_id) {
            max_id = library->entries[i].id;
        }
    }

    return max_id + 1;
}

int library_find_by_title(const MediaLibrary *library, const char *title) {
    size_t i;

    if (library == NULL || title == NULL) {
        return -1;
    }

    for (i = 0; i < library->count; i++) {
        if (util_streq_ignore_case(library->entries[i].title, title)) {
            return (int)i;
        }
    }

    return -1;
}

int library_find_by_id(const MediaLibrary *library, int id) {
    size_t i;

    if (library == NULL) {
        return -1;
    }

    for (i = 0; i < library->count; i++) {
        if (library->entries[i].id == id) {
            return (int)i;
        }
    }

    return -1;
}

static int resolve_query_index(const MediaLibrary *library, const char *query) {
    char *end;
    long id;
    int index;

    if (library == NULL || query == NULL || query[0] == '\0') {
        return -1;
    }

    id = strtol(query, &end, 10);
    if (*end == '\0' && id > 0) {
        index = library_find_by_id(library, (int)id);
        if (index >= 0) {
            return index;
        }
    }

    return library_find_by_title(library, query);
}

int library_add(MediaLibrary *library, const MediaEntry *entry, int force,
                char *error, size_t error_size) {
    MediaEntry copy;
    int existing_index;

    if (library == NULL || entry == NULL) {
        if (error != NULL && error_size > 0) {
            snprintf(error, error_size, "Invalid arguments to library_add.");
        }
        return -1;
    }

    copy = *entry;
    util_trim(copy.title);

    if (media_entry_validate(&copy, error, error_size) != 0) {
        return -1;
    }

    existing_index = library_find_by_title(library, copy.title);
    if (existing_index >= 0 && !force) {
        if (error != NULL && error_size > 0) {
            snprintf(error, error_size,
                     "Title already exists: \"%s\". Use --force to add anyway.",
                     copy.title);
        }
        return -1;
    }

    if (copy.id <= 0) {
        copy.id = library_next_id(library);
    }

    if (copy.added_at[0] == '\0') {
        util_now_iso8601(copy.added_at, sizeof(copy.added_at));
    }

    if (copy.updated_at[0] == '\0') {
        util_now_iso8601(copy.updated_at, sizeof(copy.updated_at));
    }

    if (media_library_reserve(library, library->count + 1) != 0) {
        if (error != NULL && error_size > 0) {
            snprintf(error, error_size, "Out of memory while adding entry.");
        }
        return -1;
    }

    library->entries[library->count++] = copy;
    return 0;
}

int library_remove(MediaLibrary *library, const char *query, char *error, size_t error_size) {
    int index;
    size_t i;

    if (library == NULL || query == NULL) {
        if (error != NULL && error_size > 0) {
            snprintf(error, error_size, "Invalid arguments to library_remove.");
        }
        return -1;
    }

    index = resolve_query_index(library, query);
    if (index < 0) {
        if (error != NULL && error_size > 0) {
            snprintf(error, error_size, "Entry not found: \"%s\". Try `tracker search`.", query);
        }
        return -1;
    }

    for (i = (size_t)index; i + 1 < library->count; i++) {
        library->entries[i] = library->entries[i + 1];
    }

    library->count--;
    return 0;
}

int library_set_rating(MediaLibrary *library, const char *title, int rating,
                       char *error, size_t error_size) {
    int index;

    if (library == NULL || title == NULL) {
        if (error != NULL && error_size > 0) {
            snprintf(error, error_size, "Invalid arguments to library_set_rating.");
        }
        return -1;
    }

    if (media_rating_is_valid(rating, error, error_size) != 0) {
        return -1;
    }

    index = library_find_by_title(library, title);
    if (index < 0) {
        if (error != NULL && error_size > 0) {
            snprintf(error, error_size, "Entry not found: \"%s\". Try `tracker search`.", title);
        }
        return -1;
    }

    library->entries[index].rating = rating;
    util_now_iso8601(library->entries[index].updated_at,
                     sizeof(library->entries[index].updated_at));
    return 0;
}

int library_set_status(MediaLibrary *library, const char *title, WatchStatus status,
                       char *error, size_t error_size) {
    int index;

    if (library == NULL || title == NULL) {
        if (error != NULL && error_size > 0) {
            snprintf(error, error_size, "Invalid arguments to library_set_status.");
        }
        return -1;
    }

    if (status < 0 || status >= STATUS_COUNT) {
        if (error != NULL && error_size > 0) {
            snprintf(error, error_size, "Invalid watch status.");
        }
        return -1;
    }

    index = library_find_by_title(library, title);
    if (index < 0) {
        if (error != NULL && error_size > 0) {
            snprintf(error, error_size, "Entry not found: \"%s\". Try `tracker search`.", title);
        }
        return -1;
    }

    library->entries[index].status = status;
    util_now_iso8601(library->entries[index].updated_at,
                     sizeof(library->entries[index].updated_at));
    return 0;
}

void library_compute_stats(const MediaLibrary *library, LibraryStats *stats) {
    size_t i;
    double rating_sum = 0.0;

    if (stats == NULL) {
        return;
    }

    memset(stats, 0, sizeof(*stats));

    if (library == NULL) {
        return;
    }

    stats->total = library->count;

    for (i = 0; i < library->count; i++) {
        const MediaEntry *entry = &library->entries[i];

        if (entry->type == MEDIA_ANIME) {
            stats->anime_count++;
        } else {
            stats->movie_count++;
        }

        switch (entry->status) {
            case STATUS_PLANNED:
                stats->planned_count++;
                break;
            case STATUS_WATCHING:
                stats->watching_count++;
                break;
            case STATUS_COMPLETED:
                stats->completed_count++;
                break;
            case STATUS_DROPPED:
                stats->dropped_count++;
                break;
            default:
                break;
        }

        if (entry->rating > 0) {
            stats->rated_count++;
            rating_sum += entry->rating;
        }
    }

    if (stats->rated_count > 0) {
        stats->average_rating = rating_sum / (double)stats->rated_count;
    }
}

void library_print_entry(const MediaEntry *entry) {
    if (entry == NULL) {
        return;
    }

    if (entry->rating > 0) {
        printf("[%d] %s (%s) - %s - rating %d/10\n",
               entry->id,
               entry->title,
               media_type_to_string(entry->type),
               watch_status_to_string(entry->status),
               entry->rating);
    } else {
        printf("[%d] %s (%s) - %s - unrated\n",
               entry->id,
               entry->title,
               media_type_to_string(entry->type),
               watch_status_to_string(entry->status));
    }

    if (entry->notes[0] != '\0') {
        printf("    notes: %s\n", entry->notes);
    }
}

static int entry_matches_filters(const MediaEntry *entry, const ListOptions *options) {
    if (options == NULL) {
        return 1;
    }

    if (options->has_type_filter && entry->type != options->type_filter) {
        return 0;
    }

    if (options->has_status_filter && entry->status != options->status_filter) {
        return 0;
    }

    return 1;
}

void library_list(const MediaLibrary *library, const ListOptions *options) {
    MediaEntry *filtered = NULL;
    size_t filtered_count = 0;
    size_t i;

    if (library == NULL) {
        return;
    }

    filtered = (MediaEntry *)malloc(library->count * sizeof(MediaEntry));
    if (filtered == NULL) {
        fprintf(stderr, "Out of memory while listing entries.\n");
        return;
    }

    for (i = 0; i < library->count; i++) {
        if (entry_matches_filters(&library->entries[i], options)) {
            filtered[filtered_count++] = library->entries[i];
        }
    }

    if (filtered_count == 0) {
        printf("No entries found.\n");
        free(filtered);
        return;
    }

    if (options != NULL) {
        switch (options->sort) {
            case SORT_RATING:
                qsort(filtered, filtered_count, sizeof(MediaEntry), compare_rating_desc);
                break;
            case SORT_DATE:
                qsort(filtered, filtered_count, sizeof(MediaEntry), compare_date_desc);
                break;
            case SORT_TITLE:
            default:
                qsort(filtered, filtered_count, sizeof(MediaEntry), compare_title_asc);
                break;
        }
    }

    for (i = 0; i < filtered_count; i++) {
        library_print_entry(&filtered[i]);
    }

    free(filtered);
}

int library_search(const MediaLibrary *library, const char *query) {
    size_t i;
    size_t matches = 0;

    if (library == NULL || query == NULL || query[0] == '\0') {
        fprintf(stderr, "Search query is required.\n");
        return -1;
    }

    for (i = 0; i < library->count; i++) {
        if (util_contains_ignore_case(library->entries[i].title, query)) {
            library_print_entry(&library->entries[i]);
            matches++;
        }
    }

    if (matches == 0) {
        printf("No matches for \"%s\".\n", query);
    }

    return (int)matches;
}
