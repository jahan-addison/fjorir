/*
 *   orianna is free software under GPL v3 -- see LICENSE for details.
 */

#pragma once

#include <memory>
#include <nanogui/opengl.h>

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#if defined(_MSC_VER)
#pragma warning(disable : 4505) // don't warn about dead code in stb_image.h
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wunused-function"
#endif
#include <stb_image.h>

namespace orianna {

class GLTexture
{
  public:
    using handleType = std::unique_ptr<uint8_t[], void (*)(void*)>;
    GLTexture() = default;
    GLTexture(const std::string& textureName)
      : mTextureName(textureName)
      , mTextureId(0)
    {
    }

    GLTexture(const std::string& textureName, GLint textureId)
      : mTextureName(textureName)
      , mTextureId(textureId)
    {
    }

    GLTexture(const GLTexture& other) = delete;
    GLTexture(GLTexture&& other) noexcept
      : mTextureName(std::move(other.mTextureName))
      , mTextureId(other.mTextureId)
    {
        other.mTextureId = 0;
    }
    GLTexture& operator=(const GLTexture& other) = delete;
    GLTexture& operator=(GLTexture&& other) noexcept
    {
        mTextureName = std::move(other.mTextureName);
        std::swap(mTextureId, other.mTextureId);
        return *this;
    }
    ~GLTexture() noexcept
    {
        if (mTextureId)
            glDeleteTextures(1, &mTextureId);
    }

    GLuint texture() const { return mTextureId; }
    const std::string& textureName() const { return mTextureName; }

    /**
     *  Load a file in memory and create an OpenGL texture.
     *  Returns a handle type (an std::unique_ptr) to the loaded pixels.
     */
    handleType load(const std::string& fileName)
    {
        if (mTextureId) {
            glDeleteTextures(1, &mTextureId);
            mTextureId = 0;
        }
        int force_channels = 0;
        int w, h, n;
        handleType textureData(
          stbi_load(fileName.c_str(), &w, &h, &n, force_channels),
          stbi_image_free);
        if (!textureData)
            throw std::invalid_argument(
              "Could not load texture data from file " + fileName);
        glGenTextures(1, &mTextureId);
        glBindTexture(GL_TEXTURE_2D, mTextureId);
        GLint internalFormat;
        GLint format;
        switch (n) {
            case 1:
                internalFormat = GL_R8;
                format = GL_red;
                break;
            case 2:
                internalFormat = GL_RG8;
                format = GL_RG;
                break;
            case 3:
                internalFormat = GL_RGB8;
                format = GL_RGB;
                break;
            case 4:
                internalFormat = GL_RGBA8;
                format = GL_RGBA;
                break;
            default:
                internalFormat = 0;
                format = 0;
                break;
        }
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     internalFormat,
                     w,
                     h,
                     0,
                     format,
                     GL_UNSIGNED_BYTE,
                     textureData.get());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        return textureData;
    }

  private:
    std::string mTextureName;
    GLuint mTextureId;
};
} // namespace orianna