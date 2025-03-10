#include <format>
#include <fstream>
#include <iostream>
#include <math.h>

#include <cxxopts.hpp>

#include <unicode/messageformat2.h>
#include <unicode/messageformat2_data_model.h>

using namespace icu;
using namespace message2;
using namespace std;

#define MISSING_PLURAL_CATEGORY 1
#define PARSE_ERROR 2
#define NOT_YET_IMPLEMENTED 3
#define DATA_MODEL_ERROR 4
#define ICU_INTERNAL_ERROR 5
#define NON_PLURAL_SELECTORS 6
#define ASSERTION_FAILED 7
#define IO_ERROR 8
#define PARTIAL_WILDCARDS 9
#define INCONSISTENT_PLACEHOLDERS 10

bool quiet;

void log(std::string s) {
    if (!quiet) {
        cout << s << endl;
    }
}

std::string fromUStr(const UnicodeString& uStr) {
    std::string str;
    return uStr.toUTF8String(str);
}

std::string readFile(std::string filename) {
    std::ifstream file(filename);
    std::string line;
    std::string contents;
    if (!file) {
        log(format("Error reading from file {}", filename));
        exit(IO_ERROR);
    }
    while (std::getline(file, line)) {
        contents += line;
        contents.push_back('\n');
    }
    return contents;
}

std::string localeToString(const Locale& locale) {
    UErrorCode status = U_ZERO_ERROR;
    std::string result = locale.toLanguageTag<std::string>(status);
    if (U_FAILURE(status)) {
        return "[Bad locale]";
    }
    return result;
}

void getOptions(int argc, char** argv,
                Locale& sourceLocale, Locale& targetLocale,
                std::string& sourceFilename, std::string& targetFilename,
                bool& verbose, bool& quiet) {
    cxxopts::Options options("mf2validate", "Validate a source and target MF2 message");
    options.add_options()
        ("h,help", "Print out help message", cxxopts::value<bool>()->default_value("false"))
        ("verbose", "Verbose output", cxxopts::value<bool>()->default_value("false"))
        ("q,quiet", "Suppress all output", cxxopts::value<bool>()->default_value("false"))
        ("sourceLocale", "Locale for source message", cxxopts::value<std::string>()->default_value(""))
        ("targetLocale", "Locale for target message", cxxopts::value<std::string>()->default_value(""))
        ("sourceFilename", "File name for source message", cxxopts::value<std::string>()->default_value(""))
        ("targetFilename", "File name for target message", cxxopts::value<std::string>()->default_value(""));
    auto result = options.parse(argc, argv);

    try {
        sourceLocale = Locale(result["sourceLocale"].as<std::string>().c_str());
    } catch (const cxxopts::exceptions::exception& e) {
        log("Must provide --sourceLocale flag");
        throw(e);
    }

    try {
        targetLocale = Locale(result["targetLocale"].as<std::string>().c_str());
    } catch (const cxxopts::exceptions::exception& e) {
        log("Must provide --targetLocale flag");
        throw(e);
    }

    try {
        sourceFilename = result["sourceFilename"].as<std::string>();
    } catch (const cxxopts::exceptions::exception& e) {
        log("Must provide --sourceFilename flag");
        throw(e);
    }

    try {
        targetFilename = result["targetFilename"].as<std::string>();
    } catch (const cxxopts::exceptions::exception& e) {
        log("Must provide --targetFilename flag");
        throw(e);
    }

    verbose = result["verbose"].as<bool>();
    bool help = result["help"].as<bool>();
    quiet = result["quiet"].as<bool>();

    if (help) {
        cout << options.help() << endl;
        exit(0);
    }
}

void echoOptions(const Locale& sourceLocale, const Locale& targetLocale,
                 std::string sourceMessage, std::string targetMessage) {
    cout << "=== Options provided ===\n";
    cout << "Source locale: " << localeToString(sourceLocale) << "\n";
    cout << "Target locale: " << localeToString(targetLocale) << "\n";
    cout << "== Source message ==\n";
    cout << sourceMessage;
    cout << "== Target message ==\n";
    cout << targetMessage;
}

void checkICUError(UErrorCode errorCode, std::string errorMessage) {
    if (U_FAILURE(errorCode)) {
        log(errorMessage);
        exit(ICU_INTERNAL_ERROR);
    }
}

int fact(int n) {
    if (n <= 1) {
        return 1;
    }
    return n * fact(n - 1);
}

int perms(int n, int k) {
    return std::floor(pow(n, k));
}

