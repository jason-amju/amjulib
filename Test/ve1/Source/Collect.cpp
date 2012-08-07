#include "Collect.h"
#include <Timer.h>
#include <GameObjectFactory.h> 
#include <File.h>
#include <ResourceManager.h>
#include <SceneMesh.h>
#include "Useful.h"
#include "SetObjMeshCommand.h"
#include "Terrain.h"
#include "HasCollisionMesh.h"
#include "LocalPlayer.h"

namespace Amju
{
class SceneCollectMesh : public SceneMesh
{
public:
  void SetTexture(Texture* tex) { m_tex = tex; }

  virtual void Draw()
  {
    m_tex->UseThisTexture();
    AmjuGL::Enable(AmjuGL::AMJU_LIGHTING);
    SceneMesh::Draw();
  }

private:
  RCPtr<Texture> m_tex;
};

const char* Collect::TYPENAME = "collect";

// AABB size
static const float XSIZE = 5.0f; // TODO CONFIG
static const float YSIZE = 10.0f;

static const float GRAVITY = -200.0f;

GameObject* CreateCollect()
{
  return new Collect;
}

static bool registered = TheGameObjectFactory::Instance()->Add(Collect::TYPENAME, CreateCollect);

Collect::Collect()
{
  m_rotvel = 10.0f;
  m_timer = 0;
}

const char* Collect::GetTypeName() const
{
  return TYPENAME;
}

void Collect::OnPlayerCollision(Player* p)
{
  if (m_timer < 1.0f)
  {
    return;
  }

  if (p != GetLocalPlayer())
  {
std::cout << "Non local player intersects " << *this << "\n";
    return;
  }

  // This object goes out of play.
  

  // Show effect.
  // Send msg to server that we have collected this.
std::cout << "Collected " << *this << "!!!\n";

  SetHidden(true); 
}

void Collect::OnBounceStop()
{
  SetVel(Vec3f());
}

static float rnd(float min, float max)
{
  float r = (float)rand() / (float)RAND_MAX;
  r *= (max - min);
  r += min;
  return r;
}

void Collect::OnLocationEntry()
{
  Ve1Object::OnLocationEntry();

  Vec3f vel(rnd(-20.0f, 20.0f), rnd(100.0f, 200.0f), rnd(-20.0f, 20.0f));
  SetVel(vel);
}

void Collect::Update()
{
  m_acc.y = GRAVITY;
  float dt = TheTimer::Instance()->GetDt(); 
  m_timer += dt;
  if (m_timer > 10.0f) // TODO CONFIG
  {
    // Start flashing.
    bool vis = (m_timer - (int)m_timer) > 0.5f;
    m_sceneNode->SetVisible(vis);

    if (m_timer > 15.0f) // TODO CONFIG
    {
      SetHidden(true);
    }
  }

  Ve1Object::Update();

  // Handle wall collisions with terrain and any building
  HandleWalls(GetTerrain()->GetCollisionMesh(), m_oldPos, m_pos);

  HasCollisionMesh* h = dynamic_cast<HasCollisionMesh*>(m_collidingObject);
  if (h)
  {
    HandleWalls(h->GetCollisionMesh(), m_oldPos, m_pos);
  }

  HandleFloor(GetTerrain()->GetCollisionMesh());
  if (h)
  {
    HandleFloor(h->GetCollisionMesh());
  }

  m_aabb.Set(
    m_pos.x - XSIZE, m_pos.x + XSIZE,
    m_pos.y, m_pos.y + YSIZE,
    m_pos.z - XSIZE, m_pos.z + XSIZE);

  if (m_sceneNode)
  {
    *(m_sceneNode->GetAABB()) = m_aabb;

    Matrix m;
    
    m_rot += m_rotvel * dt;
    m_rotvel *= 0.999f;
    m.RotateY(m_rot);

    m.TranslateKeepRotation(m_pos);

    m_sceneNode->SetLocalTransform(m);

    if (m_shadow)
    {
      // Set shadow AABB to same as Scene Node so we don't cull it by mistake
      *(m_shadow->GetAABB()) = *(m_sceneNode->GetAABB());
    }
  }
}

bool Collect::Load(File* f)
{
//  Assert(0);
  return false;
}

bool Collect::Create(const std::string& objFilename, const std::string& textureFilename)
{
  // Create Scene Node, but don't attach to SceneGraph until needed

  // TODO CONFIG
  m_shadow->SetSize(15.0f);
  Texture* tex = (Texture*)TheResourceManager::Instance()->GetRes("shadow.png");
  Assert(tex);
  m_shadow->SetTexture(tex);

  ObjMesh* mesh = (ObjMesh*)TheResourceManager::Instance()->GetRes(objFilename);
  if (!mesh)
  {
std::cout << "Bad mesh for " << *this << ": " << objFilename << "\n";
    return false;
  }
  Assert(mesh);
  SceneCollectMesh* sm = new SceneCollectMesh;
  sm->SetMesh(mesh);
  tex = (Texture*)TheResourceManager::Instance()->GetRes(textureFilename);
  Assert(tex);
  sm->SetTexture(tex);

  m_sceneNode = sm;

  return true;
}

void Collect::SetEditMenu(GuiMenu* menu)
{
  menu->AddChild(new GuiMenuItem("Set obj mesh...", new SetObjMeshCommand(GetId())));
}
} // namespace