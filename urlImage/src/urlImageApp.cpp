#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Thread.h"

#include "Wechat.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class urlImageApp : public App {
  public:
    ~urlImageApp();
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
    
  private:
    WechatRef                 mWechat;
    shared_ptr<thread>        mThread;
};

urlImageApp::~urlImageApp()
{
    mThread->join();
}

void urlImageApp::setup()
{
    mWechat = Wechat::create("https://pbs.twimg.com/profile_images/742658185102807040/o4gJSvNZ_400x400.jpg");
    
    gl::ContextRef backgroundCtx = gl::Context::create( gl::context() );
    mThread = shared_ptr<thread>( new thread( bind( &Wechat::downloadImageThread, mWechat, backgroundCtx ) ));
}

void urlImageApp::mouseDown( MouseEvent event )
{
}

void urlImageApp::update()
{
}

void urlImageApp::draw()
{
	gl::clear( Color( 0.2, 0, 0 ) );
    mWechat->draw(vec2(100,0));
//    console() << mWechat->getUrl() << endl;
}

CINDER_APP( urlImageApp, RendererGl )
