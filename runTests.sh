#!/bin/bash

if [[ $1 == "--verbose" ]]; then
    QUIET=""
else
    QUIET=-q
fi

doTest() {
    # For now, use a fixed source and target locale for all tests
    bash mf2validate.sh $QUIET --sourceLocale=en-US --targetLocale=cs-CZ --sourceFilename=test/$1 --targetFilename=test/$2
    exitCode=$?
    if [ $exitCode != $3 ]; then
        echo "*** Test failed ***: ($1, $2); expected $3 and got $exitCode"
    else
        echo "Test passed: ($1, $2)"
    fi
}

# Nonexistent file
doTest "bogus" "English_message_good" 8
# Good source and target
doTest "English_message_good" "Czech_message_good" 0
# Good source, bad target
doTest "English_message_good" "Czech_message_bad" 1
# Bad source, bad target
doTest "English_message_bad" "Czech_message_bad" 1
# Parse error
doTest "Czech_message_good" "parse_error" 2
# Multiple selectors (good)
doTest "English_multiple_selectors" "Czech_multiple_selectors" 0
# Multiple selectors (bad)
doTest "English_multiple_selectors" "Czech_multiple_selectors_bad" 1
# Missing * variant
doTest "English_message_good" "Czech_message_missing_wildcard" 4
# Keyword that is not a correct plural category
doTest "English_message_good" "Czech_message_not_plural_category" 1
# Message without selectors
doTest "English_message_good" "Czech_message_no_selectors" 0
# Message with selectors but no plural selectors
doTest "no_plural_selector" "no_plural_selector" 6
# Good source and target with '*' variant not positioned last
doTest "English_message_good_permuted" "Czech_message_good_permuted" 0
# Message with both plural and non-plural selectors
doTest "English_mixed_selectors" "Czech_mixed_selectors" 6
# Message with partial-wildcard variants
doTest "English_message_good" "Czech_partial_wildcards" 9
# Message with variant key mismatch
doTest "English_message_good" "Czech_message_variant_key_mismatch" 4
# Message with missing selector annotation
doTest "English_message_good" "Czech_message_missing_selector_annotation" 4
# Message with aliased selector variable
doTest "English_message_alias" "Czech_message_alias" 0
# Message with additional cases that aren't plural categories (see README)
doTest "English_message_valid_but_rejected" "Czech_message_valid_but_rejected" 1
# Messages with inconsistent placeholders
doTest "English_message_good" "Czech_message_inconsistent_placeholders" 10
# Source message with different placeholders in different variants
# (Warns, but doesn't fail)
doTest "English_message_varying_variants" "Czech_message_good" 0
# Missing "other" variant -- shouldn't be an error if there's a '*'
doTest "English_message_missing_other" "Czech_message_missing_other" 0
# Missing "other other" variant with 2 selectors
doTest "English_message_missing_other_2" "Czech_message_missing_other_2" 0
