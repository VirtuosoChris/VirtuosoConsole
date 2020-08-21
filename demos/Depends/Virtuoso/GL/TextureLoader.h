#ifndef VIRTUOSO_TEXTURE_LOADER_H_INCLUDED
#define VIRTUOSO_TEXTURE_LOADER_H_INCLUDED

#if !defined(GL_ALT_API_NAME)
#error "Missing dependency : include a glalt header before including TextureLoader.h"
#endif

#if !defined(GL_HPP)
#error "Missing dependency : include opengl.hpp before including TextureLoader.h"
#endif

#ifdef __ANDROID_API__
#include <android/asset_manager.h>
#endif

#if defined(__ANDROID_API__) && defined(USE_NATIVE_ACTIVITY)
struct android_app;
#endif

namespace Virtuoso
{
    namespace GL {
        /// Client side input data used to initialize a GL texture
        struct TextureInputData {
            int width;
            int height;
            int channels;
            int unpackAlignment;

            GLvoid *pixels;

            GLenum type;
            GLenum format;

            std::function <void(
                    void *)> deleter; ///< Function used to delete memory when TextureInputData dtor called.  Defaults to "free"

            TextureInputData() : width(0), height(0), channels(0), unpackAlignment(4), pixels(NULL),
                                 type(0), format(0), deleter(free) { }

            inline operator bool() const { return pixels != NULL; }

            void useUnpackAlignment() const;

            TextureInputData(const TextureInputData &) = delete;

            TextureInputData &operator=(const TextureInputData &) = delete;

            TextureInputData(TextureInputData &&other);

            TextureInputData &operator=(TextureInputData &&other);

            ~TextureInputData();
        };

        /*** Texture helpers + shorthands ***/

        inline void setRepeatModeUV(gl::Texture &tex, GLenum mode);

        inline void setFilterNearest(gl::Texture & tex);

        inline void setFilterNearestMip(gl::Texture & tex);

        inline void setFilterBilinear(gl::Texture & tex);

        inline void setFilterBilinearMip(gl::Texture & tex);

        inline void setFilterTrilinear(gl::Texture & tex);

        inline unsigned int maxMipmapLevelsForTexture(unsigned int width, unsigned int height);

        /*** Texture allocators and initializers ***/

        inline gl::Texture allocateTexture(unsigned int width, unsigned int height, GLenum format,
                                           unsigned int levels = 0);


        inline gl::Texture allocateTexture(const TextureInputData &data, GLenum format,
                                           unsigned int levels = 0);

        inline gl::Texture allocateCubeTexture(unsigned int width, unsigned int height,
                                               GLenum format, unsigned int levels = 0);


        inline gl::Texture allocateCubeTexture(TextureInputData *faces, GLenum format,
                                               unsigned int levels = 0);


        /// takes array of texture input data in {+-X,+-Y,+-Z} order for the faces and populates input texture tex at "level"
        void fillCubeTextureWithFaceData(gl::Texture &tex, const TextureInputData *images,
                                         int level = 0);

        /// fills "tex" at miplevel "level" with data in "image"
        void fillTextureWithData(gl::Texture &tex, const TextureInputData &image, int level = 0);

        /*** Include stb_image.h prior to this to enable these loaders ***/
///#ifdef STBI_INCLUDE_STB_IMAGE_H
        TextureInputData loadTextureDataFromFile(const std::string& filename);

        #if defined(__ANDROID_API__) && defined(USE_NATIVE_ACTIVITY)
        TextureInputData loadTextureDataFromAsset(const std::string& path,  android_app* app);
        TextureInputData loadFloatTextureDataFromAsset(const std::string& path,  android_app* app);
        #endif

#ifdef __ANDROID_API__
        TextureInputData loadTextureDataFromAsset(const std::string& path, AAssetManager* assetManager);
        TextureInputData loadFloatTextureDataFromAsset(const std::string& path,  AAssetManager* assetManager);
#endif
///#endif

        /*** INLINE IMPLEMENTATIONS ***/

        inline void setRepeatModeUV(gl::Texture &tex, GLenum mode) {
            tex.Parameter(GL_TEXTURE_WRAP_S, (GLint) mode);///\todo that's really annoying
            tex.Parameter(GL_TEXTURE_WRAP_T, (GLint) mode);
        }

        inline void setFilterNearest(gl::Texture & tex) {
            tex.Parameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            tex.Parameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        }

