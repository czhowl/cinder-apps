#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "cinder/Rand.h"
#include "cinder/CameraUi.h"
#include "cinder/Perlin.h"

#include "Particle.h"

using namespace ci;
using namespace ci::app;
using namespace std;

const int PositionIndex            = 0;
const int PositionEndIndex         = 1;
const int VelocityIndex            = 2;
const int ColorIndex               = 3;
const int RandomIndex              = 4;
const int TexCoordIndex            = 5;

class ImmersiveXsectionApp : public App {
  public:
	static void prepare(Settings *settings);
    void setup() override;
    void mouseDown( MouseEvent event ) override;
//    void mouseWheel( MouseEvent event ) override;
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
    gl::VboRef                         mPPositions[2], mPVelocities[2], mPEndPositions[2], mPColor[2], mPRandom, mPTexCoord;
    
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
    
    int numGrass            =  10000;
    int                     mWidth = 2;
    int                     mHeight = 2;
    vec2                        mTester;
    float                       mSit = 0.0f;
    
    gl::FboRef                  mColorFbo;
    
    
    vector<Particle>                    mP;
};

void ImmersiveXsectionApp::prepare(Settings *settings)
{
    settings->setTitle("XR in Real World: Immersive Xsection");
    settings->setWindowSize(800, 800);
//    settings->disableFrameRate();
}

void ImmersiveXsectionApp::setup()
{
    mWidth = sqrt(numGrass);
    mHeight = mWidth;
    numGrass = mWidth * mHeight;
    mDrawBuff = 1;
    mCamUI = CameraUi(&mCam);
    mCam.setPerspective( 10.0f, getWindowAspectRatio(), .01f, 1000.0f );
    mCam.lookAt( vec3( 0, 550, 0 ), vec3( 0, 0, 0 ) );
    
    // FBO
    gl::Fbo::Format fboFormat;
    fboFormat.setColorTextureFormat( gl::Texture2d::Format().internalFormat( GL_RGBA32F ) );
    mColorFbo = gl::Fbo::create( mWidth, mHeight, fboFormat );
    {
        gl::ScopedFramebuffer scpFbo( mColorFbo );
        gl::ScopedViewport    scpViewport( mColorFbo->getSize() );
        gl::clear();

    }
    // Ocean
    loadShaders();
    loadBuffers();
    
    for(int i = 0; i < 100; i++){
        mP.push_back(Particle(vec2(randFloat(-0.5, 0.5),randFloat(-0.5, 0.5)), vec2(randFloat(-0.1, 0.1),randFloat(-0.1, 0.1)), 0.5));
    }
    
}

void ImmersiveXsectionApp::mouseDown( MouseEvent event )
{
    mPUpdateGlsl->uniform( "Click",  getElapsedFrames() / 60.0f);
}

void ImmersiveXsectionApp::keyDown( KeyEvent event )
{
    switch (event.getCode()) {
        case KeyEvent::KEY_s:
            loadShaders();
            //            mCam.lookAt( vec3( 0, 500, 0 ), vec3( 0, 0, 0 ) );
            break;
        case KeyEvent::KEY_SPACE:
            mSit = 1.0f - mSit;
            mPUpdateGlsl->uniform( "Sit",  mSit);
            break;
        default:
            break;
    }
}

void ImmersiveXsectionApp::update()
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
    
    //    console() << mouse << endl;
    
    vec2 mouse = vec2(getWindow()->getMousePos().x, getWindow()->getMousePos().y) / vec2(getWindowSize().x, getWindowSize().y);
    mPUpdateGlsl->uniform( "Mouse",  mouse);
    
    mTester = mPerlin.dnoise( 0.0 , getElapsedSeconds()*0.3f)* 0.2f + vec2(0.5f);
    mPUpdateGlsl->uniform( "Tester",  mTester);
    // Opposite TransformFeedbackObj to catch the calculated values
    // In the opposite buffer
    mPFeedbackObj[1-mDrawBuff]->bind();
    
    // We begin Transform Feedback, using the same primitive that
    // we're "drawing". Using points for the particle system.
    gl::beginTransformFeedback( GL_POINTS );
    gl::drawArrays( GL_POINTS, 0, numGrass );
    gl::endTransformFeedback();

    for(int i = 0; i < mP.size(); i++){
//        vec2 seek = mP[i].seek(mouse - vec2(0.5));
        vec2 bound = mP[i].boundary(0.45f, 0.45f);
        mP[i].applyForce(bound * 2.0f);
        //                 ali, sep, coh, aliDist, sepDist, cohDist
        mP[i].flocking(mP, 1.0f, 1.3f, 1.f, 0.3f, 0.03f, 0.3f);
        mP[i].update();
    }
}

void ImmersiveXsectionApp::draw()
{
	// clear out the window with black
    gl::clear( Color( 0.04, 0.05, 0.16 ) );
    
    gl::pushMatrices();
    gl::setMatrices( mCam );
    gl::pushMatrices();
    gl::color(1.0, 0.5, 0.2);
    gl::rotate(M_PI/2, vec3(1.0,0.0,0.0));
    gl::drawSolidCircle((mTester - vec2(0.5)) * vec2(mWidth, mHeight), 0.5f, 30);
    for(int i = 0; i < mP.size(); i++){
        mP[i].draw(mWidth, mHeight);
    }
    gl::popMatrices();
    {
        gl::ScopedVao            vaoScope( mPVao[1-mDrawBuff] );
        gl::ScopedGlslProg        glslScope( mPRenderGlsl );
        //    gl::ScopedState            stateScope( GL_PROGRAM_POINT_SIZE, true );
        //    gl::ScopedBlend            blendScope( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        gl::ScopedBlendAdditive blend;
        gl::color(1.0, 1.0, 1.0);
        mPRenderGlsl->uniform( "Time", getElapsedFrames() / 60.0f );
        gl::setDefaultShaderVars();
        gl::drawArrays( GL_POINTS, 0, numGrass );
    }
    gl::popMatrices();
}

