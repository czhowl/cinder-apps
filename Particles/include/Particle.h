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
    constexpr float CONSTANT_A = 0.1f;
    constexpr float CONSTANT_B = 28.0f;
    constexpr float CONSTANT_C = 8.0f/3.0f;
    
    constexpr float DELTA_TIME = 0.01f;
    constexpr int   num_params = 18;
}

using namespace constant;
using namespace std;

class Particle {
public:
    void setup(ci::ColorA _color){
        StringToParams("DPPREG", params);
        color = _color;
        float t = ci::app::getElapsedSeconds() * DELTA_TIME;
        for(int i = 0; i < 100; i++){
            const double xx = x * x;
            const double yy = y * y;
            const double tt = t * t;
            const double xy = x * y;
            const double xt = x * t;
            const double yt = y * t;
            const double nx = xx*params[0] + yy*params[1] + tt*params[2] + xy*params[3] + xt*params[4] + yt*params[5] + x*params[6] + y*params[7] + t*params[8];
            const double ny = xx*params[9] + yy*params[10] + tt*params[11] + xy*params[12] + xt*params[13] + yt*params[14] + x*params[15] + y*params[16] + t*params[17];
            x = nx;
            y = ny;
            z = 0;
            
            poss.push_back(glm::vec3(x, y, z));
        }
        
    };
    void update(){
        float t = ci::app::getElapsedSeconds() * DELTA_TIME;
        for(int i = 0; i < 100; i++){
            const double xx = x * x;
            const double yy = y * y;
            const double tt = t * t;
            const double xy = x * y;
            const double xt = x * t;
            const double yt = y * t;
            const double nx = xx*params[0] + yy*params[1] + tt*params[2] + xy*params[3] + xt*params[4] + yt*params[5] + x*params[6] + y*params[7] + t*params[8];
            const double ny = xx*params[9] + yy*params[10] + tt*params[11] + xy*params[12] + xt*params[13] + yt*params[14] + x*params[15] + y*params[16] + t*params[17];
            x = nx;
            y = ny;
            z = 0;
            
            poss[i] = glm::vec3(x, y, z);
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
    float x = 0.1, y = 0.1, z = 0.1;
    
    static void RandParams(double* params) {
        for (int i = 0; i < num_params; ++i) {
            const int r = ci::randInt(4);
            if (r == 0) {
                params[i] = 1.0f;
            } else if (r == 1) {
                params[i] = -1.0f;
            } else {
                params[i] = 0.0f;
            }
        }
    }
    
    static void StringToParams(const std::string& str, double* params) {
        for (int i = 0; i < num_params/3; ++i) {
            int a = 0;
            const char c = (i < str.length() ? str[i] : '_');
            if (c >= 'A' && c <= 'Z') {
                a = int(c - 'A') + 1;
            } else if (c >= 'a' && c <= 'z') {
                a = int(c - 'a') + 1;
            }
            params[i*3 + 2] = double(a % 3) - 1.0;
            a /= 3;
            params[i*3 + 1] = double(a % 3) - 1.0;
            a /= 3;
            params[i*3 + 0] = double(a % 3) - 1.0;
        }
    }
};

#endif /* Particle_h */
