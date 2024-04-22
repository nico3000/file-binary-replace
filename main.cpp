#include <algorithm>
#include <fstream>
#include <iostream>
#include <vector>

void replace() {}

int main(int p_argc, char *p_argv[]) {
  if (p_argc != 5 && p_argc != 6) {
    std::string exe = p_argv[0];
    size_t sepPos = exe.find_last_of("/\\");
    std::cout << "Usage:" << std::endl
              << "file-binary-replace <input-file> <search-string> "
                 "<output-file> <replace-string> [--fill|-f]"
              << std::endl;
    return p_argc == 1 ? 0 : 1;
  }

  std::string inPath = p_argv[1];
  std::string search = p_argv[2];
  std::string outPath = p_argv[3];
  std::string replace = p_argv[4];
  bool fill = p_argc == 6 && (p_argv[5] == std::string("--fill") ||
                              p_argv[5] == std::string("-f"));
  if (fill && search.size() < replace.size()) {
    std::cerr << "Search string length must not be less than replace string if "
                 "fill is enabled."
              << std::endl;
    return 1;
  }
  std::ifstream in(inPath, std::ios::binary | std::ios::ate);
  if (!in) {
    std::cerr << "Failed to open for reading: " << inPath << std::endl;
    return 1;
  }
  size_t size = in.tellg();
  if (size == 0) {
    std::cerr << "Empty file: " << inPath << std::endl;
    return 1;
  }
  in.seekg(0);
  std::vector<char> contents(size);
  in.read(contents.data(), size);
  in.close();
  uint32_t count = 0;
  auto searchIt = contents.begin();
  while ((searchIt = std::search(searchIt, contents.end(), search.begin(),
                                 search.end())) != contents.end()) {
    size_t offset = searchIt - contents.begin();
    std::cout << "Occurence found at " << offset << "." << std::endl;
    if (fill) {
      memcpy(&contents[offset], replace.c_str(), replace.size());
      if (replace.size() != search.size()) {
        memset(&contents[offset + replace.size()], 0,
               search.size() - replace.size());
      }
    } else if (replace.size() <= search.size()) {
      memcpy(&contents[offset], replace.c_str(), replace.size());
      contents.erase(searchIt + replace.size(), searchIt + search.size());
    } else {
      std::vector<char> newContents(contents.begin(), searchIt);
      newContents.insert(newContents.end(), replace.begin(), replace.end());
      newContents.insert(newContents.end(), searchIt + search.size(),
                         contents.end());
      contents = std::move(newContents);
    }
    searchIt += search.size();
    ++count;
  }
  if (count == 0) {
    std::cerr << "No occurences found." << std::endl;
    return 1;
  }
  std::ofstream out(outPath, std::ios::binary);
  if (!out) {
    std::cerr << "Failed to open for writing: " << outPath << std::endl;
    return 1;
  }
  out.write(contents.data(), contents.size());
  out.close();
  std::cout << count << " occurence(s) replaced." << std::endl;
  return 0;
}