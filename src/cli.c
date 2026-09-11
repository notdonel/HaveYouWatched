#include "cli.h"

#include "storage.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int parse_sort_field(const char *value, SortField *sort, char *error, size_t error_size) {
    if (value == NULL || sort == NULL) {
        return -1;
    }

    if (util_streq_ignore_case(value, "title")) {
        *sort = SORT_TITLE;
        return 0;
    }

    if (util_streq_ignore_case(value, "rating")) {
        *sort = SORT_RATING;
        return 0;
    }

    if (util_streq_ignore_case(value, "date")) {
        *sort = SORT_DATE;
        return 0;
    }

    if (error != NULL && error_size > 0) {
        snprintf(error, error_size, "Invalid sort field: %s", value);
    }
    return -1;
}

static int next_value(int argc, char **argv, int *index, const char **value) {
    if (*index + 1 >= argc) {
        return -1;
    }

    (*index)++;
    *value = argv[*index];
    return 0;
}

void cli_print_help(const char *program) {
    const char *name = program != NULL ? program : "tracker";

    printf("Usage: %s <command> [options] [args]\n\n", name);
    printf("Commands:\n");
    printf("  add       Add a new title\n");
    printf("  list      List entries\n");
    printf("  rate      Set rating 1-10 (or 0 to clear)\n");
    printf("  status    Change watch status\n");
    printf("  search    Substring title search\n");
    printf("  remove    Delete by title or id\n");
    printf("  stats     Show counts and average rating\n");
    printf("  path      Print data file location\n");
    printf("  help      Show usage\n\n");
    printf("Examples:\n");
    printf("  %s add --type anime --title \"Steins;Gate\" --status planned\n", name);
    printf("  %s rate \"Steins;Gate\" 9\n", name);
    printf("  %s list --type anime --status watching --sort rating\n", name);
    printf("  %s search gate\n", name);
}

