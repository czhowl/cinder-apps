#include <cinder/Perlin.h>
#include "Particle.h"
#include "cinder/Rand.h"
#include "cinder/gl/gl.h"

Particle::Particle(vec2 position, vec2 velocity, float hue){
    mPosition = position;
    mVelocity = velocity;
    mAcceleration = vec2(0.0f);
    
    mVelocityNormalized = glm::normalize(velocity);
    mTailPosition = position;
    
    mMaxSpeed = Rand::randFloat(0.001f, 0.003f);
    mMaxSpeedSquared = mMaxSpeed * mMaxSpeed;
    mMinSpeed = Rand::randFloat(0.0f, 0.0001f);
    mMinSpeedSquared = mMinSpeed * mMinSpeed;
    mMaxForce = Rand::randFloat(0.00001f, 0.0001f);
    mRadius = 20.0f;
    mDecay = 0.995f;
    mArrive = 0.1f;
    mTailLength = 10.0f;
    mVelMag = 0.0f;
    
    color = Color(CM_HSV, hue + Rand::randFloat(-0.05f, 0.05f), 0.8f + Rand::randFloat(-0.2f, 0.2f), 0.9f + Rand::randFloat(-0.1f, 0.1f));
}

void Particle::update() {
    // update velocity
    mVelocity += mAcceleration;
    // store normalized velocity vector
    mVelocityNormalized = glm::normalize(mVelocity);
    // limit velocity's speed
    mVelMag = glm::length2(mVelocity);
    if (mVelMag > mMaxSpeedSquared) {
        mVelocity = mVelocityNormalized * mMaxSpeed;
    }
    
    // update positions
    mPosition += mVelocity;
    // slow down velocity
    mVelocity *= mDecay;
    // reset acceleration
    mAcceleration *= 0;
}

void Particle::draw(float width, float height) {
    gl::ScopedModelMatrix smm;
    gl::translate(mPosition * vec2(width, height));
    if(mVelMag > 1e-7) mAngle = (float)atan2(mVelocity.y, mVelocity.x);
    gl::rotate( mAngle );
    //    gl::translate( - (mTex->getrSize() / 2));
    gl::ScopedColor c(color);
    //    gl::draw(mTex);
    gl::drawSolidCircle(vec2(0), 0.5, 30);
    gl::drawSolidCircle(vec2(-0.5, 0), 0.25, 30);
}

void Particle::applyForce(const vec2 &force) {
    mAcceleration += force;
}

vec2 Particle::seek(vec2 target) {
    vec2 desired = target - mPosition;
    float d = glm::length(desired);
    vec2 steer(0.0);
    if(d > 0.001f){
        desired = glm::normalize(desired);
        if (d < mArrive) {
            float m = lmap<float>(d,0,mArrive,0,mMaxSpeed);
            desired *= m;
        } else {
            desired *= mMaxSpeed;
        }
        steer = desired - mVelocity;
        steer = glm::normalize(steer) * mMaxForce;
    }
    return steer;
}

vec2 Particle::boundary(float width, float height){
    vec2 desired;
    vec2 steer(0);
//    if (mPosition.x < -width - 0.05) {
//        mPosition.x = -mPosition.x;
//        return steer;
//    }
//    else if (mPosition.x > width + 0.05) {
//        mPosition.x = -mPosition.x;
//        return steer;
//    }
    if (mPosition.x < -width) {
        desired = vec2(mMaxSpeed, mVelocity.y);
    }
    else if (mPosition.x > width) {
        desired = vec2(-mMaxSpeed, mVelocity.y);
    }
    
    if (mPosition.y < -height) {
        desired = vec2(mVelocity.x, mMaxSpeed);
    }
    else if (mPosition.y > height) {
        desired = vec2(mVelocity.x, -mMaxSpeed);
    }
    
    if (desired != vec2(0.0)) {
        desired = glm::normalize(desired) * mMaxSpeed;
        steer = desired - mVelocity;
        steer = glm::normalize(steer) * mMaxForce;
    }
    return steer;
}

