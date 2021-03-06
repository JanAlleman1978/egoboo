#pragma once

#include "egolib/Graphics/GraphicsWindow.hpp"

namespace Ego {
namespace SDL {	
	
/// @brief An SDL/OpenGL graphics window.
class GraphicsWindow : public Ego::GraphicsWindow
{
private:
    /// @brief A pointer to the SDL window.
    SDL_Window *window;

    /// @brief The SDL display index of the SDL window.
    /// @remark The display index is POLLED when updating the window.
    int displayIndex;

public:
    /// @brief Construct this graphics window with the specified window properties.
    /// @throw idlib::runtime_error window creation failed
	GraphicsWindow();
	
	/// @brief Destruct this graphics window.
	~GraphicsWindow();

public:
    /// @copydoc Ego::GraphicsWindow::setTitle
    void setTitle(const std::string& title) override;

    /// @copydoc Ego::GraphicsWindow::getTitle
    std::string getTitle() const override;

public:
    /// @copydoc Ego::GraphicsWindow::setGrabEnabled
    void setGrabEnabled(bool enabled) override;

    /// @copydoc Ego::GraphicsWindow::isGrabEnabled
    bool isGrabEnabled() const override;

public:
    /// @copydoc Ego::GraphicsWindow::getSize
    Vector2f getSize() const override;

    /// @copydoc Ego::GraphicsWindow::setSize
    void setSize(const Vector2f& size) override;

public:
    /// @copydoc Ego::GraphicsWindow::getPosition
    virtual Point2f getPosition() const override;

    /// @copydoc Ego::GraphicsWindow::setPosition
    virtual void setPosition(const Point2f& position) override;
	
public:
    /// @copydoc Ego::GraphicsWindow::center
    void center() override;

    /// @copydoc Ego::GraphicsWindow::setIcon
    void setIcon(SDL_Surface *icon) override;

    /// @copydoc Ego::GraphicsWindow::getDrawableSize
    Vector2f getDrawableSize() const override;

    /// @brief Update this window.
    void update() override;

public:
    /// @copydoc Ego::GraphicsWindow::get
    SDL_Window *get() override;

    /// @copydoc Ego::GraphicsWindow::getDisplayIndex
    int getDisplayIndex() const override;

    /// @copydoc Ego::GraphicsWindow::getContents
    std::shared_ptr<SDL_Surface> getContents() const override;
}; // class GraphicsWindow
	
} // namespace SDL
} // namespace Ego
