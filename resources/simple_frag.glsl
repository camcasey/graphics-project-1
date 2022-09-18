#version 330 core 

out vec4 color;

uniform vec3 MatAmb;
uniform vec3 MatDif;
uniform vec3 MatSpec;
uniform float MatShine;


//interpolated normal and light vector in camera space
in vec3 fragNor;
in vec3 lightDir;
//position of the vertex in camera space
in vec3 EPos;

//define and find Kd
vec3 intensity;



//RENORMALIZE


void main()
{
	//you will need to work with these for lighting
	vec3 normal = normalize(fragNor);
	vec3 light = normalize(lightDir);
	vec3 halfwayVec = normalize(lightDir + EPos);
	
	float a = 0.9;
	float b = 0.09;
	float c = 0.032;
	float d = distance(light, normal);

	//need distance between light and point

	float distnaceAttenuation = 1 / (a + (b * d) + (c * d * d));
	intensity.xyz = vec3(1.0, 1.0, 1.0);
	
	vec3 diffuse = max(dot(normal, light), 0.0) * intensity;
														//VVV how shiny var
	vec3 specular = pow(max(dot(normal, halfwayVec), 0.0), MatShine)  * intensity;

	vec3 Ncolor = 0.5 * normal + 0.1;
	color = vec4(MatAmb + (diffuse * MatDif) + (specular * MatSpec), 1.0);
	//color = vec4(Ncolor, 1.0);
}
