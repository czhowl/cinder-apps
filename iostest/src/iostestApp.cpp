#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"

#include "CinderARKit.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class iostestApp : public App {
public:
    void setup() override;
    void touchesBegan( TouchEvent event ) override;
    void update() override;
    void draw() override;
//    void decompose ( glm::mat4x4 matrix, glm::vec3& scaling, glm::quat& rotation, glm::vec3& position);
    
    ARKit::Session mARSession;
    
    gl::Texture2dRef mTextureStar;
    gl::Texture2dRef mTextureCorona;
    gl::Texture2dRef mTextureHalo;
    
    gl::GlslProgRef  mDrawShader, mShaderHalos;
};

void iostestApp::setup()
{
    randSeed((uint32_t)clock());
    auto config = ARKit::SessionConfiguration()
    .trackingType( ARKit::TrackingType::WorldTracking )
    .planeDetection( ARKit::PlaneDetection::Horizontal );
    
    mARSession.runConfiguration( config );
    
    mTextureStar = gl::Texture2d::create( loadImage( loadAsset( "star_glow.png" ) ) );
    mTextureCorona = gl::Texture2d::create( loadImage( loadAsset( "nova.png" ) ) );
    mTextureHalo = gl::Texture2d::create( loadImage( loadAsset( "corona.png" ) ) );
    
    mDrawShader = gl::GlslProg::create(loadAsset("drawStar.vert"), loadAsset("drawStar.frag"));
    mShaderHalos = gl::GlslProg::create(loadAsset("drawStar.vert"), loadAsset("drawHalo.frag"));
}

void iostestApp::touchesBegan( TouchEvent event )
{
    // Add a point 50cm in front of camera
    mARSession.addAnchorRelativeToCamera( vec3(0.0f, 0.0f, randFloat(-1.0f, 0.0f)) );
}

void iostestApp::update()
{
}

void iostestApp::draw()
{
    gl::clear( Color( 0, 0, 0 ) );
    
    gl::color( 1.0f, 1.0f, 1.0f, 1.0f );
    mARSession.drawRGBCaptureTexture( getWindowBounds() );
    
    gl::ScopedMatrices matScp;
    gl::setViewMatrix( mARSession.getViewMatrix() );
    gl::setProjectionMatrix( mARSession.getProjectionMatrix() );
    
    gl::ScopedBlendAdditive blend;
    gl::ScopedGlslProg glslProg( mDrawShader);
    gl::ScopedTextureBind tex0( mTextureStar, (uint8_t)0 );
    gl::ScopedTextureBind tex1( mTextureCorona, (uint8_t)1 );
    mDrawShader->uniform( "time", (float)getElapsedSeconds() );
//    gl::color( 1.0f, 1.0f, 1.0f );
    glm::mat4 transformation; // your transformation matrix.
    
    vec3 right = glm::vec3( glm::row( mARSession.getViewMatrix(), 0 ) );
    vec3  up = glm::vec3( glm::row( mARSession.getViewMatrix(), 1 ) );
    for (const auto& a : mARSession.getAnchors())
    {
        gl::ScopedMatrices matScp;
//        gl::setModelMatrix( a.mTransform );
        gl::translate(a.mTransform[3][0], a.mTransform[3][1], a.mTransform[3][2]);
//        console() << a.mTransform << endl;
//        gl::drawStrokedCube( vec3(0.0f), vec3(0.02f) );
        gl::drawBillboard(vec3(0.0f), vec2(0.02f), 0, right, up);
//        gl::drawSolidRect(Rectf(0.0,0.0,0.02,0.02));
//        gl::draw(mTextureStar, Rectf(0.0,0.0,0.02,0.02));
    }
    
//    for (const auto& a : mARSession.getPlaneAnchors())
//    {
//        gl::ScopedMatrices matScp;
//        gl::setModelMatrix( a.mTransform );
//        gl::translate( a.mCenter );
//        gl::rotate( (float)M_PI * 0.5f, vec3(1,0,0) ); // Make it parallel with the ground
//        const float xRad = a.mExtent.x * 0.5f;
//        const float zRad = a.mExtent.z * 0.5f;
//        gl::color( 0.0f, 0.6f, 0.9f, 0.2f );
//        gl::drawSolidRect( Rectf( -xRad,-zRad, xRad, zRad ));
//    }
}

CINDER_APP( iostestApp, RendererGl(RendererGl::Options().msaa(16)) )
