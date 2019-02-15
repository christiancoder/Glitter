//====================================================
// Lesson4: Rasterization Stage
//====================================================

#version 330 core

//====================================================

// Uncomment this to use vertex lighting
//#define VERTEX_LIGHTING

//====================================================

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 itModel;

//====================================================
// Vertex Lighting Mode
//====================================================
#if defined VERTEX_LIGHTING
//====================================================

const int numLights = 10;
uniform vec3 lightPositions[numLights];
uniform vec3 lightColors[numLights];
uniform float lightRadii[numLights];
uniform vec3 cameraPos;
uniform float shininess;
uniform float diffuseScale;
uniform float specularScale;
out vec2 fromVtxTexCoords;
out vec3 fromVtxDiffuseColor;
out vec3 fromVtxSpecularColor;

//====================================================

void handlePointLight( out vec3 diffuseColor, out vec3 specularColor, vec3 vertPos, vec3 vertNormal, vec3 lightPos, vec3 lightColor, float lightRadius )
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
    vec3 wsPos = (model * vec4( aPos, 1.0 )).xyz;
    vec3 wsNormal = normalize( itModel * aNormal );
    fromVtxTexCoords = aTexCoords;
    fromVtxDiffuseColor = vec3( 0.0 );
    fromVtxSpecularColor = vec3( 0.0 );
    for (int i = 0; i < numLights; i++)
    {
        handlePointLight( fromVtxDiffuseColor, fromVtxSpecularColor, wsPos, wsNormal, lightPositions[i], lightColors[i], lightRadii[i] );
    }
    fromVtxDiffuseColor *= diffuseScale;
    fromVtxSpecularColor *= specularScale;
    gl_Position = projection * view * vec4( wsPos, 1.0 );
}

//====================================================
// Fragment Lighting Mode
//====================================================
#else

//====================================================

out vec3 fromVtxPos;
out vec3 fromVtxNormal;
out vec2 fromVtxTexCoords;

//====================================================

void main()
{
    fromVtxPos = (model * vec4( aPos, 1.0 )).xyz;
    fromVtxNormal = normalize( itModel * aNormal );
    fromVtxTexCoords = aTexCoords;
    gl_Position = projection * view * vec4( fromVtxPos, 1.0 );
}

//====================================================

#endif

//====================================================
