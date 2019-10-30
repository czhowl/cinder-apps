#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "cinder/audio/Voice.h"
#include "cinder/audio/Source.h"
#include "cinder/Rand.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class keyboardSoundApp : public App {
public:
    void setup() override;
    void mouseDown( MouseEvent event ) override;
    void update() override;
    void draw() override;
    void keyDown(KeyEvent event) override;
    void keyUp(KeyEvent event) override;
    
    audio::VoiceRef mBgm;
    audio::VoiceRef mDuck;
    audio::VoiceRef mKey;
    float			mVolume;
    bool			mRelease;
    audio::VoiceRef mKeys[26];
};

void keyboardSoundApp::setup()
{
    mRelease = true;
    mVolume = 1.0f;
    mBgm = audio::Voice::create(audio::load(loadAsset("bgm_mixdown.wav")));
    mDuck = audio::Voice::create(audio::load(loadAsset("duck_mixdown.wav")));
    mDuck->setVolume(5.0f);
    mKey = audio::Voice::create(audio::load(loadAsset("key_mixdown.wav")));
    for (int i = 0; i < 26; i++) {
        mKeys[i] = audio::Voice::create(audio::load(loadAsset(to_string(Rand::randInt(15)) + "_mixdown.wav")));
        mKeys[i]->setVolume(0.3f);
    }
}

void keyboardSoundApp::mouseDown( MouseEvent event )
{
}

void keyboardSoundApp::update()
{
    console() << mVolume << endl;
    if (mRelease) mVolume -= (mVolume - 1.0f) * 0.01f;
    mBgm->setVolume(mVolume);
    //mKey->setVolume(mVolume);
    if (!mBgm->isPlaying())
        mBgm->start();
    if(mVolume > 5.0f) mDuck->start();
}

void keyboardSoundApp::keyDown(KeyEvent event)
{
    mRelease = false;
    mVolume += 0.5f / mVolume;
    if(event.getChar() - 'a' >= 0 && event.getChar() - 'a' < 26)
        mKeys[event.getChar() - 'a']->start();
}

void keyboardSoundApp::keyUp(KeyEvent event)
{
    mRelease = true;
}

void keyboardSoundApp::draw()
{
    gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP( keyboardSoundApp, RendererGl )
