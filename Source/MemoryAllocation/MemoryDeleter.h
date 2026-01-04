#pragma once

#include <cstddef>
#include <memory>

template <typename T>
struct MemoryDeleter {
    void operator()(T* memory) const noexcept
    {
        delete memory;
    }
};

template <typename T>
struct MemoryDeleter<T[]> {
    explicit MemoryDeleter(std::size_t numberOfElements) noexcept
        : numberOfElements{ numberOfElements }
    {
    }

    MemoryDeleter() = default;

    void operator()(T* memory) const noexcept
    {
        delete[] memory;
    }

    [[nodiscard]] std::size_t getNumberOfElements() const noexcept
    {
        return numberOfElements;
    }

private:
    std::size_t numberOfElements = 0;
};
