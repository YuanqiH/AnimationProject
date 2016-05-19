#version 400

in vec3 texcoords;
uniform samplerCube tex_cube;
out vec4 frag_colour;

void main () {
  frag_colour = texture (tex_cube, texcoords);
}