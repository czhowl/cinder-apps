#pragma once

// main cinder header file
#include "cinder/app/App.h"
// access main cinder namespaces
using namespace ci;
using namespace ci::app;
using namespace std;

class Particle {
public:
    Particle(vec2 position, vec2 velocity, gl::Texture2dRef tex, float hue);
    void update();
    void draw();
    void applyForce(const vec2 &force);
    vec2 getPosition();
    vec2 getNormalizedVelocity();
    void pulse(vec2 speed){mVelocity += speed;};
private:
    vec2 mPosition, mVelocity, mAcceleration;
    vec2 mVelocityNormalized, mTailPosition;
    float mRadius, mDecay, mTailLength;
    float mMaxSpeed, mMaxSpeedSquared, mMinSpeed, mMinSpeedSquared;
    gl::BatchRef mSphere;
    const gl::Texture2dRef mTex;
    gl::BatchRef mSpirit;
    ColorAf color;
};