std::vector<UnicodeString> insertElementAt(std::vector<UnicodeString> v,
                                           UnicodeString s,
                                           int i) {
    std::vector<UnicodeString> result;
    for (int j = 0; j < i; j++) {
        result.push_back(v[j]);
    }
    result.push_back(s);
    for (int j = i + 1; j < v.size() + 1; j++) {
        result.push_back(v[j - 1]);
    }
    return result;
}

template <class T>
bool contains(std::vector<T> vs, T v) {
    for (auto it = vs.begin(); it != vs.end(); ++it) {
        if (*it == v) {
            return true;
        }
    }
    return false;
}

std::vector<std::vector<UnicodeString>> dedup(std::vector<std::vector<UnicodeString>> v) {
    std::vector<std::vector<UnicodeString>> result;

    for (auto it = v.begin(); it != v.end(); ++it) {
        if (!contains<std::vector<UnicodeString>>(result, *it)) {
            result.push_back(*it);
        }
    }

    return result;
}

std::vector<std::vector<UnicodeString>> insertEverywhere(UnicodeString element,
                                                         std::vector<std::vector<UnicodeString>> permutations) {
    std::vector<std::vector<UnicodeString>> result;

    if (permutations.size() == 0) {
        result.push_back({ element });
        return result;
    }

    int positions = permutations[0].size() + 1;
    for (auto perm = permutations.begin(); perm != permutations.end(); ++perm) {
        for (int i = 0; i < positions; i++) {
            result.push_back(insertElementAt(*perm, element, i));
        }
    }
    return result;
}

// The permutation-generating code isn't too efficient, but we expect `len` to be small.
std::vector<std::vector<UnicodeString>> generatePermutations(int len, std::vector<UnicodeString> strings) {

    std::vector<std::vector<UnicodeString>> result;
    if (len == 0) {
        return result;
    }
    for (auto category = strings.begin(); category != strings.end(); ++category) {
        auto tail = insertEverywhere(*category, generatePermutations(len - 1, strings));
        result.insert(result.end(), tail.begin(), tail.end());
    }
    return dedup(result);
}

std::vector<UnicodeString> stringEnumerationToVector(StringEnumeration& strings) {
    UErrorCode errorCode = U_ZERO_ERROR;

    const UnicodeString* category;
    std::vector<UnicodeString> stringVector;
    while ((category = strings.snext(errorCode)) != nullptr) {
        checkICUError(errorCode,
                      format("Internal error iterating over plural categories"));
        stringVector.push_back(*category);
    }

    return stringVector;
}

UnicodeString keyToString(const Key& k) {
    return k.asLiteral().unquoted();
}

bool checkValidKeys(const Locale& locale,
                    const std::vector<Variant>& variants,
                    const std::vector<UnicodeString>& pluralCategories) {
    for (auto it = variants.begin(); it != variants.end(); ++it) {
        const std::vector<Key> sKeys = it->getKeys().getKeys();
        for (auto k = sKeys.begin(); k != sKeys.end(); ++k) {
            if (k->isWildcard()) {
                continue;
            }
            UnicodeString ks = keyToString(*k);
            if (!contains<UnicodeString>(pluralCategories, ks)) {
                log(format("Key {} is not a valid plural category for locale {}.",
                                fromUStr(ks), localeToString(locale)));
                return false;
            }
        }
    }
    return true;
}

bool isPluralSelector(const MFDataModel& dataModel,
                      const VariableName& variableName) {
    UErrorCode status = U_ZERO_ERROR;

    // Check if this variable's RHS has a `:number` annotation
    // Walk through all local variable declarations
    std::vector<Binding> decls = dataModel.getLocalVariables();
    for (auto it = decls.begin(); it != decls.end(); ++it) {
        if (it->getVariable() == variableName) {
            const Expression& rhs = it->getValue();
            const Operator* rator = rhs.getOperator(status);
            if (U_FAILURE(status)) {
                // Also need to handle aliasing: we could have
                // .local $x = {$y} where $y has a selector annotation
                if (rhs.getOperand().isVariable()) {
                    return isPluralSelector(dataModel, rhs.getOperand().asVariable());
                }
                // Otherwise, RHS must be an unannotated literal
                return false;
            }
            if (rator->getFunctionName() == UnicodeString("number")) {
                return true;
            }
            // This means `variableName`'s RHS has an annotation that isn't the plural selector/formatter
            return false;
        }
    }
    // Variable is unbound, which means the message has an "unresolved variable" error, but we
    // just ignore that case
    return false;
}

