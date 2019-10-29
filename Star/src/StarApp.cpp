#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "Background.h"
#include "Cam.h"
#include "Conversions.h"
#include "Stars.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class StarApp : public App {
  public:
    static void prepare( Settings *settings );
	void setup() override;
    void mouseDown( MouseEvent event ) override;
    void mouseDrag( MouseEvent event ) override;
    void mouseUp( MouseEvent event ) override;
	void update() override;
	void draw() override;
    void createShader();
    void createFbo();
    void render();
    
  protected:
    double mTime = 0.0;
    
    // cursor position
    ivec2 mCursorPos;
    ivec2 mCursorPrevious;
    
    // camera
    Cam mCamera;
    
    // graphical elements
    Stars               mStars;
    Background          mBackground;
    
    gl::BatchRef mSun;
    
    // animation timer
    Timer mTimer;
    
    // toggles
    bool mIsGridVisible = false;
    bool mIsLabelsVisible = false;
    bool mIsConstellationsVisible = false;
    bool mIsConstellationArtVisible = false;
    bool mIsCursorVisible = false;
    bool mIsStereoscopic = false;
    bool mIsCylindrical = false;
    bool mIsDemo = true;
    bool mDrawUserInterface = false;
    
    // frame buffer and shader used for cylindrical projection
    gl::FboRef      mFbo;
    gl::GlslProgRef mShader;
    unsigned        mSectionCount;
    float           mSectionFovDegrees;
    float           mSectionOverlap;
};

void StarApp::setup()
{
    // cylindrical projection settings
    mSectionCount = 3;
    mSectionFovDegrees = 72.0f;
    // for values smaller than 1.0, this will cause each view to overlap the other ones
    //  (angle of overlap: (1 - mSectionOverlap) * mSectionFovDegrees)
    mSectionOverlap = 1.0f;
    
    // create stars
    mStars.setup();
    mStars.setAspectRatio( mIsStereoscopic ? 0.5f : 1.0f );
    
    // load the star database and create the VBO mesh
    if( fs::exists( getAssetPath( "" ) / "stars.cdb" ) )
        mStars.read( loadFile( getAssetPath( "" ) / "stars.cdb" ) );
    
    // initialize background image
    mBackground.setup();
    
    // initialize camera
    mCamera.setup();
    
    //
    createShader();
    
    //
    // auto mesh = gl::VboMesh::create( geom::Sphere().radius( 1 ).subdivisions( 60 ) );
    // auto glsl = gl::GlslProg::create( loadAsset( "shaders/sun.vert" ), loadAsset( "shaders/sun.frag" ) );
    // mSun = gl::Batch::create( mesh, glsl );
    
    //
    mTimer.start();
    
    mTime = getElapsedSeconds();
}

void StarApp::update()
{
    const double elapsed = getElapsedSeconds() - mTime;
    mTime += elapsed;
    
    double time = getElapsedSeconds() / 200.0;
    
    // toggle constellations in demo mode
    if( mIsDemo && time > 0.5 ) {
        mIsDemo = false;
        mIsConstellationsVisible = true;
        mIsConstellationArtVisible = true;
    }
    
    // animate camera
    mCamera.setDistanceTime( time );
    mCamera.update( elapsed );
    
    // adjust content based on camera distance
    const float distance = length( mCamera.getCamera().getEyePoint() );
    mBackground.setCameraDistance( distance );
    
    //
    if( mSun )
        mSun->getGlslProg()->uniform( "uTime", float( getElapsedSeconds() ) );
}

void StarApp::render()
{
    // draw background
    mBackground.draw();
    
    // draw stars
    mStars.draw();
}

