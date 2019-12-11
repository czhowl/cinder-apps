#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "cinder/Rand.h"
#include "cinder/CameraUi.h"
#include "cinder/Perlin.h"
#include "cinder/params/Params.h"
#include "cinder/Serial.h"

#include "Particle.h"

#include <librealsense2/rs.hpp>
#ifdef __cplusplus
#undef NO
#undef YES
#import <opencv2/opencv.hpp>
#endif

#define SCENE_SIZE 1600
#define BLUR_SIZE 200
#define FISH_NUM 50
#define IR_WIDTH 480
#define IR_HEIGHT 270
#define CONNECTIONS true

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace cv;

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
    void resize() override;
    void renderScene();
    void renderFish();
    
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
    float                       mSitA = 0.0f;
    float                       mSitB = 0.0f;
    float                       mClickA = -10.0f;
    float                       mClickB = -10.0f;
    
    vector<Particle>                    mP;
    
    gl::FboRef                          mCausticFbo;
    gl::GlslProgRef                     mCausticGlsl;
    
    gl::GlslProgRef                     mShaderBlur;
    gl::FboRef                          mFboScene;
    gl::FboRef                          mFboBlur1;
    gl::FboRef                          mFboBlur2;
    
    params::InterfaceGlRef              mParams;
    float                               mFrameRate;
    float                               mBloomEffect;
    float                               mAlignment;
    float                               mSeparation;
    float                               mCohesion;
    float                               mAlignmentDist;
    float                               mSeparationDist;
    float                               mCohesionDist;
    bool                                mDrawIr;
    bool                                mDrawFilteredIr;
    bool                                mDrawIrBlob;
    int                                 mExposure;
    int                                 mGain;
    int                                 mLowerBound;
    int                                 mUpperBound;
    int                                 mSitThresh;
    
    // Declare depth colorizer for pretty visualization of depth data
    rs2::colorizer color_map;
    // Declare rates printer for showing streaming rates of the enabled streams.
    rs2::rates_printer printer;
    
    // Declare RealSense pipeline, encapsulating the actual device and sensors
    rs2::pipeline pipe;
    rs2::pipeline_profile pipeline_profile;
    
    gl::Texture2dRef            mIrTex;
    gl::Texture2dRef            mFilteredTex;
    gl::Texture2dRef            mBlobTex;
    
    vec2                        mlastMouse;
    SerialRef                   mSerial;
    
    vec2                        mChairA;
    vec2                        mChairB;
    vec2                        mLastChairA;
    vec2                        mLastChairB;
    
    vec2                        mTracking[2];
    bool                        mFirstTrack = true;
};

void ImmersiveXsectionApp::prepare(Settings *settings)
{
    settings->setTitle("XR in Real World: Immersive Xsection");
    settings->setWindowSize(800, 800);
    settings->setHighDensityDisplayEnabled();
    //    settings->disableFrameRate();
}

