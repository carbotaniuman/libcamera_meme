#pragma once

#include <string>
#include <cstddef>

class DmaBufAlloc {
public:
    explicit DmaBufAlloc(const std::string& heap_name);
    int alloc_buf(std::size_t len);

    static void free_buf(int fd);
private:
    int m_heap_fd;
};
