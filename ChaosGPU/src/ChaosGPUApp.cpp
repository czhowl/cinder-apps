//
//    Copyright (c) 2014 David Wicks, sansumbrella.com
//    All rights reserved.
//
//    Particle Sphere sample application, GPU integration.
//
//    Author: David Wicks
//    License: BSD Simplified
//

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/Rand.h"
#include "cinder/gl/gl.h"
#include "cinder/CameraUi.h"


using namespace ci;
using namespace ci::app;
using namespace std;

/**
 Particle type holds information for rendering and simulation.
 Used to buffer initial simulation values.
 */
struct Particle
{
    vec3    pos;
    ColorA  color;
};

// How many particles to create. (600k default)
const int NUM_PARTICLES = 600e3;
/**
 Simple particle simulation with Verlet integration and mouse interaction.
 A sphere of particles is deformed by mouse interaction.
 Simulation is run using transform feedback on the GPU.
 particleUpdate.vs defines the simulation update step.
 Designed to have the same behavior as ParticleSphereCPU.
 */
class ChaosGPUApp : public App {
public:
    void setup() override;
    void update() override;
    void draw() override;
    void keyDown(KeyEvent event) override;
    void render();
private:
    gl::GlslProgRef mRenderProg;
    gl::GlslProgRef mUpdateProg;
    
    // Descriptions of particle data layout.
    gl::VaoRef        mAttributes[2];
    // Buffers holding raw particle data on GPU.
    gl::VboRef        mParticleBuffer[2];
    
    // Current source and destination buffers for transform feedback.
    // Source and destination are swapped each frame after update.
    std::uint32_t    mSourceIndex        = 0;
    std::uint32_t    mDestinationIndex    = 1;
    
    // Mouse state suitable for passing as uniforms to update program
    CameraPersp                 mCamera;
    CameraUi                 mCamUi;
    
    // equation parameters
    float            mDeltaTime = 0.01f;
    float             mConstantA =  5.0f;
    float             mConstantB = -10.0f;
    float             mConstantC = -0.38f;
    float            mRandomness = 0.1f;
    
    Font            mFont;
    
    gl::FboRef                 mFbo;
};

void ChaosGPUApp::setup()
{
    //hideCursor();
    mFont = Font("Arial", 12.0f);
    gl::Fbo::Format fboFormat;
    fboFormat.setColorTextureFormat(gl::Texture2d::Format().internalFormat(GL_RGBA32F));
    mFbo = gl::Fbo::create( 800.0f,480.0f, fboFormat);
    gl::ScopedFramebuffer scpFbo(mFbo);
    gl::ScopedViewport    scpViewport(mFbo->getSize());
    gl::clear();
    //const vec2 windowSize = toPixels(getWindowSize());
    mCamera = CameraPersp(800, 480, 45.0f, 0.01f, 3000.0f);
    mCamera.lookAt(vec3(0.0f, 0.0f, 51.0f), vec3(0.0f, 0.0f, 0.0f));
    mCamUi = CameraUi(&mCamera, getWindow());
    // Create initial particle layout.
    vector<Particle> particles;
    particles.assign( NUM_PARTICLES, Particle() );
    float x = 1;float y = 0;float z = 4.5;
    //vec3 center = vec3( getWindowCenter() + vec2( 0.0f, 40.0f ), 0.0f );
    for( int i = 0; i < particles.size(); ++i )
    {    // assign starting values to particles.
        float a = mConstantA;
        float b = mConstantB;
        float c = mConstantC;
        float dx = (a * x - y * z) * mDeltaTime;
        float dy = (b * y + x * z) * mDeltaTime;
        float dz = (c * z + x * y) * mDeltaTime;
        x += dx;
        y += dy;
        z += dz;
        
        auto &p = particles.at( i );
        p.pos = vec3(x, y, z);
        p.color = Color( dx + 0.2f, dy + 0.2f, dz + 0.1f);
    }
    
    // Create particle buffers on GPU and copy data into the first buffer.
    // Mark as static since we only write from the CPU once.
    mParticleBuffer[mSourceIndex] = gl::Vbo::create( GL_ARRAY_BUFFER, particles.size() * sizeof(Particle), particles.data(), GL_STATIC_DRAW );
    mParticleBuffer[mDestinationIndex] = gl::Vbo::create( GL_ARRAY_BUFFER, particles.size() * sizeof(Particle), nullptr, GL_STATIC_DRAW );
    
    for( int i = 0; i < 2; ++i )
    {    // Describe the particle layout for OpenGL.
        mAttributes[i] = gl::Vao::create();
        gl::ScopedVao vao( mAttributes[i] );
        
        // Define attributes as offsets into the bound particle buffer
        gl::ScopedBuffer buffer( mParticleBuffer[i] );
        gl::enableVertexAttribArray( 0 );
        gl::enableVertexAttribArray( 1 );
        gl::enableVertexAttribArray( 2 );
        gl::vertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)offsetof(Particle, pos) );
        gl::vertexAttribPointer( 1, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)offsetof(Particle, color) );
    }
    
    // Load our update program.
    // Match up our attribute locations with the description we gave.
    
    mRenderProg = gl::getStockShader( gl::ShaderDef().color() );
    mUpdateProg = gl::GlslProg::create( gl::GlslProg::Format().vertex( loadAsset( "particleUpdate.vs" ) )
                                       .feedbackFormat( GL_INTERLEAVED_ATTRIBS )
                                       .feedbackVaryings( { "position", "color"} )
                                       .attribLocation( "iPosition", 0 )
                                       .attribLocation( "iColor", 1 )
                                       );
}

