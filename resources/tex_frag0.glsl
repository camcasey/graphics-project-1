#version 330 core
uniform sampler2D Texture0;
uniform vec3 MatAmb;
uniform vec3 MatDif;
uniform vec3 MatSpec;
uniform float MatShine;
uniform int flip;

in vec2 vTexCoord;
in vec3 fragNor;
in vec3 lightDir;

out vec4 Outcolor;

void main() {
	vec3 ambient = vec3(0.9059, 0.8824, 0.8824);

  
	vec3 normal = normalize(fragNor) * flip;
	vec3 light = normalize(lightDir);
	// float dCo = max(0, dot(normal, light));
	vec4 texColor0 = (texture(Texture0, vTexCoord));
  	//to set the out color as the texture color 
	//texColor0 = texture(Texture0, vTexCoord);
  	Outcolor = texColor0;
	  
  
  	//to set the outcolor as the texture coordinate (for debugging)
	//Outcolor = vec4(vTexCoord.s, vTexCoord.t, 0, 1);
}

