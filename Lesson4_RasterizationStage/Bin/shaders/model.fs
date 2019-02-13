#version 330 core

in vec3 fromVtxPos;
in vec3 fromVtxNormal;
in vec2 fromVtxTexCoords;

const int numLights = 10;
const float screenGamma = 2.2;

uniform sampler2D texture_diffuse1;
uniform vec3 cameraPos;
uniform float shininess;

uniform vec3 lightPositions[numLights];
uniform vec3 lightColors[numLights];
uniform float lightRadii[numLights];

out vec4 fromFragColor;

vec3 handlePointLight( vec3 vertPos, vec3 vertNormal, vec3 lightPos, vec3 lightColor, float lightRadius )
{
    vec3 lightDir = lightPos - vertPos;
    float distance = length(lightDir);
    lightDir = normalize(lightDir);

    float diffuse = max(dot(lightDir,vertNormal), 0.0);
    float specular = 0.0;

    if(diffuse > 0.0)
    {
        vec3 viewDir = normalize(cameraPos - vertPos);
        vec3 halfDir = normalize(lightDir + viewDir);
        float specAngle = max(dot(halfDir, vertNormal), 0.0);
        specular = pow(specAngle, shininess);

        float atten = 1.0 - min( distance, lightRadius ) / lightRadius;
        //float atten = 1.0 / (distance * distance);
        diffuse *= atten;
        specular *= atten;
    }

    return ((lightColor * diffuse) + (lightColor * specular)) * 10.0;
}

void main()
{
    vec3 wsNormal = normalize( fromVtxNormal );
    vec3 lightClr = vec3( 0.25 );
    for (int i = 0; i < numLights; i++)
    {
        lightClr += handlePointLight( fromVtxPos, wsNormal, lightPositions[i], lightColors[i], lightRadii[i] );
    }

    vec3 txtrClr = pow( texture( texture_diffuse1, fromVtxTexCoords ).rgb, vec3( screenGamma ) );
    fromFragColor.rgb = lightClr * txtrClr;
    //fromFragColor.rgb = lightClr + (txtrClr * 0.001);

    fromFragColor.rgb = pow(fromFragColor.rgb, vec3(1.0/screenGamma));
    fromFragColor.w = 1.0;
}
