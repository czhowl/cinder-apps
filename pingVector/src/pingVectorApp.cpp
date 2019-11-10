#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "cinder/Rand.h"
#include "cinder/CameraUi.h"
#include "cinder/Perlin.h"

using namespace ci;
using namespace ci::app;
using namespace std;


const int PositionIndex            = 0;
const int PositionEndIndex         = 1;
const int VelocityIndex            = 2;
const int ColorIndex               = 3;
const int TexCoordIndex            = 4;
const int RandomIndex              = 4;
//const int StartTimeIndex        = 2;
//const int InitialVelocityIndex    = 3;
//const int ColorIndex    = 3;

float mix( float x, float y, float a )
{
    return x * ( 1 - a ) + y * a;
}

class pingVectorApp : public App {
public:
    static void prepare(Settings *settings);
    void setup() override;
    void mouseDown( MouseEvent event ) override;
    void update() override;
    void draw() override;
    void keyDown(KeyEvent event) override;
    
    void loadBuffers();
    void loadShaders();
    void loadTexture();
    void loadBatch();
    
private:
    gl::VaoRef                         mPVao[2];
    gl::TransformFeedbackObjRef        mPFeedbackObj[2];
    gl::VboRef                         mPPositions[2], mPVelocities[2], mPEndPositions[2], mPColor[2], mPRandom;
    
    gl::GlslProgRef                    mPUpdateGlsl, mPRenderGlsl;
    
    Rand                               mRand;
    CameraPersp                        mCam;
    CameraUi                           mCamUI;
    uint32_t                           mDrawBuff;
    
    vec3                               mEmitter, mPrevEmitter;
    Perlin                             mPerlin;
    
    gl::BatchRef        mBatch;
    gl::TextureRef        mTexture;
    gl::GlslProgRef        mInstancingGlsl;
    gl::VboRef            mInstanceDataVbo;
    
    int nParticles            =  10000;
    int                     mWidth = 2;
    int                     mHeight = 2;
};

void pingVectorApp::prepare(Settings *settings)
{
    settings->setTitle("XR in Real World");
    settings->setWindowSize(800, 800);
    settings->disableFrameRate();
}

void pingVectorApp::setup()
{
    mWidth = sqrt(nParticles);
    mHeight = mWidth;
    nParticles = mWidth * mHeight;
    mDrawBuff = 1;
    mCamUI = CameraUi(&mCam, getWindow() );
    mCam.setPerspective( 10.0f, getWindowAspectRatio(), .01f, 1000.0f );
    mCam.lookAt( vec3( 0, 500, 0 ), vec3( 0, 0, 0 ) );
    
    loadShaders();
    loadBuffers();
}
void pingVectorApp::keyDown( KeyEvent event )
{
    switch (event.getCode()) {
        case KeyEvent::KEY_s:
            loadShaders();
            break;
        default:
            break;
    }
}

void pingVectorApp::mouseDown( MouseEvent event )
{
}

void pingVectorApp::update()
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

