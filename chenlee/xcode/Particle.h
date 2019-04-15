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

namespace constant
{
    constexpr float CONSTANT_A =  2.500f;
    constexpr float CONSTANT_B = -6.500f;
    constexpr float CONSTANT_C = -1.349f;
    
    constexpr float DELTA_TIME = 0.01f;
    constexpr int   num_params = 18;
    constexpr int   PARTICLE_NUM = 1;
}

using namespace constant;
using namespace std;

class Particle {
public:
    void setup(ci::ColorA _color){
        color = _color;
        float t = ci::app::getElapsedSeconds() * DELTA_TIME;
        for(int i = 0; i < PARTICLE_NUM; i++){
            float dx = (CONSTANT_A * x - y * z) * DELTA_TIME;
            float dy = (CONSTANT_B * y + x * z) * DELTA_TIME;
            float dz = (CONSTANT_C * z + x * y) * DELTA_TIME;
            x += dx;
            y += dy;
            z += dz;
            poss.push_back(glm::vec3(x * 10, y * 10, z * 10));
        }
        
    };
    void update(){
        float t = ci::app::getElapsedSeconds() * DELTA_TIME;
        
        for(int i = 0; i < PARTICLE_NUM; i++){
            float dx = (CONSTANT_A * x - y * z) * DELTA_TIME;
            float dy = (CONSTANT_B * y + x * z) * DELTA_TIME;
            float dz = (CONSTANT_C * z + x * y) * DELTA_TIME;
            x += dx;
            y += dy;
            z += dz;
            
            poss[i] = glm::vec3(x * 10, y * 10, z * 10);
            ci::app::console() << poss[i] << endl;
        }
        ci::app::console() << "====================" << endl;
    };
    void draw(){
        cinder::gl::drawSphere(poss[0], 1);
    };
    
    
    
    std::vector<glm::vec3>      poss;
private:
    cinder::gl::VboMeshRef      mVboMesh;
    
    ci::ColorA                  color;
    
    
    double params[num_params];
    float x = 1, y = 0, z = 4.5;
};

#endif /* Particle_h */
