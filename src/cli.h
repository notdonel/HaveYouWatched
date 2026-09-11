#ifndef CLI_H
#define CLI_H

#include "library.h"
#include "model.h"

typedef struct {
    char data_path[512];
    int show_help;
} CliContext;

typedef enum {
    CMD_NONE,
    CMD_ADD,
    CMD_LIST,
    CMD_RATE,
    CMD_STATUS,
    CMD_SEARCH,
    CMD_REMOVE,
    CMD_STATS,
    CMD_PATH,
    CMD_HELP
} CommandType;

typedef struct {
    CommandType type;
    char title[256];
    char query[256];
    char notes[512];
    MediaType type_value;
    int has_type;
    WatchStatus status_value;
    int has_status;
    int rating;
    int has_rating;
    int force;
    ListOptions list_options;
} ParsedCommand;

void cli_print_help(const char *program);
int cli_parse(int argc, char **argv, CliContext *context, ParsedCommand *command,
              char *error, size_t error_size);

#endif
