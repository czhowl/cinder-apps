#include <cinder/Rand.h>
#include "Key.h"

// nice little type definition to make for loop code mor readable
typedef list<Particle>::iterator PIt;

Key::Key() {
    mMaxDistance = 75.0f;
    mMaxDistanceSquared = mMaxDistance * mMaxDistance;
    pulse = 0;
    hue = Rand::randFloat(0.15f, 0.85f);
}

void Key::update() {
    for (PIt p = mParticles.begin(); p != mParticles.end(); ++p) {
        p->update();
    }
    if(getElapsedFrames()%10 == 0) keyShape = Rand::randInt(0,5);
}

void Key::draw() {
    for (PIt p = mParticles.begin(); p != mParticles.end(); ++p) {
        p->draw();
    }
}

void Key::addParticles(int amount, gl::Texture2dRef spirits[4], vec2 pos) {
    int i = 0;
    while (i < amount) {
        vec2 l = Rand::randVec2() * 34.0f;
        vec2 v = Rand::randVec2() * 5.0f;
        mParticles.push_back(Particle(l, v, spirits[Rand::randInt(0, 4)], hue)  );
        ++i;
    }
    nestPos = pos;
}

void Key::applyFlocking(float zoneRadius, float lowerThreshold, float higherThreshold, float attractStrength, float repelStrength, float alignmentStrength) {
    float zoneRadiusSquared = zoneRadius * zoneRadius;
    float twoPI = (float) M_PI * 2.0f;
    
    for (PIt p1 = mParticles.begin(); p1 != mParticles.end(); ++p1) {
        
        PIt p2 = p1;
        for (++p2; p2 != mParticles.end(); ++p2) {
            vec2 dir = p1->getPosition() - p2->getPosition();
            float distSqrd = glm::length2(dir);
            if (distSqrd <= zoneRadiusSquared) { // Neighbor is in the zone
                float percent = distSqrd / zoneRadiusSquared;
                
                if (percent < lowerThreshold) { // SEPARATION
                    float F = (lowerThreshold / percent - 1.0f) * repelStrength*(1.0 + pulse * 1000.0f);
                    dir = glm::normalize(dir) * F;
                    p1->applyForce(dir); // add force
                    p2->applyForce(dir * -1.0f); // subtract force
                }
                
                else if (percent < higherThreshold) { // ALIGNMENT
                    float thresholdDelta = higherThreshold - lowerThreshold;
                    float adjustedPercent = (percent - lowerThreshold) / thresholdDelta;
                    float F = (1.0f - (cos(adjustedPercent * twoPI*(1.0 + pulse * 1000.0f)) * 0.5f + 0.5f)) * 0;
                    p1->applyForce(p2->getNormalizedVelocity() * F);
                    p2->applyForce(p1->getNormalizedVelocity() * F);
                }
                
                else { // COHESION
                    float thresholdDelta = 1.0f - higherThreshold;
                    float adjustedPercent = (percent - higherThreshold) / thresholdDelta;
                    float F = (1.0f - (cos(adjustedPercent * twoPI) * -0.5f + 0.5f)) * attractStrength*(1.0 + pulse * 1000.0f);
                    dir = glm::normalize(dir) * F;
                    p1->applyForce(dir * -1.0f); // subtract force
                    p2->applyForce(dir); // add force
                }
            }
        }
    }
}

void Key::applyCenterPull(float dist, float centerpull) {
    vec2 dir;
    float distToCenterSquared;
    if(pulse > 0.0f) pulse -= pulse*0.1f;
    for (PIt p = mParticles.begin(); p != mParticles.end(); ++p) {
        dir = p->getPosition() - vec2(0.0f);
        distToCenterSquared = glm::length2(dir);
        if (distToCenterSquared > dist * dist) {
            dir = glm::normalize(dir);
            p->applyForce(dir * (distToCenterSquared - dist * dist) * centerpull * -1.0f);
        }
        p->applyForce(pulseDir * pulse * 100.0f);
    }
}

void Key::applyPulse(float p) {
    pulse = p;
    pulseDir = vec2(Rand::randFloat(-1.0f,1.0f),Rand::randFloat(-1.0f,1.0f));
}















