//====================================================
// Lesson4: Rasterization Stage
//====================================================

#version 330 core

//====================================================

// Uncomment this to use vertex lighting
#define VERTEX_LIGHTING

//====================================================

const vec3 ambientColor = vec3( 0.25 );
const float screenGamma = 2.2;
uniform sampler2D texture_diffuse1;
in vec2 fromVtxTexCoords;
out vec4 fromFragColor;

//====================================================
// Vertex Lighting Mode
//====================================================
#if defined VERTEX_LIGHTING
//====================================================

in vec3 fromVtxDiffuseColor;
in vec3 fromVtxSpecularColor;

//====================================================

void main()
{
    vec3 txtrClr = pow( texture( texture_diffuse1, fromVtxTexCoords ).rgb, vec3( screenGamma ) );
    fromFragColor.rgb = ((ambientColor + fromVtxDiffuseColor) * txtrClr) + fromVtxSpecularColor;
    //fromFragColor.rgb = (ambientColor + fromVtxDiffuseColor) + fromVtxSpecularColor + (txtrClr * 0.001);

    fromFragColor.rgb = pow(fromFragColor.rgb, vec3(1.0/screenGamma));
    fromFragColor.w = 1.0;
}

//====================================================
// Fragment Lighting Mode
//====================================================
#else
//====================================================

const int numLights = 10;
uniform vec3 lightPositions[numLights];
uniform vec3 lightColors[numLights];
uniform float lightRadii[numLights];
uniform vec3 cameraPos;
uniform float shininess;
uniform float diffuseScale;
uniform float specularScale;
in vec3 fromVtxPos;
in vec3 fromVtxNormal;

//====================================================

void handlePointLight( in out vec3 diffuseColor, in out vec3 specularColor, vec3 vertPos, vec3 vertNormal, vec3 lightPos, vec3 lightColor, float lightRadius )
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

    diffuseColor += lightColor * diffuse;
    specularColor += lightColor * specular;
}

//====================================================

void main()
{
    vec3 wsNormal = normalize( fromVtxNormal );
    vec3 diffuseColor = vec3( 0.0 );
    vec3 specularColor = vec3( 0.0 );
    for (int i = 0; i < numLights; i++)
    {
        handlePointLight( diffuseColor, specularColor, fromVtxPos, wsNormal, lightPositions[i], lightColors[i], lightRadii[i] );
    }
    diffuseColor *= diffuseScale;
    specularColor *= specularScale;

    vec3 txtrClr = pow( texture( texture_diffuse1, fromVtxTexCoords ).rgb, vec3( screenGamma ) );
    fromFragColor.rgb = ((ambientColor + diffuseColor) * txtrClr) + specularColor;
    //fromFragColor.rgb = (ambientColor + diffuseColor) + specularColor + (txtrClr * 0.001);

    fromFragColor.rgb = pow(fromFragColor.rgb, vec3(1.0/screenGamma));
    fromFragColor.w = 1.0;
}

//====================================================

#endif

//====================================================
