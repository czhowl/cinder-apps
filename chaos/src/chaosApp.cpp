#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/CameraUi.h"
#include "cinder/Vector.h"
#include "cinder/params/Params.h"
#include "cinder/Perlin.h"

#include "Particle.h"

using namespace ci;
using namespace ci::app;
using namespace std;

const float textUp[4] = {1.0f, -1.0f, 1.0f, -1.0f};
const float textRight[4] = {1.0f, 1.0f, -1.0f, -1.0f};

class ChaosApp : public App {
public:
    void    setup() override;
    void    mouseDown(MouseEvent event) override;
    void    mouseDrag(MouseEvent event) override;
    void    mouseWheel(MouseEvent event) override;
    void    update() override;
    void    draw() override;
    void    keyDown(KeyEvent event) override;
    void    render();
    void    renderUI();
    
private:
    Particle                 p;
    CameraPersp                 mCamera;
    CameraUi                 mCamUi;
    vec3                     mEye, mCenter, mUp;
    params::InterfaceGlRef     mParams;
    
    gl::FboRef                 mFbo;
    gl::FboRef                 mFboUI;
    Font                     mFont;
    Font                     mFontUI;
    
    float                     r;
    float                       prevR;
    float                     pan;
    double                     camAngle;
    float                     fadeRate;
    bool                       equation;
    Perlin                  mPerlin;
    // =================================
    vec3                    indicationPos;
    Rectf                   loadingBar;
    Rectf                   resultBar;
    gl::Texture2dRef        mPhiTex;
    gl::Texture2dRef        mResumeTex;
    float                   timer;
    
    bool                    showInfo;
    bool                    prevShowInfo;
    float                   alpha;
    bool                    unstable;
    bool                    blink;
    float                   blinkTimer;
};

// #################################################################################

void ChaosApp::setup()
{
    alpha = 0;
    timer = 0;
    mPerlin.setSeed(clock());
    camAngle = 0;
    equation = true;
    fadeRate = 10.0f;
    r = 2;
    prevR = r;
    pan = 0;
    blink = false;
    blinkTimer = 0;
    unstable = false;
    showInfo = false;
    prevShowInfo = showInfo;
    
    loadingBar = Rectf(0.0f,0.0f,10.0f,10.0f);
    resultBar = Rectf(0.0f,0.0f,10.0f,10.0f);
    auto img = loadImage( loadAsset( "Phi.png" ) );
    mPhiTex = gl::Texture2d::create( img );
    img = loadImage( loadAsset( "resume.png" ) );
    mResumeTex = gl::Texture2d::create( img );
    
    mParams = params::InterfaceGl::create(getWindow(), "App parameters", toPixels(ivec2(200, 200)));
    mParams->addParam( "Randomness", &r ).min( 0.1f ).max( 20.0f ).keyIncr( "q" ).keyDecr( "a" ).precision( 2 ).step( 0.01f );
    mParams->addParam( "Color", &pan ).min( -5.0f ).max( 5.0f ).precision( 2 ).step( 0.01f );
    mParams->addParam( "Equation", &equation );
    mParams->addParam( "Unstable", &unstable );
    mParams->addParam( "Show Info", &showInfo );
    mParams->addParam( "alpha", &alpha ).min( 0.0f ).max( 1.0f ).precision( 2 ).step( 0.01f );
    //mParams->addParam("fade rate", &fadeRate).min(0.0f).max(255.0f).keyIncr("w").keyDecr("s").precision(2).step(0.01f);
    mFontUI = Font(loadAsset( "nidsans.ttf" ), 20.0f);
    mFont = Font("Arial", 1.0f);
    gl::clear(ColorA(0, 0, 0));
    p.setup();
    
    const vec2 windowSize = toPixels(getWindowSize());
    mCamera = CameraPersp(windowSize.x, windowSize.y, 45.0f, 0.01f, 3000.0f);
    mCamera.lookAt(vec3(0.0f, 0.0f, 51.0f), vec3(0.0f, 0.0f, 10.0f));
    mCamUi = CameraUi(&mCamera);
    
    gl::Fbo::Format fboFormat;
    fboFormat.setColorTextureFormat(gl::Texture2d::Format().internalFormat(GL_RGBA32F));
    mFbo = gl::Fbo::create(getWindowWidth(), getWindowHeight(), fboFormat);
    mFboUI = gl::Fbo::create(getWindowWidth(), getWindowHeight(), fboFormat);
    // Clear once at setup.
    gl::ScopedFramebuffer scpFbo(mFbo);
    gl::ScopedViewport    scpViewport(mFbo->getSize());
    gl::clear();
}

// =============================================================================

