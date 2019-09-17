#pragma once

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include <ctime>
// ===========================================================
typedef std::shared_ptr<class Wechat> WechatRef;

class Wechat {
public:
    static WechatRef create(std::string url) {
        auto wechat = std::make_shared<Wechat>();
        wechat->setup(url);
        return wechat;
    }
    
    Wechat();
    
    void setup(std::string url);
    void updateUrl(std::string url);
    void draw(ci::vec2 pos);
    std::string getUrl(){return mUrl;};
    void downloadImage();
    void downloadImageThread(ci::gl::ContextRef context);
    
private:
    std::string                     mUrl;
    ci::gl::Texture2dRef            mTexture;
};
