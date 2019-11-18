#pragma once

// main cinder header file
#include "cinder/app/App.h"
// access main cinder namespaces
using namespace ci;
using namespace ci::app;
using namespace std;

class Particle {
public:
    Particle(vec2 position, vec2 velocity, float hue);
    Particle(){};
    void update();
    void draw(float width, float height);
    void applyForce(const vec2 &force);
    void flocking(vector<Particle> boids, float ali, float sep, float coh, float aliDist, float sepDist, float cohDist);
    void checkGrass(vec2 chair, bool sit, float click, float time);
    vec2 seek(vec2 target);
    vec2 seperation(float sep, vector<Particle> boids);
    vec2 cohesion(float coh, vector<Particle> boids);
    vec2 alignment(float ali, vector<Particle> boids);
    vec2 boundary(float width, float height);
    vec2 getPosition();
    vec2 getNormalizedVelocity();
private:
    vec2 mPosition, mVelocity, mAcceleration;
    vec2 mVelocityNormalized, mTailPosition;
    float mRadius, mDecay, mTailLength, mArrive;
    float mMaxSpeed, mMaxSpeedSquared, mMinSpeed, mMinSpeedSquared, mMaxForce, mAngle, mVelMag;
    ColorAf color;
    
};