void ChaosApp::mouseDown(MouseEvent event)
{
    console()<<event.getPos()<<endl;
    float x = event.getPos().x;
    float y = event.getPos().y;
    if(x < getWindowWidth() / 2 + 25 && x > getWindowWidth() / 2 - 25 && y > getWindowHeight() - 50){
        cout<< "clicked" <<endl;
    }
    mCamUi.mouseDown( event );
}

void ChaosApp::mouseDrag( MouseEvent event )
{
    mCamUi.mouseDrag( event );
}

void ChaosApp::mouseWheel( MouseEvent event )
{
    mCamUi.mouseWheel( event );
}

// #################################################################################

void ChaosApp::keyDown(KeyEvent event)
{
    if (event.getChar() == 'c') {
        gl::ScopedFramebuffer scpFbo(mFbo);
        gl::ScopedViewport    scpViewport(mFbo->getSize());
        gl::clear();
    }
    /*if (event.getChar() == 'q') {
     if (r < 5) r += 0.01f;
     }
     if (event.getChar() == 'a') {
     if (r > 0) r -= 0.01f;
     }*/
}

// #################################################################################

void ChaosApp::update()
{
   // console() << getmouse() << endl;
    timer = getElapsedSeconds();
    //float n = mPerlin.fBm(getElapsedSeconds());
    //console() << n * 20 << endl;
    camAngle += 0.5 * pow(lmap(r, 0.1f, 5.0f, 0.0f, 5.0f), 2);
    p.setRandomness(r);
    p.setColorPan(pan);
    p.setEquation(equation);
    //p.setRandomness(dist_to_center);
    p.update();
    //console() << to_string(r != 0.1f) << endl;
    if(r != 0.1f){
        mCamera.lookAt(vec3(r * 30.0f * cos(camAngle), r * 30.0f * sin(camAngle), 50.0f - r * 10), vec3(0.0f, 0.0f, 10.0f));
    }
    
    //console()<< prevR << " " << r << endl;
    if(prevR > r && r == 0.1){
        indicationPos = p.poss[randInt(constant::PARTICLE_NUM)];
        console()<< indicationPos << endl;
    }
    prevR = r;
}

// #################################################################################

void ChaosApp::draw()
{
    
    render();
    renderUI();
    // Now render the scene to the main buffer.
    //gl::scale(vec2(0.5f,0.5f));
    gl::draw(mFbo->getColorTexture());
    gl::draw(mFboUI->getColorTexture());
    gl::drawString(to_string(App::get()->getAverageFps()), glm::vec2(10.0f, 10.0f), Color::white(), mFont);
    mParams->draw();
    //console() << mCamera.getEyePoint() << endl;
}

// #################################################################################

void ChaosApp::render()
{
    // Bind the Fbo and update the viewport.
    gl::ScopedFramebuffer scpFbo(mFbo);
    
    gl::pushMatrices();
    gl::setMatricesWindow(mFbo->getSize());
    // Enable pre-multiplied alpha blending.
    gl::ScopedBlendPremult scpBlend;
    // Draw transparent black rectangle.
    gl::ScopedColor scpColor(0, 0, 0, 20.0f / 255.0f);
    //    gl::ScopedColor scpColor( 0, 0, 0, 1.0f );
    
    gl::drawSolidRect({ 0, 0, (float)getWindowWidth(), (float)getWindowHeight() });
    
    gl::popMatrices();
    

    gl::pushMatrices();
    gl::ScopedViewport scpVp( ivec2( 0 ), mFbo->getSize() );
    gl::setMatrices(mCamera);
    // Draw a white circle.
    
    gl::pushMatrices();
    gl::color(Color(1, 1, 1));
    //gl::rotate(camAngle);
    //gl::scale(vec3(clamp(lmap((float)r, 0.1f, 5.0f, 1.0f, 0.1f), 0.1f, 1.0f)));
    p.draw();
    gl::popMatrices();
    gl::popMatrices();
}

