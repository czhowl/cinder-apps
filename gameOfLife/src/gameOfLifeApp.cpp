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
	void mouseDrag(MouseEvent event) override;
	void mouseWheel(MouseEvent event) override;
	void mouseUp(MouseEvent event) override;
	void update() override;
	void draw() override;
	void keyDown(KeyEvent event) override;
	
	static void prepare(App::Settings *settings);
	void					forceShowCursor();
	void					compileShaders();

	int						cellWidth, cellHeight;
	gl::GlslProgRef			mUpdateShader, mRenderShader, mDrawShader;
	CameraPersp                 mCamera;
	CameraUi                 mCamUi;
	bool					mIsCursorVisible;
	bool					mEvolve;
	bool					mDrawMode;
    float                   mPenSize;

	int					mCurrentFBO, mOtherFBO;
	gl::FboRef  mFbos[2];
    gl::TextureRef mTex;
};

void gameOfLifeApp::setup()
{
	mEvolve = false;
	mDrawMode = false;
    mPenSize = 0.01;

	randSeed(clock());
	compileShaders();

	cellWidth = getWindowWidth();
	cellHeight = getWindowHeight();

	mCamera = CameraPersp(cellWidth, cellHeight, 45.0f, 0.01f, 5000.0f);
	mCamera.lookAt(vec3(0.0f,0.0f,100.0f), vec3(0.0f, 0.0f, 0.0f));
	mCamUi = CameraUi(&mCamera);

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
    gl::enable(GL_TEXTURE_RECTANGLE);
	fmt.setColorTextureFormat(ci::gl::Texture2d::Format().target(GL_TEXTURE_RECTANGLE).internalFormat(GL_RGB32F_ARB).minFilter(GL_NEAREST).magFilter(GL_NEAREST).wrap(GL_CLAMP_TO_EDGE));
	for (int i = 0; i < 2; i++) {
		mFbos[i] = ci::gl::Fbo::create(cellWidth, cellHeight, fmt);
	}
    mTex = gl::Texture::create(result, gl::Texture::Format().target(GL_TEXTURE_RECTANGLE).minFilter(GL_NEAREST).magFilter(GL_NEAREST));
	for (int i = 0; i < 2; i++) {
		ci::gl::ScopedFramebuffer scpFbo(mFbos[i]);
        gl::pushMatrices();
		gl::setMatricesWindow(mFbos[i]->getSize());
		ci::gl::clear();
        gl::draw(mTex);
        gl::popMatrices();
	}
}


void gameOfLifeApp::mouseDown(MouseEvent event)
{
	if (event.isLeftDown() && !mEvolve) {
		mDrawMode = true;
	}
}

void gameOfLifeApp::mouseUp(MouseEvent event)
{
	if (event.isLeft()) {
		mDrawMode = false;
	}
	mCamUi.mouseUp(event);
}

void gameOfLifeApp::mouseDrag(MouseEvent event)
{
	////mCamUi.mouseDrag(event);
	if (event.isRightDown()) mCamUi.mouseDrag(event.getPos(), false, true, false);
}

