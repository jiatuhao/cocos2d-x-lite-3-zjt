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
#include "PipelineStateManager.h"
#include "gfx/GFXDevice.h"
#include "gfx/GFXInputAssembler.h"
#include "gfx/GFXRenderPass.h"
#include "gfx/GFXShader.h"
#include "helper/SharedMemory.h"

namespace cc {
namespace pipeline {
map<uint, gfx::PipelineState *> PipelineStateManager::_PSOHashMap;
gfx::PipelineState *PipelineStateManager::getOrCreatePipelineState(const PassView *pass,
                                                                   gfx::Shader *shader,
                                                                   gfx::InputAssembler *inputAssembler,
                                                                   gfx::RenderPass *renderPass) {
    const auto passHash = pass->hash;
    const auto renderPassHash = renderPass->getHash();
    const auto iaHash = inputAssembler->getAttributesHash();
    const auto shaderID = shader->getID();
    const auto hash = passHash ^ renderPassHash ^ iaHash ^ shaderID;

    auto pso = _PSOHashMap[hash];
    if (!pso) {
        auto pipelineLayout = pass->getPipelineLayout();
        gfx::PipelineStateInfo info = {
            shader,
            pipelineLayout,
            renderPass,
            {inputAssembler->getAttributes()},
            *(pass->getRasterizerState()),
            *(pass->getDepthStencilState()),
            *(pass->getBlendState()),
            pass->getPrimitive(),
            pass->getDynamicState()};

        pso = gfx::Device::getInstance()->createPipelineState(std::move(info));
        _PSOHashMap[hash] = pso;
    }

    return pso;
}

gfx::PipelineState *PipelineStateManager::getOrCreatePipelineStateByJS(uint32_t passHandle,
                                                                       gfx::Shader *shader,
                                                                       gfx::InputAssembler *inputAssembler,
                                                                       gfx::RenderPass *renderPass) {
    const auto pass = GET_PASS(passHandle);
    CC_ASSERT(pass);
    return PipelineStateManager::getOrCreatePipelineState(pass, shader, inputAssembler, renderPass);
}

} // namespace pipeline
} // namespace cc
