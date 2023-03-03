#pragma once
#include<algorithm> 
#include <string>

namespace Base62 {
std::string encode(uint64_t decoding);
uint64_t decode(std::string encoding);
const std::string characterSet =
    "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

} // namespace Base62
