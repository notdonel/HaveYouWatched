#include "cli.h"
#include "library.h"
#include "storage.h"
#include "util.h"

#include <stdio.h>
#include <string.h>

static int command_changes_data(CommandType type) {
    return type == CMD_ADD || type == CMD_RATE || type == CMD_STATUS || type == CMD_REMOVE;
}

static int run_command(const ParsedCommand *command, MediaLibrary *library) {
    char error[256];

    switch (command->type) {
        case CMD_ADD: {
            MediaEntry entry;

            memset(&entry, 0, sizeof(entry));
            util_copy_string(entry.title, sizeof(entry.title), command->title);
            util_trim(entry.title);
            entry.type = command->type_value;
            entry.status = command->has_status ? command->status_value : STATUS_PLANNED;
            entry.rating = 0;
            util_copy_string(entry.notes, sizeof(entry.notes), command->notes);

            if (library_add(library, &entry, command->force, error, sizeof(error)) != 0) {
                fprintf(stderr, "Error: %s\n", error);
                return 1;
            }

            printf("Added \"%s\".\n", entry.title);
            return 0;
        }

        case CMD_LIST:
            library_list(library, &command->list_options);
            return 0;

        case CMD_RATE:
            if (library_set_rating(library, command->title, command->rating, error, sizeof(error)) != 0) {
                fprintf(stderr, "Error: %s\n", error);
                return 1;
            }
            if (command->rating == 0) {
                printf("Cleared rating for \"%s\".\n", command->title);
            } else {
                printf("Rated \"%s\" %d/10.\n", command->title, command->rating);
            }
            return 0;

        case CMD_STATUS:
            if (library_set_status(library, command->title, command->status_value, error, sizeof(error)) != 0) {
                fprintf(stderr, "Error: %s\n", error);
                return 1;
            }
            printf("Updated \"%s\" status to %s.\n",
                   command->title,
                   watch_status_to_string(command->status_value));
            return 0;

        case CMD_SEARCH:
            library_search(library, command->query);
            return 0;

        case CMD_REMOVE:
            if (library_remove(library, command->query, error, sizeof(error)) != 0) {
                fprintf(stderr, "Error: %s\n", error);
                return 1;
            }
            printf("Removed \"%s\".\n", command->query);
            return 0;

        case CMD_STATS: {
            LibraryStats stats;

            library_compute_stats(library, &stats);
            printf("Total entries: %zu\n", stats.total);
            printf("Anime: %zu | Movies: %zu\n", stats.anime_count, stats.movie_count);
            printf("Planned: %zu | Watching: %zu | Completed: %zu | Dropped: %zu\n",
                   stats.planned_count,
                   stats.watching_count,
                   stats.completed_count,
                   stats.dropped_count);
            if (stats.rated_count > 0) {
                printf("Average rating: %.2f/10 (%zu rated)\n",
                       stats.average_rating,
                       stats.rated_count);
            } else {
                printf("Average rating: n/a (no rated entries)\n");
            }
            return 0;
        }

        case CMD_PATH:
            return 0;

        case CMD_HELP:
        case CMD_NONE:
        default:
            return 0;
    }
}

int main(int argc, char **argv) {
    CliContext context;
    ParsedCommand command;
    MediaLibrary library;
    char error[256];
    int exit_code = 0;

    if (cli_parse(argc, argv, &context, &command, error, sizeof(error)) != 0) {
        fprintf(stderr, "Error: %s\n", error);
        cli_print_help(argv[0]);
        return 1;
    }

    if (context.show_help || command.type == CMD_HELP) {
        cli_print_help(argv[0]);
        return 0;
    }

    if (command.type == CMD_PATH) {
        printf("%s\n", context.data_path);
        return 0;
    }

    if (storage_load(&library, context.data_path, error, sizeof(error)) != 0) {
        fprintf(stderr, "Error: %s\n", error);
        return 1;
    }

    exit_code = run_command(&command, &library);

    if (exit_code == 0 && command_changes_data(command.type)) {
        if (storage_save(&library, context.data_path, error, sizeof(error)) != 0) {
            fprintf(stderr, "Error: %s\n", error);
            media_library_free(&library);
            return 1;
        }
    }

    media_library_free(&library);
    return exit_code;
}
