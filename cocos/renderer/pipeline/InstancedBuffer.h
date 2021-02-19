/****************************************************************************
Copyright (c) 2020 Xiamen Yaji Software Co., Ltd.

http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/
#pragma once

#include "Define.h"

namespace cc {
namespace gfx {
class Device;
}
namespace pipeline {
struct ModelView;
struct SubModelView;
struct PassView;
struct InstancedAttributeBlock;
struct PSOInfo;

#if defined(INITIAL_CAPACITY)
#undef INITIAL_CAPACITY
#endif

struct CC_DLL InstancedItem {
    uint count = 0;
    uint capacity = 0;
    gfx::Buffer *vb = nullptr;
    uint8_t *data = nullptr;
    gfx::InputAssembler *ia = nullptr;
    uint stride = 0;
    gfx::Shader *shader = nullptr;
    gfx::DescriptorSet *descriptorSet = nullptr;
    gfx::Texture *lightingMap = nullptr;
};
typedef vector<InstancedItem> InstancedItemList;
typedef vector<uint> DynamicOffsetList;

class InstancedBuffer : public Object {
public:
    static constexpr uint INITIAL_CAPACITY = 32;
    static constexpr uint MAX_CAPACITY = 1024;
    static InstancedBuffer *get(uint pass);
    static InstancedBuffer *get(uint pass, uint extraKey);

    InstancedBuffer(const PassView *pass);
    virtual ~InstancedBuffer();

    void destroy();
    void merge(const ModelView *, const SubModelView *, uint);
    void uploadBuffers(gfx::CommandBuffer *cmdBuff);
    void clear();
    void setDynamicOffset(uint idx, uint value);

    CC_INLINE const InstancedItemList &getInstances() const { return _instances; }
    CC_INLINE const PassView *getPass() const { return _pass; }
    CC_INLINE bool hasPendingModels() const { return _hasPendingModels; }
    CC_INLINE const DynamicOffsetList &dynamicOffsets() const { return _dynamicOffsets; }

private:
    static map<uint, map<uint, InstancedBuffer *>> _buffers;
    InstancedItemList _instances;
    const PassView *_pass = nullptr;
    bool _hasPendingModels = false;
    DynamicOffsetList _dynamicOffsets;
    gfx::Device *_device = nullptr;
};

} // namespace pipeline
} // namespace cc
