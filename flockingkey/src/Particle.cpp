#include <cinder/Perlin.h>
#include "Particle.h"
#include "cinder/Rand.h"
#include "cinder/gl/gl.h"

Particle::Particle(vec2 position, vec2 velocity, gl::Texture2dRef tex, float hue): mTex(tex) {
    mPosition = position;
    mVelocity = velocity;
    mAcceleration = vec2(0.0f);

    mVelocityNormalized = glm::normalize(velocity);
    mTailPosition = position;

    mMaxSpeed = Rand::randFloat(2.0f, 5.0f);
    mMaxSpeedSquared = mMaxSpeed * mMaxSpeed;
    mMinSpeed = Rand::randFloat(0.0f, 1.5f);
    mMinSpeedSquared = mMinSpeed * mMinSpeed;

    mRadius = 20.0f;
    mDecay = 0.99f;
    mTailLength = 10.0f;

    // bake mesh and shader into batch reference
//    auto mesh = geom::Sphere().radius(mRadius).subdivisions(16);
    auto mesh = geom::Plane().size(vec2(10,10)).origin(vec3(5,5,0)).normal(vec3(0,0,1));
//    // color with zero saturation => white
//    auto color = geom::Constant(geom::COLOR, Color(CM_HSV, 0.0f, 0.0f, 1.0f));
    color = Color(CM_HSV, hue + Rand::randFloat(-0.1f, 0.1f), 0.8f + Rand::randFloat(-0.2f, 0.2f), 0.9f + Rand::randFloat(-0.1f, 0.1f));
//    mTex = tex;
}

void Particle::update() {
    // update velocity
    mVelocity += mAcceleration;
    // store normalized velocity vector
    mVelocityNormalized = glm::normalize(mVelocity);
    // limit velocity's speed
    // take the squared length value for performance reasons!
    // the squared length does not take the square root in order to get the actual vector length.
    float v = glm::length2(mVelocity);
    if (v > mMaxSpeedSquared) {
        mVelocity = mVelocityNormalized * mMaxSpeed;
    } else if (v < mMinSpeedSquared) {
        mVelocity = mVelocityNormalized * mMinSpeed;
    }
    // update positions
    mPosition += mVelocity;
    mTailPosition = mPosition - mVelocityNormalized * mTailLength;
    // slow down velocity
    mVelocity *= mDecay;
    // reset acceleration
    mAcceleration *= 0;
}

void Particle::draw() {
    gl::ScopedModelMatrix smm;
    gl::translate(mPosition);
    float angle = (float)atan2(mVelocity.y, mVelocity.x);
    gl::rotate( angle );
    gl::translate( - (mTex->getSize() / 2));
    gl::ScopedColor c(color);
    gl::draw(mTex);
}

void Particle::applyForce(const vec2 &force) {
    mAcceleration += force;
}

vec2 Particle::getPosition() {
    return mPosition;
}

vec2 Particle::getNormalizedVelocity() {
    return mVelocityNormalized;
}























