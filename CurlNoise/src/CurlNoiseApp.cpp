#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "cinder/gl/TransformFeedbackObj.h"
#include "cinder/Rand.h"
#include "cinder/CameraUi.h"

using namespace ci;
using namespace ci::app;
using namespace std;

const int nParticles            = 40000;
const int PositionIndex            = 0;
const int VelocityIndex            = 1;
const int StartTimeIndex        = 2;
const int InitialVelocityIndex    = 4;
const int ColorIndex    = 3;

float mix( float x, float y, float a )
{
    return x * ( 1 - a ) + y * a;
}

class CurlNoiseApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
    void keyDown( KeyEvent event ) override;
	void update() override;
	void draw() override;
    
    void loadBuffers();
    void loadShaders();
    void loadTexture();
    
  private:
    gl::VaoRef                         mPVao[2];
    gl::TransformFeedbackObjRef        mPFeedbackObj[2];
    gl::VboRef                         mPPositions[2], mPVelocities[2], mPStartTimes[2], mPInitVelocity, mPColor[2];

    gl::GlslProgRef                    mPUpdateGlsl, mPRenderGlsl;

    Rand                               mRand;
    CameraPersp                        mCam;
    CameraUi                           mCamUI;
    uint32_t                           mDrawBuff;
};

void CurlNoiseApp::setup()
{
    mDrawBuff = 1;
    
    mCam.setPerspective( 60.0f, getWindowAspectRatio(), .01f, 1000.0f );
    mCam.lookAt( vec3( 10, 10, 10 ), vec3( 0, 0, 0 ) );
    mCamUI = CameraUi(&mCam, getWindow() );
    loadShaders();
    loadBuffers();
}

void CurlNoiseApp::mouseDown( MouseEvent event )
{
}

void CurlNoiseApp::keyDown( KeyEvent event )
{
    switch (event.getCode()) {
        case KeyEvent::KEY_s:
            loadShaders();
            break;
        default:
            break;
    }
    loadShaders();
}

void CurlNoiseApp::update()
{
    // This equation just reliably swaps all concerned buffers
    mDrawBuff = 1 - mDrawBuff;
    
    gl::ScopedGlslProg    glslScope( mPUpdateGlsl );
    // We use this vao for input to the Glsl, while using the opposite
    // for the TransformFeedbackObj.
    gl::ScopedVao        vaoScope( mPVao[mDrawBuff] );
    // Because we're not using a fragment shader, we need to
    // stop the rasterizer. This will make sure that OpenGL won't
    // move to the rasterization stage.
    gl::ScopedState        stateScope( GL_RASTERIZER_DISCARD, true );
    
    mPUpdateGlsl->uniform( "Time", getElapsedFrames() / 60.0f );
    
    // Opposite TransformFeedbackObj to catch the calculated values
    // In the opposite buffer
    mPFeedbackObj[1-mDrawBuff]->bind();
    
    // We begin Transform Feedback, using the same primitive that
    // we're "drawing". Using points for the particle system.
    gl::beginTransformFeedback( GL_POINTS );
    gl::drawArrays( GL_POINTS, 0, nParticles );
    gl::endTransformFeedback();
}

