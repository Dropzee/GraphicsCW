#version 150
uniform sampler2D diffuseTex;

in Vertex {
	vec2 texCoord;
} IN;

out vec4 fragColour;

void main(void){
	fragColour = texture(diffuseTex, IN.texCoord);
}