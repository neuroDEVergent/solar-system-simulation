#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float exposure;

void main()
{    
  float gamma = 1.7;
  vec3 color = texture(screenTexture, TexCoords).rgb;

  vec3 result = vec3(1.0) - exp(-color * exposure);
  result = pow(result, vec3(1.0/gamma));


  FragColor = vec4(result, 1.0f);
}
