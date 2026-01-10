#include "binary.hpp"

namespace io
{
BinaryFile::Ptr BinaryFile::Load(const std::string &filename)
{
    auto ptr = std::make_shared<BinaryFile>();

    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        return ptr;
    }

    size_t fileSize = (size_t)file.tellg();
    ptr->data.resize(fileSize);
    file.seekg(0);
    file.read(ptr->data.data(), fileSize);
    file.close();

    return ptr;
}
} // namespace io