bool allWildcards(const std::vector<Key>& keys) {
    for (auto it = keys.begin(); it != keys.end(); ++it) {
        if (!it->isWildcard()) {
            return false;
        }
    }
    return true;
}

bool partialWildcards(const std::vector<Key>& keys) {
    bool wildcardSeen = false;
    bool nonWildcardSeen = false;
    for (auto it = keys.begin(); it != keys.end(); ++it) {
        if (it->isWildcard()) {
            wildcardSeen = true;
        } else {
            nonWildcardSeen = true;
        }
    }
    return wildcardSeen && nonWildcardSeen;
}

bool keysEqual(const std::vector<Key>& variantKeys,
               const std::vector<UnicodeString>& expectedKeys) {
    // variantKeys.size() == expectedKeys.size() (already checked)
    for (int i = 0; i < variantKeys.size(); i++) {
        if (keyToString(variantKeys[i]) != expectedKeys[i]) {
            return false;
        }
    }
    return true;
}

std::string uStrsToString(const std::vector<UnicodeString>& keys) {
    std::string result;
    bool first = true;

    for (auto it = keys.begin(); it != keys.end(); ++it) {
        if (!first) {
            result += ' ';
        }
        first = false;
        std::string temp;
        result += it->toUTF8String(temp);
    }
    return result;
}

std::string keysToString(const std::vector<Key>& keys) {
    std::vector<UnicodeString> strings;

    for (auto it = keys.begin(); it != keys.end(); ++it) {
        strings.push_back(keyToString(*it));
    }
    return uStrsToString(strings);
}

bool allOther(const std::vector<UnicodeString>& keys) {
    UnicodeString other("other");
    for (auto it = keys.begin(); it != keys.end(); ++it) {
        if (*it != other) {
            return false;
        }
    }
    return true;
}

bool allOther(const std::vector<Key>& keys) {
    UnicodeString other("other");
    for (auto it = keys.begin(); it != keys.end(); ++it) {
        if (it->isWildcard()) {
            return false;
        }
        if (keyToString(*it) != other) {
            return false;
        }
    }
    return true;
}

bool missingOtherVariant(const std::vector<Variant>& variants) {
    for (auto it = variants.begin(); it != variants.end(); ++it) {
        if (allOther(it->getKeys().getKeys())) {
            return false;
        }
    }
    return true;
}

bool variantExistsFor(const std::vector<Variant>& variants,
                      const std::vector<UnicodeString>& keys) {
    // Special case: it's OK to omit the 'other' variant if a '*'
    // variant is present (which is checked separately.)
    if (allOther(keys)) {
        return true;
    }

    for (auto it = variants.begin(); it != variants.end(); ++it) {
        const std::vector<Key> sKeys = it->getKeys().getKeys();
        if (sKeys.size() != keys.size()) {
            log("Warning: variant has fewer keys than there are selectors");
            return false;
        }
        if (allWildcards(sKeys)) {
            continue;
        }
        if (partialWildcards(sKeys)) {
            return false;
        }
        if (keysEqual(sKeys, keys)) {
            return true;
        }
    }
    log(format("Omitted variant: {}", uStrsToString(keys)));
    return false;
}

void checkDataModelErrors(MessageFormatter& mf) {
    // Data model errors are reported after formatting,
    // not after construction of the formatter; so we have to call
    // format() to get these errors. We don't have the arguments, so
    // there will likely be resolution errors, but we can distinguish
    // those from data model errors easily.
    UErrorCode errorCode = U_ZERO_ERROR;
    std::map<UnicodeString, message2::Formattable> empty;
    MessageArguments args(empty, errorCode);
    mf.formatToString(args, errorCode);
    bool dataModelError = false;
    if (U_FAILURE(errorCode)) {
        switch (errorCode) {
        case U_MF_VARIANT_KEY_MISMATCH_ERROR:
            log("Data model error: One or more variants has a different number of keys from the number of selectors.\n");
            dataModelError = true;
            break;
        case U_MF_NONEXHAUSTIVE_PATTERN_ERROR:
            log("Data model error: Missing '*' variant.\n");
            dataModelError = true;
            break;
        case U_MF_MISSING_SELECTOR_ANNOTATION_ERROR:
            log("Data model error: A selector variable refers to an expression with no annotation.\n");
            dataModelError = true;
            break;
        default:
            break;
        }
        // Not all data model errors are reported, just the ones related
        // to variants.
    }
    if (dataModelError) {
        exit(DATA_MODEL_ERROR);
    }
}

