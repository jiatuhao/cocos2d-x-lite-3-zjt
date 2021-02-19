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
#include "GLES3Std.h"

#include "GLES3Commands.h"
#include "GLES3Texture.h"

namespace cc {
namespace gfx {

GLES3Texture::GLES3Texture(Device *device)
: Texture(device) {
}

GLES3Texture::~GLES3Texture() {
}

bool GLES3Texture::initialize(const TextureInfo &info) {

    _type = info.type;
    _usage = info.usage;
    _format = info.format;
    _width = info.width;
    _height = info.height;
    _depth = info.depth;
    _layerCount = info.layerCount;
    _levelCount = info.levelCount;
    _samples = info.samples;
    _flags = info.flags;
    _size = FormatSize(_format, _width, _height, _depth);

#if CC_DEBUG > 0
    switch (_format) { // device feature validation
        case Format::D16:
            if (_device->hasFeature(Feature::FORMAT_D16)) break;
            CC_LOG_ERROR("D16 texture format is not supported on this backend");
            return false;
        case Format::D16S8:
            if (_device->hasFeature(Feature::FORMAT_D16S8)) break;
            CC_LOG_WARNING("D16S8 texture format is not supported on this backend");
            return false;
        case Format::D24:
            if (_device->hasFeature(Feature::FORMAT_D24)) break;
            CC_LOG_WARNING("D24 texture format is not supported on this backend");
            return false;
        case Format::D24S8:
            if (_device->hasFeature(Feature::FORMAT_D24S8)) break;
            CC_LOG_WARNING("D24S8 texture format is not supported on this backend");
            return false;
        case Format::D32F:
            if (_device->hasFeature(Feature::FORMAT_D32F)) break;
            CC_LOG_WARNING("D32F texture format is not supported on this backend");
            return false;
        case Format::D32F_S8:
            if (_device->hasFeature(Feature::FORMAT_D32FS8)) break;
            CC_LOG_WARNING("D32FS8 texture format is not supported on this backend");
            return false;
        case Format::RGB8:
            if (_device->hasFeature(Feature::FORMAT_RGB8)) break;
            CC_LOG_WARNING("RGB8 texture format is not supported on this backend");
            return false;
        default:
            break;
    }
#endif

    if (_flags & TextureFlags::BAKUP_BUFFER) {
        _buffer = (uint8_t *)CC_MALLOC(_size);
        if (!_buffer) {
            CC_LOG_ERROR("GLES3Texture: CC_MALLOC backup buffer failed.");
            return false;
        }
        _device->getMemoryStatus().textureSize += _size;
    }

    _gpuTexture = CC_NEW(GLES3GPUTexture);
    if (!_gpuTexture) {
        CC_LOG_ERROR("GLES3Texture: CC_NEW GLES3GPUTexture failed.");
        return false;
    }
    _gpuTexture->type = _type;
    _gpuTexture->format = _format;
    _gpuTexture->usage = _usage;
    _gpuTexture->width = _width;
    _gpuTexture->height = _height;
    _gpuTexture->depth = _depth;
    _gpuTexture->size = _size;
    _gpuTexture->arrayLayer = _layerCount;
    _gpuTexture->mipLevel = _levelCount;
    _gpuTexture->samples = _samples;
    _gpuTexture->flags = _flags;
    _gpuTexture->isPowerOf2 = math::IsPowerOfTwo(_width) && math::IsPowerOfTwo(_height);

    GLES3CmdFuncCreateTexture((GLES3Device *)_device, _gpuTexture);
    _device->getMemoryStatus().textureSize += _size;

    return true;
}

bool GLES3Texture::initialize(const TextureViewInfo &info) {
    _isTextureView = true;

    CC_LOG_ERROR("GLES3 doesn't support texture view.");
    return false;
}

void GLES3Texture::destroy() {
    if (_gpuTexture) {
        GLES3CmdFuncDestroyTexture((GLES3Device *)_device, _gpuTexture);
        _device->getMemoryStatus().textureSize -= _size;
        CC_DELETE(_gpuTexture);
        _gpuTexture = nullptr;
    }

    if (_buffer) {
        CC_FREE(_buffer);
        _device->getMemoryStatus().textureSize -= _size;
        _buffer = nullptr;
    }
}

void GLES3Texture::resize(uint width, uint height) {
    if (_width != width || _height != height) {
        uint size = FormatSize(_format, width, height, _depth);
        const uint oldSize = _size;
        _width = width;
        _height = height;
        _size = size;

        MemoryStatus &status = _device->getMemoryStatus();
        _gpuTexture->width = _width;
        _gpuTexture->height = _height;
        _gpuTexture->size = _size;
        GLES3CmdFuncResizeTexture((GLES3Device *)_device, _gpuTexture);
        status.bufferSize -= oldSize;
        status.bufferSize += _size;

        if (_buffer) {
            const uint8_t *oldBuffer = _buffer;
            uint8_t *buffer = (uint8_t *)CC_MALLOC(_size);
            if (!buffer) {
                CC_LOG_ERROR("GLES3Texture: CC_MALLOC backup buffer failed when resize the texture.");
                return;
            }
            memcpy(buffer, oldBuffer, std::min(oldSize, size));
            _buffer = buffer;
            CC_FREE(oldBuffer);
            status.bufferSize -= oldSize;
            status.bufferSize += _size;
        }
    }
}

} // namespace gfx
} // namespace cc
