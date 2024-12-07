#pragma once

#include <fstream>
#include <memory>
#include <memory>

class BinaryFile
{
public:
    using Ptr = std::shared_ptr<BinaryFile>;
    static Ptr Load(const std::string &filename);

    bool IsEmpty() const
    {
        return data.empty();
    }

    const std::vector<char> &Bytes() const
    {
        return data;
    }

private:
    std::vector<char> data;
};
