#version 410 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

vec3 lightPos;
vec3 lightColor;
uniform vec3 viewPos;
uniform sampler2D defaultTexture;
uniform sampler2D normalMap;
uniform sampler2D specularMap;
uniform sampler2D nightMap;
uniform sampler2D clouds;

void main()
{
  lightPos = vec3(0.0f);
  lightColor = vec3(1.0f);
  vec3 color = texture(defaultTexture, TexCoords).rgb;

  vec3 L = normalize(lightPos - FragPos);
  vec3 N = normalize(Normal);
  vec3 V = normalize(viewPos - FragPos);
  vec3 H = normalize(L + V);

  // Ambient
  float ambient_intensity = 0.2;
  vec3 ambient = ambient_intensity * lightColor;

  // Diffuse
  float diffuse_intensity = max(dot(N, L), 0.0);
  vec3 diffuse = diffuse_intensity * lightColor;

  // Specular
  float specular_intensity = pow(max(dot(N, H), 0.0f), 4.0);
  vec3 specular = specular_intensity * (lightColor * 0.5) * texture(specularMap, TexCoords).rgb;


  vec3 result = (ambient + diffuse + specular) * color;
  FragColor = vec4(result, 1.0f);
}
