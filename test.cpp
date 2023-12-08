#include <iostream>
#include "clip_tokenizer.h"




int main() {

    CLIPTokenizer tokenizer("vocab.txt");
    TokenizerResult result = tokenizer.tokenize({"hello world"});

    std::cout << "Tokens: " << std::endl;
    for (auto& token : result.tokens[0]) {
        std::cout << token << " ";
    }
    std::cout << std::endl;

    std::cout << "Attention mask: " << std::endl;
    for (auto& mask : result.attention_mask[0]) {
        std::cout << mask << " ";
    }
    return 0;
}