        inline void setFilterNearestMip(gl::Texture & tex) {
            tex.Parameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            tex.Parameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
        }

        inline void setFilterBilinear(gl::Texture & tex) {
            tex.Parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            tex.Parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        }

        inline void setFilterBilinearMip(gl::Texture & tex) {
            tex.Parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            tex.Parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        }

        inline void setFilterTrilinear(gl::Texture & tex) {
            tex.Parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            tex.Parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        }

        inline unsigned int maxMipmapLevelsForTexture(unsigned int width, unsigned int height) {
            unsigned int largestDim = std::max < unsigned
            int > (width, height);

            int tmp = largestDim;

            unsigned int levels = 1;

            while (tmp >>= 1) {
                levels++;
            }

            return levels;
        }

        inline gl::Texture allocateTexture(unsigned int width, unsigned int height, GLenum format,
                                           unsigned int levels) {
            gl::Texture rval;

            if (!levels) levels = maxMipmapLevelsForTexture(width, height);

            rval.Storage2D(levels, format, width, height);

            return rval;
        }

        inline gl::Texture allocateCubeTexture(unsigned int width, unsigned int height,
                                               GLenum format, unsigned int levels) {
            gl::Texture rval;

            if (!levels) levels = maxMipmapLevelsForTexture(width, height);

            ///texturestorage doesn't take a target so DSA will fail if we don't bind the texture to the correct target first
            rval.Bind(GL_TEXTURE_CUBE_MAP);
            rval.Storage2D(GL_TEXTURE_CUBE_MAP, levels, format, width, height);

            return rval;
        }

        inline gl::Texture allocateTexture(const TextureInputData &data, GLenum format,
                                           unsigned int levels) {

            if (!levels) levels = maxMipmapLevelsForTexture(data.width, data.height);

            gl::Texture rval;

            rval.Storage2D(levels, format, data.width, data.height);

            fillTextureWithData(rval, data, 0);

            return rval;
        }

        inline gl::Texture allocateCubeTexture(TextureInputData *faces, GLenum format,
                                               unsigned int levels) {
            gl::Texture rval;

            if (!levels) levels = maxMipmapLevelsForTexture(faces[0].width, faces[0].height);

            ///texturestorage doesn't take a target so DSA will fail if we don't bind the texture to the correct target first
            ///\todo this comment is not accurate, test once we have the cube test back
            rval.Bind(GL_TEXTURE_CUBE_MAP);

            rval.Storage2D(GL_TEXTURE_CUBE_MAP, levels, format, faces[0].width, faces[0].height);

            fillCubeTextureWithFaceData(rval, faces, 0);

            return rval;
        }


        /// fills a previously allocated texture level.
        ///\todo should make this an interface function to a fillTextureSubdataFromFile, eg, if we fill in a tile atlas
        inline void fillTextureWithData(gl::Texture &tex, const TextureInputData &img, int level)
        {
            GLint unpackAlignment = gl::Get<GLint>(GL_UNPACK_ALIGNMENT);
            img.useUnpackAlignment();

            tex.SubImage2D(level, 0, 0, img.width, img.height, img.format, img.type, img.pixels);

            glPixelStorei(GL_UNPACK_ALIGNMENT, unpackAlignment);
        }
    }
} //namespace Virtuoso

#endif

#ifdef VIRTUOSO_TEXTURELOADER_IMPLEMENTATION
namespace Virtuoso
{
    namespace GL
    {
        void TextureInputData::useUnpackAlignment()const
        {
            glPixelStorei(GL_UNPACK_ALIGNMENT, unpackAlignment);
        }

        TextureInputData::~TextureInputData()
        {
            if (pixels)
            {
                deleter(pixels);
                pixels = NULL;
            }
        }

        TextureInputData::TextureInputData(TextureInputData&& other)
            : width(other.width), height(other.height), channels(other.channels),
            unpackAlignment(other.unpackAlignment), pixels(other.pixels), type(other.type), format(other.format), deleter(other.deleter)
        {
            other.pixels = NULL;
        }

        TextureInputData& TextureInputData::operator=(TextureInputData&& other)
        {
            if (this != &other)
            {
                width = other.width;
                height = other.height;
                channels = other.channels;
                unpackAlignment = other.unpackAlignment;
                pixels = other.pixels;
                type = other.type;
                format = other.format;
                deleter = other.deleter;
                other.pixels = NULL;
            }

            return *this;
        }


