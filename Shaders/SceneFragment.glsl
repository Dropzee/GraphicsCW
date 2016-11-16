#version 150 core

uniform sampler2D diffuseTex;
uniform int useTexture;

in Vertex {
	vec2 texCoord;
	vec4 colour;
} IN;

out vec4 fragColor;

void main ( void ) {
	fragColor = IN.colour;
	if( useTexture > 0) {
		fragColor *= texture ( diffuseTex , IN.texCoord );
	}
}