MFDataModel getDataModel(const Locale& locale, std::string message) {
    UErrorCode errorCode = U_ZERO_ERROR;
    UParseError parseError;

    MessageFormatter::Builder builder(errorCode);
    MessageFormatter mf = builder.setPattern(UnicodeString(message.c_str()),
                                             parseError, errorCode)
        .setLocale(locale)
        // Need strict error handling so we can detect data model errors
        .setErrorHandlingBehavior(MessageFormatter::U_MF_STRICT)
        .build(errorCode);

    if (U_FAILURE(errorCode)) {
        log(format("Couldn't parse message {}", message));
        exit(PARSE_ERROR);
    }

    checkDataModelErrors(mf);

    return mf.getDataModel();
}

bool checkPluralCategories(const Locale& locale, bool isSource, const MFDataModel& dataModel) {
    UErrorCode errorCode = U_ZERO_ERROR;

    std::vector<VariableName> selectors = dataModel.getSelectors();
    int numSelectors = selectors.size();
    if (numSelectors == 0) {
        log(format("Warning: {} message is not made up of a .match construct. Trivially correct.",
                        isSource ? "source" : "target"));
        return true;
    }

    std::vector<Variant> variants = dataModel.getVariants();
    // Get plural rules for this locale
    LocalPointer<PluralRules> pluralRules(PluralRules::forLocale(locale, errorCode));
    checkICUError(errorCode, format("Error getting plural rules for locale {}",
                                    localeToString(locale)));

    LocalPointer<StringEnumeration> pluralCategoriesEnumeration(pluralRules->getKeywords(errorCode));
    checkICUError(errorCode, format("Error getting categories from plural rules for locale {}",
                                    localeToString(locale)));

    // Check that all selectors are plural; if any are non-plural, we can't check
    // how many variants there should be
    bool allPlural = true;
    for (auto it = selectors.begin(); it != selectors.end(); ++it) {
        if (!isPluralSelector(dataModel, *it)) {
            allPlural = false;
            break;
        }
    }
    if (!allPlural) {
        log("Message uses non-plural selectors. Can't check exhaustiveness.\n");
        exit(NON_PLURAL_SELECTORS);
    }

    // Check for partial wildcard variants (variants with multiple keys where some are wildcards
    // and some aren't)
    for (auto it = variants.begin(); it != variants.end(); ++it) {
        if (partialWildcards(it->getKeys().getKeys())) {
            log("Partial wildcard variant is present; not all permutations of categories are explicitly enumerated.\n");
            exit(PARTIAL_WILDCARDS);
        }
    }

    // Convert pluralCategoriesEnumeration to a vector for convenience
    std::vector<UnicodeString> pluralCategories = stringEnumerationToVector(*pluralCategoriesEnumeration);

    // Generate all n-permutations of plural categories, where n is the number of selectors
    std::vector<std::vector<UnicodeString>> permutations = generatePermutations(numSelectors, pluralCategories);
    // Consistency check
#ifdef DEBUG
    int categoryCount = pluralCategories.size();
    int expectedPerms = perms(categoryCount, numSelectors);
    if (permutations.size() != expectedPerms) {
        log(format("Error calculating permutations of plural categories (this is a bug)\n\
Actual size: {}\nExpected size: {}\n", permutations.size(), expectedPerms));
        exit(ASSERTION_FAILED);
    }
#endif

    bool allOK = true;
    // It's OK if a variant all of whose keys are `other` is missing
    int32_t expectedSize = permutations.size() + 1;
    if (missingOtherVariant(variants)) {
        expectedSize--;
    }

    // Check that the number of variants == the number of permutations plus 1
    if (variants.size() != expectedSize) {
        log(format("Incorrect number of variants; there are {} and should\
 be {} including the wildcard variant.", variants.size(), expectedSize));
        allOK = false;
    }

    // Check that each permutation has a corresponding variant
    for (auto it = permutations.begin(); it != permutations.end(); ++it) {
        allOK &= variantExistsFor(variants, *it);
    }

    // Check for keys that aren't valid plural category names
    allOK &= checkValidKeys(locale, variants, pluralCategories);
    return allOK;
}