void ImmersiveXsectionApp::loadShaders()
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
        .attribLocation( "VertexRandom",    RandomIndex )
        .attribLocation( "VertexTexCoord",    TexCoordIndex );
        
        mPUpdateGlsl = ci::gl::GlslProg::create( mUpdateParticleGlslFormat );
    }
    catch( const ci::gl::GlslProgCompileExc &ex ) {
        console() << "PARTICLE UPDATE GLSL ERROR: " << ex.what() << std::endl;
    }
    
    mPUpdateGlsl->uniform( "H", 1.0f / 60.0f );
    mPUpdateGlsl->uniform( "Accel", vec3( 0.0f ) );
    mPUpdateGlsl->uniform( "ParticleLifetime", 3.0f );
    mPUpdateGlsl->uniform( "Click",  -10.0f);
    mPUpdateGlsl->uniform( "Sit",  mSit);
    
    try {
        ci::gl::GlslProg::Format mRenderParticleGlslFormat;
        // This being the render glsl, we provide a fragment shader.
        mRenderParticleGlslFormat.vertex( loadAsset( "render.vert" ) )
        .fragment( loadAsset( "render.frag" ) ).geometry( loadAsset( "render.geom" ) )
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

void ImmersiveXsectionApp::loadBuffers()
{
    // Initialize positions
    std::vector<vec3> positions( numGrass, vec3( 0.0f ) );
    std::vector<vec2> texcoord( numGrass, vec2( 0.0f ) );
    int i = 0;
    for( int x = 0; x < mWidth; ++x ) {
        for( int z = 0; z < mHeight; ++z ) {
            positions[i] = vec3( (x - mWidth / 2.0f), 0.0f, (z - mHeight / 2.0f));
            //            *vertColorIter++ = vec3( 1.0f );
            const float u = float( x ) / mWidth;
            const float v = float( z ) / mHeight;
            texcoord[i++] = vec2( u, v );
        }
    }
    
    mPTexCoord = ci::gl::Vbo::create( GL_ARRAY_BUFFER, texcoord.size() * sizeof(vec2), texcoord.data(), GL_STATIC_DRAW );
    
    // Create Position Vbo with the initial position data
    mPPositions[0] = ci::gl::Vbo::create( GL_ARRAY_BUFFER, positions.size() * sizeof(vec3), positions.data(), GL_STATIC_DRAW );
    // Create another Position Buffer that is null, for ping-ponging
    mPPositions[1] = ci::gl::Vbo::create( GL_ARRAY_BUFFER, positions.size() * sizeof(vec3), nullptr, GL_STATIC_DRAW );
    
    std::vector<vec3> ends = std::move( positions );
    
    mPEndPositions[0] = ci::gl::Vbo::create( GL_ARRAY_BUFFER, ends.size() * sizeof(vec3), ends.data(), GL_STATIC_DRAW );
    // Create another Position Buffer that is null, for ping-ponging
    mPEndPositions[1] = ci::gl::Vbo::create( GL_ARRAY_BUFFER, ends.size() * sizeof(vec3), nullptr, GL_STATIC_DRAW );
    
    std::vector<vec3> velocities( numGrass, vec3( 0.0f ) );
    
    // Create the Velocity Buffer using the newly buffered velocities
    mPVelocities[0] = ci::gl::Vbo::create( GL_ARRAY_BUFFER, velocities.size() * sizeof(vec3), velocities.data(), GL_STATIC_DRAW );
    // Create another Velocity Buffer that is null, for ping-ponging
    mPVelocities[1] = ci::gl::Vbo::create( GL_ARRAY_BUFFER, velocities.size() * sizeof(vec3), nullptr, GL_STATIC_DRAW );
    
    std::vector<vec4> colors( numGrass, vec4( 1.0f ) );
    
    // Create the Velocity Buffer using the newly buffered velocities
    mPColor[0] = ci::gl::Vbo::create( GL_ARRAY_BUFFER, colors.size() * sizeof(vec4), colors.data(), GL_STATIC_DRAW );
    // Create another Velocity Buffer that is null, for ping-ponging
    mPColor[1] = ci::gl::Vbo::create( GL_ARRAY_BUFFER, colors.size() * sizeof(vec4), nullptr, GL_STATIC_DRAW );
    
    std::vector<float> randoms( numGrass, 1000.0f );
    for( i = 0; i < numGrass; i++ ) {
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
        
        mPTexCoord->bind();
        ci::gl::vertexAttribPointer( TexCoordIndex, 2, GL_FLOAT, GL_FALSE, 0, 0 );
        ci::gl::enableVertexAttribArray( TexCoordIndex );
        
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

CINDER_APP( ImmersiveXsectionApp, RendererGl(RendererGl::Options().msaa(16)), &ImmersiveXsectionApp::prepare )
