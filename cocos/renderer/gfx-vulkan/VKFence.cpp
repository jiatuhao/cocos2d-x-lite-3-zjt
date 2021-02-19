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
#include "VKStd.h"

#include "VKDevice.h"
#include "VKFence.h"
#include "VKCommands.h"

namespace cc {
namespace gfx {

CCVKFence::CCVKFence(Device *device)
: Fence(device) {
}

CCVKFence::~CCVKFence() {
}

bool CCVKFence::initialize(const FenceInfo &info) {
    _gpuFence = CC_NEW(CCVKGPUFence);
    if (!_gpuFence) {
        CC_LOG_ERROR("CCVKFence: CC_NEW CCVKGPUFence failed.");
        return false;
    }

    CCVKCmdFuncCreateFence((CCVKDevice *)_device, _gpuFence);

    return true;
}

void CCVKFence::destroy() {
    if (_gpuFence) {
        ((CCVKDevice *)_device)->gpuRecycleBin()->collect(_gpuFence);
        _gpuFence = nullptr;
    }
}

// TODO: move these two to device
void CCVKFence::wait() {
    VK_CHECK(vkWaitForFences(((CCVKDevice *)_device)->gpuDevice()->vkDevice, 1, &_gpuFence->vkFence, VK_TRUE, DEFAULT_TIMEOUT));
}

void CCVKFence::reset() {
    VK_CHECK(vkResetFences(((CCVKDevice *)_device)->gpuDevice()->vkDevice, 1, &_gpuFence->vkFence));
}

} // namespace gfx
} // namespace cc
