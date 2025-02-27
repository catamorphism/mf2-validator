mf2validate is a C++ program that compares two MessageFormat 2.0 messages, presumably but not
necessarily in different languages, and checks that plural selectors are used in a consistent
way. It checks that:

  * every plural category is present in the `.match` construct, or for multiple selectors,
    every permutation of plural categories is present
  * every plural category (or permutation) is _explicitly_ present (key sets like `few *`
    are prohibited)
  * every key is a valid plural category for the given locale

The validator only handles messages that only use the plural selector, called `:number`.
Messages that use other selector functions will fail, as the set of possible keys for
matching is unknown.

For more about MessageFormat 2.0, see https://github.com/unicode-org/message-format-wg

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
