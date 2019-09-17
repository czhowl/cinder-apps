#version 150

uniform sampler2DRect uLifeTex;

in vec2		vTexCoord0;
in vec2 	vPosition;

out vec4	oColor;

void main()
{	
	// retrieve normal from texture
	vec4 smpColor = texture( uLifeTex, vPosition );

	// output color
//    oColor = smpColor;
	if(smpColor.g == 1.0){
	oColor = vec4(smpColor.g,smpColor.g,smpColor.g,1.0);
	}else if(smpColor.b > 0.0){
	vec3 color = mix(vec3(1.0,0.3,0.0), vec3(0.0,0.0,1.0), 1.3-smpColor.b) * smpColor.b;
	oColor = vec4(color,1.0);
	}else{
	oColor = vec4(0.0,0.0,0.0,1.0);
	}
}
