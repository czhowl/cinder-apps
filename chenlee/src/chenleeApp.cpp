#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/CameraUi.h"
#include "cinder/Vector.h"
#include "cinder/params/Params.h"

#include <librealsense2/rs.hpp>

#include "Particle.h"

using namespace ci;
using namespace ci::app;
using namespace std;

#define STREAM          RS2_STREAM_DEPTH  // rs2_stream is a types of data provided by RealSense device           //
#define FORMAT          RS2_FORMAT_Z16    // rs2_format is identifies how binary data is encoded within a frame   //
#define WIDTH           640               // Defines the number of columns for each frame                         //
#define HEIGHT          480               // Defines the number of lines for each frame                           //
#define FPS             30                // Defines the rate of frames per second                                //
#define STREAM_INDEX    0                 // Defines the stream index, used for multiple streams of the same type //

class chenleeApp : public App {
  public:
	void    setup() override;
	void    mouseDown( MouseEvent event ) override;
	void    update() override;
	void    draw() override;
    void    keyDown( KeyEvent event ) override;
    void    render();
    
private:
    Particle           p;
    CameraPersp        mCamera;
    CameraUi           mCamUi;
    vec3              mEye, mCenter, mUp;
    params::InterfaceGlRef    mParams;
    
    gl::FboRef         mFbo;
    Font               mFont;
    Font               mFont1;
    
    float              r;
    
    
    rs2::pipeline       pipe;
    gl::TextureRef   mTexture;
    rs2::colorizer color_map;
    Surface         mSurface;
    // =================================
//    rs2_context* ctx;
//    rs2_config* config;
//    rs2_error* e = 0;
//    rs2_device_list* device_list;
//    rs2_device* dev;
//    rs2_pipeline_profile* pipeline_profile;
//    rs2_pipeline* pipeline;
//    float dist_to_center;
    
    // Declare depth colorizer for pretty visualization of depth data
    
    // Declare RealSense pipeline, encapsulating the actual device and sensors
};

// #################################################################################

void chenleeApp::setup()
{
    pipe.start();
    color_map = rs2::colorizer(4);
}

// =============================================================================

void chenleeApp::mouseDown( MouseEvent event )
{
}

// #################################################################################

void chenleeApp::keyDown( KeyEvent event )
{
    
//    if( event.getChar() == 'c' ){
//        gl::ScopedFramebuffer scpFbo( mFbo );
//        gl::ScopedViewport    scpViewport( mFbo->getSize() );
//        gl::clear();
//    }
//    if( event.getChar() == 'q' ){
//        if(r < 5) r += 0.01f;
//    }
//    if( event.getChar() == 'a' ){
//        if(r > 0) r -= 0.01f;
//    }
}

// #################################################################################

void chenleeApp::update()
{
    rs2::frameset data = pipe.wait_for_frames();
    
    rs2::frame      depth = data.get_depth_frame().apply_filter(color_map);
    rs2::video_frame      color = data.get_color_frame();
    const int w = depth.as<rs2::video_frame>().get_width();
    const int h = depth.as<rs2::video_frame>().get_height();
    const int wc = color.get_width();
    const int hc = color.get_height();
    mTexture = gl::Texture2d::create((void *)color.get_data(), GL_RGB, wc, hc);
    int center_pos = (wc * hc / 2 + wc / 2) * 3;
    const uint8_t r = ((char*)color.get_data())[center_pos];
    const uint8_t g = ((char*)color.get_data())[center_pos + 1];
    const uint8_t b = ((char*)color.get_data())[center_pos + 2];
    cout << (int)r << "  " << (int)g << "  " << (int)b << endl;
    //mSurface
//    // This call waits until a new composite_frame is available
//    // composite_frame holds a set of frames. It is used to prevent frame drops
//    // The retunred object should be released with rs2_release_frame(...)
//    rs2_frame* frames = rs2_pipeline_wait_for_frames(pipeline, 5000, &e);
//    check_error(e);
//
//    // Returns the number of frames embedded within the composite frame
//    int num_of_frames = rs2_embedded_frames_count(frames, &e);
//    check_error(e);
//
//    int i;
//    for (i = 0; i < num_of_frames; ++i)
//    {
//        // The retunred object should be released with rs2_release_frame(...)
//        rs2_frame* frame = rs2_extract_frame(frames, i, &e);
//        check_error(e);
//
//        // Check if the given frame can be extended to depth frame interface
//        // Accept only depth frames and skip other frames
//        if (0 == rs2_is_frame_extendable_to(frame, RS2_EXTENSION_DEPTH_FRAME, &e))
//            continue;
//
//        // Get the depth frame's dimensions
//        int width = rs2_get_frame_width(frame, &e);
//        check_error(e);
//        int height = rs2_get_frame_height(frame, &e);
//        check_error(e);
//
//        // Query the distance from the camera to the object in the center of the image
//        dist_to_center = rs2_depth_frame_get_distance(frame, width / 2, height / 2, &e);
//        check_error(e);
//
//        // Print the distance
////        printf("The camera is facing an object %.3f meters away.\n", dist_to_center);
//
//        rs2_release_frame(frame);
//    }
//
//    rs2_release_frame(frames);
    
    // ==============================================================================
    
//    p.setRandomness(r);
}

// #################################################################################

void chenleeApp::draw()
{
    gl::clear();
    gl::pushMatrices();
    //gl::translate(0, getWindowHeight());
    //gl::scale(1, -1);
    gl::draw(mTexture);
    gl::popMatrices();
//    render();
//    // Now render the scene to the main buffer.
//    gl::draw( mFbo->getColorTexture() );
//    gl::drawString(to_string( App::get()->getAverageFps() ), glm::vec2(10.0f, 10.0f), Color::white(), mFont1);
}

// #################################################################################

void chenleeApp::render()
{
    // Bind the Fbo and update the viewport.
    gl::ScopedFramebuffer scpFbo( mFbo );
    
    gl::pushMatrices();
    gl::setMatricesWindow( mFbo->getSize() );
    // Enable pre-multiplied alpha blending.
    gl::ScopedBlendPremult scpBlend;
    // Draw transparent black rectangle.
    gl::ScopedColor scpColor( 0, 0, 0, 50.0f / 255.0f );
//    gl::ScopedColor scpColor( 0, 0, 0, 1.0f );
    
    gl::drawSolidRect( { 0, 0, (float)getWindowWidth(), (float)getWindowHeight() } );
    
    gl::popMatrices();
    
    gl::pushMatrices();
    gl::setMatrices( mCamera );
    // Draw a white circle.
    

    gl::color(Color(1,1,1));
    p.draw();
//    gl::drawSphere(vec3(0,0,0), 1);
    //    for(int i = 0; i < 4; i++){
    ////        cinder::gl::color(color / i);
    //        cinder::gl::drawSphere(p.poss[i], 1);
    //        console() << p.poss[i] << endl;
    //    }
    //    console() << "====================" << endl;
    gl::popMatrices();
}

// #################################################################################

CINDER_APP( chenleeApp, RendererGl, [] ( App::Settings *settings ) {
    settings->setWindowSize( 1280, 720 );
//    settings->setFullScreen();
//    settings->setHighDensityDisplayEnabled( true );
//    settings->setMultiTouchEnabled( false );
} )
