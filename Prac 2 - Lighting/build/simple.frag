#version 330 core

uniform vec3 objectColor; //object color
uniform vec3 lightColor; //ambient color
uniform vec3 lightColor1; //light color 1
uniform vec3 lightColor2; //light color 2
uniform vec3 alightPos; //light position 1
uniform vec3 blightPos; //light position 2
uniform int bump; //bump mapping on or off.

uniform sampler2D myTexture; //texture
uniform sampler2D normTexture; //bump mapping texture



in vec2 UV; //texture coordinates
in vec3 position_worldspace; //fragment position in world space
in vec3 Normal_cameraspace; //normal in camera space
in vec3 LightDirection_cameraspace; //light direction in camera space
in vec3 LightDirection_cameraspace2; //light direction 2 in camera space 
in vec3 EyeDirection_cameraspace; //eye direction

in vec3 lightDirection_ts; //light direction in tangent space
in vec3 eyeDirection_ts; //viewer direction in tangent space
in vec3 lightDirection_ts2; //light direction 2 in tangent space


out vec4 outColor; //final fragment result

void main()
{      
    //Setup
    float specularStrength = 0.3;
    float distance1 = length(alightPos-position_worldspace);
    float distance2 = length(blightPos-position_worldspace);
    float lightPower = 12.0f;
    vec3 texColor = texture2D(myTexture, UV).rgb;

    //Normalize vectors
    vec3 normal = Normal_cameraspace; 

    vec3 NN = normalize(normal); 
    
    vec3 LL = normalize(LightDirection_cameraspace);
    vec3 LL2 = normalize(LightDirection_cameraspace2);

    vec3 E = normalize(EyeDirection_cameraspace);
    
    //If bump mapping is enabled, use normals in tangent space rather than normal in camera space.
    if (bump == 1)
    {
        vec3 TextureNormal_tangentspace = normalize(texture2D(myTexture, vec2(UV.x,-UV.y) ).rgb*2.0 - 1.0);
        NN = TextureNormal_tangentspace;

    //I attempted bump mapping using light direction and eye direction in tangent space, but it didn't look correct.
    //I have left it in comments so that you can see the result if you uncomment it.
        /*LL = normalize(lightDirection_ts);
        LL2 = normalize(lightDirection_ts2);

        E = normalize(eyeDirection_ts);
        */
    }

    //Ambient 
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength*lightColor;

    
    //Diffuse
    float diffuse = clamp(dot (NN, LL),0,1);
    float diffuse2 = clamp(dot(NN,LL2),0,1);

    //Specular
    vec3 R = reflect(-LL, NN);
    vec3 R2 = reflect(-LL2, NN);

    float spectral = clamp(dot(E, R), 0,1);
    float spectral2 = clamp(dot(E,R2), 0, 1);

    //Combining results
    vec3 result = 
            (texColor * objectColor) +
            (ambient) + 
            (diffuse * lightPower *lightColor1)/(distance1*distance1)+(diffuse2 * lightPower*lightColor2)/(distance2*distance2) +
            (lightColor1 * lightPower * pow(spectral,5))/(distance1*distance1)+(lightColor2 *  lightPower * pow(spectral2,5))/(distance2*distance2);
    
    //Output
    outColor = vec4(result,1);
}
