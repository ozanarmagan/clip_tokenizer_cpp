#include <iostream>
#include "clip_tokenizer.h"


bool is_chinese_char(UChar32 ch) {
    return (ch >= 0x4E00 && ch <= 0x9FFF)   || 
           (ch >= 0x3400 && ch <= 0x4DBF)   || 
           (ch >= 0x20000 && ch <= 0x2A6DF) || 
           (ch >= 0x2A700 && ch <= 0x2B73F) || 
           (ch >= 0x2B740 && ch <= 0x2B81F) || 
           (ch >= 0x2B820 && ch <= 0x2CEAF) || 
           (ch >= 0xF900 && ch <= 0xFAFF)   || 
           (ch >= 0x2F800 && ch <= 0x2FA1F);
}

icu::UnicodeString tokenize_chinese(const icu::UnicodeString& text) {
    icu::UnicodeString result;
    for (int32_t i = 0; i < text.length(); ++i) {
        UChar32 ch = text.char32At(i);
        if (is_chinese_char(ch)) {
            result += " ";
            result += ch;
            result += " ";
        } else {
            result += ch;
        }
    }
    return result;
}

std::vector<int> get_bytes_to_unicode_vec() {
    std::vector<int> result;
    for (int i = 0; i < 256; ++i) {
        if((i < 33) || (i > 126 && i < 161) || (i == 173))
            continue;
        result.push_back(i);
    }

    // copy range to range2
    std::vector<int> range2 = result;
    
    int n = 0;
    for (int b = 0; b < 256; ++b) {
        if (std::find(result.begin(), result.end(), b) == result.end()) {
            result.push_back(256 + n);
            ++n;
        }
    }

    return result;
}

std::unordered_map<int, UChar32> CLIPTokenizer::bytes_to_unicode() {
    std::unordered_map<int, UChar32> byteToUnicode;

    std::vector<int> range, range2;

    for (int i = 0; i < 256; ++i) {
        if((i < 33) || (i > 126 && i < 161) || (i == 173))
            continue;
        range.push_back(i);
    }

    // copy range to range2
    range2 = range;
    
    int n = 0;
    for (int b = 0; b < 256; ++b) {
        if (std::find(range.begin(), range.end(), b) == range.end()) {
            range.push_back(b);
            range2.push_back(256 + n);
            ++n;
        }
    }

    for (size_t i = 0; i < range.size(); ++i) {
        byteToUnicode[range[i]] = UChar32(range2[i]);
    }

    return byteToUnicode;
}


std::set<std::pair<icu::UnicodeString, icu::UnicodeString>> CLIPTokenizer::get_pairs(const std::vector<icu::UnicodeString>& word) {
    std::set<std::pair<icu::UnicodeString, icu::UnicodeString>> pairs;
    if (word.size() <= 1) {
        return pairs;  // No pairs if the word has one or zero characters
    }

    icu::UnicodeString prev_char = word[0];
    for (size_t i = 1; i < word.size(); ++i) {
        pairs.insert(std::make_pair(prev_char, word[i]));
        prev_char = word[i];
    }

    return pairs;
}

std::vector<std::tuple<std::string, std::string>> CLIPTokenizer::get_merges(const std::string& file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        std::cout << "Error: Could not open file " << file_path << std::endl;
        return {};
    }
    std::string line;
    std::vector<std::string> merges;

    while (std::getline(file, line)) {
        merges.push_back(line);
    }

    std::vector<std::tuple<std::string, std::string>> result;
    for (size_t i = 1; i <= 49152 - 256 - 2; ++i) {
        std::istringstream merge_stream(merges[i]);
        std::string first, second;
        merge_stream >> first >> second;
        result.emplace_back(first, second);
    }

    return result;
}

