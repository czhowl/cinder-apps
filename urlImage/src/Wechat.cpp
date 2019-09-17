#include "Wechat.h"

using namespace ci;
using namespace ci::app;
using namespace std;

// ====================================================================== Wave

Wechat::Wechat()
{ }

// ====================================================================== DownloadImage

void Wechat::downloadImage(){
    mTexture = gl::Texture2d::create(loadImage(loadUrl(mUrl)));
}

// ====================================================================== DownloadImage

void Wechat::downloadImageThread(gl::ContextRef context){
    ci::ThreadSetup threadSetup;
    context->makeCurrent();
    mTexture = gl::Texture2d::create(loadImage(loadUrl(mUrl)));
}

// ====================================================================== Setup

void Wechat::setup(std::string url){
    mUrl = url;
}

// ====================================================================== Update

void Wechat::updateUrl(std::string url){
    mUrl = url;
}

// ====================================================================== Draw

void Wechat::draw(ci::vec2 pos){
    if(mTexture) gl::draw(mTexture, pos);
}
