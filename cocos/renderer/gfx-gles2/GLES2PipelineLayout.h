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
#ifndef CC_GFXGLES2_PIPELINE_LAYOUT_H_
#define CC_GFXGLES2_PIPELINE_LAYOUT_H_

namespace cc {
namespace gfx {

class GLES2GPUPipelineLayout;

class CC_GLES2_API GLES2PipelineLayout final : public PipelineLayout {
public:
    GLES2PipelineLayout(Device *device);
    ~GLES2PipelineLayout();

public:
    virtual bool initialize(const PipelineLayoutInfo &info) override;
    virtual void destroy() override;

    CC_INLINE GLES2GPUPipelineLayout *gpuPipelineLayout() const { return _gpuPipelineLayout; }

private:
    GLES2GPUPipelineLayout *_gpuPipelineLayout = nullptr;
};

} // namespace gfx
} // namespace cc

#endif // CC_GFXGLES2_PIPELINE_LAYOUT_H_