std::vector<UnicodeString> collectPlaceholders(const MFDataModel& dataModel) {
    // This collects the placeholders from the first variant,
    // then warns if any other variants use different placeholders.
    std::vector<Variant> variants = dataModel.getVariants();
    std::vector<UnicodeString> placeholders;
    bool first = true;
    for (auto variant = variants.begin(); variant != variants.end(); ++variant) {
        const Pattern& pat = variant->getPattern();
        for (auto patternPart = pat.begin(); patternPart != pat.end(); ++patternPart) {
            if (std::holds_alternative<Expression>(*patternPart)) {
                const Expression& expr = std::get<Expression>(*patternPart);
                 if (expr.getOperand().isVariable()) {
                     const VariableName& placeholder = expr.getOperand().asVariable();
                     if (!contains(placeholders, placeholder) && !first) {
                         log(format("Warning: not all variants in source message\
 contain the same set of placeholders. The placeholder ${} does not appear in\
 every variant.",
                                         fromUStr(placeholder)));
                     } else if (first) {
                         placeholders.push_back(expr.getOperand().asVariable());
                     }
                }
            }
        }
        first = false;
    }
    return placeholders;
}

bool variantContains(const Variant& variant, const UnicodeString& placeholder) {
    const Pattern& pat = variant.getPattern();
    for (auto patternPart = pat.begin(); patternPart != pat.end(); ++patternPart) {
        if (std::holds_alternative<Expression>(*patternPart)) {
            const Expression& expr = std::get<Expression>(*patternPart);
            if (expr.getOperand().isVariable()) {
                if (expr.getOperand().asVariable() == placeholder) {
                    return true;
                }

            }
        }
    }
    return false;
}

bool checkPlaceholders(const MFDataModel& sourceDataModel, const MFDataModel& targetDataModel) {
    std::vector<UnicodeString> sourcePlaceholders = collectPlaceholders(sourceDataModel);
    std::vector<Variant> targetVariants = targetDataModel.getVariants();
    for (auto variant = targetVariants.begin(); variant != targetVariants.end(); ++variant) {
        for (auto it = sourcePlaceholders.begin(); it != sourcePlaceholders.end(); ++it) {
            if (!variantContains(*variant, *it)) {
                log(format("In target message, variant with keys «{}» omits placeholder: ${}",
                                keysToString(variant->getKeys().getKeys()), fromUStr(*it)));
                return false;
            }
        }
    }
    return true;
}

void reportResults(const Locale& sourceLocale, const Locale& targetLocale,
                   bool sourceOK, bool targetOK, bool placeholdersOK) {
    log(format("Source locale: {}",
               localeToString(sourceLocale)));

    if (sourceOK) {
        log("Source message covers all plural categories.");
    } else {
        log("Source message does not cover all plural categories, or has extraneous categories.");
    }

    log(format("Target locale: {}",
               localeToString(targetLocale)));

    if (targetOK) {
        log("Target message covers all plural categories.");
    } else {
        log("Target message does not cover all plural categories, or has extraneous categories.");
    }

    if (placeholdersOK) {
        log("All variants in target message include placeholders from source message.");
    } else {
        log("One or more variants in target message omit placeholders from target message.");
    }
}

int main(int argc, char** argv) {
    Locale sourceLocale;
    Locale targetLocale;
    std::string sourceFilename;
    std::string targetFilename;
    bool verbose;

    // --locale_source --locale_target --message_source --message_target
    // first two flags are locale tags; second two are filenames
    getOptions(argc, argv, sourceLocale, targetLocale,
               sourceFilename, targetFilename, verbose, quiet);

    std::string sourceMessage = readFile(sourceFilename);
    std::string targetMessage = readFile(targetFilename);

    if (verbose) {
        echoOptions(sourceLocale, targetLocale, sourceMessage, targetMessage);
    }

    MFDataModel sourceDataModel = getDataModel(sourceLocale, sourceMessage);
    MFDataModel targetDataModel = getDataModel(targetLocale, targetMessage);

    log("== Checking source message ==");
    bool sourceOK = checkPluralCategories(sourceLocale, true, sourceDataModel);
    log("== Checking target message ==");
    bool targetOK = checkPluralCategories(targetLocale, false, targetDataModel);
    log("== Checking placeholder consistency ==");
    bool placeholdersOK = checkPlaceholders(sourceDataModel, targetDataModel);

    log("== Results ==");
    reportResults(sourceLocale, targetLocale, sourceOK, targetOK, placeholdersOK);

    return (sourceOK && targetOK && placeholdersOK) ? 0
        : !placeholdersOK ? INCONSISTENT_PLACEHOLDERS
        : MISSING_PLURAL_CATEGORY;
}
