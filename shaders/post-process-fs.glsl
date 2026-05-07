#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float exposure;

void main()
{    
  const float gamma = 2.2;
  vec3 color = texture(screenTexture, TexCoords).rgb;

  vec3 mapped = color / (color + vec3(1.0f));
  mapped = vec3(1.0f) - exp(-color * exposure);
  mapped = pow(mapped, vec3(1.0/gamma));



  FragColor = vec4(mapped, 1.0f);
}
