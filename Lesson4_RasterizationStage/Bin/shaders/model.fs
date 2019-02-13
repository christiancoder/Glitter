#version 330 core

in vec3 fromVtxPos;
in vec3 fromVtxNormal;
in vec2 fromVtxTexCoords;

const int numLights = 10;
const float shininess = 70.0;
const float screenGamma = 2.2;

uniform sampler2D texture_diffuse1;
uniform vec3 cameraPos;

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
        diffuse *= atten;
        specular *= atten;
    }

    vec3 colorLinear = (lightColor * diffuse) + (lightColor * specular);
    //return colorLinear;

    // apply gamma correction (assume ambientColor, diffuseColor and specColor
    // have been linearized, i.e. have no gamma correction in them)
    vec3 colorGammaCorrected = pow(colorLinear, vec3(1.0/screenGamma));
    return colorGammaCorrected;
}

void main()
{
    vec3 wsNormal = normalize( fromVtxNormal );
    vec3 lightClr = vec3( 0.25 );
    for (int i = 0; i < numLights; i++)
    {
        lightClr += handlePointLight( fromVtxPos, wsNormal, lightPositions[i], lightColors[i], lightRadii[i] );
    }

    vec3 txtrClr = texture( texture_diffuse1, fromVtxTexCoords ).rgb;
    fromFragColor.rgb = lightClr * txtrClr * 2.5;
    //fromFragColor.rgb = lightClr + (txtrClr * 0.001);
    fromFragColor.w = 1.0;
}
