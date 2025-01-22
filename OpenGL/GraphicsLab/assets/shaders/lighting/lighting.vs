#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

out VS_OUT
{
    vec3 pos;
    vec3 normal;
    vec2 texCoord;
} vs_out;

void main() 
{
    gl_Position = u_projection * u_view * u_model * vec4(aPos, 1.0f);

    // calculate world space position and normal
    vs_out.pos = vec3(u_model * vec4(aPos, 1.0));
    // Correct normal for scaling;
    vs_out.normal = mat3(transpose(inverse(u_model))) * aNormal;
    vs_out.texCoord = aTexCoord;
}