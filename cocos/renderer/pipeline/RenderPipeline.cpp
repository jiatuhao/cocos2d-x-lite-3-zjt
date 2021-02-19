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
#include "RenderPipeline.h"
#include "RenderFlow.h"
#include "gfx/GFXCommandBuffer.h"
#include "gfx/GFXDescriptorSet.h"
#include "gfx/GFXDescriptorSetLayout.h"
#include "gfx/GFXDevice.h"
#include "gfx/GFXTexture.h"

namespace cc {
namespace pipeline {
RenderPipeline *RenderPipeline::_instance = nullptr;

RenderPipeline *RenderPipeline::getInstance() {
    return RenderPipeline::_instance;
}

RenderPipeline::RenderPipeline()
: _device(gfx::Device::getInstance()) {
    RenderPipeline::_instance = this;

    setDescriptorSetLayout();
}

RenderPipeline::~RenderPipeline() {
}

void RenderPipeline::setDescriptorSetLayout() {
    globalDescriptorSetLayout.bindings.resize(static_cast<size_t>(PipelineGlobalBindings::COUNT));

    globalDescriptorSetLayout.blocks[UBOGlobal::NAME] = UBOGlobal::LAYOUT;
    globalDescriptorSetLayout.bindings[UBOGlobal::BINDING] = UBOGlobal::DESCRIPTOR;
    globalDescriptorSetLayout.blocks[UBOCamera::NAME] = UBOCamera::LAYOUT;
    globalDescriptorSetLayout.bindings[UBOCamera::BINDING] = UBOCamera::DESCRIPTOR;
    globalDescriptorSetLayout.blocks[UBOShadow::NAME] = UBOShadow::LAYOUT;
    globalDescriptorSetLayout.bindings[UBOShadow::BINDING] = UBOShadow::DESCRIPTOR;
    globalDescriptorSetLayout.samplers[SHADOWMAP::NAME] = SHADOWMAP::LAYOUT;
    globalDescriptorSetLayout.bindings[SHADOWMAP::BINDING] = SHADOWMAP::DESCRIPTOR;
    globalDescriptorSetLayout.samplers[ENVIRONMENT::NAME] = ENVIRONMENT::LAYOUT;
    globalDescriptorSetLayout.bindings[ENVIRONMENT::BINDING] = ENVIRONMENT::DESCRIPTOR;
    globalDescriptorSetLayout.samplers[SPOT_LIGHTING_MAP::NAME] = SPOT_LIGHTING_MAP::LAYOUT;
    globalDescriptorSetLayout.bindings[SPOT_LIGHTING_MAP::BINDING] = SPOT_LIGHTING_MAP::DESCRIPTOR;

    localDescriptorSetLayout.bindings.resize(static_cast<size_t>(ModelLocalBindings::COUNT));
    localDescriptorSetLayout.blocks[UBOLocalBatched::NAME] = UBOLocalBatched::LAYOUT;
    localDescriptorSetLayout.bindings[UBOLocalBatched::BINDING] = UBOLocalBatched::DESCRIPTOR;
    localDescriptorSetLayout.blocks[UBOLocal::NAME] = UBOLocal::LAYOUT;
    localDescriptorSetLayout.bindings[UBOLocal::BINDING] = UBOLocal::DESCRIPTOR;
    localDescriptorSetLayout.blocks[UBOForwardLight::NAME] = UBOForwardLight::LAYOUT;
    localDescriptorSetLayout.bindings[UBOForwardLight::BINDING] = UBOForwardLight::DESCRIPTOR;
    localDescriptorSetLayout.blocks[UBOSkinningTexture::NAME] = UBOSkinningTexture::LAYOUT;
    localDescriptorSetLayout.bindings[UBOSkinningTexture::BINDING] = UBOSkinningTexture::DESCRIPTOR;
    localDescriptorSetLayout.blocks[UBOSkinningAnimation::NAME] = UBOSkinningAnimation::LAYOUT;
    localDescriptorSetLayout.bindings[UBOSkinningAnimation::BINDING] = UBOSkinningAnimation::DESCRIPTOR;
    localDescriptorSetLayout.blocks[UBOSkinning::NAME] = UBOSkinning::LAYOUT;
    localDescriptorSetLayout.bindings[UBOSkinning::BINDING] = UBOSkinning::DESCRIPTOR;
    localDescriptorSetLayout.blocks[UBOMorph::NAME] = UBOMorph::LAYOUT;
    localDescriptorSetLayout.bindings[UBOMorph::BINDING] = UBOMorph::DESCRIPTOR;
    localDescriptorSetLayout.samplers[JOINT_TEXTURE::NAME] = JOINT_TEXTURE::LAYOUT;
    localDescriptorSetLayout.bindings[JOINT_TEXTURE::BINDING] = JOINT_TEXTURE::DESCRIPTOR;
    localDescriptorSetLayout.samplers[POSITION_MORPH::NAME] = POSITION_MORPH::LAYOUT;
    localDescriptorSetLayout.bindings[POSITION_MORPH::BINDING] = POSITION_MORPH::DESCRIPTOR;
    localDescriptorSetLayout.samplers[NORMAL_MORPH::NAME] = NORMAL_MORPH::LAYOUT;
    localDescriptorSetLayout.bindings[NORMAL_MORPH::BINDING] = NORMAL_MORPH::DESCRIPTOR;
    localDescriptorSetLayout.samplers[TANGENT_MORPH::NAME] = TANGENT_MORPH::LAYOUT;
    localDescriptorSetLayout.bindings[TANGENT_MORPH::BINDING] = TANGENT_MORPH::DESCRIPTOR;
    localDescriptorSetLayout.samplers[LIGHTMAP_TEXTURE::NAME] = LIGHTMAP_TEXTURE::LAYOUT;
    localDescriptorSetLayout.bindings[LIGHTMAP_TEXTURE::BINDING] = LIGHTMAP_TEXTURE::DESCRIPTOR;
    localDescriptorSetLayout.samplers[SPRITE_TEXTURE::NAME] = SPRITE_TEXTURE::LAYOUT;
    localDescriptorSetLayout.bindings[SPRITE_TEXTURE::BINDING] = SPRITE_TEXTURE::DESCRIPTOR;
}

bool RenderPipeline::initialize(const RenderPipelineInfo &info) {
    _flows = info.flows;
    _tag = info.tag;
    return true;
}

bool RenderPipeline::activate() {
    if (_descriptorSetLayout) {
        _descriptorSetLayout->destroy();
        CC_DELETE(_descriptorSetLayout);
    }
    _descriptorSetLayout = _device->createDescriptorSetLayout({globalDescriptorSetLayout.bindings});

    if (_descriptorSet) {
        _descriptorSet->destroy();
        CC_DELETE(_descriptorSet);
    }
    _descriptorSet = _device->createDescriptorSet({_descriptorSetLayout});

    for (const auto flow : _flows)
        flow->activate(this);

    // has not initBuiltinRes,
    // create temporary default Texture to binding sampler2d
    if (!_defaultTexture) {
        _defaultTexture = _device->createTexture({
            gfx::TextureType::TEX2D,
            gfx::TextureUsageBit::COLOR_ATTACHMENT | gfx::TextureUsageBit::SAMPLED,
            gfx::Format::RGBA8,
            1u,
            1u,
        });
    }

    return true;
}

void RenderPipeline::render(const vector<uint> &cameras) {
    for (const auto flow : _flows) {
        for (const auto cameraID : cameras) {
            Camera* camera = GET_CAMERA(cameraID);
            flow->render(camera);
        }
    }
}

void RenderPipeline::destroy() {
    for (auto flow : _flows) {
        flow->destroy();
    }
    _flows.clear();

    CC_SAFE_DESTROY(_descriptorSetLayout);
    CC_SAFE_DESTROY(_descriptorSet);

    for (const auto cmdBuffer : _commandBuffers) {
        cmdBuffer->destroy();
    }
    _commandBuffers.clear();

    CC_SAFE_DESTROY(_defaultTexture);

    CC_SAFE_DELETE(_defaultTexture);
}

} // namespace pipeline
} // namespace cc
