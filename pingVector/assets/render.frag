#version 330
#extension all : warn

//uniform sampler2D ParticleTex;

//in float Transp;

out vec4 FragColor;

const vec3 pink = vec3(1.0, 0.07, 0.57);

void main() {
//    FragColor = texture( ParticleTex, gl_PointCoord );
//    FragColor = vec4((vec3(1 - FragColor.r, 1 - FragColor.g, 1 - FragColor.b) * pink), FragColor.a);
//    FragColor.a = 1.0;
//    FragColor.a -= .1;
    FragColor = vec4(1.0);
}
