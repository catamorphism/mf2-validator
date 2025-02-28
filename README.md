# Validation

mf2validate is a C++ program that compares two MessageFormat 2.0 messages, presumably but not
necessarily in different languages, and checks that plural selectors are used in a consistent
way.

For more about MessageFormat 2.0, see https://github.com/unicode-org/message-format-wg

## Plural category checking

The validator checks that:

  * every plural category is present in the `.match` construct, or for multiple selectors,
    every permutation of plural categories is present
  * every plural category (or permutation) is _explicitly_ present (key sets like `few *`
    are prohibited)
  * every key is a valid plural category for the given locale

The validator only handles messages that only use the plural selector, called `:number`.
Messages that use other selector functions will fail, as the set of possible keys for
matching is unknown.

The validator also assumes that `:number` is only used for plural matching. In fact,
some messages use it for numeric matching. For example, the following is a perfectly
good message:

.input {$numDays :number}
.match $numDays
1     {{{$numDays} day}}
one   {{{$numDays} day}}
2     {{two days}}
other {{{$numDays} days}}
*     {{{$numDays} days}}

but the validator will reject it because it contains additional cases that are not
CLDR plural categories for English (the variants with keys "1" and "2").

## Placeholder checking

The validator also checks that all the placeholders used in the source message are
present in the target message. It assumes that each variant in the source message
uses the same set of placeholders (and prints a warning if this assumption is violated).
Then, it checks that each variant in the target message uses that set of placeholders.

For example, if the source message is:

.input {$numDays :number}
.match $numDays
one   {{{$numDays} day}}
other {{{$numDays} days}}
*     {{{$numDays} days}}

and the target message is:

.input {$numDays :number}
.match $numDays
one   {{{$numDays} den}}
few   {{{$numDays} dny}}
many  {{{$numDays} dne}}
other {{numDays dni}}
*     {{{$numDays} dní}}

then the target message is rejected, because its `other` variant does not contain
the placeholder `$numDays`.

# Usage

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
- Delete the icu_release/ subdirectory and re-run `make icu` (slow)
- Go into the icu_release/ subdirectory, run git commands as necessary, `cd ..` and `make icu` (faster).

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
* The Makefile assumes we are building on Linux/clang when building ICU. This would need to be changed
to enable building on Windows/etc.
