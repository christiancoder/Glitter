#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 itModel;

out vec3 fromVtxPos;
out vec3 fromVtxNormal;
out vec2 fromVtxTexCoords;

void main()
{
    gl_Position = projection * view * model * vec4( aPos, 1.0 );
    fromVtxPos = (model * vec4( aPos, 1.0 )).xyz;
    fromVtxNormal = itModel * aNormal;
    fromVtxTexCoords = aTexCoords;
}

