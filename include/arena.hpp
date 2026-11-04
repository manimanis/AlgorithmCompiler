#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>
#include <utility>
#include <algorithm>
#include <new>

namespace algo {

class Arena {
public:
    explicit Arena(size_t chunkSize = 64 * 1024)
        : defaultChunkSize_(chunkSize) {}

    ~Arena() {
        reset();
    }

    Arena(const Arena&) = delete;
    Arena& operator=(const Arena&) = delete;
    Arena(Arena&&) noexcept = default;
    Arena& operator=(Arena&&) noexcept = default;

    void* allocate(size_t bytes, size_t alignment = alignof(std::max_align_t)) {
        if (bytes == 0) return nullptr;

        // Try allocating in the current chunk
        if (!chunks_.empty()) {
            Chunk& current = chunks_.back();
            uintptr_t currentAddr = reinterpret_cast<uintptr_t>(current.data.get() + current.used);
            uintptr_t alignedAddr = (currentAddr + alignment - 1) & ~(alignment - 1);
            size_t padding = alignedAddr - currentAddr;

            if (current.used + padding + bytes <= current.capacity) {
                current.used += padding + bytes;
                totalUsed_ += padding + bytes;
                return reinterpret_cast<void*>(alignedAddr);
            }
        }

        // Allocate a new chunk large enough
        size_t newCapacity = std::max(defaultChunkSize_, bytes + alignment);
        Chunk newChunk;
        newChunk.data = std::make_unique<uint8_t[]>(newCapacity);
        newChunk.capacity = newCapacity;

        uintptr_t baseAddr = reinterpret_cast<uintptr_t>(newChunk.data.get());
        uintptr_t alignedAddr = (baseAddr + alignment - 1) & ~(alignment - 1);
        size_t padding = alignedAddr - baseAddr;

        newChunk.used = padding + bytes;
        void* result = reinterpret_cast<void*>(alignedAddr);

        chunks_.push_back(std::move(newChunk));
        totalAllocated_ += newCapacity;
        totalUsed_ += padding + bytes;

        return result;
    }

    template <typename T, typename... Args>
    T* create(Args&&... args) {
        void* mem = allocate(sizeof(T), alignof(T));
        T* obj = ::new (mem) T(std::forward<Args>(args)...);

        if constexpr (!std::is_trivially_destructible_v<T>) {
            destructors_.push_back([](void* p) {
                static_cast<T*>(p)->~T();
            });
            destructorObjects_.push_back(mem);
        }
        return obj;
    }

    template <typename T, typename... Args>
    std::shared_ptr<T> makeShared(Args&&... args) {
        T* obj = create<T>(std::forward<Args>(args)...);
        // Custom no-op deleter: memory lifetime is owned by this Arena
        return std::shared_ptr<T>(obj, [](T*) { /* Arena owns memory */ });
    }

    void reset() {
        // Run destructors in reverse order of allocation
        for (size_t i = destructors_.size(); i > 0; --i) {
            destructors_[i - 1](destructorObjects_[i - 1]);
        }
        destructors_.clear();
        destructorObjects_.clear();
        chunks_.clear();
        totalAllocated_ = 0;
        totalUsed_ = 0;
    }

    size_t totalAllocated() const { return totalAllocated_; }
    size_t totalUsed() const { return totalUsed_; }
    size_t chunkCount() const { return chunks_.size(); }

private:
    struct Chunk {
        std::unique_ptr<uint8_t[]> data;
        size_t capacity = 0;
        size_t used = 0;
    };

    size_t defaultChunkSize_;
    std::vector<Chunk> chunks_;
    std::vector<void (*)(void*)> destructors_;
    std::vector<void*> destructorObjects_;
    size_t totalAllocated_ = 0;
    size_t totalUsed_ = 0;
};

} // namespace algo
