#include "clip_tokenizer.h"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/complex.h>

PYBIND11_MODULE(clip_tokenizer_py, m) {
    pybind11::class_<CLIPTokenizer>(m, "CLIPTokenizer")
        .def(pybind11::init<const std::string&>())
        .def("tokenize", &CLIPTokenizer::tokenize);
    
    pybind11::class_<TokenizerResult>(m, "TokenizerResult")
        .def(pybind11::init<>())
        .def_readwrite("tokens", &TokenizerResult::tokens)
        .def_readwrite("attention_mask", &TokenizerResult::attention_mask)
        .def("__repr__", [](const TokenizerResult &a) {
            std::string tokens;
            for (auto& token : a.tokens[0]) {
                tokens += std::to_string(token) + ", ";
            }
            tokens.pop_back();
            tokens.pop_back();
            std::string attention_mask;
            for (auto& mask : a.attention_mask[0]) {
                attention_mask += std::to_string(mask) + ", ";
            }
            attention_mask.pop_back();
            attention_mask.pop_back();
            return "<clip_tokenizer_py.TokenizerResult tokens=[" + tokens + "] attention_mask=[" + attention_mask + "]>";
        });
}   