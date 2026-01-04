#pragma once

#include <algorithm>
#include <cstddef>
#include <new>
#include <span>

#include <BuildConfig.h>
#include <Utils/Align.h>

#include "FreeMemoryRegionList.h"
#include "MemoryAllocatorBase.h"

namespace
{
struct MemoryBlockHeader {
    std::size_t size;
};

constexpr auto kHeaderSize = utils::align<sizeof(MemoryBlockHeader), FreeMemoryRegionList::minimumAlignment()>();

[[nodiscard]] std::size_t normalizedSize(std::size_t requestedSize) noexcept
{
    requestedSize = (std::max)(requestedSize, static_cast<std::size_t>(1));
    return utils::align(requestedSize, FreeMemoryRegionList::minimumAlignment());
}

[[nodiscard]] auto& freeRegionList() noexcept
{
    alignas(FreeMemoryRegionList::minimumAlignment()) static constinit std::byte storage[build::MEMORY_CAPACITY];
    static constinit FreeMemoryRegionList list{ std::span<std::byte>{ storage } };
    return list;
}

[[nodiscard]] std::byte* allocateBlock(std::size_t requestedSize) noexcept
{
    const auto allocationSize = kHeaderSize + normalizedSize(requestedSize);
    if (const auto region = freeRegionList().allocate(allocationSize)) {
        auto* header = reinterpret_cast<MemoryBlockHeader*>(region);
        header->size = allocationSize;
        return reinterpret_cast<std::byte*>(header) + kHeaderSize;
    }
    return nullptr;
}

void deallocateBlock(std::byte* memory) noexcept
{
    if (!memory)
        return;

    auto* header = reinterpret_cast<MemoryBlockHeader*>(memory - kHeaderSize);
    freeRegionList().deallocate(reinterpret_cast<std::byte*>(header), header->size);
}
}

std::byte* MemoryAllocatorBase::allocate(std::size_t size) noexcept
{
    return allocateBlock(size);
}

void MemoryAllocatorBase::deallocate(std::byte* memory, std::size_t) noexcept
{
    deallocateBlock(memory);
}

void* operator new(std::size_t size)
{
    if (const auto memory = MemoryAllocatorBase::allocate(size))
        return memory;
    throw std::bad_alloc{};
}

void* operator new(std::size_t size, const std::nothrow_t&) noexcept
{
    return MemoryAllocatorBase::allocate(size);
}

void* operator new[](std::size_t size)
{
    if (const auto memory = MemoryAllocatorBase::allocate(size))
        return memory;
    throw std::bad_alloc{};
}

void* operator new[](std::size_t size, const std::nothrow_t&) noexcept
{
    return MemoryAllocatorBase::allocate(size);
}

void operator delete(void* memory) noexcept
{
    MemoryAllocatorBase::deallocate(static_cast<std::byte*>(memory), 0);
}

void operator delete(void* memory, std::size_t) noexcept
{
    MemoryAllocatorBase::deallocate(static_cast<std::byte*>(memory), 0);
}

void operator delete(void* memory, const std::nothrow_t&) noexcept
{
    MemoryAllocatorBase::deallocate(static_cast<std::byte*>(memory), 0);
}

void operator delete[](void* memory) noexcept
{
    MemoryAllocatorBase::deallocate(static_cast<std::byte*>(memory), 0);
}

void operator delete[](void* memory, std::size_t) noexcept
{
    MemoryAllocatorBase::deallocate(static_cast<std::byte*>(memory), 0);
}

void operator delete[](void* memory, const std::nothrow_t&) noexcept
{
    MemoryAllocatorBase::deallocate(static_cast<std::byte*>(memory), 0);
}
