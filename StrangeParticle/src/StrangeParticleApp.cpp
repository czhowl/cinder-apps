#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"
#include "cinder/Vector.h"
#include "cinder/Perlin.h"
#include "cinder/Color.h"
#include "cinder/CameraUi.h"

#include <vector>

using namespace ci;
using namespace ci::app;
using namespace std;
// ==============================================================
class Particle {
public:
    Particle( const vec3 &position )
    : mPosition( position ), mLastPosition( position ), mVelocity( vec3( 0 ) ), mZ( 0 ) {}
    
    void reset( const vec3 &position )
    {
        mPosition = mLastPosition = position;
        mVelocity = vec3( 0 );
        mZ = 0;
    }
    
    vec3 mPosition, mVelocity, mLastPosition;
    float mZ;
};
// ==============================================================

class StrangeParticleApp : public App {
  public:
    void    setup();
    void    update();
    void    draw();
    
    void    keyDown( KeyEvent event );
    
    bool    isOffscreen( const vec3 &v );
    
  public:
    Font                mFont;
    static const int    NUM_PARTICLES = 10000;
    
    float                mConservationOfVelocity;
    float                mSpeed;
    float                mAnimationCounter;
    
    Perlin                mPerlin;
    vector<Particle>    mParticles;
    
    CameraPersp        mCamera;
    CameraUi           mCamUi;
};

void StrangeParticleApp::setup()
{
    mFont = Font("Arial", 12.0f);
    // Randomize the Perlin noise function.
    mPerlin.setSeed( clock() );
    
    // Create particles.
    mParticles.reserve( NUM_PARTICLES );
    for( int s = 0; s < NUM_PARTICLES; ++s )
    mParticles.push_back( Particle( vec3( Rand::randFloat( getWindowWidth() ), Rand::randFloat( getWindowHeight() ), Rand::randFloat( getWindowHeight() ) ) ) );
    
    mConservationOfVelocity = 0.9f;
    mSpeed = 5.0f;
    mAnimationCounter = 0.0f;
    
    mCamera.setFarClip (5000.0f);
    mCamUi = CameraUi( &mCamera, getWindow() );
}

//void StrangeParticleApp::mouseDown( MouseEvent event )
//{
//}

void StrangeParticleApp::update()
{
    // Move ahead in time, which becomes the z-axis of our 3D noise.
    mAnimationCounter += 10.0f;
    
    for( auto &particle : mParticles ) {
        // Save off the last position for drawing lines.
        particle.mLastPosition = particle.mPosition;
        
        // Add some perlin noise to the velocity.
        vec3 deriv = mPerlin.dfBm( vec3( particle.mPosition.x, particle.mPosition.y, mAnimationCounter ) * 0.001f );
        particle.mZ = deriv.z;
        vec3 deriv2 = normalize( vec3( deriv.x, deriv.y, deriv.x ) );
        particle.mVelocity += deriv2 * mSpeed;
        
        // Move the particles according to their velocities.
        particle.mPosition += particle.mVelocity;
        
        // Dampen the velocities for the next frame.
        particle.mVelocity *= mConservationOfVelocity;
        
        // Replace any particles that have gone offscreen with a random onscreen position.
        if( isOffscreen( particle.mPosition ) )
        particle.reset( vec3( Rand::randFloat( getWindowWidth() ), Rand::randFloat( getWindowHeight() ), Rand::randFloat( getWindowHeight() ) ) );
    }
}

void StrangeParticleApp::draw()
{
    // Clear the window.
    gl::clear( Color::gray( 0.1f ) );
    
    // Enable additive blending.
    gl::ScopedBlendAdditive blend;
    
    // Draw all the particles as lines from mPosition to mLastPosition.
    // We use the convenience methods begin(), color(), vertex() and end() for simplicity,
    // see the ParticleSphere* samples for a faster method.
    gl::pushMatrices();
    gl::begin( GL_LINES );
    gl::setMatrices( mCamera );
    for( auto &particle : mParticles ) {
        // Color according to velocity.
        gl::color( 0.5f + particle.mVelocity.x / ( mSpeed * 2 ), 0.5f + particle.mVelocity.y / ( mSpeed * 2 ), 0.5f + particle.mZ * 0.5f );
        gl::vertex( particle.mLastPosition );
        gl::vertex( particle.mPosition );
    }
    gl::end();
    
    //    gl::begin( GL_POINTS );
    //    gl::setMatrices( mCamera );
    //    for( auto &particle : mParticles ) {
    //        // Color according to velocity.
    //        gl::color( 0.5f + particle.mVelocity.x / ( mSpeed * 2 ), 0.5f + particle.mVelocity.y / ( mSpeed * 2 ), 0.5f + particle.mZ * 0.5f );
    //        gl::vertex( particle.mPosition );
    //    }
    //    gl::end();
    
    
    gl::popMatrices();
    gl::drawString("Framerate: " + to_string( App::get()->getAverageFps() ), glm::vec2(10.0f, 10.0f), Color::white(), mFont);
}

void StrangeParticleApp::keyDown( KeyEvent event )
{
    switch( event.getCode() ) {
        case KeyEvent::KEY_ESCAPE:
            if( isFullScreen() )
            setFullScreen( false );
            else
            quit();
            break;
        case KeyEvent::KEY_f:
            setFullScreen( !isFullScreen() );
            break;
        case KeyEvent::KEY_v:
            gl::enableVerticalSync( !gl::isVerticalSyncEnabled() );
            break;
    }
}

// Returns \c true if a given point is visible onscreen.
bool StrangeParticleApp::isOffscreen( const vec3 &v )
{
    return ( ( v.x < 0 ) || ( v.x > getWindowWidth() ) || ( v.y < 0 ) || ( v.y > getWindowHeight() ) );
}

CINDER_APP( StrangeParticleApp, RendererGl, [] ( App::Settings *settings ) {
    settings->setWindowSize( 1280, 720 );
    settings->setHighDensityDisplayEnabled( true );
    settings->setMultiTouchEnabled( false );
} )
