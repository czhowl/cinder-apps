#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/params/Params.h"

#include <cinder/Perlin.h>
#include <cinder/Rand.h>
#include "cinder/app/App.h"
#include "Particle.h"
#include "Key.h"

using namespace ci;
using namespace ci::app;
using namespace std;

typedef vector<Particle>::iterator PIt;

class flockingkeyApp : public App {
public:
    void setup() override;
    void mouseDown( MouseEvent event ) override;
    void update() override;
    void draw() override;
    void keyDown( KeyEvent event ) override;
    
private:
    Rectf nest;
    Key flock[33];
    params::InterfaceGlRef mParams;
    
    float mZoneRadius;
    float mLowerThreshold, mHigherThreshold;
    float mAttractStrength, mRepelStrength, mAlignStrength, mCenterPullStrength, mKeyDis, mPulse;
    bool mCentralGravity;
    
    gl::Texture2dRef mNests[5];
    gl::Texture2dRef mSpirits[4];
    int xOrigin = -500;
};

void flockingkeyApp::setup()
{
//    getWindow()->setAlwaysOnTop();
    mZoneRadius = 50.0f;
    mLowerThreshold = 0.4f;
    mHigherThreshold = 0.75f;
    mAttractStrength = 0.005f;
    mRepelStrength = 0.01f;
    mAlignStrength = 0.01f;
    mKeyDis = 34.0f;
    mCenterPullStrength = 0.01f;
    
//    mParams = params::InterfaceGl::create("Flocking", vec2(250, 350));
//    mParams->addParam("Zone Radius", &mZoneRadius, "min=0.0 max=100.0, step=2.5 keyIncr=z keyDecr=Z");
//    mParams->addParam("Lower Threshold", &mLowerThreshold, "min=0.025 max=1.0, step=0.025 keyIncr=l keyDecr=L");
//    mParams->addParam("Higher Threshold", &mHigherThreshold, "min=0.025 max=1.0, step=0.025 keyIncr=h keyDecr=H");
//    mParams->addSeparator();
//    mParams->addParam("Attract Strength", &mAttractStrength, "min=0.001 max=0.1 step=0.001 keyIncr=a keyDecr=A");
//    mParams->addParam("Repel Strength", &mRepelStrength, "min=0.001 max=0.1 step=0.001 keyIncr=r keyDecr=R");
//    mParams->addParam("Orient Strength", &mAlignStrength, "min=0.001 max=0.1 step=0.001 keyIncr=o keyDecr=O");
//    mParams->addParam("center raduis", &mKeyDis, "min=10.0 max=200.0 step=1.0 keyIncr=x keyDecr=X");
    
    mNests[0] = gl::Texture2d::create(loadImage(loadAsset("nest0.png")));
    mNests[1] = gl::Texture2d::create(loadImage(loadAsset("nest1.png")));
    mNests[2] = gl::Texture2d::create(loadImage(loadAsset("nest2.png")));
    mNests[3] = gl::Texture2d::create(loadImage(loadAsset("nest3.png")));
    mNests[4] = gl::Texture2d::create(loadImage(loadAsset("nest4.png")));
    
    mSpirits[0] = gl::Texture2d::create(loadImage(loadAsset("spirit0.png")));
    mSpirits[1] = gl::Texture2d::create(loadImage(loadAsset("spirit1.png")));
    mSpirits[2] = gl::Texture2d::create(loadImage(loadAsset("spirit2.png")));
    mSpirits[3] = gl::Texture2d::create(loadImage(loadAsset("spirit3.png")));
    
    for(int i = 0; i< 12; i ++){
        flock[i].addParticles(Rand::randInt(0, 2), mSpirits, vec2(xOrigin+i*68.5f, -68.5f));
    }
    for(int i = 12; i< 23; i ++){
        flock[i].addParticles(Rand::randInt(0, 2), mSpirits, vec2(xOrigin+15+(i-12)*68.5f, 0.0f));
    }
    for(int i = 23; i< 33; i ++){
        flock[i].addParticles(Rand::randInt(0, 2), mSpirits, vec2(xOrigin+52+(i-23)*68.5f, 68.5f));
    }
}

void flockingkeyApp::mouseDown( MouseEvent event )
{
}

void flockingkeyApp::update()
{
//    for(int i = 0; i< 12; i ++){
//        if(flock[i].pulse == 500.0f)
//        flock[i].addParticles(1, mSpirits, vec2(xOrigin+i*110.0f, -110.0f));
//    }
//    for(int i = 12; i< 23; i ++){
//        if(flock[i].pulse == 500.0f)
//        flock[i].addParticles(1, mSpirits, vec2(xOrigin+50+(i-12)*110.0f, 0.0f));
//    }
//    for(int i = 23; i< 33; i ++){
//        if(flock[i].pulse == 500.0f)
//        flock[i].addParticles(1, mSpirits, vec2(xOrigin+100+(i-23)*110.0f, 110.0f));
//    }
    for(int i = 0; i< 33; i ++){
        flock[i].applyFlocking(mZoneRadius, mLowerThreshold, mHigherThreshold, mAttractStrength, mRepelStrength, mAlignStrength);
        flock[i].applyCenterPull(mKeyDis, mCenterPullStrength);
        flock[i].update();
    }
}

