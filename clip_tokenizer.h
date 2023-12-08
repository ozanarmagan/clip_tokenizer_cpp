#pragma once
#include <set>
#include <unordered_map>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <regex>
#include <locale>
#include <codecvt>
#include <memory>
#include <unicode/unistr.h>
#include <unicode/ustream.h>
#include <unicode/locid.h>
#include <unicode/uchar.h>
#include <unicode/regex.h>
#include <unicode/normalizer2.h>
#include <unicode/translit.h>
#include <unicode/ustream.h>

namespace std
{
    template<>
    class hash<icu::UnicodeString> {
    public:
        size_t operator()(const icu::UnicodeString &s) const 
        {
            return (size_t) s.hashCode();
        }
    };
};



struct TokenizerResult {
    std::vector<std::vector<int>> tokens;
    std::vector<std::vector<int>> attention_mask;
};


class CLIPTokenizer {
    private:
        std::unordered_map<int, UChar32> bytes_to_unicode();
        std::set<std::pair<icu::UnicodeString, icu::UnicodeString>> get_pairs(const std::vector<icu::UnicodeString>& word);
        std::vector<std::tuple<std::string, std::string>> get_merges(const std::string& file_path);
        icu::UnicodeString whitespace_clean(const icu::UnicodeString& text);
        std::vector<icu::UnicodeString> bpe(const icu::UnicodeString& text);
        std::unordered_map<int, UChar32> byte_encoder;
        std::unordered_map<UChar32, int> byte_decoder;
        std::vector<icu::UnicodeString> vocab;
        std::unordered_map<icu::UnicodeString, std::vector<icu::UnicodeString>> cache;
        std::unique_ptr<icu::RegexMatcher> matcher;
        std::unordered_map<icu::UnicodeString, int, std::hash<icu::UnicodeString>> bpe_ranks;
        std::unordered_map<icu::UnicodeString, int, std::hash<icu::UnicodeString>> encoder;
        std::unordered_map<int, icu::UnicodeString, std::hash<icu::UnicodeString>> decoder;
        UErrorCode status = U_ZERO_ERROR;
        std::vector<int> encode(icu::UnicodeString text);
    public:
        CLIPTokenizer(const std::string& vocab_file);
        TokenizerResult tokenize(const std::vector<std::string>& texts);
};