void ImmersiveXsectionApp::setup()
{
    // -------------------------------- Fish Parameters
    mFrameRate = 0.0f;
    mAlignment = 0.2f;
    mSeparation = 1.1f;
    mCohesion = 0.13f;
    mAlignmentDist = 0.1f;
    mSeparationDist = 0.03f;
    mCohesionDist = 0.06f;
    mBloomEffect = 1.0f;
    mDrawIr = false;
    mDrawFilteredIr = false;
    mDrawIrBlob = false;
    mExposure = 1000;
    mGain = 16;
    mLowerBound = 100;
    mUpperBound = 255;
    mSitThresh = 30;
    mParams = params::InterfaceGl::create( getWindow(), "App parameters", toPixels(ivec2( 300, 400 )) );
    mParams->addParam( "Frame rate",        &mFrameRate,                    "", true );
    mParams->addParam( "Bloom Attenuation", &mBloomEffect ).min( 0.0f ).max( 2.5f ).step( 0.01f );
    mParams->addSeparator();
    mParams->addParam( "Alignment", &mAlignment ).min( 0.0f ).max( 2.0f ).step( 0.01f );
    mParams->addParam( "Alignment Distance", &mAlignmentDist ).min( 0.0f ).max( 0.5f ).step( 0.01f );
    mParams->addSeparator();
    mParams->addParam( "Separation", &mSeparation ).min( 0.0f ).max( 2.0f ).step( 0.01f );
    mParams->addParam( "Separation Distance", &mSeparationDist ).min( 0.0f ).max( 0.5f ).step( 0.01f );
    mParams->addSeparator();
    mParams->addParam( "Cohesion", &mCohesion ).min( 0.0f ).max( 2.0f ).step( 0.01f );
    mParams->addParam( "Cohesion Distance", &mCohesionDist ).min( 0.0f ).max( 0.5f ).step( 0.01f );
    mParams->addSeparator();
    mParams->addParam( "Draw IR Texture", &mDrawIr );
    mParams->addParam( "Draw IR Filtered", &mDrawFilteredIr );
    mParams->addParam( "Draw IR Blob", &mDrawIrBlob );
    mParams->addParam( "Exporsure", &mExposure ).min( 20 ).max( 166000 ).step( 20 ).updateFn( [this] {
        rs2::depth_sensor depth_sensor = pipeline_profile.get_device().first<rs2::depth_sensor>();
        if( depth_sensor.supports( rs2_option::RS2_OPTION_EXPOSURE ) ){
            depth_sensor.set_option( rs2_option::RS2_OPTION_EXPOSURE, mExposure ); // def:8500 (min:20 - max:166000), step:20
        }
    } );
    mParams->addParam( "Gain", &mGain ).min( 16 ).max( 248 ).step( 1 ).updateFn( [this] {
        rs2::depth_sensor depth_sensor = pipeline_profile.get_device().first<rs2::depth_sensor>();
        if( depth_sensor.supports( rs2_option::RS2_OPTION_GAIN ) ){
            depth_sensor.set_option( rs2_option::RS2_OPTION_GAIN, mGain ); // def:16 (min:16 - max:248), step:1
        }
    } );
    mParams->addSeparator();
    mParams->addParam( "Lower Bound", &mLowerBound ).min( 0 ).max( 254 ).step( 1 ).updateFn( [this] {
        if(mLowerBound >= mUpperBound){
            mLowerBound = mUpperBound - 1;
        }
    } );
    mParams->addParam( "Upper Bound", &mUpperBound ).min( 1 ).max( 255 ).step( 1 ).updateFn( [this] {
        if(mLowerBound >= mUpperBound){
            mUpperBound = mLowerBound + 1;
        }
    } );
    mParams->addParam( "Sit Threshold", &mSitThresh ).min( 0 ).max( 480 ).step( 1 );
    mParams->addButton("Switch Chair", [this]{
        vec2 temp = mTracking[0];
        mTracking[0] = mTracking[1];
        mTracking[1] = temp;
    });
    
    for(int i = 0; i < FISH_NUM; i++){
        mP.push_back(Particle(vec2(randFloat(-0.5, 0.5),randFloat(-0.5, 0.5)), vec2(randFloat(-0.1, 0.1),randFloat(-0.1, 0.1)), 0.13));
    }
    // -------------------------------- Coral Parameters
    mWidth = sqrt(numGrass);
    mHeight = mWidth;
    numGrass = mWidth * mHeight;
    mDrawBuff = 1;
    // -------------------------------- Camera
    mCamUI = CameraUi(&mCam);
    mCam.setPerspective( 10.0f, getWindowAspectRatio(), .01f, 1000.0f );
    mCam.lookAt( vec3( 0, 550, 0 ), vec3( 0, 0, 0 ) );
    
    // -------------------------------- Caustic Light
    gl::Fbo::Format fboFormat;
    fboFormat.setColorTextureFormat( gl::Texture2d::Format().internalFormat( GL_RGBA32F ) );
    mCausticFbo = gl::Fbo::create( mWidth, mHeight, fboFormat );
    {
        gl::ScopedFramebuffer scpFbo( mCausticFbo );
        gl::ScopedViewport    scpViewport( mCausticFbo->getSize() );
        gl::clear(ColorA( 1.0, 0.00, 0.0, 1.0 ));
    }
    // -------------------------------- Bloom
    gl::Fbo::Format fmt;
    fmt.setSamples( 16 );
    
    // setup our scene Fbo
    mFboScene = gl::Fbo::create( SCENE_SIZE, SCENE_SIZE, fmt );
    
    // setup our blur Fbo's, smaller ones will generate a bigger blur
    mFboBlur1 = gl::Fbo::create( BLUR_SIZE, BLUR_SIZE );
    mFboBlur2 = gl::Fbo::create( BLUR_SIZE, BLUR_SIZE );
    
    // -------------------------------- Shaders & Buffers
    loadShaders();
    loadBuffers();
    
    if(CONNECTIONS){
        // -------------------------------- Realsense + OpenCV
        
        rs2::config cfg;
        cfg.enable_stream(RS2_STREAM_INFRARED, IR_WIDTH, IR_HEIGHT, RS2_FORMAT_Y8, 30);
        cfg.enable_stream(RS2_STREAM_DEPTH, IR_WIDTH, IR_HEIGHT, RS2_FORMAT_Z16, 30);
        pipeline_profile = pipe.start( cfg );
        rs2::frameset frames;
        // Set Sensor Option
        // IR Emitter
        rs2::depth_sensor depth_sensor = pipeline_profile.get_device().first<rs2::depth_sensor>();
        if( depth_sensor.supports( rs2_option::RS2_OPTION_EMITTER_ENABLED ) ){
            depth_sensor.set_option( rs2_option::RS2_OPTION_EMITTER_ENABLED, 0.0f ); // Disable IR Emitter
            // depth_sensor.set_option( rs2_option::RS2_OPTION_EMITTER_ENABLED, 1.0f ); // Enable IR Emitter
        }
        
        // Exposure
        if( depth_sensor.supports( rs2_option::RS2_OPTION_EXPOSURE ) ){
            //        const rs2::option_range option_range = depth_sensor.get_option_range( rs2_option::RS2_OPTION_EXPOSURE );
            depth_sensor.set_option( rs2_option::RS2_OPTION_EXPOSURE, mExposure ); // def:8500 (min:20 - max:166000), step:20
        }
        //
        //    // Gain
        if( depth_sensor.supports( rs2_option::RS2_OPTION_GAIN ) ){
            //        const rs2::option_range option_range = depth_sensor.get_option_range( rs2_option::RS2_OPTION_GAIN );
            depth_sensor.set_option( rs2_option::RS2_OPTION_GAIN, mGain ); // def:16 (min:16 - max:248), step:1
        }
        for (int i = 0; i < 30; i++)
        {
            frames = pipe.wait_for_frames();
        }
        
        // -------------------------------- Chair Communication
        for( const auto &dev : Serial::getDevices() )
            console() << "Device: " << dev.getName() << endl;
        
        try {
            Serial::Device dev = Serial::findDeviceByNameContains( "cu.usbmodem14403" );
            mSerial = Serial::create( dev, 9600 );
        }
        catch( SerialExc &exc ) {
            console() << "coult not initialize the serial device " << endl;
            exit( -1 );
        }
        
        mTracking[0] = vec2(0);
        mTracking[1] = vec2(0);
    }
}

