#pragma once

#include "PoolType.h"
#include "cocos/base/Macros.h"
#include "cocos/base/Object.h"
#include "cocos/base/TypeDef.h"
#include "cocos/base/memory/StlAlloc.h"
#include "cocos/bindings/jswrapper/Object.h"

namespace se {

class CC_DLL BufferAllocator final : public cc::Object {
public:
    template <class T>
    CC_INLINE static T *getBuffer(PoolType type, uint index) {
        uint size = 0;
        return BufferAllocator::getBuffer<T>(type, index, &size);
    }

    template <class T>
    static T *getBuffer(PoolType type, uint index, uint *size) {
        index &= _bufferMask;
        if (BufferAllocator::_pools.count(type) != 0) {
            const auto pool = BufferAllocator::_pools[type];
            if (pool->_buffers.count(index) != 0) {
                T *ret = nullptr;
                size_t len;
                pool->_buffers[index]->getArrayBufferData((uint8_t **)&ret, &len);
                *size = (uint)len;
                return ret;
            } else {
                return nullptr;
            }
        } else {
            return nullptr;
        }
    }

    BufferAllocator(PoolType type);
    ~BufferAllocator();

    Object *alloc(uint index, uint bytes);
    void free(uint index);

private:
    static cc::map<PoolType, BufferAllocator *> _pools;
    static constexpr uint _bufferMask = ~(1 << 30);

    cc::map<uint, Object *> _buffers;
    PoolType _type = PoolType::UNKNOWN;
};

} // namespace se
