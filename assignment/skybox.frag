#version 120 // Use GLSL version 1.20

varying vec3 TexCoords; // Receive coordinate from vertex shader

uniform samplerCube skybox; // Cubemap sampler uniform

void main() {
    // Sample the cubemap using the interpolated vertex position
    gl_FragColor = textureCube(skybox, TexCoords);
}

