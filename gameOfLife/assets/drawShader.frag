#version 150

uniform sampler2DRect uLifeTex;

uniform vec3 uPos;

in vec2		vTexCoord0;
in vec2 	vPosition;

out vec4	oColor;

void main()
{	
	// retrieve normal from texture
	vec4 smpColor = texture( uLifeTex, vPosition );
	if(smpColor.g == 1.0){
	oColor = vec4(smpColor.g,smpColor.g,smpColor.g,1.0);
	}else if(smpColor.b > 0.0){
	oColor = vec4(0.0,0.0,smpColor.b,1.0);
	}else{
	oColor = vec4(0.0,0.0,0.0,1.0);
	}
	if(distance(vec2(1.0-uPos.x/1080.0f, uPos.y/1080.0f),vTexCoord0) < 0.1) oColor = vec4(1.0);

	// output color
//    oColor = smpColor;
	
}