void flockingkeyApp::draw()
{
    gl::clear( Color( 0, 0, 0 ) );
    
    gl::pushMatrices();
    gl::translate(vec2(getWindowWidth()/2, getWindowHeight()/2));
    for(int i = 0; i< 33; i ++){
        gl::ScopedModelMatrix smm;
        gl::translate(flock[i].nestPos);
        int k = flock[i].keyShape;
        console()<<k<<endl;
        gl::draw(mNests[k], Rectf(-35,-35,35,35));
        flock[i].draw();
    }
    gl::popMatrices();
//    mParams->draw();
}

void flockingkeyApp::keyDown( KeyEvent event )
{
    switch( event.getCode() ) {
        case KeyEvent::KEY_q:
            flock[0].addParticles(1, mSpirits, vec2(xOrigin+0*68.5f, -68.5f));
            if(flock[0].pulse < 1.0f)
            flock[0].applyPulse(500.0f);
            break;
        case KeyEvent::KEY_w:
            flock[1].addParticles(1, mSpirits, vec2(xOrigin+1*68.5f, -68.5f));
            if(flock[1].pulse < 1.0f)
            flock[1].applyPulse(500.0f);
            break;
        case KeyEvent::KEY_e:
            flock[2].addParticles(1, mSpirits, vec2(xOrigin+2*68.5f, -68.5f));
            if(flock[2].pulse < 1.0f)
            flock[2].applyPulse(500.0f);
            break;
        case KeyEvent::KEY_r:
            flock[3].addParticles(1, mSpirits, vec2(xOrigin+3*68.5f, -68.5f));
            if(flock[3].pulse < 1.0f)
            flock[3].applyPulse(500.0f);
            break;
        case KeyEvent::KEY_t:
            flock[4].addParticles(1, mSpirits, vec2(xOrigin+4*68.5f, -68.5f));
            if(flock[4].pulse < 1.0f)
            flock[4].applyPulse(500.0f);
            break;
        case KeyEvent::KEY_y:
            flock[5].addParticles(1, mSpirits, vec2(xOrigin+5*68.5f, -68.5f));
            if(flock[5].pulse < 1.0f)
            flock[5].applyPulse(500.0f);
            break;
        case KeyEvent::KEY_u:
            flock[6].addParticles(1, mSpirits, vec2(xOrigin+6*68.5f, -68.5f));
            if(flock[6].pulse < 1.0f)
            flock[6].applyPulse(500.0f);
            break;
        case KeyEvent::KEY_i:
            flock[7].addParticles(1, mSpirits, vec2(xOrigin+7*68.5f, -68.5f));
            if(flock[7].pulse < 1.0f)
            flock[7].applyPulse(500.0f);
            break;
        case KeyEvent::KEY_o:
            flock[8].addParticles(1, mSpirits, vec2(xOrigin+8*68.5f, -68.5f));
            if(flock[8].pulse < 1.0f)
            flock[8].applyPulse(500.0f);
            break;
        case KeyEvent::KEY_p:
            flock[9].addParticles(1, mSpirits, vec2(xOrigin+9*68.5f, -68.5f));
            if(flock[9].pulse < 1.0f)
            flock[9].applyPulse(500.0f);
            break;
        case KeyEvent::KEY_LEFTBRACKET:
            flock[10].addParticles(1, mSpirits, vec2(xOrigin+10*68.5f, -68.5f));
            if(flock[10].pulse < 1.0f)
            flock[10].applyPulse(500.0f);
            break;
        case KeyEvent::KEY_RIGHTBRACKET:
            flock[11].addParticles(1, mSpirits, vec2(xOrigin+11*68.5f, -68.5f));
            if(flock[11].pulse < 1.0f)
            flock[11].applyPulse(500.0f);
            break;
        case KeyEvent::KEY_a:
            flock[12].addParticles(1, mSpirits, vec2(xOrigin+15+(12-12)*68.5f, 0.0f));
            if(flock[12].pulse < 1.0f)
            flock[12].applyPulse(500.0f);
            break;
        case KeyEvent::KEY_s:
            flock[13].addParticles(1, mSpirits, vec2(xOrigin+15+(13-12)*68.5f, 0.0f));
            if(flock[13].pulse < 1.0f)
            flock[13].applyPulse(500.0f);
            break;
        case KeyEvent::KEY_d:
            flock[14].addParticles(1, mSpirits, vec2(xOrigin+15+(14-12)*68.5f, 0.0f));
            if(flock[14].pulse < 1.0f)
            flock[14].applyPulse(500.0f);
            break;
        case KeyEvent::KEY_f:
            flock[15].addParticles(1, mSpirits, vec2(xOrigin+15+(15-12)*68.5f, 0.0f));
            if(flock[15].pulse < 1.0f)
            flock[15].applyPulse(500.0f);
            break;
        case KeyEvent::KEY_g:
            flock[16].addParticles(1, mSpirits, vec2(xOrigin+15+(16-12)*68.5f, 0.0f));
            if(flock[16].pulse < 1.0f)
            flock[16].applyPulse(500.0f);
            break;
        case KeyEvent::KEY_h:
            flock[17].addParticles(1, mSpirits, vec2(xOrigin+15+(17-12)*68.5f, 0.0f));
            if(flock[17].pulse < 1.0f)
            flock[17].applyPulse(500.0f);
            break;
        case KeyEvent::KEY_j:
            flock[18].addParticles(1, mSpirits, vec2(xOrigin+15+(18-12)*68.5f, 0.0f));
            if(flock[18].pulse < 1.0f)
            flock[18].applyPulse(500.0f);
            break;
        case KeyEvent::KEY_k:
            flock[19].addParticles(1, mSpirits, vec2(xOrigin+15+(19-12)*68.5f, 0.0f));
            if(flock[19].pulse < 1.0f)
            flock[19].applyPulse(500.0f);
            break;
        case KeyEvent::KEY_l:
            flock[20].addParticles(1, mSpirits, vec2(xOrigin+15+(20-12)*68.5f, 0.0f));
            if(flock[20].pulse < 1.0f)
            flock[20].applyPulse(500.0f);
            break;
        case KeyEvent::KEY_SEMICOLON:
            flock[21].addParticles(1, mSpirits, vec2(xOrigin+15+(21-12)*68.5f, 0.0f));
            if(flock[21].pulse < 1.0f)
            flock[21].applyPulse(500.0f);
            break;
        case KeyEvent::KEY_QUOTE:
            flock[22].addParticles(1, mSpirits, vec2(xOrigin+15+(22-12)*68.5f, 0.0f));
            if(flock[22].pulse < 1.0f)
            flock[22].applyPulse(500.0f);
            break;
        case KeyEvent::KEY_z:
            flock[23].addParticles(1, mSpirits, vec2(xOrigin+52+(23-23)*68.5f, 68.5f));
            if(flock[23].pulse < 1.0f)
            flock[23].applyPulse(500.0f);
            break;
        case KeyEvent::KEY_x:
            flock[24].addParticles(1, mSpirits, vec2(xOrigin+52+(24-23)*68.5f, 68.5f));
            if(flock[24].pulse < 1.0f)
            flock[24].applyPulse(500.0f);
            break;
        case KeyEvent::KEY_c:
            flock[25].addParticles(1, mSpirits, vec2(xOrigin+52+(25-23)*68.5f, 68.5f));
            if(flock[25].pulse < 1.0f)
            flock[25].applyPulse(500.0f);
            break;
        case KeyEvent::KEY_v:
            flock[26].addParticles(1, mSpirits, vec2(xOrigin+52+(26-23)*68.5f, 68.5f));
            if(flock[26].pulse < 1.0f)
            flock[26].applyPulse(500.0f);
            break;
        case KeyEvent::KEY_b:
            flock[27].addParticles(1, mSpirits, vec2(xOrigin+52+(27-23)*68.5f, 68.5f));
            if(flock[27].pulse < 1.0f)
            flock[27].applyPulse(500.0f);
            break;
        case KeyEvent::KEY_n:
            flock[28].addParticles(1, mSpirits, vec2(xOrigin+52+(28-23)*68.5f, 68.5f));
            if(flock[28].pulse < 1.0f)
            flock[28].applyPulse(500.0f);
            break;
        case KeyEvent::KEY_m:
            flock[29].addParticles(1, mSpirits, vec2(xOrigin+52+(29-23)*68.5f, 68.5f));
            if(flock[29].pulse < 1.0f)
            flock[29].applyPulse(500.0f);
            break;
        case KeyEvent::KEY_COMMA:
            flock[30].addParticles(1, mSpirits, vec2(xOrigin+52+(30-23)*68.5f, 68.5f));
            if(flock[30].pulse < 1.0f)
            flock[30].applyPulse(500.0f);
            break;
        case KeyEvent::KEY_PERIOD:
            flock[31].addParticles(1, mSpirits, vec2(xOrigin+52+(31-23)*68.5f, 68.5f));
            if(flock[31].pulse < 1.0f)
            flock[31].applyPulse(500.0f);
            break;
        case KeyEvent::KEY_SLASH:
            flock[32].addParticles(1, mSpirits, vec2(xOrigin+52+(32-23)*68.5f, 68.5f));
            if(flock[32].pulse < 1.0f)
            flock[32].applyPulse(500.0f);
            break;
    }
}

CINDER_APP( flockingkeyApp, RendererGl(RendererGl::Options().msaa(16)),
           [&](App::Settings *settings){
           settings->setWindowSize(1600, 720);
           settings->setFrameRate(60.0f);
           } )
