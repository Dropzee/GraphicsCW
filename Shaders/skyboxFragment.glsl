#version 150 core

uniform samplerCube cubeTex;
uniform vec3 cameraPos;

in Vertex {
	vec3 normal;
} IN;

out vec4 fragColoir;

void main ( void ) {
	fragColoir = texture ( cubeTex , normalize ( IN.normal ));
}
