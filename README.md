# Media Tracker

A personal anime and movie list tracker written in C. Store titles, watch status, and ratings in a human-readable JSON file.

## Features

- Add anime and movie entries with status and optional notes
- Rate titles from 0 (unrated) to 10
- List, filter, and sort your library
- Search by title substring
- View library statistics

## Build (Windows)

Requires [CMake](https://cmake.org/) (3.16+) and a C compiler (MinGW-w64 or MSVC). cJSON is fetched automatically on first configure.

```powershell
cd C:\Users\user\Projects\media-tracker
cmake -B build
cmake --build build
```

With MinGW explicitly:

```powershell
cmake -B build -G "MinGW Makefiles"
cmake --build build
```

The executable is written to `build\tracker.exe` (or `build\tracker` on Unix).

## Data File

By default, your library is stored at:

```
%USERPROFILE%\.media-tracker\library.json
```

Use `tracker path` to print the exact location, or override it with `--data`:

```powershell
.\build\tracker.exe --data .\data\sample_library.json list
```

## Usage

```text
tracker <command> [options] [args]

Commands:
  add       Add a new title
  list      List entries
  rate      Set rating 1-10 (or 0 to clear)
  status    Change watch status
  search    Substring title search
  remove    Delete by title or id
  stats     Show counts and average rating
  path      Print data file location
  help      Show usage
```

### Examples

```powershell
# Add entries
.\build\tracker.exe add --type anime --title "Cowboy Bebop" --status planned
.\build\tracker.exe add --type movie --title "Spirited Away" --status completed --notes "Rewatch yearly"

# Rate and update status
.\build\tracker.exe rate "Cowboy Bebop" 9
.\build\tracker.exe status "Cowboy Bebop" watching

# List and search
.\build\tracker.exe list
.\build\tracker.exe list --type anime --status watching --sort rating
.\build\tracker.exe search bebop

# Remove and stats
.\build\tracker.exe remove "Cowboy Bebop"
.\build\tracker.exe stats
```

### Duplicate titles

Adding a title that already exists (case-insensitive) is rejected unless you pass `--force`:

```powershell
.\build\tracker.exe add --type anime --title "Steins;Gate" --status planned --force
```

## JSON Format

```json
{
  "version": 1,
  "entries": [
    {
      "id": 1,
      "title": "Steins;Gate",
      "type": "anime",
      "status": "watching",
      "rating": 9,
      "notes": "",
      "added_at": "2026-09-11T19:00:00Z",
      "updated_at": "2026-09-11T19:00:00Z"
    }
  ]
}
```

Valid values:

- `type`: `anime`, `movie`
- `status`: `planned`, `watching`, `completed`, `dropped`
- `rating`: `0`–`10`

## Dependencies

[cJSON](https://github.com/DaveGamble/cJSON) (MIT) is downloaded automatically by CMake on first build.

## License

Project code is yours to use.
