#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float exposure;
uniform float avgLum;

void main()
{    
  float gamma = 2.2;
  vec3 color = texture(screenTexture, TexCoords).rgb;

  const float targetGray = 0.18;
  float bias = 0.03;
  float minLum = 0.01;
  float maxLum = 1.0;

  float expo = targetGray / clamp(avgLum + bias, minLum, maxLum);

//  vec3 mapped = vec3(1.0) - exp(-color * exposure);
  vec3 mapped = color * exposure;
  mapped = mapped / (mapped + vec3(1.0f));

  mapped = pow(mapped, vec3(1.0/gamma));


  FragColor = vec4(mapped, 1.0f);
}
