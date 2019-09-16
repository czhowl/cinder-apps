#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/CameraUi.h"
#include "cinder/Rand.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class gameOfLifeApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
	void keyDown(KeyEvent event) override;
	void					forceShowCursor();
	void					compileShaders();

	int						cellWidth, cellHeight;
	gl::GlslProgRef			mUpdateShader, mRenderShader;
	CameraPersp                 mCamera;
	CameraUi                 mCamUi;
	bool mIsCursorVisible;

	int					mCurrentFBO, mOtherFBO;
	gl::FboRef  mFbos[2];
};

void gameOfLifeApp::setup()
{
	

	randSeed(clock());
	compileShaders();

	cellWidth = getWindowWidth();
	cellHeight = getWindowHeight();

	mCamera = CameraPersp(cellWidth, cellHeight, 45.0f, 0.01f, 5000.0f);
	mCamera.lookAt(vec3(0.0f,0.0f,500.0f), vec3(0.0f, 0.0f, 0.0f));
	mCamUi = CameraUi(&mCamera, getWindow(), -1);

	vector<float> color(cellWidth * cellHeight * 3);
	Surface32f result(cellWidth, cellHeight, false);
	for (int x = 0; x < cellWidth; x++) {
		for (int y = 0; y < cellHeight; y++) {
			int i = cellWidth * y + x;
			float coin = randFloat(2) > 1 ? 0.0 : 1.0;
			color[i * 3 + 0] = 0.0;
			color[i * 3 + 1] = coin;
			color[i * 3 + 2] = 0.0;
			result.setPixel(ivec2(x, y), Color(color[i * 3 + 0], color[i * 3 + 1], color[i * 3 + 2]));
		}
	}

	mCurrentFBO = 0;
	mOtherFBO = 1;
	gl::Fbo::Format fmt;
	fmt.enableDepthBuffer(false);
	// use a single channel (red) for the displacement map
	gl::enable(GL_TEXTURE_RECTANGLE_ARB);
	fmt.setColorTextureFormat(ci::gl::Texture2d::Format().target(GL_TEXTURE_RECTANGLE_ARB).internalFormat(GL_RGB32F_ARB).minFilter(GL_NEAREST).magFilter(GL_NEAREST).wrap(GL_CLAMP_TO_EDGE));
	for (int i = 0; i < 2; i++) {
		mFbos[i] = ci::gl::Fbo::create(cellWidth, cellHeight, fmt);
	}
	for (int i = 0; i < 2; i++) {
		ci::gl::ScopedFramebuffer scpFbo(mFbos[i]);
		ci::gl::ScopedViewport    scpViewport(mFbos[i]->getSize());
		ci::gl::clear();
	}
	
	for (vector<float>::const_iterator ptIt = color.begin(); ptIt != color.end(); ptIt += 3) {
		
	}
	gl::TextureRef m_tex = gl::Texture::create(result, gl::Texture::Format().target(GL_TEXTURE_RECTANGLE_ARB));
	for (int i = 0; i < 2; i++) {
		gl::ScopedFramebuffer fboBind(mFbos[i]);
		gl::setMatricesWindow(mFbos[i]->getSize());
		gl::draw(m_tex);
	}
}

void gameOfLifeApp::mouseDown( MouseEvent event )
{
}

void gameOfLifeApp::keyDown(KeyEvent event)
{
	switch (event.getCode()) {
	case KeyEvent::KEY_s:
		// reload shaders
		compileShaders();
		break;
	case KeyEvent::KEY_f:
		setFullScreen(!isFullScreen());
		if (!isFullScreen())
			forceShowCursor();
		break;
	}
}

void gameOfLifeApp::forceShowCursor()
{

	showCursor();
	mIsCursorVisible = true;
}

void gameOfLifeApp::update()
{
	mCurrentFBO = (mCurrentFBO + 1) % 2;
	mOtherFBO = (mCurrentFBO + 1) % 2;
	{
		gl::ScopedFramebuffer fboBind(mFbos[mCurrentFBO]);
		gl::ScopedModelMatrix modelScope;
		gl::setMatricesWindow(mFbos[mCurrentFBO]->getSize());
		gl::clear(Color(0, 0, 0));
		gl::ScopedTextureBind tex0(mFbos[mOtherFBO]->getColorTexture());
		gl::ScopedGlslProg    scpProg(mUpdateShader);
		mUpdateShader->uniform("uBackBuffer", 0);
		gl::drawSolidRect(mFbos[mCurrentFBO]->getBounds());
	}
	
}

void gameOfLifeApp::draw()
{
	
	gl::clear( Color( 0, 0, 0 ) );
	if (true) {
		gl::ScopedModelMatrix modelScope;
		gl::setMatrices(mCamera);
		gl::ScopedTextureBind tex0(mFbos[mCurrentFBO]->getColorTexture());
		gl::ScopedGlslProg    scpProg(mRenderShader);
		mRenderShader->uniform("uLifeTex", 0);
		Rectf	window = getWindowBounds();
		gl::pushMatrices();
		gl::translate(
			-getWindowWidth() / 2, -getWindowHeight() / 2
		);
		gl::drawSolidRect(Rectf(0.0, 0.0, getWindowWidth(), getWindowHeight()));
		gl::popMatrices();
	}
}

void gameOfLifeApp::compileShaders()
{
	try {
		mUpdateShader = gl::GlslProg::create(loadAsset("updateShader.vert"), loadAsset("updateShader.frag"));
		mRenderShader = gl::GlslProg::create(loadAsset("renderShader.vert"), loadAsset("renderShader.frag"));
	}
	catch (const std::exception &e) {
		console() << e.what() << std::endl;
	}
}

CINDER_APP( gameOfLifeApp, RendererGl )
