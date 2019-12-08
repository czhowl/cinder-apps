#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#import <Cocoa/Cocoa.h>

#include "cinder/app/cocoa/AppCocoaView.h"
#include "cinder/app/cocoa/CinderViewMac.h"
#include "cinder/app/cocoa/PlatformCocoa.h"
#include "cinder/Log.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class TransparentWindowApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
};

void TransparentWindowApp::setup()
{
    NSView *cinderView = reinterpret_cast<NSView *>( getWindow()->getNative() );
    NSWindow *parentWindow = [cinderView window];
//    [parentWindow setOpaque:NO];
//
//    NSColor *backgroundColor = [parentWindow backgroundColor];
//    backgroundColor = [backgroundColor colorWithAlphaComponent:0.0];
    [parentWindow setOpaque:NO];
//    [parentWindow setAlphaValue:0.9];
    [cinderView setWantsLayer: YES];
    [cinderView.layer setBackgroundColor: [NSColor whiteColor].CGColor];
    [parentWindow setBackgroundColor:[NSColor whiteColor]];
//    NSArray *subviews = [parentWindow subviews];
//    NSView *view = [[NSView alloc] initWithFrame:NSMakeRect(100, 100, 100, 100)];
//    [view setWantsLayer:YES];
//    view.layer.backgroundColor = [[NSColor whiteColor] CGColor];
//
//    [cinderView addSubview:view];
    NSArray *subviews = [cinderView subviews];
    for (NSView *view in subviews) {
        [view setWantsLayer: YES];
        [view.layer setBackgroundColor: [NSColor whiteColor].CGColor];
    }
    NSLog(@"subviews are %@", [cinderView subviews]);
    
    
}

void TransparentWindowApp::mouseDown( MouseEvent event )
{
}

void TransparentWindowApp::update()
{
}

void TransparentWindowApp::draw()
{
	gl::clear( ColorAf( 0.0f, 0.0f, 0.0f, 0.0f ) );
    gl::color(ColorAf( 1.0f, 0.0f, 0.0f, 1.0f ));
    gl::drawSolidCircle(vec2(100, 100), 10, 30);
    
    

}

CINDER_APP( TransparentWindowApp, RendererGl )
