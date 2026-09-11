#ifndef LIBRARY_H
#define LIBRARY_H

#include "model.h"

typedef enum {
    SORT_TITLE,
    SORT_RATING,
    SORT_DATE
} SortField;

typedef struct {
    MediaType type_filter;
    int has_type_filter;
    WatchStatus status_filter;
    int has_status_filter;
    SortField sort;
} ListOptions;

typedef struct {
    size_t total;
    size_t anime_count;
    size_t movie_count;
    size_t planned_count;
    size_t watching_count;
    size_t completed_count;
    size_t dropped_count;
    size_t rated_count;
    double average_rating;
} LibraryStats;

int library_next_id(const MediaLibrary *library);
int library_find_by_title(const MediaLibrary *library, const char *title);
int library_find_by_id(const MediaLibrary *library, int id);

int library_add(MediaLibrary *library, const MediaEntry *entry, int force,
                char *error, size_t error_size);
int library_remove(MediaLibrary *library, const char *query, char *error, size_t error_size);
int library_set_rating(MediaLibrary *library, const char *title, int rating,
                       char *error, size_t error_size);
int library_set_status(MediaLibrary *library, const char *title, WatchStatus status,
                       char *error, size_t error_size);

void library_compute_stats(const MediaLibrary *library, LibraryStats *stats);
void library_print_entry(const MediaEntry *entry);
void library_list(const MediaLibrary *library, const ListOptions *options);
int library_search(const MediaLibrary *library, const char *query);

#endif
