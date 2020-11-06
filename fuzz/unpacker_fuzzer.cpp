// Fuzzer for the decompression mode of UPX, using libfuzzer.
#include <stddef.h>
#include <stdint.h>

#include <iostream>
#include <vector>
#include <fstream>

// From ../src/conf.h
extern int real_main(int argc, char** argv);

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  // This is the minimum size of file accepted. The fuzzing is faster if we
  // check this early on.
  if (size < 512) {
    return 0;
  }
  // Write file to disk.
  std::string name = std::tmpnam(nullptr);
  std::ofstream outfile;
  outfile.open(name, std::ios::binary | std::ios::out);
  outfile.write(reinterpret_cast<const char*>(data), size);
  outfile.close();

  std::vector<const char*> argv;
  argv.push_back("upx");
  // Test the UPX file
  argv.push_back("-t");
  argv.push_back(name.c_str());
  real_main(argv.size(), const_cast<char**>(argv.data()));

  // And cleanup temp file.
  std::remove(name.c_str());
  return 0;
}

