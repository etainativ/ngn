#include "utils/file.h"
#include <fstream>

namespace utils {
    std::vector<uint32_t> readFile(const char *fp) {
	std::ifstream file(fp, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
	    abort();
	}

	// find what the size of the file is by looking up the location of the cursor
	// because the cursor is at the end, it gives the size directly in bytes
	size_t fileSize = (size_t)file.tellg();

	// spirv expects the buffer to be on uint32, so make sure to reserve a int
	// vector big enough for the entire file
	std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

	// put file cursor at beginning
	file.seekg(0);

	// load the entire file into the buffer
	file.read((char*)buffer.data(), fileSize);

	// now that the file is loaded into the buffer, we can close it
	file.close();
	return buffer;
    }
}
