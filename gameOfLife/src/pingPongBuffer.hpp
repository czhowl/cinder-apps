#pragma once

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

struct pingPongBuffer {
public:
	void allocate(int _width, int _height, int _internalformat = GL_RGBA) {
		// Allocate
		ci::gl::Fbo::Format fmt;
		fmt.enableDepthBuffer(false);

		// use a single channel (red) for the displacement map
		fmt.setColorTextureFormat(ci::gl::Texture2d::Format().internalFormat(_internalformat).minFilter(GL_NEAREST).magFilter(GL_NEAREST));
		for (int i = 0; i < 2; i++) {
			FBOs[i] = ci::gl::Fbo::create(_width, _height, fmt);
		}

		// Clean
		clear();
	}

	void swap() { 
		std::swap(src, dst);
	}

	void clear() {
		for (int i = 0; i < 2; i++) {
			ci::gl::ScopedFramebuffer scpFbo(FBOs[i]);
			ci::gl::ScopedViewport    scpViewport(FBOs[i]->getSize());
			ci::gl::clear();
		}
	}

	ci::gl::FboRef operator[](int n) { return FBOs[n]; }
	ci::gl::FboRef   src = FBOs[0];       // Source       ->  Ping
	ci::gl::FboRef   dst = FBOs[1];       // Destination  ->  Pong

private:
	ci::gl::FboRef  FBOs[2];    // Real addresses of ping/pong FBO´s
};