icu::UnicodeString CLIPTokenizer::whitespace_clean(const icu::UnicodeString& text) {
    // Remove consecutive whitespace characters and replace with a single space
    icu::UnicodeString result;

    for (int32_t i = 0; i < text.length(); ++i) {
        UChar32 ch = text.char32At(i);
        if (u_isWhitespace(ch)) {
            if (result.length() == 0 || result.char32At(result.length() - 1) == ' ') {
                continue;
            } else {
                result += ' ';
            }
        } else {
            result += ch;
        }
    }

    return result;
}


CLIPTokenizer::CLIPTokenizer(const std::string& vocab_file) {
    matcher = std::make_unique<icu::RegexMatcher>("<\\|startoftext\\|>|<\\|endoftext\\|>|'s|'t|'re|'ve|'m|'ll|'d|[\\p{L}]+|[\\p{N}]|[^\\s\\p{L}\\p{N}]+", 0, status);
    byte_encoder = bytes_to_unicode();
    for (auto& it : byte_encoder) {
        byte_decoder[it.second] = it.first;
    }
    auto merges = get_merges(vocab_file);
    auto bytes_to_unicode_vec = get_bytes_to_unicode_vec();
    vocab.reserve(bytes_to_unicode_vec.size() + merges.size() + 2);
    for(const auto& v : get_bytes_to_unicode_vec()) {
        vocab.push_back(std::move(icu::UnicodeString(v)));
    }

    for(const auto& val : vocab) {
        vocab.push_back(val + "</w>");
    }

    for(const auto& [k,v] : merges) {
        vocab.push_back(icu::UnicodeString::fromUTF8(k + v));
    }


    vocab.push_back("<|startoftext|>");
    vocab.push_back("<|endoftext|>");
    for(size_t i = 0; i < vocab.size(); i++) {
        encoder[vocab[i]] = i;
        decoder[i] = vocab[i];
    }

    for(size_t i = 0; i < merges.size(); ++i) {
        bpe_ranks[icu::UnicodeString::fromUTF8(std::get<0>(merges[i]) + std::get<1>(merges[i]))] = i;
    }

    cache["<|startoftext|>"] = {"<|startoftext|>"};
    cache["<|endoftext|>"] = {"<|endoftext|>"};
}

std::vector<icu::UnicodeString> CLIPTokenizer::bpe(const icu::UnicodeString& text) {
    if (cache.find(text) != cache.end()) {
        return cache[text];
    }

    std::vector<icu::UnicodeString> word;
    for (int32_t i = 0; i < text.length() - 1; ++i) {
        word.push_back(text.tempSubString(i, 1));
    }
    word.push_back(text.tempSubString(text.length() - 1) + "</w>");
    auto pairs = get_pairs(word);
    if (pairs.empty()) {
        return {text + "</w>"};
    }

    while(1) {
        auto bigram = std::min_element(pairs.begin(), pairs.end(), [&](const std::pair<icu::UnicodeString, icu::UnicodeString>& l, const std::pair<icu::UnicodeString, icu::UnicodeString>& r) {
            int rank1 = INT32_MAX;
            if (bpe_ranks.find(l.first + l.second) != bpe_ranks.end()) {
                rank1 = bpe_ranks[l.first + l.second];
            }
            int rank2 = INT32_MAX;
            if (bpe_ranks.find(r.first + r.second) != bpe_ranks.end()) {
                rank2 = bpe_ranks[r.first + r.second];
            }
            return rank1 < rank2;
        });

        if (bpe_ranks.find(bigram->first + bigram->second) == bpe_ranks.end()) {
            break;
        }

        auto first = bigram->first;
        auto second = bigram->second;
        std::vector<icu::UnicodeString> new_word;

        size_t i = 0;

        while (i < word.size()) {
            size_t j = i;
            while (j < word.size()) {
                if (word[j] == first) {
                    break;
                }
                ++j;
            }
            for (size_t k = i; k < j; ++k) {
                new_word.push_back(word[k]);
            }

            if (j == word.size()) {
                break;
            } else {
                i = j;
            }

            if(word[i] == first && i < word.size() - 1 && word[i + 1] == second) {
                new_word.push_back(first + second);
                i += 2;
            } else {
                new_word.push_back(word[i]);
                ++i;
            }
        }

        word = new_word;
        if (word.size() == 1) {
            break;
        } else {
            pairs = get_pairs(word);
        }
    }

    cache[text] = word;
    return word;
}

