#version 150

uniform mat4 projection_matrix;
uniform mat4 model_matrix;
uniform vec4 LightPosition;
uniform vec4 SpotLightPosition;
uniform vec4 SpotLightDirection;
uniform float SpotLightAngle;

in  vec4 vPosition;
in  vec4 vColor;
in	vec4 vNormal;
flat out vec4 color;

void main() 
{
  gl_Position = projection_matrix * model_matrix * vPosition;

  vec3 pos = (model_matrix * vPosition).xyz;

  vec3 L = normalize(LightPosition.xyz - pos);
  vec4 norm = vNormal;
  norm.a = 0.0;
  vec3 N = normalize(model_matrix * norm).xyz;

  float lambertian = max(dot(L, N), 0.0);
  vec4 diffuse = vColor * lambertian;

  vec3 spotDir = SpotLightDirection.xyz;
  vec3 spotToSurf = pos - SpotLightPosition.xyz;
  float lightToSurfaceAngle = degrees(acos(dot(normalize(spotToSurf), normalize(spotDir))));
  float diff = SpotLightAngle - lightToSurfaceAngle;
  float spot = max(diff / abs(diff), 0.0);

  vec4 light = vColor * spot;

  color = diffuse + light;
  color.a = 1.0;
} 