void pingVectorApp::draw()
{
    // clear out the window with black
    gl::clear( Color( 0, 0, 0 ) );
    static float rotateRadians = 0.0f;
    rotateRadians += 0.01f;
    
    gl::ScopedVao            vaoScope( mPVao[1-mDrawBuff] );
    gl::ScopedGlslProg        glslScope( mPRenderGlsl );
    gl::ScopedState            stateScope( GL_PROGRAM_POINT_SIZE, true );
    gl::ScopedBlend            blendScope( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    
    gl::pushMatrices();
    gl::setMatrices( mCam );
    //    gl::multModelMatrix( rotate( rotateRadians, vec3( 0, 1, 0 ) ) );
    
    mPRenderGlsl->uniform( "Time", getElapsedFrames() / 60.0f );
    gl::setDefaultShaderVars();
    gl::drawArrays( GL_POINTS, 0, nParticles );
    
    gl::popMatrices();
}

void pingVectorApp::loadShaders()
{
    try {
        // Create a vector of Transform Feedback "Varyings".
        // These strings tell OpenGL what to look for when capturing
        // Transform Feedback data. For instance, Position, Velocity,
        // and StartTime are variables in the updateSmoke.vert that we
        // write our calculations to.
        std::vector<std::string> transformFeedbackVaryings( 4 );
        transformFeedbackVaryings[PositionIndex] = "Position";
        transformFeedbackVaryings[PositionEndIndex] = "EndPosition";
        transformFeedbackVaryings[VelocityIndex] = "Velocity";
        transformFeedbackVaryings[ColorIndex] = "Color";
        
        ci::gl::GlslProg::Format mUpdateParticleGlslFormat;
        // Notice that we don't offer a fragment shader. We don't need
        // one because we're not trying to write pixels while updating
        // the position, velocity, etc. data to the screen.
        mUpdateParticleGlslFormat.vertex( loadAsset( "update.vert" ) )
        // This option will be either GL_SEPARATE_ATTRIBS or GL_INTERLEAVED_ATTRIBS,
        // depending on the structure of our data, below. We're using multiple
        // buffers. Therefore, we're using GL_SEPERATE_ATTRIBS
        .feedbackFormat( GL_SEPARATE_ATTRIBS )
        // Pass the feedbackVaryings to glsl
        .feedbackVaryings( transformFeedbackVaryings )
        .attribLocation( "VertexPosition",            PositionIndex )
        .attribLocation( "VertexEndPosition",         PositionEndIndex )
        .attribLocation( "VertexVelocity",            VelocityIndex )
        .attribLocation( "VertexColor",               ColorIndex )
        .attribLocation( "VertexRandom",    RandomIndex );
//        .attribLocation( "VertexTexCoord",    InitialVelocityIndex );
        
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
        mRenderParticleGlslFormat.vertex( loadAsset( "render.vert" ) )
        .fragment( loadAsset( "render.frag" ) ).geometry( loadAsset( "test.geom" ) )
        .attribLocation( "VertexPosition",            PositionIndex )
        .attribLocation( "VertexEndPosition",         PositionEndIndex )
        .attribLocation( "VertexColor",               ColorIndex );;
        
        mPRenderGlsl = ci::gl::GlslProg::create( mRenderParticleGlslFormat );
    }
    catch( const ci::gl::GlslProgCompileExc &ex ) {
        console() << "PARTICLE RENDER GLSL ERROR: " << ex.what() << std::endl;
    }
    
//    mPRenderGlsl->uniform( "ParticleTex", 0 );
    mPRenderGlsl->uniform( "MinParticleSize", 1.0f );
    mPRenderGlsl->uniform( "MaxParticleSize", 64.0f );
    mPRenderGlsl->uniform( "ParticleLifetime", 3.0f );
}

void pingVectorApp::loadBuffers()
{
    // Initialize positions
    std::vector<vec3> positions( nParticles, vec3( 0.0f ) );
    int i = 0;
    for( int x = 0; x < mWidth; ++x ) {
        for( int z = 0; z < mHeight; ++z ) {
            positions[i++] = vec3( (x - mWidth / 2.0f), 0.0f, (z - mHeight / 2.0f));
//            *vertColorIter++ = vec3( 1.0f );
//            const float u = float( x ) / mWidth;
//            const float v = float( z ) / mHeight;
//            *vertCoordIter++ = vec2( u, v );
        }
    }
    
    // Create Position Vbo with the initial position data
    mPPositions[0] = ci::gl::Vbo::create( GL_ARRAY_BUFFER, positions.size() * sizeof(vec3), positions.data(), GL_STATIC_DRAW );
    // Create another Position Buffer that is null, for ping-ponging
    mPPositions[1] = ci::gl::Vbo::create( GL_ARRAY_BUFFER, positions.size() * sizeof(vec3), nullptr, GL_STATIC_DRAW );
    
    std::vector<vec3> ends = std::move( positions );
    
    mPEndPositions[0] = ci::gl::Vbo::create( GL_ARRAY_BUFFER, ends.size() * sizeof(vec3), ends.data(), GL_STATIC_DRAW );
    // Create another Position Buffer that is null, for ping-ponging
    mPEndPositions[1] = ci::gl::Vbo::create( GL_ARRAY_BUFFER, ends.size() * sizeof(vec3), nullptr, GL_STATIC_DRAW );
    
    std::vector<vec3> velocities( nParticles, vec3( 0.0f ) );
    
    // Create the Velocity Buffer using the newly buffered velocities
    mPVelocities[0] = ci::gl::Vbo::create( GL_ARRAY_BUFFER, velocities.size() * sizeof(vec3), velocities.data(), GL_STATIC_DRAW );
    // Create another Velocity Buffer that is null, for ping-ponging
    mPVelocities[1] = ci::gl::Vbo::create( GL_ARRAY_BUFFER, velocities.size() * sizeof(vec3), nullptr, GL_STATIC_DRAW );
    
    std::vector<vec4> colors( nParticles, vec4( 1.0f ) );
    
    // Create the Velocity Buffer using the newly buffered velocities
    mPColor[0] = ci::gl::Vbo::create( GL_ARRAY_BUFFER, colors.size() * sizeof(vec4), colors.data(), GL_STATIC_DRAW );
    // Create another Velocity Buffer that is null, for ping-ponging
    mPColor[1] = ci::gl::Vbo::create( GL_ARRAY_BUFFER, colors.size() * sizeof(vec4), nullptr, GL_STATIC_DRAW );
    
    std::vector<float> randoms( nParticles, 1000.0f );
//    for(auto r:randoms){
//        r = randFloat(500, 1000);
//    }
    for( i = 0; i < nParticles; i++ ) {
        randoms[i] = randFloat(1000, 2000);
    }
    mPRandom  = ci::gl::Vbo::create( GL_ARRAY_BUFFER,    randoms.size() * sizeof(float), randoms.data(), GL_STATIC_DRAW );
    for( int i = 0; i < 2; i++ ) {
        // Initialize the Vao's holding the info for each buffer
        mPVao[i] = ci::gl::Vao::create();
        
        // Bind the vao to capture index data for the glsl
        mPVao[i]->bind();
        mPPositions[i]->bind();
        ci::gl::vertexAttribPointer( PositionIndex, 3, GL_FLOAT, GL_FALSE, 0, 0 );
        ci::gl::enableVertexAttribArray( PositionIndex );
        
        mPEndPositions[i]->bind();
        ci::gl::vertexAttribPointer( PositionEndIndex, 3, GL_FLOAT, GL_FALSE, 0, 0 );
        ci::gl::enableVertexAttribArray( PositionEndIndex );
        
        mPVelocities[i]->bind();
        ci::gl::vertexAttribPointer( VelocityIndex, 3, GL_FLOAT, GL_FALSE, 0, 0 );
        ci::gl::enableVertexAttribArray( VelocityIndex );
        
        mPColor[i]->bind();
        ci::gl::vertexAttribPointer( ColorIndex, 4, GL_FLOAT, GL_FALSE, 0, 0 );
        ci::gl::enableVertexAttribArray( ColorIndex );
        
        mPRandom->bind();
        ci::gl::vertexAttribPointer( RandomIndex, 1, GL_FLOAT, GL_FALSE, 0, 0 );
        ci::gl::enableVertexAttribArray( RandomIndex );
        
        // Create a TransformFeedbackObj, which is similar to Vao
        // It's used to capture the output of a glsl and uses the
        // index of the feedback's varying variable names.
        mPFeedbackObj[i] = gl::TransformFeedbackObj::create();
        
        // Bind the TransformFeedbackObj and bind each corresponding buffer
        // to it's index.
        mPFeedbackObj[i]->bind();
        gl::bindBufferBase( GL_TRANSFORM_FEEDBACK_BUFFER, PositionIndex, mPPositions[i] );
        gl::bindBufferBase( GL_TRANSFORM_FEEDBACK_BUFFER, PositionEndIndex, mPEndPositions[i] );
        gl::bindBufferBase( GL_TRANSFORM_FEEDBACK_BUFFER, VelocityIndex, mPVelocities[i] );
        gl::bindBufferBase( GL_TRANSFORM_FEEDBACK_BUFFER, ColorIndex, mPColor[i] );
        mPFeedbackObj[i]->unbind();
    }
}

CINDER_APP( pingVectorApp, RendererGl(RendererGl::Options().msaa(16)), &pingVectorApp::prepare )
