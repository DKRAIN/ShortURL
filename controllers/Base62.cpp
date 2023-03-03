#include "Base62.h"
std::string Base62::encode(uint64_t decoding) {
  std::string encoding{};

  do {
    encoding.insert(0, std::string(1, characterSet[decoding % 62]));
    decoding /= 62;
  } while (decoding > 0);

  return encoding;
}

uint64_t Base62::decode(std::string encoding) {
  uint64_t decoding{};
  uint64_t count{1};

  std::reverse(encoding.begin(), encoding.end());
  for (const char &character : encoding) {
    size_t f = characterSet.find(character);
    if (f == std::string::npos) {
      throw -1;
    }
    decoding += f * count;
    count *= 62;
  }

  return decoding;
}