#version 330

in vec3 n_eye;

out vec4 frag_colour;

void main(){
	frag_colour = vec4 (n_eye, 1.0);
}