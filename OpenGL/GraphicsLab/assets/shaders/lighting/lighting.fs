#version 450 core
in VS_OUT
{
    vec3 pos;
    vec3 normal;
    vec2 texCoord;
} fs_in;

// layout(binding = 0): >= 420
layout(binding = 0) uniform sampler2D u_texDiffuse;
layout(binding = 1) uniform sampler2D u_texSpecular;

struct Material {
    float shininess;
}; 
uniform Material u_material;

struct DirLight {
    vec3 direction;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
  
    float constant;
    float linear;
    float quadratic;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;       
};

#define NR_POINT_LIGHTS 4

uniform DirLight u_dirLight;
uniform PointLight u_pointLights[NR_POINT_LIGHTS];
uniform SpotLight u_spotLight;

// Light properties
uniform vec3 u_lightPos;  // Position of the light source
uniform vec3 u_lightColor; // Color of the light source
uniform vec3 u_viewPos;   // Position of the camera/viewer

out vec4 FragColor;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
void main()
{
    // properties
    vec3 norm = normalize(fs_in.normal);
    vec3 viewDir = normalize(u_viewPos - fs_in.pos);
    
    // == =====================================================
    // Our lighting is set up in 3 phases: directional, point lights and an optional flashlight
    // For each phase, a calculate function is defined that calculates the corresponding color
    // per lamp. In the main() function we take all the calculated colors and sum them up for
    // this fragment's final color.
    // == =====================================================
    // phase 1: directional lighting
    vec3 result = CalcDirLight(u_dirLight, norm, viewDir);
    // phase 2: point lights
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalcPointLight(u_pointLights[i], norm, fs_in.pos, viewDir);    
    // phase 3: spot light
    result += CalcSpotLight(u_spotLight, norm, fs_in.pos, viewDir);    
    
    FragColor = vec4(result, 1.0);
}



// calculates the color when using a directional light.
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_material.shininess);
    // combine results
    vec3 ambient = light.ambient * vec3(texture(u_texDiffuse, fs_in.texCoord));
    vec3 diffuse = light.diffuse * diff * vec3(texture(u_texDiffuse, fs_in.texCoord));
    vec3 specular = light.specular * spec * vec3(texture(u_texSpecular, fs_in.texCoord));
    return (ambient + diffuse + specular);
}


// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // combine results
    vec3 ambient = light.ambient * vec3(texture(u_texDiffuse, fs_in.texCoord));
    vec3 diffuse = light.diffuse * diff * vec3(texture(u_texDiffuse, fs_in.texCoord));
    vec3 specular = light.specular * spec * vec3(texture(u_texSpecular, fs_in.texCoord));
    //ambient *= attenuation;
    //diffuse *= attenuation;
    //specular *= attenuation;
    return (ambient + diffuse + specular);
}

// calculates the color when using a spot light.
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // combine results
    vec3 ambient = light.ambient * vec3(texture(u_texDiffuse, fs_in.texCoord));
    vec3 diffuse = light.diffuse * diff * vec3(texture(u_texDiffuse, fs_in.texCoord));
    vec3 specular = light.specular * spec * vec3(texture(u_texSpecular, fs_in.texCoord));
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);
}