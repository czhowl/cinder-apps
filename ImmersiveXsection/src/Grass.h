#pragma once

#include "cinder/app/App.h"
#include "cinder/audio/audio.h"
#include "cinder/gl/gl.h"
// access main cinder namespaces

typedef std::shared_ptr<class Grass> GrassRef;

class Grass {
public:
    static GrassRef create() {
        auto grass = std::make_shared<Grass>();
        return grass;
    }
    
    void setup();
    void update();
    void draw();
private:
};