void CurlNoiseApp::draw()
{
    gl::pushMatrices();
    gl::setMatrices( mCam );
	gl::clear( Color( 0, 0, 0 ) );
//    gl::drawSphere( vec3(), 1.0f );
    static float rotateRadians = 0.0f;
    rotateRadians += 0.01f;
    
    gl::ScopedVao            vaoScope( mPVao[1-mDrawBuff] );
    gl::ScopedGlslProg        glslScope( mPRenderGlsl );
    gl::ScopedState            stateScope( GL_PROGRAM_POINT_SIZE, true );
    gl::ScopedBlend            blendScope( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    
    gl::multModelMatrix( rotate( rotateRadians, vec3( 0, 1, 0 ) ) );
    
    mPRenderGlsl->uniform( "Time", getElapsedFrames() / 60.0f );
    gl::setDefaultShaderVars();
    gl::drawArrays( GL_POINTS, 0, nParticles );
    
    gl::popMatrices();
}

void CurlNoiseApp::loadShaders()
{
    try {
        // Create a vector of Transform Feedback "Varyings".
        // These strings tell OpenGL what to look for when capturing
        // Transform Feedback data. For instance, Position, Velocity,
        // and StartTime are variables in the updateSmoke.vert that we
        // write our calculations to.
        std::vector<std::string> transformFeedbackVaryings( 4 );
        transformFeedbackVaryings[PositionIndex] = "Position";
        transformFeedbackVaryings[VelocityIndex] = "Velocity";
        transformFeedbackVaryings[ColorIndex] = "Color";
        transformFeedbackVaryings[StartTimeIndex] = "StartTime";
        
        ci::gl::GlslProg::Format mUpdateParticleGlslFormat;
        // Notice that we don't offer a fragment shader. We don't need
        // one because we're not trying to write pixels while updating
        // the position, velocity, etc. data to the screen.
        mUpdateParticleGlslFormat.vertex( loadAsset( "updateSmoke.vert" ) )
        // This option will be either GL_SEPARATE_ATTRIBS or GL_INTERLEAVED_ATTRIBS,
        // depending on the structure of our data, below. We're using multiple
        // buffers. Therefore, we're using GL_SEPERATE_ATTRIBS
            .feedbackFormat( GL_SEPARATE_ATTRIBS )
        // Pass the feedbackVaryings to glsl
            .feedbackVaryings( transformFeedbackVaryings )
            .attribLocation( "VertexPosition",            PositionIndex )
            .attribLocation( "VertexVelocity",            VelocityIndex )
            .attribLocation( "VertexStartTime",            StartTimeIndex )
            .attribLocation( "VertexColor",    ColorIndex )
            .attribLocation( "VertexInitialVelocity",    InitialVelocityIndex );
        
        mPUpdateGlsl = ci::gl::GlslProg::create( mUpdateParticleGlslFormat );
    }
    catch( const ci::gl::GlslProgCompileExc &ex ) {
        console() << "PARTICLE UPDATE GLSL ERROR: " << ex.what() << std::endl;
    }
    
    mPUpdateGlsl->uniform( "H", 1.0f / 60.0f );
    mPUpdateGlsl->uniform( "Accel", vec3( 0.0f ) );
    mPUpdateGlsl->uniform( "ParticleLifetime", 3.0f );
    
    try {
        ci::gl::GlslProg::Format mRenderParticleGlslFormat;
        // This being the render glsl, we provide a fragment shader.
        mRenderParticleGlslFormat.vertex( loadAsset( "renderSmoke.vert" ) )
            .fragment( loadAsset( "renderSmoke.frag" ) )
            .attribLocation("VertexPosition",            PositionIndex )
            .attribLocation( "VertexStartTime",            StartTimeIndex )
            .attribLocation( "VertexColor",             ColorIndex );
        
        mPRenderGlsl = ci::gl::GlslProg::create( mRenderParticleGlslFormat );
    }
    catch( const ci::gl::GlslProgCompileExc &ex ) {
        console() << "PARTICLE RENDER GLSL ERROR: " << ex.what() << std::endl;
    }
    
    mPRenderGlsl->uniform( "ParticleTex", 0 );
    mPRenderGlsl->uniform( "MinParticleSize", 1.0f );
    mPRenderGlsl->uniform( "MaxParticleSize", 64.0f );
    mPRenderGlsl->uniform( "ParticleLifetime", 3.0f );
}

void CurlNoiseApp::loadBuffers()
{
    // Initialize positions
    std::vector<vec3> positions( nParticles, vec3( 0.0f ) );
    std::vector<vec4> colors( nParticles, vec4( 1.0f ) );
    // Create Position Vbo with the initial position data
    mPPositions[0] = ci::gl::Vbo::create( GL_ARRAY_BUFFER, positions.size() * sizeof(vec3), positions.data(), GL_STATIC_DRAW );
    // Create another Position Buffer that is null, for ping-ponging
    mPPositions[1] = ci::gl::Vbo::create( GL_ARRAY_BUFFER, positions.size() * sizeof(vec3), nullptr, GL_STATIC_DRAW );
    
    // Reuse the positions vector that we've already made
    std::vector<vec3> normals = std::move( positions );
    
    for( auto normalIt = normals.begin(); normalIt != normals.end(); ++normalIt ) {
        // Creating a random velocity for each particle in a unit sphere
        *normalIt = ci::randVec3() * mix( 0.0f, 1.5f, mRand.nextFloat() );
    }
    
    // Create the Velocity Buffer using the newly buffered velocities
    mPVelocities[0] = ci::gl::Vbo::create( GL_ARRAY_BUFFER, normals.size() * sizeof(vec3), normals.data(), GL_STATIC_DRAW );
    // Create another Velocity Buffer that is null, for ping-ponging
    mPVelocities[1] = ci::gl::Vbo::create( GL_ARRAY_BUFFER, normals.size() * sizeof(vec3), nullptr, GL_STATIC_DRAW );
    // Create an initial velocity buffer, so that you can reset a particle's velocity after it's dead
    mPInitVelocity = ci::gl::Vbo::create( GL_ARRAY_BUFFER,    normals.size() * sizeof(vec3), normals.data(), GL_STATIC_DRAW );
    
    // Create time data for the initialization of the particles
    array<GLfloat, nParticles> timeData;
    float time = 0.0f;
    float rate = 0.001f;
    for( int i = 0; i < nParticles; i++ ) {
        timeData[i] = time;
        time += rate;
    }

    // Create the StartTime Buffer, so that we can reset the particle after it's dead
    mPStartTimes[0] = ci::gl::Vbo::create( GL_ARRAY_BUFFER, timeData.size() * sizeof( float ), timeData.data(), GL_DYNAMIC_COPY );
    // Create the StartTime ping-pong buffer
    mPStartTimes[1] = ci::gl::Vbo::create( GL_ARRAY_BUFFER, nParticles * sizeof( float ), nullptr, GL_DYNAMIC_COPY );
    
    // Create the StartTime Buffer, so that we can reset the particle after it's dead
    mPColor[0] = ci::gl::Vbo::create( GL_ARRAY_BUFFER, colors.size() * sizeof(vec4), colors.data(), GL_STATIC_DRAW );
    // Create the StartTime ping-pong buffer
    mPColor[1] = ci::gl::Vbo::create( GL_ARRAY_BUFFER, colors.size() * sizeof(vec4), nullptr, GL_STATIC_DRAW );
    
    for( int i = 0; i < 2; i++ ) {
        // Initialize the Vao's holding the info for each buffer
        mPVao[i] = ci::gl::Vao::create();
        
        // Bind the vao to capture index data for the glsl
        mPVao[i]->bind();
        mPPositions[i]->bind();
        ci::gl::vertexAttribPointer( PositionIndex, 3, GL_FLOAT, GL_FALSE, 0, 0 );
        ci::gl::enableVertexAttribArray( PositionIndex );
        
        mPVelocities[i]->bind();
        ci::gl::vertexAttribPointer( VelocityIndex, 3, GL_FLOAT, GL_FALSE, 0, 0 );
        ci::gl::enableVertexAttribArray( VelocityIndex );
        
        mPStartTimes[i]->bind();
        ci::gl::vertexAttribPointer( StartTimeIndex, 1, GL_FLOAT, GL_FALSE, 0, 0 );
        ci::gl::enableVertexAttribArray( StartTimeIndex );
        
        mPInitVelocity->bind();
        ci::gl::vertexAttribPointer( InitialVelocityIndex, 3, GL_FLOAT, GL_FALSE, 0, 0 );
        ci::gl::enableVertexAttribArray( InitialVelocityIndex );
        
        mPColor[i]->bind();
        ci::gl::vertexAttribPointer( ColorIndex, 4, GL_FLOAT, GL_FALSE, 0, 0 );
        ci::gl::enableVertexAttribArray( ColorIndex );
        
        // Create a TransformFeedbackObj, which is similar to Vao
        // It's used to capture the output of a glsl and uses the
        // index of the feedback's varying variable names.
        mPFeedbackObj[i] = gl::TransformFeedbackObj::create();
        
        // Bind the TransformFeedbackObj and bind each corresponding buffer
        // to it's index.
        mPFeedbackObj[i]->bind();
        gl::bindBufferBase( GL_TRANSFORM_FEEDBACK_BUFFER, PositionIndex, mPPositions[i] );
        gl::bindBufferBase( GL_TRANSFORM_FEEDBACK_BUFFER, VelocityIndex, mPVelocities[i] );
        gl::bindBufferBase( GL_TRANSFORM_FEEDBACK_BUFFER, StartTimeIndex, mPStartTimes[i] );
        gl::bindBufferBase( GL_TRANSFORM_FEEDBACK_BUFFER, ColorIndex, mPColor[i] );
        mPFeedbackObj[i]->unbind();
    }
}

CINDER_APP( CurlNoiseApp, RendererGl )
