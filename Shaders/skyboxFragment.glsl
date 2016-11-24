#version 150 core

uniform samplerCube cubeTex;
uniform samplerCube cubeTex2;
uniform float blendFactor;
uniform vec3 cameraPos;

in Vertex {
	vec3 normal;
} IN;

out vec4 fragColor;

void main ( void ) {
	vec3 tex1 = texture (cubeTex, normalize(IN.normal)).rgb;
	vec3 tex2 = texture (cubeTex2, normalize(IN.normal)).rgb;
	fragColor.rgb = mix(tex1, tex2, blendFactor);
	fragColor.a = 1.0f;
}
