#include <GameObjectFactory.h>
#include <StringUtils.h>
#include <LoadVec2.h>
#include <AABB.h>
#include "GameConsts.h"
#include "LocalPlayer.h"
#include "ObjectUpdater.h"
#include "Room.h"
#include "RoomNode.h"
#include "SoundManager.h"
#include "UnCollide.h"
#include "Ve1SceneGraph.h"

namespace Amju
{
static Room* s_room = 0;
Room* GetRoom()
{
  Assert(s_room);
  return s_room;
}

std::string MakeRoomFilename(int loc)
{
  return "rooms/room-" + ToString(loc) + ".txt";
}

GameObject* CreateRoom()
{
  return new Room;
}

static bool registered = TheGameObjectFactory::Instance()->Add(Room::TYPENAME, CreateRoom);

Room::Room()
{
  m_tilesize = Vec2f(100, 100);
  m_isPickable = false;
}

Vec2f Room::GetSize() const
{
  return Vec2f(m_tilesize.x * m_gridsize.x, m_tilesize.y * m_gridsize.y);
}

static Vec2f GetRoomSize(int dest)
{
  // Expensive - only done when we are definitely changing room
  std::string filename = MakeRoomFilename(dest);
  File f;
  if (!f.OpenRead(filename))
  {
    Assert(0);
    return Vec2f();
  }
  std::string s;
  if (!f.GetDataLine(&s))
  {
    Assert(0);
    return Vec2f();
  }
  GameObject* go = TheGameObjectFactory::Instance()->Create(s);
  Room* room = dynamic_cast<Room*>(go);
  if (!room)
  {
    Assert(0);
    return Vec2f();
  }
  room->Load(&f);
  return room->GetSize();
}

void Room::Update()
{
  float east = m_tilesize.x * m_gridsize.x * 0.5f;
  float west = -east;
  float south = m_tilesize.y * m_gridsize.y * 0.5f;
  float north = -south;

  float smallDistX = 20.0f; // TODO CONFIG
  float smallDistY = 40.0f; // TODO CONFIG

  bool changeLoc = false;
  int destLocation = 0;
  Vec3f destPos;

  // Check if local player has left room
  Player* p = GetLocalPlayer();
  if (p)
  {
    Vec3f pos = p->GetPos();
    if (pos.x < west + smallDistX)
    {
      int w = GetDest(AMJU_ROOM_W);
      if (w)
      {
        // Go to West room
        changeLoc = true;
        destLocation = w;
        destPos = pos;
        Vec2f size = GetRoomSize(w); // get size of dest room
        destPos.x = size.x * 0.5f - smallDistX; 
      }
    }
    else if (pos.x > east - smallDistX)
    {
      int e = GetDest(AMJU_ROOM_E); 
      if (e)
      {
        // Go to East room
        changeLoc = true;
        destLocation = e;
        destPos = pos;
        Vec2f size = GetRoomSize(e); // get size of dest room
        destPos.x = size.x * -0.5f + smallDistX; 
      }
    }

    if (pos.z > south - smallDistY) // off bottom
    {
      int s = GetDest(AMJU_ROOM_S);
      if (s)
      {
        // Go to West room
        changeLoc = true;
        destLocation = s;
        destPos = pos;
        Vec2f size = GetRoomSize(s); // Get size of dest room
        destPos.z = size.y * -0.5f + smallDistY; 
      }
    }
    else if (pos.z < north + smallDistY) // up, off top of room
    {
      int n = GetDest(AMJU_ROOM_N); 
      if (n)
      {
        // Go to East room
        changeLoc = true;
        destLocation = n;
        destPos = pos;
        Vec2f size = GetRoomSize(n); // Get size of dest room
        destPos.z = size.y * 0.5f - smallDistY; 
      }
    }

    if (changeLoc)
    {
      SetLocalPlayerLocation(destLocation); // LocalPlayer

      // Just do this, and wait for round trip from server. Ensures consistency..?
      TheObjectUpdater::Instance()->SendChangeLocationReq(p->GetId(), destPos, destLocation);

      // Set new position immediately
      p->SetPos(destPos);
      p->MoveTo(destPos); // stop moving ?

      return;
    }

    // Check for collision with obstacles layer (tilemap [1])
    // (Another way could be to create an Obstacle game object for each tile,
    //  but this is linear time anyway.)

    AABB a1 = *(p->GetAABB());
    GameObject* go = p;
    AABB a2 = (const_cast<const GameObject*>(go))->GetAABB();

    TileMap& tm = m_tilemap[1];
    for (TileMap::iterator it = tm.begin(); it != tm.end(); ++it)
    {
      Room::TileVec& tilevec = it->second;
      for (unsigned int i = 0; i < tilevec.size(); i++)
      {
        Vec2f& pos = tilevec[i].m_pos;
        AABB aabb(pos.x * m_tilesize.x, (pos.x + 1) * m_tilesize.x, 
          0, 10, pos.y * m_tilesize.y, (pos.y + 1) * m_tilesize.y);
        if (aabb.Intersects(a1))
        {
          // Slow down if in contact with obstacle??
          //Vec3f vel = p->GetVel();
          //vel *= 0.5f; // TODO Config
          //p->SetVel(vel); 

          // TEMP "FIX" - Uncollide is throwing player too far
          //  (maybe because old pos is (0, 0, 0) ??
          //
//          UnCollide(p, p->GetOldPos(), aabb);
        }
      }
    }
  }
}

bool Room::Load(File* f)
{
  SetId(THE_ROOM_ID); // only one room exists at a time, with this fixed ID
  s_room = this;

  int numTextures = 0;
  if (!f->GetInteger(&numTextures))
  {
    f->ReportError("Expected num textures.");
    return false;
  }

  m_texNames.reserve(numTextures);
  std::string s;

  for (int i = 0; i < numTextures; i++)
  {
    if (!f->GetDataLine(&s))
    {
      f->ReportError("Expected texture name");
      return false;
    }
    if (!TheResourceManager::Instance()->GetRes(s))
    {
      f->ReportError("Not a valid texture name: " + s);
      return false;
    }
    m_texNames.push_back(s);
  }

  if (!LoadVec2(f, &m_gridsize))
  {
    f->ReportError("Expected grid size");
    return false;
  }

  for (int i = 0; i < NUM_TILE_MAPS; i++)
  {
    if (!LoadGrid(i, f))
    {
      return false;
    }
  }

  // Load destination room numbers to NSEW
  if (!f->GetDataLine(&s))
  {
    f->ReportError("Expected destination rooms");
    return false;
  }
  Strings dests = Split(s, ',');
  if (dests.size() != 4)
  {
    f->ReportError("Expected FOUR comma-sep destination rooms");
    return false;
  }
  bool nonz = false;
  for (int i = 0; i < 4; i++)
  {
    m_dest[i] = ToInt(dests[i]);
    if (m_dest[i])
    {
      nonz = true;
    }
  }
  if (!nonz)
  {
    f->ReportError("Room has no destinations from it");
    return false;
  }

  if (!f->GetDataLine(&m_music))
  {
    f->ReportError("Room has no music");
    return false;
  }
  // TODO TEMP
  //m_music = "sound/rainforest-ambience1.ogg"; 

std::cout << "Room music: " << m_music << "\n";

  return true;
}

bool Room::LoadGrid(int grid, File* f)
{
  std::string s;
  for (int y = 0; y < m_gridsize.y; y++)
  {
    if (!f->GetDataLine(&s))
    {
      f->ReportError("Expected grid line " + ToString(y));
      return false;
    }
    Strings strs = Split(s, ' ');
    if ((int)strs.size() != m_gridsize.x)
    {
      f->ReportError("Grid size does not match tile data.");
      return false;
    }
    for (int x = 0; x < m_gridsize.x; x++)
    {
      int t = ToInt(strs[x]);
      if (t == 0)
      {
        continue; // no tile here
      }

      if (t > (int)m_texNames.size())
      {
        f->ReportError("Tile number too big: " + strs[x] + " in grid pos (" + ToString(x) + ", " + ToString(y) + ")");
        return false;
      }
      std::string tex = m_texNames[t - 1];

      // Group all tiles with same texture
      TileVec& tilevec = m_tilemap[grid][tex];
      // Centre grid on origin
      Vec2f tilepos((float)x - (float)m_gridsize.x * 0.5f, (float)y - (float)m_gridsize.y * 0.5f);
      tilevec.push_back(Tile(tilepos, Vec2f((float)m_gridsize.x, (float)m_gridsize.y)));
    }
  }
  return true;
}

int Room::GetDest(RoomDir d) const
{
  return m_dest[(int)d];
}

const char* Room::TYPENAME = "room";

const char* Room::GetTypeName() const
{
  return TYPENAME;
}

void Room::OnLocationEntry()
{
  RoomNode* rn = new RoomNode(this);
  SceneNode* root = GetVe1SceneGraph()->GetRootNode(SceneGraph::AMJU_OPAQUE);
  root->AddChild(rn);
  rn->Build(); // TODO Use tile data we loaded to make tiled textured quads..?

  // Big AABB so not culled
  m_aabb.Set(-10000, 10000, -1000, 1000, -10000, 10000);
  rn->SetAABB(m_aabb);

  m_sceneNode = rn;

  TheSoundManager::Instance()->PlaySong(m_music);
}

}