void ImmersiveXsectionApp::resize(){
    mCamUI.setWindowSize( getWindow()->getSize() );
    mCam.setAspectRatio( getWindow()->getAspectRatio() );
}

void ImmersiveXsectionApp::mouseDown( MouseEvent event )
{
    mClickA = getElapsedFrames() / 60.0f;
    mPUpdateGlsl->uniform( "ClickA", mClickA);
}

void ImmersiveXsectionApp::keyDown( KeyEvent event )
{
    switch (event.getCode()) {
        case KeyEvent::KEY_s:
            loadShaders();
            //            mCam.lookAt( vec3( 0, 500, 0 ), vec3( 0, 0, 0 ) );
            break;
        case KeyEvent::KEY_SPACE:
            break;
        default:
            break;
    }
}

void ImmersiveXsectionApp::update()
{
    mFrameRate    = getAverageFps();
    vec2 mouse;
    // -----------
    if(!CONNECTIONS){
        //        hideCursor();
        mouse = vec2(getWindow()->getMousePos().x, getWindow()->getMousePos().y) / vec2(getWindowSize().x, getWindowSize().y);;
    }else{
        uint8_t b = 0;
        if( mSerial->getNumBytesAvailable() > 0 ) {
            mSerial->readBytes(&b, 1);
//            console() << (int)b << endl;
            if(b & 0b00000001){
                mClickA = getElapsedFrames() / 60.0f;
                mPUpdateGlsl->uniform( "ClickA", mClickA);
            }
            if(b & 0b00000010){
                mClickB = getElapsedFrames() / 60.0f;
                mPUpdateGlsl->uniform( "ClickB", mClickB);
            }
            mSerial->flush();
        }
        // -------------------------------- Camera
        mChairA = mLastChairA;
        mChairB = mLastChairB;
        rs2::frameset fs;
        if (pipe.poll_for_frames(&fs))
        {
            rs2::frame ir_frame = fs.first(RS2_STREAM_INFRARED);
            //        rs2::frame depth_frame = fs.get_depth_frame();
            Mat ir(cv::Size(IR_WIDTH, IR_HEIGHT), CV_8UC1, (void*)ir_frame.get_data(), Mat::AUTO_STEP);
            mIrTex = gl::Texture2d::create((void*)ir.data, GL_RED, IR_WIDTH, IR_HEIGHT);
            
            Mat filtered;
            blur(ir, ir, cv::Size(11, 11), cv::Point(0, 0));
            inRange(ir, cv::Scalar(mLowerBound, 0,0), cv::Scalar(mUpperBound,0,0), filtered);
            mFilteredTex = gl::Texture2d::create((void*)filtered.data, GL_RED, IR_WIDTH, IR_HEIGHT);
            
            vector<vector<cv::Point> > contours;
            vector<Vec4i> hierarchy;
            findContours( filtered, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE );
            
            if(mDrawIrBlob){
                Mat drawing = Mat::zeros( ir.size(), CV_8UC3 );
                RNG rng(12345);
                //            console() << contours.size() << endl;
                for( size_t i = 0; i< contours.size(); i++ )
                {
                    Scalar color = Scalar( rng.uniform(0, 256), rng.uniform(0,256), rng.uniform(0,256) );
                    drawContours( drawing, contours, (int)i, color, 2, LINE_8, hierarchy, 0 );
                }
                mBlobTex = gl::Texture2d::create((void*)drawing.data, GL_RGB, IR_WIDTH, IR_HEIGHT);
            }
            
            if(contours.size() > 0){
                vector<cv::Rect> boxes;
                vector<cv::Rect> seats;
                for( size_t i = 0; i< contours.size(); i++ )
                {
                    boxes.push_back(boundingRect( contours[i] ));
                    seats.push_back(boundingRect( contours[i] ));
                }
                mSitA = 0.0f;
                mSitB = 0.0f;
                if(mFirstTrack && seats.size() == 2){
                    for( size_t i = 0; i< seats.size(); i++ )
                    {
                        mTracking[i] = vec2(1.0f - (seats[i].y+seats[i].height/2) / 270.0f, (seats[i].x+seats[i].width/2) / 480.0f);
                    }
                    mFirstTrack = false;
                }
                if(seats.size() >= 2){
                    for( size_t i = 0; i< 2; i++ )
                    {
                        float shortestDist = 1.0f;
                        vec2 chairPos = mTracking[i];
                        for( size_t j = 0; j< seats.size(); j++ )
                        {
                            vec2 newPos = vec2(1.0f - (seats[j].y+seats[j].height/2) / 270.0f, (seats[j].x+seats[j].width/2) / 480.0f);
                            float dist = distance(newPos, mTracking[i]);
                            if(dist < shortestDist){
                                chairPos = newPos;
                                shortestDist = dist;
                            }
                        }
                        mTracking[i] = chairPos;
                    }
                }
                if(seats.size() > 2){
//                    console() << "here?" << endl;
                    vec2 extraPos;
                    for( size_t j = 0; j< seats.size(); j++ )
                    {
                        vec2 pos = vec2(1.0f - (seats[j].y+seats[j].height/2) / 270.0f, (seats[j].x+seats[j].width/2) / 480.0f);
                        if(distance(pos, mTracking[0]) != 0.0f && distance(pos, mTracking[1]) != 0.0f){
                            extraPos = pos;
                        }
                    }
                    float shortestDist = 1.0f;
                    int sittenChair;
                    for( size_t i = 0; i< 2; i++ )
                    {
                        float dist = distance(extraPos, mTracking[i]);
                        if(dist < shortestDist){
                            sittenChair = i;
                            shortestDist = dist;
                        }
                    }
//                    console() << sittenChair << endl;
                    if(sittenChair == 0){
                        mSitA = 1.0f;
                    }else{
                        mSitB = 1.0f;
                    }
                }else if(seats.size() == 1){
                    vec2 remainPos = vec2(1.0f - (seats[0].y+seats[0].height/2) / 270.0f, (seats[0].x+seats[0].width/2) / 480.0f);
                    float shortestDist = 1.0f;
                    int notSittenChair;
                    for( size_t i = 0; i< 2; i++ )
                    {
                        float dist = distance(remainPos, mTracking[i]);
                        if(dist < shortestDist){
                            notSittenChair = i;
                            shortestDist = dist;
                        }
                    }
                    mTracking[notSittenChair] = remainPos;
                    if(notSittenChair == 0){
                        mSitB = 1.0f;
                    }else{
                        mSitA = 1.0f;
                    }
                }
            }else{
                mSitA = 1.0f;
                mSitB = 1.0f;
            }
            uint8_t byteSent = 0;
            if(mSitA) byteSent |= 0b00000001;
            if(mSitB) byteSent |= 0b00000010;
            mSerial->writeByte(byteSent);
            mChairA = vec2(1.0f - mTracking[0].y, mTracking[0].x);
            mLastChairA = mChairA;
            mChairB = vec2(1.0f - mTracking[1].y, mTracking[1].x);
            mLastChairB = mChairB;
            mPUpdateGlsl->uniform( "SitA",  mSitA);
            mPUpdateGlsl->uniform( "SitB",  mSitB);
        }
    }
    // -------------------------------- Scene
    
    vec2 fishPos[FISH_NUM];
    vec2 chairA = mChairA - vec2(0.5) + vec2(0.0, -0.025);
    vec2 chairB = mChairB - vec2(0.5) + vec2(0.0, -0.025);
    float time = getElapsedFrames() / 60.0f ;
    // update fishes
    for(int i = 0; i < FISH_NUM; i++){
        //        vec2 seek = mP[i].seek(mouse - vec2(0.5));
        vec2 bound = mP[i].boundary(0.45f, 0.45f);
        mP[i].applyForce(bound * 2.0f);
        //                 ali, sep, coh, aliDist, sepDist, cohDist
        mP[i].flocking(mP, mAlignment, mSeparation, mCohesion, mAlignmentDist, mSeparationDist, mCohesionDist);
        mP[i].checkGrass(chairA, mSitA, mClickA, time);
        mP[i].checkGrass(chairB, mSitB, mClickB, time);
        mP[i].update();
        fishPos[i] = mP[i].getPosition();
    }
    
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
    
    mPUpdateGlsl->uniform( "Time", time);
    
    //    console() << mouse << endl;
    
    mPUpdateGlsl->uniform( "ChairA",  mChairA);
    mPUpdateGlsl->uniform( "ChairB",  mChairB);
    
    mPUpdateGlsl->uniform( "FishPos",  (vec2*)fishPos, 50);
    //    mTester = mPerlin.dnoise( 0.0 , getElapsedSeconds()*0.3f)* 0.2f + vec2(0.5f);
    //    mPUpdateGlsl->uniform( "Tester",  mTester);
    // Opposite TransformFeedbackObj to catch the calculated values
    // In the opposite buffer
    mPFeedbackObj[1-mDrawBuff]->bind();
    
    // We begin Transform Feedback, using the same primitive that
    // we're "drawing". Using points for the particle system.
    gl::beginTransformFeedback( GL_POINTS );
    gl::drawArrays( GL_POINTS, 0, numGrass );
    gl::endTransformFeedback();
}

