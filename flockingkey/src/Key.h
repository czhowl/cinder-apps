#pragma once

#include <cinder/Perlin.h>
#include "cinder/app/App.h"
#include "Particle.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class Key {
public:
    Key();
    void update();
    void draw();
    void addParticles(int amount, gl::Texture2dRef spirits[4], vec2 pos);
    void applyFlocking(float zoneRadius, float lowerThreshold, float higherThreshold, float attractStrength, float repelStrength, float alignmentStrength);
    void applyCenterPull(float dist, float centerpull);
    void applyPulse(float p);
    vec2 nestPos;
    int keyShape;
    float pulse;
private:
    list<Particle> mParticles;
    float mMaxDistance, mMaxDistanceSquared, mCenterPullStrength;
    float hue;
    vec2 pulseDir;
    
};
