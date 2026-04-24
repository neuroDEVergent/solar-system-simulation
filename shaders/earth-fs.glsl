#version 410 core
out vec4 FragColor;

in vec3 FragPos;
in vec2 TexCoords;
in vec3 TangentLightPos;
in vec3 TangentViewPos;
in vec3 TangentFragPos;

vec3 lightColor;
uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform sampler2D specularMap;
uniform sampler2D nightMap;

void main()
{
  lightColor = vec3(1.0f);
  vec3 color = texture(diffuseMap, TexCoords).rgb;
  vec3 normal = texture(normalMap, TexCoords).rgb;
  vec3 night = texture(nightMap, TexCoords).rgb;

  vec3 L = normalize(TangentLightPos - TangentFragPos);
  vec3 N = normalize(normal * 2.0 - 1.0);
  vec3 V = normalize(TangentViewPos - TangentFragPos);
  vec3 H = normalize(L + V);

  // Ambient
  float ambient_intensity = 0.0;
  vec3 ambient = ambient_intensity * lightColor;

  // Diffuse
  float diffuse_intensity = max(dot(N, L), 0.0);
  vec3 diffuse = diffuse_intensity * lightColor;

  // Specular
  float specular_intensity = pow(max(dot(N, H), 0.0f), 4.0);
  vec3 specular = specular_intensity * (lightColor * 0.5) * texture(specularMap, TexCoords).rgb;

  float dayFactor = smoothstep(0.0, 0.1, diffuse_intensity);

  vec3 day = (ambient + diffuse + specular) * color;

  vec3 result = mix(night, day, dayFactor);

  FragColor = vec4(result, 1.0f);
}
