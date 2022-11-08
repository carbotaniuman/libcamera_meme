#pragma once

#include <string>
#include <cstddef>

class DmaBufAlloc {
public:
    explicit DmaBufAlloc(const std::string& heap_name);
    ~DmaBufAlloc();

    int alloc_buf_fd(std::size_t len);
private:
    int m_heap_fd;
};
