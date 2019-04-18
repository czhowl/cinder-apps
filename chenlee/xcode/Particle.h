//
//  Particle.h
//  Particles
//
//  Created by Zihao Chen on 4/15/19.
//

#ifndef Particle_h
#define Particle_h

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"
#include "cinder/Perlin.h"

namespace constant
{
    constexpr float CONSTANT_A =  5.0f;
    constexpr float CONSTANT_B = -10.0f;
    constexpr float CONSTANT_C = -0.38f;
    
    constexpr float DELTA_TIME = 0.005f;
    constexpr int   num_params = 18;
    constexpr int   PARTICLE_NUM = 1500;
}

using namespace constant;
using namespace std;

class Particle {
public:
    void setup(ci::ColorA _color){
        mPerlin.setSeed( clock() );
        color = _color;
        randomness = 0.1;
//        float t = ci::app::getElapsedSeconds() * DELTA_TIME;
//        for(int i = 0; i < PARTICLE_NUM; i++){
//            float a = CONSTANT_A + ci::randPosNegFloat(0, 1.5*randomness);
//            float b = CONSTANT_B + ci::randPosNegFloat(0, 2*randomness);
//            float c = CONSTANT_C + ci::randPosNegFloat(0, 1*randomness);
//            float dx = (a * x - y * z) * DELTA_TIME;
//            float dy = (b * y + x * z) * DELTA_TIME;
//            float dz = (c * z + x * y) * DELTA_TIME;
//            x += dx;
//            y += dy;
//            z += dz;
//            poss.push_back(glm::vec3(x, y, z));
//        }
        poss.assign( PARTICLE_NUM, glm::vec3(0) );
        
        
    };
    void update(){
//        ci::app::console() << randomness << endl;
        float t = ci::app::getElapsedSeconds() * DELTA_TIME;
//        x = poss[1].x;
//        y = poss[1].y;
//        z = poss[1].z;
        x = 1; y = 0; z = 4.5;
        float randA = 1.5 * ci::randPosNegFloat(0, randomness);
        float randB = 2 * ci::randPosNegFloat(0, randomness);
        float randC = 1 * ci::randPosNegFloat(0, randomness);
        for(int i = 0; i < PARTICLE_NUM; i++){
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
//            ci::app::console() << poss[i] << endl;
        }
        
        
//        ci::app::console() << "====================" << endl;
    };
    
    void draw(){
        for(int i = 0; i < PARTICLE_NUM; i++){
//            cinder::gl::color(ci::Color(ci::randFloat(1.0f),ci::randFloat(0.3f),ci::randFloat(1.0f)));
            cinder::gl::drawSphere(poss[i] * 10.0f, 0.5);
        }
    };
    
    void setRandomness(float r){
        if(r == 0.0f){
            randomness = 0.1;
        }else{
            randomness = r;
        }
        ci::app::console() << randomness << endl;
    };
    
    std::vector<glm::vec3>      poss;
    
    
private:
    cinder::gl::VboMeshRef      mVboMesh;
    ci::ColorA                  color;
    double                      params[num_params];
    float                       x = 1, y = 0, z = 4.5;
    float                       randomness;
    ci::Perlin                  mPerlin;
};

#endif /* Particle_h */
