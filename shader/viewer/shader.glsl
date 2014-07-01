//Vertex Shader
#shader(vs, vsmain)
//--------------------------------------------------------------------------------------

uniform vec4 minMaxBound;

out vec3 viewVector;
out vec3 lightVector;
out vec3 normalVertex;
out vec4 frontColor;

void main() {
  //norm texture coordinates to bounds of the terrain
  float xNorm = 1.0f / (minMaxBound.z - minMaxBound.x);
  float yNorm = 1.0f / (minMaxBound.w - minMaxBound.y);

  //calculate texture coordinates through normalized coordinates
  gl_TexCoord[0]  = vec4((gl_Vertex.x - minMaxBound.x) * xNorm, (gl_Vertex.y - minMaxBound.y) * yNorm, 0, 0);

  //calculate view vector
  normalVertex   = normalize(gl_NormalMatrix * gl_Normal);
  viewVector     = vec3(gl_ModelViewMatrix * gl_Vertex);
  lightVector    = normalize(gl_LightSource[0].position.xyz);
 
  frontColor     = gl_Color;
 
  gl_Position    = gl_ModelViewProjectionMatrix * gl_Vertex;
}

//Fragment Shader
#shader(fs, fsmain)
//--------------------------------------------------------------------------------------


uniform sampler2D normalMap;
uniform sampler2D aoMap;
uniform sampler2D terrainTexture;

in vec3 viewVector;
in vec3 lightVector;
in vec3 normalVertex;
in vec4 frontColor;

void main()
{
  //textures
  vec4 normalTexel    = texture2D(normalMap,gl_TexCoord[0].st) * 2.0 - 1.0;
  vec4 aoTexel        = texture2D(aoMap,gl_TexCoord[0].st) * 0.5;
  vec4 terrainTexel   = texture2D(terrainTexture, gl_TexCoord[0].st);
  
  vec3 texNormal      = normalize( vec3(normalTexel) );

  vec3 eye       = normalize(-viewVector);
  vec3 reflected = normalize(reflect( -lightVector, normalVertex)); 
  
  float lambert  = max(dot(texNormal, lightVector), 0.0);
  float specular = pow( clamp(dot(reflected, eye), 0.0, 1.0), gl_FrontMaterial.shininess );
  
  vec4 IAmbient  = (gl_LightSource[0].ambient * gl_FrontMaterial.ambient) + (gl_LightSource[0].ambient * aoTexel);
  vec4 IDiffuse  = gl_LightSource[0].diffuse * lambert * gl_FrontMaterial.diffuse;
  vec4 ISpecular = gl_LightSource[0].specular * specular * gl_FrontMaterial.specular;
  ISpecular = max(ISpecular, vec4(0.0,0.0,0.0,0.0));
  //gl_FragColor   = vec4((gl_FrontLightModelProduct.sceneColor + IAmbient + IDiffuse) + ISpecular);
  gl_FragColor   = vec4( ((gl_FrontLightModelProduct.sceneColor + IAmbient + IDiffuse) * vec3(terrainTexel)) + ISpecular); 
  //gl_FragColor   = terrainTexel;
}