std::vector<int> CLIPTokenizer::encode(icu::UnicodeString unicode_text) {
    std::vector<int> bpe_tokens;
    unicode_text = whitespace_clean(unicode_text);
    icu::Transliterator* strip_accents = icu::Transliterator::createInstance("NFD; [:Mn:] Remove", UTRANS_FORWARD, status);
    strip_accents->transliterate(unicode_text);
    if (status != U_ZERO_ERROR) {
        if(status == U_STRING_NOT_TERMINATED_WARNING) {
            status = U_ZERO_ERROR;
        } else {
            std::cout << "Error: " << u_errorName(status) << std::endl;
            bpe_tokens.push_back(encoder[icu::UnicodeString::fromUTF8("<|endoftext|>")]);
            return bpe_tokens;
        }
    }
    unicode_text = tokenize_chinese(unicode_text);
    unicode_text.toLower();
    unicode_text.trim();
    icu::UnicodeString word;
    size_t start = 0;
    matcher->reset(unicode_text);
    while (matcher->find()) {
        word = matcher->group(status);
        if (status != U_ZERO_ERROR) {
            if(status == U_STRING_NOT_TERMINATED_WARNING) {
                status = U_ZERO_ERROR;
            } else {
                std::cout << "Error: " << u_errorName(status) << std::endl;
                bpe_tokens.push_back(encoder[icu::UnicodeString::fromUTF8("<|endoftext|>")]);
                return bpe_tokens;
            }
        }
        std::string word_str;
        word.toUTF8String(word_str);
        if (word.length() == 0) {
            continue;
        }
        icu::UnicodeString encoder_result;

        for (int32_t j = 0; j < word_str.length(); ++j) {
            encoder_result += byte_encoder[(int)(word_str[j] & 0xff)];
        }
        auto bpe_res = bpe(encoder_result);
        for (auto& token : bpe_res) {
            if(bpe_tokens.size() >= MAX_LEN - 1) {
                bpe_tokens.push_back(encoder[icu::UnicodeString::fromUTF8("<|endoftext|>")]);
                break;
            } else if (encoder.find(token) == encoder.end()) {
                bpe_tokens.push_back(encoder[icu::UnicodeString::fromUTF8("<|endoftext|>")]);
            } else {
                bpe_tokens.push_back(encoder[token]);
            }
        }
    }

    delete strip_accents;

    return bpe_tokens;
}

TokenizerResult CLIPTokenizer::tokenize(const std::vector<std::string>& texts) {
    std::vector<std::vector<int>> result;
    for (const auto& text : texts) {
        icu::UnicodeString unicode_text = icu::UnicodeString::fromUTF8("<|startoftext|> ");
        unicode_text += icu::UnicodeString::fromUTF8(text);
        unicode_text += icu::UnicodeString::fromUTF8(" <|endoftext|>");
        std::vector<int> tokens = encode(unicode_text);
        result.push_back(tokens);
    }
    
    TokenizerResult tokenizer_result;

    size_t max_len = 0;
    for (const auto& tokens : result) {
        if (tokens.size() > max_len) {
            max_len = tokens.size();
        }
    }

    std::vector<std::vector<int>> attention_mask;
    std::vector<std::vector<int>> input_ids;
    for (auto& tokens : result) {
        std::vector<int> mask;
        for (size_t i = 0; i < tokens.size(); ++i) {
            mask.push_back(1);
        }
        while (mask.size() < max_len) {
            mask.push_back(0);
            tokens.push_back(encoder[icu::UnicodeString::fromUTF8("<|endoftext|>")]);
        }
        input_ids.push_back(tokens);
        attention_mask.push_back(mask);
    }

    tokenizer_result.attention_mask = attention_mask;
    tokenizer_result.tokens = input_ids;

    return tokenizer_result;
}