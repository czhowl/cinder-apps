#include "Particle.h"

using namespace constant;
using namespace ci;
using namespace ci::app;
using namespace std;

void Particle::setup() {
    mPerlinA.setSeed(clock());
    //mPerlinB.setSeed(getElapsedSeconds() + 2);
    //mPerlinC.setSeed(getElapsedSeconds() + 3);
    colors.assign(PARTICLE_NUM, Colorf(0,0,0));
    randomness = 0.1;
    poss.assign(PARTICLE_NUM, glm::vec3(0));
    mColorPan = 0;
};
void Particle::update() {
    //        ci::app::console() << randomness << endl;
    float t = ci::app::getElapsedSeconds() * MULT_TIME;
    x = 1; y = 0; z = 4.5;
//    float pA = mPerlinA.fBm(t) * randomness;
//     float randA = 1.5 * (pA * 2 );
//     float randB = 2 * (pA * 2);
//     float randC = 1 * (pA * 2);
    //float pB = mPerlinB.fBm(t) * randomness;
    //float pC = mPerlinC.fBm(t) * randomness;
    
    float colorR = cos(mColorPan) / 8 + 0.125 + randFloat(0, randomness/20);
    float colorG = cos(mColorPan + 1.07) / 10 + 0.1 + randFloat(0, randomness/20);
    float colorB = sin(mColorPan) / 10 + 0.1 + randFloat(0, randomness/20);
    //console() << pA << endl;
    if(mEquation){
        float randA = 1.5 * randPosNegFloat(0, randomness);
        float randB = 2 * randPosNegFloat(0, randomness);
        float randC = 1 * randPosNegFloat(0, randomness);
        for (int i = 0; i < PARTICLE_NUM; i++) {
            float a = CONSTANT_A + randA;
            float b = CONSTANT_B + randB;
            float c = CONSTANT_C + randC;
            float dx = (a * x - y * z) * DELTA_TIME;
            float dy = (b * y + x * z) * DELTA_TIME;
            float dz = (c * z + x * y) * DELTA_TIME;
            x += dx;
            y += dy;
            z += dz;
            poss[i] = glm::vec3(x, y, z);
            float len = glm::length(poss[i]);
            colors[i] = ColorAf(dx + 0.1 + colorR, dy + 0.1 + colorG, dz + 0.1 + colorB, lmap(clamp(len, 0.0f, 50.0f), 0.0f, 50.0f, 1.0f, 0.0f));
            //            ci::app::console() << poss[i] << endl;
        }
    }else{
        float randA = -6 * randPosNegFloat(0, randomness);
        float randB = 4 * randPosNegFloat(0, randomness);
        float randC = -2 * randPosNegFloat(0, randomness);
        for (int i = 0; i < PARTICLE_NUM; i++) {
            float a = -6.0 + randA;
            float b = 4.3 + randB;
            float c = -2 + randC;
            float dx = y  * DELTA_TIME;
            float dy = z * DELTA_TIME;
            float dz = (-a * x - b * y - z + c * x*y*z)  * DELTA_TIME;
            x += dx;
            y += dy;
            z += dz;
            poss[i] = glm::vec3(x, y, z);
            float len = glm::length(poss[i]);
            colors[i] = ColorAf(dx + 0.1 + colorR, dy + 0.1 + colorG, dz + 0.1 + colorB, lmap(clamp(len, 0.0f, 50.0f), 0.0f, 50.0f, 1.0f, 0.0f));
            //            ci::app::console() << poss[i] << endl;
        }
    }
    //        ci::app::console() << "====================" << endl;
};

void Particle::draw() {
    gl::ScopedBlendAdditive blend;
    //gl::begin(GL_LINE_STRIP);
    gl::begin(GL_POINTS);
    glPointSize(0.5f);
    for (int i = 0; i < PARTICLE_NUM; i++) {
        
        gl::color(colors[i]);
        //            cinder::gl::drawSphere(poss[i] * 10.0f, 0.5);
        gl::vertex(poss[i]);
    }
    gl::end();
};

void Particle::setRandomness(float r) {
    if (r == 0.0f) {
        randomness = 0.01;
    }
    else {
        randomness = r;
    }
    //ci::app::console() << randomness << endl;
};


void Particle::setColorPan(float p) {
    mColorPan = p;
    //ci::app::console() << randomness << endl;
};

void Particle::setEquation(bool e){
    mEquation = e;
}
