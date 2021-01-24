#version 330 core
out vec4 fragColor;
in vec2 texCoord;
in vec3 normal;
in vec3 fragPos;

struct Material {
    vec3 ambient;	// Color for ambient lighting
    vec3 diffuse;	// Color for diffuse lighting
    vec3 specular;	// Color for specular lighting
    float shininess;
}; 
  
uniform Material material;


uniform sampler2D texImg0;
uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightSrcPos;
uniform vec3 viewPos;
uniform float DEBUG_power;

void main()
{
	// Ambient light
	float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

	// Diffuse light
	vec3 norm = normalize(normal);
	vec3 lightDir = normalize(lightSrcPos - fragPos); 
	float diffu = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diffu * lightColor;

	// Specular light
	float specularStrength = 1;
	vec3 viewDir = normalize(viewPos - fragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), DEBUG_power);
	vec3 specular = specularStrength * spec * lightColor;  

    vec3 result = (ambient + diffuse + specular) * objectColor;

	fragColor = vec4(result, 1.0);
	//fragColor = vec4(objectColor, 1.0);
	//fragColor = vec4( result, 1.0) * texture(texImg0, texCoord);
    // fragColor = vec4(0.7);

}