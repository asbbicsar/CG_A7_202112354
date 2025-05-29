#version 330 core
out vec4 FragColor;

in vec3 wPosition;
in vec3 wNormal;
in vec3 wColor;

uniform vec4 lightPos;
uniform vec4 lightColor; 
uniform vec4 diffuseColor; 
uniform vec4 specularColor; 
uniform float intensity; 
uniform float gamma; 

vec4 Shading(vec3 wPosition, vec3 wNormal, vec3 wColor);

void main()
{
    vec4 linearColor = Shading(wPosition, wNormal, wColor);
    vec3 gammaCorrected = pow(linearColor.rgb, vec3(1.0 / gamma));
    FragColor = vec4(gammaCorrected, linearColor.a);
}

vec4 Shading(vec3 wPosition, vec3 wNormal, vec3 wColor)
{
    vec4 ambient = vec4(intensity * wColor, 1.0f); 
    float shininess = 32.0; 
    vec3 N = normalize(wNormal); 
    vec3 L = normalize((vec3(lightPos) - wPosition)); 
    vec3 V = normalize(-wPosition);
    vec3 H = normalize(L + V);
    float diff = max(dot(N, L), 0.0f); 
    float spec = pow(max(dot(N, H), 0.0f), shininess); 
    vec4 diffuse = lightColor * diffuseColor * diff; 
    vec4 specular = lightColor * specularColor * spec;
    return ambient + diffuse + specular;
}