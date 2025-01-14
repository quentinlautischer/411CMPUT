
uniform sampler2D textureSample_0;
uniform sampler2D textureSample_1;
uniform float currentTime;

void main() 
{
  float sinWave = sin(currentTime);
  if (sinWave < 0.0)
    sinWave *= -1.0;
  vec4 light = texture2D(textureSample_1, gl_TexCoord[0].st) * vec4(sinWave*1.0, 0.0, 0.0, 1.0);

	gl_FragColor = gl_Color * texture2D(textureSample_0, gl_TexCoord[0].st) + light;
}