void gameOfLifeApp::mouseWheel(MouseEvent event)
{
	//if(mCamera.getEyePoint().z > 0 && mCamera.getEyePoint().z < 600)
	mCamUi.mouseWheel(event);
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
	case KeyEvent::KEY_SPACE:
		mEvolve = !mEvolve;
		mDrawMode = false;
		break;
    case KeyEvent::KEY_DOWN:
        if(mPenSize / 2 > 0.0005) mPenSize = mPenSize / 2;
        break;
    case KeyEvent::KEY_UP:
        if(mPenSize * 2 < 0.8) mPenSize = mPenSize * 2;
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
	
	if (mDrawMode) {
		mCurrentFBO = (mCurrentFBO + 1) % 2;
		mOtherFBO = (mCurrentFBO + 1) % 2;
		ivec2 mouse = getWindow()->getMousePos();

		float u = mouse.x / (float)getWindowWidth();
		float v = mouse.y / (float)getWindowHeight();
		Ray ray = mCamera.generateRay(mouse, getWindowSize());
		float dist;
		if (ray.calcPlaneIntersection(vec3(0.0f), vec3(0, 0, -1), &dist)) {
			auto rayPos = -ray.calcPosition(dist); // This may be the droid you're looking for
			rayPos += vec3(500.0f, 500.0f, 0.0f);
			console() << rayPos << endl;
			gl::ScopedFramebuffer fboBind(mFbos[mCurrentFBO]);
			//        gl::ScopedViewport scpVp( ivec2( 0 ), mFbos[ mOtherFBO ]->getSize() );
			//        gl::setMatricesWindow(mFbos[mOtherFBO]->getSize());
			gl::setMatricesWindow(mFbos[mOtherFBO]->getSize(), false);
			gl::clear(Color(0, 0, 0));
			gl::ScopedTextureBind tex0(mFbos[mOtherFBO]->getColorTexture());
			gl::ScopedGlslProg    scpProg(mDrawShader);
			mDrawShader->uniform("uLifeTex", 0);
			mDrawShader->uniform("uPos", rayPos);
            mDrawShader->uniform("uSize", mPenSize);
			gl::drawSolidRect(mFbos[mOtherFBO]->getBounds());
		}
	}
	if (mEvolve) {
		mCurrentFBO = (mCurrentFBO + 1) % 2;
		mOtherFBO = (mCurrentFBO + 1) % 2;
		{
			gl::ScopedFramebuffer fboBind(mFbos[mCurrentFBO]);
			//        gl::ScopedViewport scpVp( ivec2( 0 ), mFbos[ mOtherFBO ]->getSize() );
			//        gl::setMatricesWindow(mFbos[mOtherFBO]->getSize());
			gl::setMatricesWindow(mFbos[mOtherFBO]->getSize(), false);
			gl::clear(Color(0, 0, 0));
			gl::ScopedTextureBind tex0(mFbos[mOtherFBO]->getColorTexture());
			gl::ScopedGlslProg    scpProg(mUpdateShader);
			mUpdateShader->uniform("uBackBuffer", 0);
			gl::drawSolidRect(mFbos[mOtherFBO]->getBounds());
		}
	}
	
}

void gameOfLifeApp::draw()
{
	
	gl::clear( Color( 0, 0, 0 ) );
    gl::pushMatrices();
    gl::setMatrices(mCamera);
    gl::pushMatrices();
    gl::translate(-getWindowWidth() / 2, -getWindowHeight() / 2);
//    gl::draw(mTex);
//    gl::draw(mFbos[mCurrentFBO]->getColorTexture());
    if (true) {
        gl::ScopedTextureBind tex0(mFbos[mCurrentFBO]->getColorTexture());
        gl::ScopedGlslProg    scpProg(mRenderShader);
        mRenderShader->uniform("uLifeTex", 0);
        gl::drawSolidRect(Rectf(0.0, 0.0, getWindowWidth(), getWindowHeight()));
    }
    gl::popMatrices();
    gl::popMatrices();
}

void gameOfLifeApp::compileShaders()
{
	try {
		mUpdateShader = gl::GlslProg::create(loadAsset("updateShader.vert"), loadAsset("updateShader.frag"));
		mRenderShader = gl::GlslProg::create(loadAsset("renderShader.vert"), loadAsset("renderShader.frag"));
		mDrawShader = gl::GlslProg::create(loadAsset("drawShader.vert"), loadAsset("drawShader.frag"));
	}
	catch (const std::exception &e) {
		console() << e.what() << std::endl;
	}
}

void gameOfLifeApp::prepare(App::Settings *settings)
{
	settings->setWindowSize(1000, 1000);
}

CINDER_APP(gameOfLifeApp, RendererGl(RendererGl::Options().msaa(16)), &gameOfLifeApp::prepare)

