#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "cinder/CameraUi.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class VectorFieldApp : public App {
public:
    static void prepare(Settings *settings);
    void setup() override;
    void mouseDown( MouseEvent event ) override;
    void update() override;
    void draw() override;
    void keyDown(KeyEvent event) override;
    
private:
    void                compileShaders();
    void                buildMesh();
    
    gl::FboRef          mVectorFieldFbo;
    gl::GlslProgRef        mNoiseShader;
    
    gl::GlslProgRef        mCoralShader;
    gl::VboMeshRef          mCoral;
    gl::BatchRef            mCoralBatch;
    int                     mWidth = 200;
    int                     mHeight = 200;
    
    CameraPersp                        mCam;
    CameraUi                           mCamUI;
};

void VectorFieldApp::prepare(Settings *settings)
{
    settings->setTitle("XR in Real World");
    settings->setWindowSize(800, 800);
    settings->disableFrameRate();
}

void VectorFieldApp::setup()
{
    mCam.setPerspective( 45.0f, getWindowAspectRatio(), .01f, 1000.0f );
        mCam.lookAt( vec3( 0.000f, 10, 0.000f ), vec3( 0, 0, 0 ) );
    mCamUI = CameraUi(&mCam, getWindow() );
    
    gl::Fbo::Format fmt;
    fmt.enableDepthBuffer(false);
    // use a single channel (red) for the displacement map
    fmt.setColorTextureFormat(gl::Texture2d::Format().wrap(GL_CLAMP_TO_EDGE).internalFormat(GL_RGB32F));
    
    mVectorFieldFbo = gl::Fbo::create(512, 512, fmt);
    compileShaders();
    buildMesh();
}

void VectorFieldApp::mouseDown( MouseEvent event )
{
}

void VectorFieldApp::update()
{
    if (mVectorFieldFbo) {
        // bind frame buffer
        gl::ScopedFramebuffer fbo(mVectorFieldFbo);
        
        // setup viewport and matrices
        gl::ScopedViewport viewport(0, 0, mVectorFieldFbo->getWidth(), mVectorFieldFbo->getHeight());
        
        gl::pushMatrices();
        gl::setMatricesWindow(mVectorFieldFbo->getSize());
        
        // clear the color buffer
        gl::clear();
        
        //        gl::ScopedTextureBind tex0(mRippleFboB->getColorTexture());
        // render the displacement map
        gl::ScopedGlslProg shader(mNoiseShader);
        mNoiseShader->uniform("uTime", float(getElapsedSeconds()));
        
        gl::drawSolidRect(mVectorFieldFbo->getBounds());
        
        // clean up after ourselves
        gl::popMatrices();
    }
}

void VectorFieldApp::draw()
{
    gl::pushMatrices();
    gl::setMatrices( mCam );
    gl::clear( Color( 0, 0, 0 ) );
    gl::ScopedState            stateScope( GL_PROGRAM_POINT_SIZE, true );
    gl::ScopedTextureBind tex0( mVectorFieldFbo->getColorTexture() );
    mCoralShader->uniform( "uTex0", 0 );
    mCoralShader->uniform("uTime", float(getElapsedSeconds()));
    //    gl::draw(mVectorFieldFbo->getColorTexture(), vec2(0));
    gl::enableDepthRead();
    gl::enableDepthWrite();
    
    mCoralBatch->draw();
    
    gl::popMatrices();
}

void VectorFieldApp::keyDown(KeyEvent event)
{
    switch (event.getCode()) {
        case KeyEvent::KEY_s:
            // reload shaders
            compileShaders();
            buildMesh();
            break;
    }
}

void VectorFieldApp::compileShaders()
{
    try {
        mCoralShader = gl::GlslProg::create(
                    gl::GlslProg::Format().vertex( loadAsset( "coral.vert" ) )
                    .fragment( loadAsset( "coral.frag" ) )
                    .geometry( loadAsset( "coral.geom" ) ));
        mNoiseShader = gl::GlslProg::create(loadAsset("noise.vert"), loadAsset("noise.frag"));
    }
    catch (const std::exception &e) {
        console() << e.what() << std::endl;
    }
}

void VectorFieldApp::buildMesh()
{
//    geom::BufferLayout vboDataLayout;
//    vboDataLayout.
//    .append( geom::Attrib::CUSTOM_0, 3, 0, 0, 1 /* per instance */ );
    mCoral = gl::VboMesh::create( mWidth * mHeight, GL_POINTS, { gl::VboMesh::Layout().usage(GL_DYNAMIC_DRAW).attrib(geom::POSITION, 3).attrib(geom::COLOR, 3).attrib(geom::TEX_COORD_0, 2) } );
    mCoralBatch = gl::Batch::create( mCoral, mCoralShader );
    auto vertPosIter = mCoral->mapAttrib3f( geom::POSITION );
    auto vertColorIter = mCoral->mapAttrib3f( geom::COLOR );
    auto vertCoordIter = mCoral->mapAttrib2f( geom::TEX_COORD_0 );
    for( int x = 0; x < mWidth; ++x ) {
        for( int z = 0; z < mHeight; ++z ) {
            *vertPosIter++ = vec3( (x - mWidth / 2.0f) * 0.5f, 0.0f, (z - mHeight / 2.0f) * 0.5f );
            *vertColorIter++ = vec3( 1.0f );
            const float u = float( x ) / mWidth;
            const float v = float( z ) / mHeight;
            *vertCoordIter++ = vec2( u, v );
        }
    }
    vertPosIter.unmap();
    vertColorIter.unmap();
    vertCoordIter.unmap();
}

CINDER_APP( VectorFieldApp, RendererGl(RendererGl::Options().msaa(16)), &VectorFieldApp::prepare )