void ChaosGPUApp::update()
{
    mConstantA = 1.5 * randPosNegFloat(0, mRandomness);
    mConstantB = 2 * randPosNegFloat(0, mRandomness);
    mConstantC = 1 * randPosNegFloat(0, mRandomness);
    // Update particles on the GPU
    gl::ScopedGlslProg prog( mUpdateProg );
    gl::ScopedState rasterizer( GL_RASTERIZER_DISCARD, true );    // turn off fragment stage
    mUpdateProg->uniform( "uDeltaTime", mDeltaTime );
    mUpdateProg->uniform( "uConstantA", mConstantA );
    mUpdateProg->uniform( "uConstantB", mConstantB );
    mUpdateProg->uniform( "uConstantC", mConstantC );
    
    // Bind the source data (Attributes refer to specific buffers).
    gl::ScopedVao source( mAttributes[mSourceIndex] );
    // Bind destination as buffer base.
    gl::bindBufferBase( GL_TRANSFORM_FEEDBACK_BUFFER, 0, mParticleBuffer[mDestinationIndex] );
    gl::beginTransformFeedback( GL_POINTS );
    
    // Draw source into destination, performing our vertex transformations.
    gl::drawArrays( GL_POINTS, 0, NUM_PARTICLES );
    
    gl::endTransformFeedback();
    
    // Swap source and destination for next loop
    std::swap( mSourceIndex, mDestinationIndex );
}

void ChaosGPUApp::draw()
{
    render();
    gl::draw(mFbo->getColorTexture());
    gl::drawString(to_string(App::get()->getAverageFps()), glm::vec2(10.0f, 10.0f), Color::white(), mFont);
}

void ChaosGPUApp::keyDown(KeyEvent event)
{
    /*    if (event.getChar() == 'c') {
     gl::ScopedFramebuffer scpFbo(mFbo);
     gl::ScopedViewport    scpViewport(mFbo->getSize());
     gl::clear();
     }*/
    if(event.getCode() == KeyEvent::KEY_ESCAPE){
        if( isFullScreen() )
            setFullScreen( false );
        else
            quit();
    }
    
    /*    if (event.getChar() == 'q') {
     if (r < 5) r += 0.01f;
     }
     if (event.getChar() == 'a') {
     if (r > 0) r -= 0.01f;
     }*/
}

void ChaosGPUApp::render(){
    // Bind the Fbo and update the viewport.
    gl::ScopedFramebuffer scpFbo(mFbo);
    
    gl::pushMatrices();
    gl::setMatricesWindow(mFbo->getSize());
    gl::ScopedBlendPremult scpBlend;
    // Draw transparent black rectangle.
    gl::ScopedColor scpColor(0, 0, 0, 20.0f/255.0f);
    
    gl::drawSolidRect({ 0, 0, 800, 480 });
    
    gl::popMatrices();
    
    gl::pushMatrices();
    gl::setMatrices(mCamera);
    gl::ScopedBlendAdditive blend;
    gl::ScopedGlslProg render( mRenderProg );
    gl::ScopedVao vao( mAttributes[mSourceIndex] );
    gl::context()->setDefaultShaderVars();
    gl::drawArrays( GL_POINTS, 0, NUM_PARTICLES );
    gl::popMatrices();
}

CINDER_APP(ChaosGPUApp, RendererGl, [](App::Settings *settings) {
    settings->setWindowSize( 800, 480 );
    //settings->setFullScreen();
    //settings->setMultiTouchEnabled( false );
} )
