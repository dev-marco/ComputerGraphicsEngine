#version 130

in vec4 vertex;

out vec4 position;  // position of the vertex (and fragment) in world space
out vec3 varyingNormalDirection;  // surface normal vector in world space

void main (void) {
    position = gl_ModelViewMatrix * vertex;
    varyingNormalDirection = normalize(gl_NormalMatrix * gl_Normal.xyz);

    gl_Position = gl_ModelViewProjectionMatrix * vertex;
}
