
uniform sampler2D textureSample_0;
uniform sampler2D textureSample_1;

varying vec4 CoordsCopy;

void main() 
{
  float scale = 20.0;
  vec4 ScaledCoordsCopy = CoordsCopy*scale;
  vec4 sand = texture2D(textureSample_0, ScaledCoordsCopy.st);

  vec4 original_grass = texture2D(textureSample_1, gl_TexCoord[0].st);
  vec4 scaled_grass = texture2D(textureSample_1, ScaledCoordsCopy.st);
  vec4 new_grass = vec4(scaled_grass.x,scaled_grass.y, scaled_grass.z, original_grass.a);

	gl_FragColor = mix(sand,new_grass,new_grass.a);
  gl_FragColor = gl_FragColor * gl_Color * texture2D(textureSample_0, gl_TexCoord[0].st);
}

