#version 430 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture1;
uniform bool visualLinearDepth;
uniform bool visualUnlinearDepth;

float near = 0.1; 
float far = 100.0; 

// [0,1]-> [-1,1]->[near,far]
float LinearizeDepth(float depth){
    float z = depth * 2.0 - 1.0; // back to NDC
   
    return (2.0 * near * far) / (far + near - z * (far - near));    
}

void main()
{    
    if(visualLinearDepth){
        float depth = (LinearizeDepth(gl_FragCoord.z) - near) / (far - near) ; // 除以far [near,far]->[0,1]
        FragColor = vec4(vec3(depth), 1.0);
    }else if(visualUnlinearDepth){
        FragColor = vec4(vec3(gl_FragCoord.z), 1.0);
    }else{
        FragColor = texture(texture1, TexCoords);
    }

}