void StarApp::draw()
{
    int w = getWindowWidth();
    int h = getWindowHeight();
    
    gl::clear( Color::black() );
#if 1
    if( mIsStereoscopic ) {
        gl::ScopedViewport viewport( 0, 0, w / 2, h );
        gl::pushMatrices();
        
        // render left eye
        mCamera.enableStereoLeft();
        
        gl::setMatrices( mCamera.getCamera() );
        render();
        
        // render right eye
        mCamera.enableStereoRight();
        
        gl::viewport( w / 2, 0, w / 2, h );
        gl::setMatrices( mCamera.getCamera() );
        render();
        
        gl::popMatrices();
    }
    else if( mIsCylindrical ) {
        // make sure we have a frame buffer to render to
        createFbo();
        
        // determine correct aspect ratio and vertical field of view for each of the 3 views
        w = mFbo->getWidth() / mSectionCount;
        h = mFbo->getHeight();
        
        const float aspect = float( w ) / float( h );
        // const float hFoV = mSectionFovDegrees;
        // const float vFoV = toDegrees( 2.0f * math<float>::atan( math<float>::tan( toRadians(hFoV) * 0.5f ) / aspect ) );
        const float vFoVDegrees = float( mCamera.getFov() );
        const float vFovRadians = glm::radians( vFoVDegrees );
        const float hFoVRadians = 2.0f * math<float>::atan( math<float>::tan( vFovRadians * 0.5f ) * aspect );
        const float hFovDegrees = glm::degrees( hFoVRadians );
        
        // bind the frame buffer object
        {
            gl::ScopedFramebuffer fbo( mFbo );
            
            // store viewport and matrices, so we can restore later
            gl::ScopedViewport viewport( ivec2( 0 ), mFbo->getSize() );
            gl::pushMatrices();
            
            gl::clear();
            
            // setup camera
            CameraStereo cam = mCamera.getCamera();
            cam.disableStereo();
            cam.setAspectRatio( aspect );
            cam.setFov( vFoVDegrees );
            
            vec3 right, up;
            cam.getBillboardVectors( &right, &up );
            const vec3 forward = cross( up, right );
            
            // render sections
            const float offset = 0.5f * ( mSectionCount - 1 );
            for( unsigned i = 0; i < mSectionCount; ++i ) {
                gl::ScopedViewport scpViewport( i * w, 0, w, h );
                
                cam.setViewDirection( glm::angleAxis( -mSectionOverlap * hFoVRadians * ( i - offset ), up ) * forward );
                cam.setWorldUp( up );
                gl::setMatrices( cam );
                render();
            }
            
            // draw user interface
            gl::setMatrices( mCamera.getCamera() );
            
            // restore states
            gl::popMatrices();
        }
        
        // draw frame buffer and perform cylindrical projection using a fragment shader
        if( mShader ) {
            gl::ScopedTextureBind tex0( mFbo->getColorTexture() );
            
            gl::ScopedGlslProg shader( mShader );
            mShader->uniform( "tex", 0 );
            mShader->uniform( "sides", float( mSectionCount ) );
            mShader->uniform( "radians", mSectionCount * hFoVRadians );
            mShader->uniform( "reciprocal", 0.5f / mSectionCount );
            
            const Rectf centered = Rectf( mFbo->getBounds() ).getCenteredFit( getWindowBounds(), false );
            gl::drawSolidRect( centered );
            // gl::draw( mFbo->getColorTexture(), centered );
        }
    }
    else {
        mCamera.disableStereo();
        
        gl::pushMatrices();
        gl::setMatrices( mCamera.getCamera() );
        render();
        gl::popMatrices();
    }
    
    /*// fade in at start of application
     gl::ScopedAlphaBlend blend(false);
     double t = math<double>::clamp( mTimer.getSeconds() / 3.0, 0.0, 1.0 );
     float a = ci::lerp<float>( 1.0f, 0.0f, (float) t );
     
     if( a > 0.0f ) {
     gl::color( ColorA( 0, 0, 0, a ) );
     gl::drawSolidRect( getWindowBounds() );
     }//*/
#endif
    
    if( mSun ) {
        gl::ScopedDepth       scpDepth( true );
        gl::ScopedFaceCulling scpCull( true );
        gl::ScopedMatrices    scpMatrices;
        
        gl::setMatrices( mCamera.getCamera() );
        mSun->draw();
    }
}

void StarApp::createShader()
{
    const fs::path vs = getAssetPath( "" ) / "shaders/cylindrical.vert";
    const fs::path fs = getAssetPath( "" ) / "shaders/cylindrical.frag";
    
    try {
        mShader = gl::GlslProg::create( loadFile( vs ), loadFile( fs ) );
    }
    catch( const std::exception &e ) {
        console() << e.what() << std::endl;
        mShader = gl::GlslProgRef();
    }
}

void StarApp::createFbo()
{
    // determine the size of the frame buffer
    const int w = getWindowWidth() * 2;
    const int h = getWindowHeight() * 2;
    
    if( mFbo && mFbo->getSize() == ivec2( w, h ) )
        return;
    
    // create the FBO
    gl::Texture2d::Format tfmt;
    tfmt.setWrap( GL_REPEAT, GL_CLAMP_TO_BORDER );
    
    gl::Fbo::Format fmt;
    fmt.setColorTextureFormat( tfmt );
    
    mFbo = gl::Fbo::create( w, h, fmt );
}

void StarApp::mouseDown( MouseEvent event )
{
    // allow user to control camera
    mCursorPos = mCursorPrevious = event.getPos();
    mCamera.mouseDown( mCursorPos );
}

void StarApp::mouseDrag( MouseEvent event )
{
    mCursorPos += event.getPos() - mCursorPrevious;
    mCursorPrevious = event.getPos();
    
    // allow user to control camera
    mCamera.mouseDrag( mCursorPos, event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );
}

void StarApp::mouseUp( MouseEvent event )
{
    // allow user to control camera
    mCursorPos = mCursorPrevious = event.getPos();
    mCamera.mouseUp( mCursorPos );
}

void StarApp::prepare( Settings *settings )
{
//    auto displays = Display::getDisplays();
//    settings->disableFrameRate();
//    settings->setBorderless( true );
//    settings->setWindowPos( 0, 0 );
    settings->setWindowSize( 1280, 768 );
}

CINDER_APP( StarApp, RendererGl( RendererGl::Options().msaa( 16 ) ), &StarApp::prepare )
