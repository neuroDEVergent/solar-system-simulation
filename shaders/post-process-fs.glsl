#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

void main()
{    
  /*
  float depth = texture(screenTexture, TexCoords).r;
  depth = 1.0 - (1.0 - depth) * 25.0f;
  FragColor = vec4(depth);
  */

  FragColor = texture(screenTexture, TexCoords);
}