void Particle::flocking(vector<Particle> boids, float ali, float sep, float coh, float aliDist, float sepDist, float cohDist){
    vec2 aliForce(0);
    vec2 sepForce(0);
    vec2 cohForce(0);
    int count = 0;
    for (auto other : boids) {
        float d = glm::length(mPosition - other.mPosition);
        
        if (d > 0) {
            count++;            // Keep track of how many
            vec2 diff = mPosition - other.mPosition;
            
            // Seperation
            if(sep > 0.0f && d < sepDist){
                vec2 sepDif = glm::normalize(diff) / d; // Weight by distance
                sepForce += sepDif;
            }
            //
            // Alignment
            if(ali > 0.0f && d < aliDist){
                aliForce += other.mVelocity;
            }
            //
            // Cohesion
            if(coh > 0.0f && d < cohDist){
                cohForce += other.mPosition; // Add position
            }
        }
    }
    
    if (count > 0) {
        // Seperation
        if (glm::length(sepForce) > 0) {
            sepForce /= (float)count;
            // Implement Reynolds: Steering = Desired - Velocity
            sepForce = glm::normalize(sepForce) * mMaxSpeed - mVelocity;
            sepForce = glm::normalize(sepForce) * mMaxForce;
        }
        applyForce(sepForce * sep);
        // Alignment
        if (glm::length(aliForce) > 0) {
            aliForce /= (float)count;
            aliForce = glm::normalize(aliForce) * mMaxSpeed - mVelocity;
            aliForce = glm::normalize(aliForce) * mMaxForce;
        }
        applyForce(aliForce * ali);
        
        // Cohesion
        if (glm::length(cohForce) > 0) {
            cohForce /= (float)count;
            cohForce = seek(cohForce);  // Steer towards the position
        }
        applyForce(cohForce * coh);
        
    }
}

void Particle::checkGrass(vec2 chair, bool sit, float click, float time){
    vec2 dir = glm::normalize(mPosition - chair);
    float d = glm::length(mPosition - chair);
    vec2 desired;
    vec2 steer(0);
    //    if (d < 0.1){
    //        vec2 v1 = glm::normalize(dir);
    //        vec2 v2 = glm::normalize(mVelocity);
    //        float mag = glm::dot(vec3(v1.x, v1.y, 0.0), vec3(v2.x, v2.y, 0.0));
    ////        if()
    ////        desired = vec2(mMaxSpeed * dir, mMaxSpeed * dir));
    //        desired = -dir * mMaxSpeed * mag;
    //        steer = desired - mVelocity;
    //        steer = glm::normalize(steer) * mMaxForce;
    //    }
    //    applyForce(steer);
    //    if(!sit){
    if (d < 0.1 && sit){
//        desired = -dir * mMaxSpeed * 0.55f;
//        steer = desired - mVelocity;
//        steer = glm::normalize(steer) * mMaxForce;
        steer = seek(chair);
        applyForce(steer * 1.0f);
    }
    
    if (d < 0.075 && !sit){
        desired = dir * mMaxSpeed * 0.5f;
        steer = desired - mVelocity;
        steer = glm::normalize(steer) * mMaxForce;
        applyForce(steer * 1.0f);
    }
    
    if (d < 0.15 && time - click < 0.2){
        steer = dir * min((0.15f - d) * 0.003f, 0.003f);
        //        steer = desired - mVelocity;
        //        steer = dir * mMaxForce;
        applyForce(steer * 1.0f);
    }
    //    }else{
//    if (d < 0.1 && sit){
//        desired = vec2(0);
//        steer = desired - mVelocity;
//        steer = glm::normalize(steer) * mMaxForce;
//        applyForce(steer * (0.1f - d) * 10.0f);
//    }
    //    }
}

vec2 Particle::alignment (float ali, vector<Particle> boids) {
    vec2 steer(0, 0);
    int count = 0;
    for (auto other : boids) {
        float d = glm::length(mPosition - other.mPosition);
        if ((d > 0) && (d < ali)) {
            steer += other.mVelocity;
            count++;
        }
    }
    if (count > 0) {
        steer /= (float)count;
        steer = glm::normalize(steer) * mMaxSpeed - mVelocity;
        steer = glm::normalize(steer) * mMaxForce;
    }
    
    return steer;
}

vec2 Particle::seperation(float sep, vector<Particle> boids){
    vec2 steer(0);
    int count = 0;
    for (auto other : boids) {
        float d = glm::length(mPosition - other.mPosition);
        if ((d > 0) && (d < sep)) {
            // Calculate vector pointing away from neighbor
            vec2 diff = mPosition - other.mPosition;
            
            diff = glm::normalize(diff) / d; // Weight by distance
            steer += diff;
            count++;            // Keep track of how many
        }
    }
    // Average -- divide by how many
    if (count > 0) {
        steer /= (float)count;
    }
    // As long as the vector is greater than 0
    if (glm::length(steer) > 0) {
        // Implement Reynolds: Steering = Desired - Velocity
        steer = glm::normalize(steer) * mMaxSpeed - mVelocity;
        steer = glm::normalize(steer) * mMaxForce;
    }
    
    return steer;
}

vec2 Particle::cohesion (float coh, vector<Particle> boids) {
    vec2 steer(0, 0);   // Start with empty vector to accumulate all positions
    int count = 0;
    for (auto other : boids) {
        float d = glm::length(mPosition - other.mPosition);
        if ((d > 0) && (d < coh)) {
            steer += other.mPosition; // Add position
            count++;
        }
    }
    if (count > 0) {
        steer /= count;
        steer = seek(steer);  // Steer towards the position
    }
    return steer;
}

vec2 Particle::getPosition() {
    return mPosition;
}

vec2 Particle::getNormalizedVelocity() {
    return mVelocityNormalized;
}
