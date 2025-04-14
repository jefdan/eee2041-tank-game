#version 120

// Attributes
attribute vec3 aVertexPosition;
attribute vec3 aVertexNormal;
attribute vec2 aVertexTexcoord; // Added texture coordinate attribute

uniform mat4x4 MVMatrix_uniform;
uniform mat4x4 ProjMatrix_uniform;
uniform vec3   LightDirection_uniform; // Renamed from LightPosition_uniform

varying vec3 ViewDirection;
varying vec3 LightDirection;
varying vec3 Normal;
varying vec2 Texcoord; // Added varying for texture coordinates

void main( void )
{
   ViewDirection  = -vec3(MVMatrix_uniform * vec4(aVertexPosition, 1.0));
   // Transform LightDirection_uniform to view space
   LightDirection = (MVMatrix_uniform * vec4(LightDirection_uniform, 0.0)).xyz; // Changed this line
   Normal         = (MVMatrix_uniform * vec4(aVertexNormal,0.0)).xyz;
   Texcoord       = aVertexTexcoord; // Pass texture coordinates

   gl_Position = ProjMatrix_uniform * MVMatrix_uniform * vec4(aVertexPosition,1.0);
}