        void fillCubeTextureWithFaceData(gl::Texture& tex, const TextureInputData* images, int level)
        {
            GLint unpackAlignment = gl::Get<GLint>(GL_UNPACK_ALIGNMENT);

            if (glGetError() != GL_NO_ERROR)
            {
                std::cout << "ERROR AFTER GET PACK ALIGN" << std::endl;
            }

            for (int i = 0; i < 6; i++)
            {
                images[i].useUnpackAlignment();

                tex.SubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, level, 0, 0, images[i].width, images[i].height, images[i].format, images[i].type, images[i].pixels);

                if (images[i].width != images[0].width || images[i].height != images[0].height)
                {
                    throw std::runtime_error("Inputs for cubemap loader have mismatched dimensions");
                }

                if (images[i].width != images[i].height)
                {
                    throw std::runtime_error("Cube map face inputs not square");
                }
            }

            glPixelStorei(GL_UNPACK_ALIGNMENT, unpackAlignment);
        }

        #ifdef STBI_INCLUDE_STB_IMAGE_H
        TextureInputData loadTextureDataFromFile(const std::string& filename)
        {
            TextureInputData rval;

            std::clog << "Loading texture from file : " << filename << std::endl;

            ///\todo this works based on the contents of the file.  this will not work with an android asset
            if (stbi_is_hdr(filename.c_str()))
            {
                rval.pixels = stbi_loadf(filename.c_str(), &rval.width, &rval.height, &rval.channels, 0);

                if (!rval.pixels)
                {
                    throw std::runtime_error(std::string("HDRImage load error: \nFile ") + filename + "\nReason:" + stbi_failure_reason());
                }

                rval.type = GL_FLOAT;
            }
            else
            {
                rval.pixels = stbi_load(filename.c_str(), &rval.width, &rval.height, &rval.channels, 0);

                if (!rval.pixels)
                {
                    throw std::runtime_error(std::string("LDRImage load error: \nFile ") + filename + "\nReason:" + stbi_failure_reason());
                }

                rval.type = GL_UNSIGNED_BYTE;
            }

            #if defined(GL_LUMINANCE) && defined(GL_LUMINANCE_ALPHA)
            GLenum formatFromChannels[] = { GL_LUMINANCE, GL_LUMINANCE_ALPHA, GL_RGB, GL_RGBA };
            #else
            GLenum formatFromChannels[] = { GL_RED, GL_RG, GL_RGB, GL_RGBA };
            #endif

            rval.format = formatFromChannels[rval.channels - 1];
            rval.deleter = stbi_image_free;
            rval.unpackAlignment = 1;

            std::clog<<"\ttexture dimensions : "<<rval.width<<" "<<rval.height<<std::endl;

            return rval;
        }

        #if defined(__ANDROID_API__) && defined(USE_NATIVE_ACTIVITY)
        TextureInputData loadTextureDataFromAsset(const std::string& path, struct android_app* app)
        {
            std::clog << "Loading texture from asset : " << path << std::endl;

            TextureInputData rval;

            AAsset* asset = AAssetManager_open(app->activity->assetManager, path.c_str(), AASSET_MODE_UNKNOWN);

            if(!asset) throw std::runtime_error((std::string("Unable to open asset ") + path).c_str());

            const unsigned char* buffer = static_cast<const unsigned char*>(AAsset_getBuffer(asset));

            ///\todo hdr and ldr
            rval.pixels =  stbi_load_from_memory( buffer, AAsset_getLength(asset), &rval.width, &rval.height, &rval.channels, 0);

            rval.deleter = stbi_image_free;
            rval.unpackAlignment = 1; // data is tightly packed

            AAsset_close(asset);

            #if defined(GL_LUMINANCE) && defined(GL_LUMINANCE_ALPHA)
            GLenum formatFromChannels[] = { GL_LUMINANCE, GL_LUMINANCE_ALPHA, GL_RGB, GL_RGBA };
            #else
            GLenum formatFromChannels[] = { GL_RED, GL_RG, GL_RGB, GL_RGBA };
            #endif

            rval.format = formatFromChannels[rval.channels - 1];
            rval.type = GL_UNSIGNED_BYTE;

            return rval;
        }