void ChaosApp::renderUI(){
    
    if(prevShowInfo != showInfo || alpha != 0.0f || alpha != 1.0f){
        if(showInfo){
            if(alpha + 0.05f < 1.0f)alpha += 0.1f;
            else{
                alpha = 1.0f;
                prevShowInfo = showInfo;
            }
        }else{
            if(alpha - 0.05f > 0.0f)alpha -= 0.1f;
            else{
                alpha = 0.0f;
                prevShowInfo = showInfo;
            }
        }
        
    }
    vec3 up, right;
    vec3 begin[4], pos[4], end[4];
    
    mCamera.getBillboardVectors(&right, &up);
    gl::ScopedFramebuffer scpFbo(mFboUI);
    gl::clear(ColorAf(1.0f, 1.0f, 1.0f, 0.0f));
    
    gl::pushMatrices();
    gl::ScopedViewport scpVp( ivec2( 0 ), mFboUI->getSize() );
    gl::enableAlphaBlending( true );
    gl::setMatrices(mCamera);
    gl::ScopedBlendAlpha();
    gl::color(Color::white());
    
    vec3 viewDirection = mCamera.getViewDirection();
    vec3 eyePoint = mCamera.getEyePoint();
    float w = getWindowWidth();
    float h = getWindowHeight();
//    vec3 viewDirection = mCamera.getViewDirection();
    
    for(int i = 0; i < 3; i++){
        pos[i] = eyePoint + 5.0f * viewDirection - right * textRight[i] * 2.0f + up * textUp[i] * 1.0f;
        vec3 endPos = pos[i] - right * textRight[i] * 0.5f;
        
        vec3 dirBegin = indicationPos - pos[i];
        vec3 dirEnd = endPos - pos[i];
        float pct = alpha * alpha * alpha;
        begin[i] = pct * length(dirBegin) * normalize(dirBegin) + pos[i];
        end[i] = pct * length(dirEnd) * normalize(dirEnd) + pos[i];
        
        //console()<< begin[i] << endl;
        glLineWidth(2.0f);
        gl::begin(GL_LINE_STRIP);
        gl::vertex(begin[i]);
        gl::vertex(pos[i]);
        gl::vertex(end[i]);
        gl::end();
    }
    
    gl::popMatrices();
    ColorAf textCol(1.0f, 1.0f, 1.0f, alpha);
    ColorAf negTextCol(1.0f, 1.0f, 1.0f, 1.0f - alpha);
    ColorAf textColB(0.0f, 0.0f, 0.0f, alpha);
    ColorAf textColRed(1.0f, 0.0f, 0.0f, alpha);
    
    gl::color(textCol);
    for(int i = 0; i < 2; i++){
     gl::drawString("AA " + to_string(i), mCamera.worldToScreen(end[i], w, h) - vec2(0, 20), textCol, mFontUI);
    }
    gl::drawString("AA " + to_string(2), mCamera.worldToScreen(end[2], w, h) - vec2(90, 20), textCol, mFontUI);
    pos[3] = eyePoint + 5.0f * viewDirection - right * textRight[3] * 2.0f + up * textUp[3] * 1.0f;
    end[3] = pos[3] - right * textRight[3] * 0.5f;
    vec2 phiPos = mCamera.worldToScreen(end[3], w, h);
    
    gl::drawString(" 3.244", phiPos - vec2(90, 20), textCol, mFontUI);
    gl::drawStringCentered("SYSTEM IDENTIFIED", mCamera.worldToScreen(eyePoint + 5.0f * viewDirection + up * 1.9f, w, h), textCol, mFontUI);
    gl::drawStringCentered("ANALYZING", mCamera.worldToScreen(eyePoint + 5.0f * viewDirection + up * 1.9f, w, h), negTextCol, mFontUI);

    gl::draw( mPhi, mCamera.worldToScreen(eyePoint + 5.0f * viewDirection - up * 1.6f, w, h);
    gl::draw( mResumeTex, Rectf( resumePos.x - 10, resumePos.y + 50, resumePos.x - 10 + mResumeTex->getWidth()/1.2, resumePos.y + 50 + mResumeTex->getHeight()/1.2 ) );
    
    if(timer - blinkTimer > 0.5){
        blink = !blink;
        blinkTimer = timer;
    }
    
    resultBar.set(0.0f,0.0f,10.0f,10.0f);
    resultBar.scaleCentered(vec2(10.0f, 2.2f));
    resultBar.offsetCenterTo(mCamera.worldToScreen(eyePoint + 5.0f * viewDirection - up * 1.635f, w, h));
    if(blink){
        if(unstable){
            gl::color(textColRed);
            gl::drawSolidRect(resultBar);
            gl::drawStringCentered("UNSTABLE", resumePos, textCol, mFontUI);
        }else{
            gl::color(textCol);
            gl::drawSolidRect(resultBar);
            gl::drawStringCentered("PASS", resumePos, textColB, mFontUI);
        }
    }
    
    

    
    
    gl::color(Color::white());
    loadingBar.set(0.0f,0.0f,10.0f,10.0f);
    float scale = clamp(lmap(r, 20.0f, 0.0f, 0.1f, 18.0f), 0.1f, 18.0f);
    loadingBar.scaleCentered(vec2(scale, 1.0f));
    loadingBar.offsetCenterTo(mCamera.worldToScreen(eyePoint + 5.0f * viewDirection + up * 1.7f, w, h));
    gl::drawSolidRect(loadingBar);
    
    gl::enableAlphaBlending( false );
}

// #################################################################################

CINDER_APP(ChaosApp, RendererGl, [](App::Settings *settings) {
    settings->setWindowSize(1280, 800);
    //    settings->setFullScreen();
    //settings->setHighDensityDisplayEnabled( true );
    //    settings->setMultiTouchEnabled( false );
})
