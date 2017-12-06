
// This pixel shader basically says: "I don't care about anything else,
// just paint this pixel (aka "fragment") white no matter what!"

uniform float currentTime;

void main() 
{
  float sinWave = sin(currentTime);
  if (sinWave < 0.0)
    sinWave *= -1.0;
	gl_FragColor = vec4(sinWave*1.0, 0.0, 0.0, 1.0);
}

