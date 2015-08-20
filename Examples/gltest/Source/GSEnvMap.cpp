#include <AmjuGL.h>
#include <ResourceManager.h>
#include <Texture.h>
#include <Timer.h>
#include "GSEnvMap.h"
#include "GSFinish.h"

namespace Amju
{
GSEnvMap::GSEnvMap()
{
  m_name = "Sphere map";
}

void GSEnvMap::DrawScene()
{
  static Shader* shader = AmjuGL::LoadShader("Shaders/" + AmjuGL::GetShaderDir() + "/normal-mapped");
  shader->Begin(); // so we find attrib var locations when we build tri list:

  static Texture* diffuseMap = 
    (Texture*)TheResourceManager::Instance()->GetRes("spheremap_bar.png");
  
//  static Texture* normalMap = (Texture*)TheResourceManager::Instance()->GetRes("16_face.png"); 

//  diffuseMap->SetTextureType(AmjuGL::AMJU_TEXTURE_SPHERE_MAP);
  diffuseMap->UseThisTexture(0);
//  normalMap->UseThisTexture(1);

//  shader->Set("normalMap", (AmjuGL::TextureHandle)1);
  shader->Set("diffuseMap", (AmjuGL::TextureHandle)0);

  DrawTeapot();

  shader->End();
}

} // namespace