void ImmersiveXsectionApp::renderScene(){
    float t = getElapsedSeconds();
    gl::setMatrices( mCam );
    gl::pushMatrices();
    gl::rotate(M_PI/2, vec3(1.0,0.0,0.0));
    gl::pushMatrices();
    gl::translate(-mWidth/2, -mHeight/2);
    if( mCausticGlsl ) {
        gl::ScopedGlslProg    shader( mCausticGlsl );
        mCausticGlsl->uniform( "Time", t);
        gl::drawSolidRect( Rectf(0.0, 0.0, mWidth, mHeight) );
    }
    gl::popMatrices();
    for(int i = 0; i < FISH_NUM; i++){
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
        mPRenderGlsl->uniform( "Time", t);
        gl::setDefaultShaderVars();
        gl::drawArrays( GL_POINTS, 0, numGrass );
    }
}

void ImmersiveXsectionApp::renderFish(){
    gl::setMatrices( mCam );
    gl::pushMatrices();
    //    gl::color(1.0, 0.5, 0.2);
    gl::rotate(M_PI/2, vec3(1.0,0.0,0.0));
    //    gl::drawSolidCircle((mTester - vec2(0.5)) * vec2(mWidth, mHeight), 0.5f, 30);
    for(int i = 0; i < FISH_NUM; i++){
        mP[i].draw(mWidth, mHeight);
    }
    gl::popMatrices();
}

