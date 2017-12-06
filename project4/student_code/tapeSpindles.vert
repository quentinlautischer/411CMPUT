
// A super simple vertex shader that just mimics exactly what OpenGL's 
// default fixed pipeline does

uniform float currentTime;

void main() 
{
	vec3 normal, lightDir;
  vec4 diffuse, ambient, globalAmbient;
  float NdotL;

  normal = normalize(gl_NormalMatrix * gl_Normal);
  lightDir = normalize(vec3(gl_LightSource[0].position));
  NdotL = max(dot(normal, lightDir), 0.0);
  diffuse = gl_FrontMaterial.diffuse * gl_LightSource[0].diffuse;

  ambient = gl_FrontMaterial.ambient * gl_LightSource[0].ambient;
  gl_FrontColor =  (NdotL * diffuse) + ambient;

  gl_Position = ftransform();

  vec4 oldSpindle = gl_MultiTexCoord0;
  vec4 spindle = gl_MultiTexCoord0;
  spindle.x = -sin(currentTime)*oldSpindle.y + cos(currentTime)*oldSpindle.x;
  spindle.y = sin(currentTime)*oldSpindle.x + cos(currentTime)*oldSpindle.y;

  gl_TexCoord[0] = spindle;
}

