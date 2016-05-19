#version 400

in vec3 vp;
uniform mat4 projectionMat, viewMat;
out vec3 texcoords;

void main () {
  texcoords = vp;
  
	mat4 viewn = mat4(mat3(viewMat)); // zero all the translation part
  //gl_Position = projectionMat * viewMat * vec4 (vp, 1.0);
  gl_Position = projectionMat * viewn * vec4 (vp, 1.0);
}