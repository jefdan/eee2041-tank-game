#version 120 // Use GLSL version 1.20

attribute vec3 aVertexPosition;

uniform mat4 MVMatrix_uniform;
uniform mat4 ProjMatrix_uniform;

varying vec3 TexCoords; // Pass vertex position to fragment shader

void main() {
    TexCoords = aVertexPosition; // Use vertex position as 3D texture coordinate
    vec4 pos = ProjMatrix_uniform * MVMatrix_uniform * vec4(aVertexPosition, 1.0);
    // Ensure skybox is always drawn at the far plane (z = w)
    gl_Position = pos.xyww;
}
