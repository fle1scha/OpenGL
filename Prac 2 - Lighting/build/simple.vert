#version 330 core
//Input data
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 vertexUV;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;  

//Output data
out vec3 position_worldspace;
out vec3 Normal_cameraspace;
out vec3 EyeDirection_cameraspace;
out vec3 LightDirection_cameraspace;
out vec3 LightDirection_cameraspace2;
out vec2 UV;
out mat3 TBN;
out vec3 lightDirection_ts;
out vec3 lightDirection_ts2;
out vec3 eyeDirection_ts;

//Uniforms
uniform mat4 MVP;
uniform mat4 Model;
uniform vec3 alightPos;
uniform vec3 blightPos;
uniform mat4 View;

void main()
{   
    //Set position of vertex in homogeneous co-ordinates
    gl_Position = MVP*vec4(position,1.0f);
    
    //The position of the vertex in world space
    position_worldspace = vec3(Model * vec4(position, 1.0)).xyz;

    //Position of the eye in camera space
    vec3 position_cameraspace = ( View * Model * vec4(position,1)).xyz;
    EyeDirection_cameraspace = vec3(0,0,0) - position_cameraspace;

    //Direction of the first light in camera space
    vec3 LightPosition_cameraspace = ( View *Model *  vec4(alightPos,1)).xyz;
    LightDirection_cameraspace = LightPosition_cameraspace + EyeDirection_cameraspace;

    //Direction of the second light in camera space
    vec3 LightPosition_cameraspace2 = ( View * vec4(blightPos,1)).xyz;
    LightDirection_cameraspace2 = LightPosition_cameraspace2 + EyeDirection_cameraspace;

    //Texture coordinates
    UV = vertexUV;

    //TBN vectors in camera space
	vec3 tangent_cs = (View * Model * vec4(aTangent,0)).xyz;
	vec3 bitangent_cs =  (View * Model* vec4(aBitangent,0)).xyz;
	vec3 normal_cs =  (View * Model * vec4(aNormal,0)).xyz;
	Normal_cameraspace = normal_cs;
	mat3 TBN = transpose(mat3(tangent_cs,bitangent_cs,normal_cs));

    //light directions and view direction in tangent space
	lightDirection_ts = TBN * LightDirection_cameraspace;
    lightDirection_ts2 = TBN * LightDirection_cameraspace2;
	eyeDirection_ts =  TBN * EyeDirection_cameraspace;

	
	

}
