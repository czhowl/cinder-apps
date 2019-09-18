#version 150

uniform sampler2DRect uBackBuffer;

in vec2		vTexCoord0;
in vec2 	vPosition;
//in vec3		vertNormal;
//in vec4		vertColor;
//in vec4		vertColorBack;
//in vec4     vertUp;
//in vec3		viewDir;

out vec4	oColor;

void main()
{	
	 float life = texture(uBackBuffer, vPosition).g;
	 float prevLife = texture(uBackBuffer, vPosition).b;

    float count = 0.0;
    for(int x=-1; x<=1; x++){
        for(int y=-1; y<=1; y++){
            if(!(x == 0 && y == 0)){
                count += texture(uBackBuffer, vPosition+vec2(x,y)).g;
            }
        }
    }
    
    float nextlife = 0.0;
    if(life == 0.0){
        if(count == 3.0){
            nextlife = 1.0;
        } else {
            nextlife = 0.0;
        }
    } else {
        if(count == 0 || count == 1){
            nextlife = 0.0;
        } else if(count == 2 || count == 3){
            nextlife = 1.0;
        } else {
            nextlife = 0.0;
        }
    }
	float b = 0.0;
	if(prevLife > 0.01) b = prevLife*0.995;
	if(life == 1.0 && nextlife == 0.0) b = 1.0;
    
    oColor = vec4(0.0, nextlife, b, 1.0);
    
//    vec4 smpColor = texture( uBackBuffer, vPosition );
//    
//    // output color
//    oColor = smpColor;
}