int cli_parse(int argc, char **argv, CliContext *context, ParsedCommand *command,
              char *error, size_t error_size) {
    int i;
    const char *subcommand = NULL;

    if (context == NULL || command == NULL) {
        if (error != NULL && error_size > 0) {
            snprintf(error, error_size, "Invalid CLI parser arguments.");
        }
        return -1;
    }

    memset(context, 0, sizeof(*context));
    memset(command, 0, sizeof(*command));
    command->list_options.sort = SORT_TITLE;

    if (storage_get_default_path(context->data_path, sizeof(context->data_path)) != 0) {
        if (error != NULL && error_size > 0) {
            snprintf(error, error_size, "Failed to resolve default data path.");
        }
        return -1;
    }

    if (argc < 2) {
        context->show_help = 1;
        command->type = CMD_HELP;
        return 0;
    }

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            context->show_help = 1;
            command->type = CMD_HELP;
            return 0;
        }

        if (strcmp(argv[i], "--data") == 0) {
            const char *path;
            if (next_value(argc, argv, &i, &path) != 0) {
                if (error != NULL && error_size > 0) {
                    snprintf(error, error_size, "--data requires a path.");
                }
                return -1;
            }
            util_copy_string(context->data_path, sizeof(context->data_path), path);
            continue;
        }

        if (subcommand == NULL) {
            subcommand = argv[i];
            continue;
        }

        if (argv[i][0] != '-') {
            continue;
        }

        if (strcmp(argv[i], "--type") == 0) {
            const char *value;
            if (next_value(argc, argv, &i, &value) != 0 ||
                media_type_from_string(value, &command->type_value) != 0) {
                if (error != NULL && error_size > 0) {
                    snprintf(error, error_size, "Invalid or missing --type value.");
                }
                return -1;
            }
            command->has_type = 1;
            continue;
        }

        if (strcmp(argv[i], "--status") == 0) {
            const char *value;
            if (next_value(argc, argv, &i, &value) != 0 ||
                watch_status_from_string(value, &command->status_value) != 0) {
                if (error != NULL && error_size > 0) {
                    snprintf(error, error_size, "Invalid or missing --status value.");
                }
                return -1;
            }
            command->has_status = 1;
            continue;
        }

        if (strcmp(argv[i], "--title") == 0) {
            const char *value;
            if (next_value(argc, argv, &i, &value) != 0) {
                if (error != NULL && error_size > 0) {
                    snprintf(error, error_size, "--title requires a value.");
                }
                return -1;
            }
            util_copy_string(command->title, sizeof(command->title), value);
            continue;
        }

        if (strcmp(argv[i], "--notes") == 0) {
            const char *value;
            if (next_value(argc, argv, &i, &value) != 0) {
                if (error != NULL && error_size > 0) {
                    snprintf(error, error_size, "--notes requires a value.");
                }
                return -1;
            }
            util_copy_string(command->notes, sizeof(command->notes), value);
            continue;
        }

        if (strcmp(argv[i], "--sort") == 0) {
            const char *value;
            if (next_value(argc, argv, &i, &value) != 0 ||
                parse_sort_field(value, &command->list_options.sort, error, error_size) != 0) {
                return -1;
            }
            continue;
        }

        if (strcmp(argv[i], "--force") == 0) {
            command->force = 1;
            continue;
        }

        if (error != NULL && error_size > 0) {
            snprintf(error, error_size, "Unknown option: %s", argv[i]);
        }
        return -1;
    }

    if (subcommand == NULL) {
        context->show_help = 1;
        command->type = CMD_HELP;
        return 0;
    }

    if (util_streq_ignore_case(subcommand, "add")) {
        command->type = CMD_ADD;
        if (!command->has_type || command->title[0] == '\0') {
            if (error != NULL && error_size > 0) {
                snprintf(error, error_size, "add requires --type and --title.");
            }
            return -1;
        }
        if (!command->has_status) {
            command->status_value = STATUS_PLANNED;
        }
        return 0;
    }

    if (util_streq_ignore_case(subcommand, "list")) {
        command->type = CMD_LIST;
        if (command->has_type) {
            command->list_options.type_filter = command->type_value;
            command->list_options.has_type_filter = 1;
        }
        if (command->has_status) {
            command->list_options.status_filter = command->status_value;
            command->list_options.has_status_filter = 1;
        }
        return 0;
    }

    if (util_streq_ignore_case(subcommand, "rate")) {
        const char *rating_text = NULL;
        command->type = CMD_RATE;

        for (i = 2; i < argc; i++) {
            if (argv[i][0] == '-') {
                continue;
            }
            if (command->title[0] == '\0') {
                util_copy_string(command->title, sizeof(command->title), argv[i]);
            } else if (rating_text == NULL) {
                rating_text = argv[i];
            }
        }

        if (command->title[0] == '\0' || rating_text == NULL) {
            if (error != NULL && error_size > 0) {
                snprintf(error, error_size, "rate requires <title> <rating>.");
            }
            return -1;
        }

        command->rating = (int)strtol(rating_text, NULL, 10);
        command->has_rating = 1;
        return 0;
    }

    if (util_streq_ignore_case(subcommand, "status")) {
        command->type = CMD_STATUS;

        for (i = 2; i < argc; i++) {
            if (argv[i][0] == '-') {
                continue;
            }
            if (command->title[0] == '\0') {
                util_copy_string(command->title, sizeof(command->title), argv[i]);
            } else if (!command->has_status) {
                if (watch_status_from_string(argv[i], &command->status_value) != 0) {
                    if (error != NULL && error_size > 0) {
                        snprintf(error, error_size, "Invalid status value.");
                    }
                    return -1;
                }
                command->has_status = 1;
            }
        }

        if (command->title[0] == '\0' || !command->has_status) {
            if (error != NULL && error_size > 0) {
                snprintf(error, error_size, "status requires <title> <status>.");
            }
            return -1;
        }

        return 0;
    }

    if (util_streq_ignore_case(subcommand, "search")) {
        command->type = CMD_SEARCH;
        for (i = 2; i < argc; i++) {
            if (argv[i][0] == '-') {
                continue;
            }
            util_copy_string(command->query, sizeof(command->query), argv[i]);
            break;
        }
        if (command->query[0] == '\0') {
            if (error != NULL && error_size > 0) {
                snprintf(error, error_size, "search requires a query.");
            }
            return -1;
        }
        return 0;
    }

    if (util_streq_ignore_case(subcommand, "remove")) {
        command->type = CMD_REMOVE;
        for (i = 2; i < argc; i++) {
            if (argv[i][0] == '-') {
                continue;
            }
            util_copy_string(command->query, sizeof(command->query), argv[i]);
            break;
        }
        if (command->query[0] == '\0') {
            if (error != NULL && error_size > 0) {
                snprintf(error, error_size, "remove requires a title or id.");
            }
            return -1;
        }
        return 0;
    }

    if (util_streq_ignore_case(subcommand, "stats")) {
        command->type = CMD_STATS;
        return 0;
    }

    if (util_streq_ignore_case(subcommand, "path")) {
        command->type = CMD_PATH;
        return 0;
    }

    if (util_streq_ignore_case(subcommand, "help")) {
        context->show_help = 1;
        command->type = CMD_HELP;
        return 0;
    }

    if (error != NULL && error_size > 0) {
        snprintf(error, error_size, "Unknown command: %s", subcommand);
    }
    return -1;
}
