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
    
    constexpr float DELTA_TIME = 0.001f;
    constexpr float MULT_TIME = 1000.0f;
    constexpr int   num_params = 18;
    constexpr int   PARTICLE_NUM = 300e3;
}



class Particle {
public:
    void setup();
    
    void update();
    
    void draw();
    
    void setRandomness(float r);
    void setColorPan(float p);
    void setEquation(bool e);
    
    std::vector<glm::vec3>      poss;
    
    
private:
    cinder::gl::VboMeshRef      mVboMesh;
    std::vector<ci::ColorAf>     colors;
    float                       x = 1, y = 0, z = 4.5;
    float                       randomness;
    ci::Perlin                  mPerlinA;
    ci::Perlin                  mPerlinB;
    ci::Perlin                  mPerlinC;
    float                       mColorPan;
    bool                        mEquation;
};

#endif /* Particle_h */
