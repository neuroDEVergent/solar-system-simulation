#version 410 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;
in vec4 FragPosLightSpace;

vec3 lightPos;
vec3 lightColor;
uniform vec3 viewPos;
uniform sampler2D diffuseTexture;
uniform sampler2D shadowMap;

float ShadowCalculation(vec4 FragPosLightSpace)
{
  // Perspective division
  vec3 ProjCoords = FragPosLightSpace.xyz / FragPosLightSpace.w;
  vec2 UV;
  UV.x = 0.5 * ProjCoords.x + 0.5;
  UV.y = 0.5 * ProjCoords.y + 0.5;
  float z = 0.5 * ProjCoords.z + 0.5;
  float depth = texture(shadowMap, UV).r;
  if (depth < z - 0.00001) return 0.0;
  else return 1.0;
}

void main()
{
  lightPos = vec3(0.0f);
  lightColor = vec3(1.0f);
  vec3 color = texture(diffuseTexture, TexCoords).rgb;

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
  vec3 specular = specular_intensity * (lightColor * 0.1f);

  float shadow = ShadowCalculation(FragPosLightSpace);

  vec3 result = (ambient + shadow * (diffuse + specular)) * color;
  FragColor = vec4(result, 1.0f);
}
