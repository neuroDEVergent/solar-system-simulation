#version 410 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

uniform float far_plane;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform sampler2D diffuseTexture;
uniform samplerCube shadowMap;

vec3 gridSamplingDisk[20] = vec3[]
(
  vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
  vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
  vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
  vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
  vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);

float shadowCalculation(vec3 fragPos)
{
    vec3 fragToLight = fragPos - lightPos;
    float currentDepth = length(fragToLight);
    float shadow = 0.0;
    float bias = 0.3;
    int samples = 20;
    float viewDistance = length(viewPos - fragPos);
    float diskRadius = (1.0 + (viewDistance / far_plane)) / 25.0;
    for(int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(shadowMap, fragToLight + gridSamplingDisk[i] * diskRadius).r;
        closestDepth *= far_plane;   // undo mapping [0;1]
        if(currentDepth - bias > closestDepth)
            shadow += 1.0;
    }
    shadow /= float(samples);

    return shadow;
}

void main()
{
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

  float shadow = shadowCalculation(FragPos);

  vec3 result = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;

  FragColor = vec4(result, 1.0f);
}