        TextureInputData loadFloatTextureDataFromAsset(const std::string& path, struct android_app* app)
        {
            std::clog << "Loading float texture from asset : " << path << std::endl;

            TextureInputData rval;

            AAsset* asset = AAssetManager_open(app->activity->assetManager, path.c_str(), AASSET_MODE_UNKNOWN);

            if(!asset) throw std::runtime_error((std::string("Unable to open asset ") + path).c_str());

            const unsigned char* buffer = static_cast<const unsigned char*>(AAsset_getBuffer(asset));

            ///\todo hdr and ldr
            rval.pixels =  stbi_loadf_from_memory( buffer, AAsset_getLength(asset), &rval.width, &rval.height, &rval.channels, 0);

            rval.deleter = stbi_image_free;
            rval.unpackAlignment = 1; // data is tightly packed

            AAsset_close(asset);

            #if defined(GL_LUMINANCE) && defined(GL_LUMINANCE_ALPHA)
            GLenum formatFromChannels[] = { GL_LUMINANCE, GL_LUMINANCE_ALPHA, GL_RGB, GL_RGBA };
            #else
            GLenum formatFromChannels[] = { GL_RED, GL_RG, GL_RGB, GL_RGBA };
            #endif

            rval.format = formatFromChannels[rval.channels - 1];
            rval.type = GL_FLOAT;

            return rval;
        }
        #endif //endif __ANDROID_API__


        #ifdef __ANDROID_API__
        TextureInputData loadTextureDataFromAsset(const std::string& path, AAssetManager* assetManager)
        {
            std::clog << "Loading texture from asset : " << path << std::endl;

            TextureInputData rval;

            if (!assetManager)
            {
                std::clog <<"Asset manager is null!" << std::endl;
            }

            AAsset* asset = AAssetManager_open(assetManager, path.c_str(), AASSET_MODE_UNKNOWN);

            if(!asset)
            {
                std::clog <<"unable to load asset " << std::endl;
                throw std::runtime_error((std::string("Unable to open asset ") + path).c_str());
            }

            std::clog <<" asset loaded " <<std::endl;

            const unsigned char* buffer = static_cast<const unsigned char*>(AAsset_getBuffer(asset));

            ///\todo hdr and ldr
            rval.pixels =  stbi_load_from_memory( buffer, AAsset_getLength(asset), &rval.width, &rval.height, &rval.channels, 0);

            rval.deleter = stbi_image_free;
            rval.unpackAlignment = 1; // data is tightly packed

            AAsset_close(asset);

            #if defined(GL_LUMINANCE) && defined(GL_LUMINANCE_ALPHA)
            GLenum formatFromChannels[] = { GL_LUMINANCE, GL_LUMINANCE_ALPHA, GL_RGB, GL_RGBA };
            #else
            GLenum formatFromChannels[] = { GL_RED, GL_RG, GL_RGB, GL_RGBA };
            #endif

            rval.format = formatFromChannels[rval.channels - 1];
            rval.type = GL_UNSIGNED_BYTE;

            return rval;
        }

        TextureInputData loadFloatTextureDataFromAsset(const std::string& path, AAssetManager* assetManager)
        {
            std::clog << "Loading texture from asset : " << path << std::endl;

            TextureInputData rval;

            AAsset* asset = AAssetManager_open(assetManager, path.c_str(), AASSET_MODE_UNKNOWN);

            if(!asset) throw std::runtime_error((std::string("Unable to open asset ") + path).c_str());

            const unsigned char* buffer = static_cast<const unsigned char*>(AAsset_getBuffer(asset));

            ///\todo hdr and ldr
            rval.pixels =  stbi_loadf_from_memory( buffer, AAsset_getLength(asset), &rval.width, &rval.height, &rval.channels, 0);

            rval.deleter = stbi_image_free;
            rval.unpackAlignment = 1; // data is tightly packed

            AAsset_close(asset);

            #if defined(GL_LUMINANCE) && defined(GL_LUMINANCE_ALPHA)
            GLenum formatFromChannels[] = { GL_LUMINANCE, GL_LUMINANCE_ALPHA, GL_RGB, GL_RGBA };
            #else
            GLenum formatFromChannels[] = { GL_RED, GL_RG, GL_RGB, GL_RGBA };
            #endif

            rval.format = formatFromChannels[rval.channels - 1];
            rval.type = GL_FLOAT;

            return rval;
        }
        #endif

        #endif //endif STBI_INCLUDE_STB_IMAGE_H
    }
}
#endif
