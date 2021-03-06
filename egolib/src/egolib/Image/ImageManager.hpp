//********************************************************************************************
//*
//*    This file is part of Egoboo.
//*
//*    Egoboo is free software: you can redistribute it and/or modify it
//*    under the terms of the GNU General Public License as published by
//*    the Free Software Foundation, either version 3 of the License, or
//*    (at your option) any later version.
//*
//*    Egoboo is distributed in the hope that it will be useful, but
//*    WITHOUT ANY WARRANTY; without even the implied warranty of
//*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//*    General Public License for more details.
//*
//*    You should have received a copy of the GNU General Public License
//*    along with Egoboo.  If not, see <http://www.gnu.org/licenses/>.
//*
//********************************************************************************************

/// @file egolib/Image/ImageManager.hpp
/// @brief An image manager.
/// @author Michael Heilmann

#pragma once

#include "egolib/Graphics/PixelFormat.hpp"
#include <SDL.h>
#undef main

namespace Ego {

// Forward declaration.
class ImageLoader;

/// @brief An image manager.
/// @todo
/// The image manager currently abstracts away the SDL_image/SDL image loading facilities.
/// It is - in the end - just a minor improvement over the previous code, just enough to get going.
class ImageManager : public idlib::singleton<ImageManager>
{
private:
    using Loaders = std::vector<std::unique_ptr<ImageLoader>>;

    /// Vector of available image loaders, ordered by priority from highest to lowest.
    Loaders loaders;

    struct Iterator : public std::iterator<std::forward_iterator_tag, ImageLoader>,
                      public idlib::increment_expr<Iterator>,
                      public idlib::equal_to_expr<Iterator>
    {
        ImageManager::Loaders::const_iterator m_inner;
    public:
        Iterator(const ImageManager::Loaders::const_iterator& inner);
    
	public:	
		// CRTP
        void increment();
		
		// CRTP
        bool equal_to(const Iterator& other) const;

    public:
        Iterator();

        Iterator(const Iterator& other);

        Iterator& operator=(const Iterator& other);

        reference operator*() const;

        reference operator->() const;
    };

private:
    /// @brief Construct this image manager.
    /// @remark Intentionally private.
    ImageManager();

    /// @brief Destruct this image manager.
    /// @remark Intentionally private.
    virtual ~ImageManager();

private:
    /// @brief Register the image loaders supported by SDL or SDL image.
    void registerImageLoaders();

public:
    friend idlib::default_new_functor<ImageManager>;
    friend idlib::default_delete_functor<ImageManager>;
    
    /// @brief Get an iterator pointing to the first loader supporting one of the specified extensions
    /// if such a loader exists, <tt>end()</tt> otherwise. The search range is <tt>[start, end())</tt>.
    Iterator find(std::unordered_set<std::string> extensions, Iterator start) const;

    /// @brief Get an iterator pointing to the first loader supporting one of the specified extensions
    /// if such a loader exists, <tt>end()</tt> otherwise. The search range is <tt>[start(), end())</tt>.
    /// @remark <tt>o.find(s)</tt> is equivalent to <tt>o.find(s,o.begin())</tt>.
    Iterator find(std::unordered_set<std::string> extensions);

    /// @brief Get an iterator pointing to the beginning of the loader list.
    /// @return an iterator pointing to the beginning of the loader list
    Iterator begin() const;

    /// @brief Get an iterator pointing to the end of the loader list.
    /// @return an iterator pointing to the end of the loader list
    Iterator end() const;

    /// @brief Get a cute default software(!) surface.
    /// @return a pointer to the surface on success, a null pointer on failure
    /// @remark The default image is a checkerboard texture consisting of
    /// 8 x 8 checkers each of a width and height of 16 x 16 pixels.
    /// The x,y-the checker is black if z = x + y * 8 is odd and is white otherwise.
	std::shared_ptr<SDL_Surface> getDefaultImage();

    /// @brief Create a software(!) surface of the specified width, height, and pixel format.
    /// @param width, height the width and the height
    /// @param pixel_descriptor the pixel descriptor of the pixels
    /// @param pixels a pointer to the pixels
    std::shared_ptr<SDL_Surface> createImage(size_t width, size_t height, size_t pitch, const pixel_descriptor& pixel_descriptor, void *pixels);

    /// @brief Create a software(!) surface of the specified width, height, and pixel format.
    /// @param width, height the width and the height
    /// @param pixel_descriptor the pixel descriptor of the pixels
    std::shared_ptr<SDL_Surface> createImage(size_t width, size_t height, const pixel_descriptor& pixel_descriptor);

    /// @brief Create a software(!) surface of the specified width and height.
    /// The pixel format is <c>Ego::pixel_format::R8G8B8A8</c>.
    /// @param width, height the width and the height
    /// @return a pointer to the surface on success, a null pointer on failure
    std::shared_ptr<SDL_Surface> createImage(size_t width, size_t height);

    /// @brief Save pixels to a BMP file.
    /// @param pixels the pixels to save
    /// @param pathname the pathname (relative or absolute path, filename, and extension) to save the image to
    void save_as_bmp(const std::shared_ptr<SDL_Surface>& pixels, const std::string& pathname);

    /// @brief Save pixels to a PNG file.
    /// @param pixels the pixels to save
    /// @param pathname the pathname (relative or absolute path, filename, and extension) to save the image to
    void save_as_png(const std::shared_ptr<SDL_Surface>& pixels, const std::string& pathname);
};

} // namespace Ego

/**
 * @todo
 *  This is by-passing the image loader, remove.
 */
std::shared_ptr<SDL_Surface> gfx_loadImage(const std::string& pathname);
