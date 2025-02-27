## Prerequisites

Before doing anything else, run:

```
make icu
```

This will clone ICU from GitHub and build it, which takes a long time.
The step doesn't need to be repeated unless ICU changes upstream. (See the next section.)

## Building

Currently requires clang.

```
make
```

or to compile with debugging enabled:

```
make DEBUG=1
```

Not usually necessary: to rebuild ICU (for example, if changes have been made upstream), either:
- Delete the icu_release/ subdirectory and re-run `make` (slow)
- Go into the icu_release/ subdirectory, run git commands as necessary, `cd ..` and `make` (faster).

## Running

Example:

```
sh mf2validate.sh  --sourceLocale=en-US --targetLocale=cs-CZ --sourceFilename=test/English_message_good --targetFilename=test/Czech_message_good
```

or with verbose output:

```
sh mf2validate.sh  --verbose --sourceLocale=en-US --targetLocale=cs-CZ --sourceFilename=test/English_message_good --targetFilename=test/Czech_message_good
```

### Tests

```
make test
```

## TODO

* Checking that source and target have same set of placeholders