void ImmersiveXsectionApp::draw()
{
    // clear out the window with black
    gl::clear( Color( 0.04, 0.05, 0.16 ) );
    
    {
        gl::ScopedFramebuffer fbo( mFboScene );
        gl::ScopedViewport    viewport( 0, 0, mFboScene->getWidth(), mFboScene->getHeight() );
        gl::setMatricesWindow( SCENE_SIZE, SCENE_SIZE );
        gl::clear( Color::black() );
        
        renderScene();
    }
    // bind the blur shader
    {
        gl::ScopedGlslProg shader( mShaderBlur );
        mShaderBlur->uniform( "tex0", 0 ); // use texture unit 0
        
        // tell the shader to blur horizontally and the size of 1 pixel
        mShaderBlur->uniform( "sample_offset", vec2( 1.0f / mFboBlur1->getWidth(), 0.0f ) );
        mShaderBlur->uniform( "attenuation", mBloomEffect );
        
        // copy a horizontally blurred version of our scene into the first blur Fbo
        {
            gl::ScopedFramebuffer fbo( mFboBlur1 );
            gl::ScopedViewport    viewport( 0, 0, mFboBlur1->getWidth(), mFboBlur1->getHeight() );
            
            gl::ScopedTextureBind tex0( mFboScene->getColorTexture(), (uint8_t)0 );
            
            gl::setMatricesWindow( BLUR_SIZE, BLUR_SIZE );
            gl::clear( Color::black() );
            
            gl::drawSolidRect( mFboBlur1->getBounds() );
        }
        
        // tell the shader to blur vertically and the size of 1 pixel
        mShaderBlur->uniform( "sample_offset", vec2( 0.0f, 1.0f / mFboBlur2->getHeight() ) );
        mShaderBlur->uniform( "attenuation", mBloomEffect );
        
        // copy a vertically blurred version of our blurred scene into the second blur Fbo
        {
            gl::ScopedFramebuffer fbo( mFboBlur2 );
            gl::ScopedViewport    viewport( 0, 0, mFboBlur2->getWidth(), mFboBlur2->getHeight() );
            
            gl::ScopedTextureBind tex0( mFboBlur1->getColorTexture(), (uint8_t)0 );
            
            gl::setMatricesWindow( BLUR_SIZE, BLUR_SIZE );
            gl::clear( Color::black() );
            
            gl::drawSolidRect( mFboBlur2->getBounds() );
        }
    }
    //    {
    //        gl::ScopedFramebuffer fbo( mFboScene );
    //        gl::ScopedViewport    viewport( 0, 0, mFboScene->getWidth(), mFboScene->getHeight() );
    //        gl::setMatricesWindow( SCENE_SIZE, SCENE_SIZE );
    //
    //        renderFish();
    //    }
    gl::pushMatrices();
    gl::setMatricesWindow( 800, 800 );
    
    gl::draw( mFboScene->getColorTexture(), Rectf( 0, 0, 800, 800 ) );
    gl::enableAdditiveBlending();
    gl::draw( mFboBlur2->getColorTexture(), Rectf( 0, 0, 800, 800 ) );
    gl::disableAlphaBlending();
    
    if (mDrawIr && mIrTex) gl::draw(mIrTex, Rectf( 0, 0, 400, 400 ) );
    if (mDrawFilteredIr && mFilteredTex) gl::draw(mFilteredTex, Rectf( 400, 0, 800, 400 ) );
    if (mDrawIrBlob && mBlobTex) gl::draw(mBlobTex, Rectf( 0, 400, 400, 800 ) );
    gl::popMatrices();
    mParams->draw();
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
    mPUpdateGlsl->uniform( "ClickA",  mClickA);
    mPUpdateGlsl->uniform( "SitA",  mSitA);
    
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
    
    try {
        ci::gl::GlslProg::Format mCausticGlslFormat;
        // This being the render glsl, we provide a fragment shader.
        mCausticGlslFormat.vertex( loadAsset( "caustic.vert" ) )
        .fragment( loadAsset( "caustic.frag" ) );
        
        mCausticGlsl = ci::gl::GlslProg::create( mCausticGlslFormat );
    }
    catch( const ci::gl::GlslProgCompileExc &ex ) {
        console() << "CAUSTIC GLSL ERROR: " << ex.what() << std::endl;
    }
    
    try {
        mShaderBlur = gl::GlslProg::create( loadAsset( "blur.vert" ), loadAsset( "blur.frag" ) );
    }
    catch( const ci::gl::GlslProgCompileExc &ex ) {
        console() << "CAUSTIC GLSL ERROR: " << ex.what() << std::endl;
    }
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

CINDER_APP( ImmersiveXsectionApp, RendererGl(RendererGl::Options().msaa(4)), &ImmersiveXsectionApp::prepare )
