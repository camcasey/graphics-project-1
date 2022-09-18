#version 330 core

uniform sampler2D alphaTexture;

in vec3 partCol;
in vec4 vCol;

out vec4 outColor;


void main()
{
	float alpha = texture(alphaTexture, gl_PointCoord).r + vCol.a;

	outColor = vec4(vec3(vCol), alpha);
}
