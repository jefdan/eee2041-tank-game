#version 120

uniform vec4    Ambient_uniform;
uniform vec4    Specular_uniform;
uniform float   SpecularPower_uniform;
uniform vec3    Colour_uniform;
uniform vec3    LightColor_uniform; // Added for light color
uniform sampler2D TextureMap_uniform; // Added texture sampler

varying vec3    ViewDirection;
varying vec3    LightDirection;
varying vec3    Normal;
varying vec2    Texcoord; // Added varying for texture coordinates

void main( void )
{
   vec3  fvLightDirection = normalize(LightDirection );
   vec3  fvNormal         = normalize( Normal );
   float fNDotL           = max(0.0, dot( fvNormal, fvLightDirection )); // Ensure diffuse is not negative

   vec3  fvReflection     = normalize( ( ( 2.0 * fvNormal ) * fNDotL ) - fvLightDirection );
   vec3  fvViewDirection  = normalize( ViewDirection );
   float fRDotV           = max( 0.0, dot( fvReflection, fvViewDirection ) );

   // Sample the texture
   vec4 texColor = texture2D(TextureMap_uniform, Texcoord);

   // Apply light color to diffuse and specular components
   // Modulate base color by texture color before lighting calculation
   vec3 baseColor = Colour_uniform * texColor.rgb;
   vec3  fvTotalDiffuse   = fNDotL * baseColor * LightColor_uniform;
   vec3  fvTotalSpecular  = Specular_uniform.rgb * ( pow( fRDotV, SpecularPower_uniform ) ) * LightColor_uniform;

   // Combine ambient, diffuse, and specular contributions (modulated by texture)
   // Ambient should also be modulated by texture color
   vec3 finalColor = (Ambient_uniform.rgb * texColor.rgb) + fvTotalDiffuse + fvTotalSpecular;

   gl_FragColor = vec4(finalColor, texColor.a); // Use texture alpha
}


