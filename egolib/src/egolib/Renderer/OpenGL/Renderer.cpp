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

/// @file   egolib/Renderer/OpenGL/Renderer.cpp
/// @brief  OpenGL 2.1 based renderer
/// @author Michael Heilmann

#include "egolib/Renderer/OpenGL/Renderer.hpp"
#include "egolib/Core/StringUtilities.hpp"
#include "egolib/Core/CollectionUtilities.hpp"

namespace Ego
{
	namespace OpenGL
	{

        /**
         * @brief
         *  Get the name of this OpenGL implementation.
         * @return
         *  the name of this OpenGL implementation
         */
        std::string getName()
        {
            while (GL_NO_ERROR != glGetError()) {}
            const GLubyte *bytes = glGetString(GL_RENDERER);
            GLenum error = glGetError();
            if (GL_NO_ERROR != error)
            {
                throw std::runtime_error("unable to acquire renderer back-end information");
            }
            return (const char *)bytes;
        }
        /**
         * @brief
         *  Get the name of the vendor of this OpenGL implementation.
         * @return
         *  the name of the vendor of this OpenGL implementation
         */
        std::string getVendor()
        {
            while (GL_NO_ERROR != glGetError()) {}
            const GLubyte *bytes = glGetString(GL_RENDERER);
            GLenum error = glGetError();
            if (GL_NO_ERROR != error)
            {
                throw std::runtime_error("unable to acquire renderer back-end information");
            }
            return (const char *)bytes;
        }
        /**
         * @brief
         *  Get the list of extensions supported by this OpenGL implementation.
         * @param [out] extensions
         *  a set of strings
         * @post
         *  the set of extensions supported by this OpenGL implementation was added to the set
         */
        std::vector<std::string> getExtensions()
        {
            while (GL_NO_ERROR != glGetError()) {}
            const GLubyte *bytes = glGetString(GL_EXTENSIONS);
            GLenum error = glGetError();
            if (GL_NO_ERROR != error)
            {
                throw std::runtime_error("unable to acquire renderer back-end information");
            }
            return Ego::split(std::string((const char *)bytes),std::string(" "));
        }

        Renderer::Renderer() :
            _name(getName()), _vendor(getVendor()),
            _extensions(make_unordered_set(getExtensions()))
        {
        }

		Renderer::~Renderer()
		{
			//dtor
		}

        void Renderer::setClearColour(const Colour4f& colour)
        {
            glClearColor(colour.getRed(), colour.getGreen(), colour.getBlue(), colour.getAlpha());
        }

        void Renderer::setAlphaTestEnabled(bool enabled)
        {
            if (enabled)
            {
                glEnable(GL_ALPHA_TEST);
            }
            else
            {
                glDisable(GL_ALPHA_TEST);
            }
        }

        void Renderer::setBlendingEnabled(bool enabled)
        {
            if (enabled)
            {
                glEnable(GL_BLEND);
            }
            else
            {
                glDisable(GL_BLEND);
            }
        }

		void Renderer::multiplyMatrix(const fmat_4x4_t& matrix)
		{
			// fmat_4x4_t will not remain a simple array, hence the data must be packed explicitly to be passed
			// to the OpenGL API. However, currently this code is redundant.
			GLXmatrix t;
			for (size_t i = 0; i < 4; ++i)
			{
				for (size_t j = 0; j < 4; ++j)
				{
					t[i * 4 + j] = matrix.v[i * 4 + j];
				}
			}
			GL_DEBUG(glMultMatrixf)(t);
		}

		void Renderer::loadMatrix(const fmat_4x4_t& matrix)
		{
			// fmat_4x4_t will not remain a simple array, hence the data must be packed explicitly to be passed
			// to the OpenGL API. However, currently this code is redundant.
			GLXmatrix t;
			for (size_t i = 0; i < 4; ++i)
			{
				for (size_t j = 0; j < 4; ++j)
				{
					t[i * 4 + j] = matrix.v[i * 4 + j];
				}
			}
			GL_DEBUG(glLoadMatrixf)(t);
		}

		void Renderer::setColour(const Colour4f& colour)
		{
			GL_DEBUG(glColor4f(colour.getRed(), colour.getGreen(),
				               colour.getBlue(), colour.getAlpha()));
		}

		void Renderer::setDepthTestEnabled(bool enabled)
		{
			if (enabled)
			{
				GL_DEBUG(glEnable)(GL_DEPTH_TEST);
			}
			else
			{
				GL_DEBUG(glDisable)(GL_DEPTH_TEST);
			}
		}

		void Renderer::setStencilTestEnabled(bool enabled)
		{
			if (enabled)
			{
				GL_DEBUG(glEnable)(GL_STENCIL_TEST);
			}
			else
			{
				GL_DEBUG(glDisable)(GL_STENCIL_TEST);
			}
		}

		void Renderer::setScissorTestEnabled(bool enabled)
		{
			if (enabled)
			{
				GL_DEBUG(glEnable)(GL_SCISSOR_TEST);
			}
			else
			{
				GL_DEBUG(glDisable)(GL_SCISSOR_TEST);
			}
		}
	};

};
