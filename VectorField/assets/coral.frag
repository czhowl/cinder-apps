#version 330

//uniform sampler2D uTex0;
//
//in vec4    Color;
//in vec3    Normal;
//in vec2    TexCoord;
//
//out vec4             oColor;
//
//void main( void )
//{
//    vec3 normal = normalize( -Normal );
//    float diffuse = max( dot( normal, vec3( 0, 0, -1 ) ), 0 );
//    oColor = texture( uTex0, TexCoord.st ) * Color * diffuse;
//}

in vec3 gFlow;
//in vec4    Color;
//in vec2 vTexCoord0;
//in vec3 vNormal;

out vec4     oColor;

void main( void )
{
//    oColor = vec4(gFlow.xy, 1.0, 1.0);
    oColor = vec4(1.0);
    oColor = mix(vec4(0.3, 0.4, 1.0, 1.0), vec4(0.4, 1.0, 0.8, 1.0), (gFlow.z + 1.0) * 0.5);
}
