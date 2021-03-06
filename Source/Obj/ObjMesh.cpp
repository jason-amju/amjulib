#include <AmjuFirst.h>
#include <assert.h>
#include <ReportError.h>
#include "ObjMesh.h"
#include "File.h"
#include "StringUtils.h"
#include "AmjuGL.h"
#include "CollisionMesh.h"
#include <Matrix.h>
#include <AmjuFinal.h>

#ifdef _DEBUG
#define OBJ_DEBUG
#endif

// Empty string for texture and shader names which are not set
#define EMPTY_STRING "$EMPTY$"

namespace Amju
{
static bool s_showInfo = false;

void ObjMesh::SetShowInfo(bool show)
{
  s_showInfo = show;
}

bool ObjMesh::ShowInfo()
{
  return s_showInfo;
}


static bool s_requiresTextures = true;

void ObjMesh::SetRequireTextures(bool b)
{
  s_requiresTextures = b;
}

bool ObjMesh::RequiresTextures()
{
  return s_requiresTextures;
}


Resource* TextObjLoader(const std::string& resName)
{
  ObjMesh* obj = new ObjMesh;
  obj->SetIsBinary(false);
  if (!obj->Load(resName))
  {
    //Assert(0);
    return nullptr;
  }
  return obj;
}

Resource* BinaryObjLoader(const std::string& resName)
{
  ObjMesh* obj = new ObjMesh;
  obj->SetIsBinary(true);
  if (!obj->Load(resName))
  {
    //Assert(0);
    return nullptr;
  }
  return obj;
}

ObjMesh* LoadObjMesh(const std::string& pathFile, bool binary)
{
  std::string path = GetFilePath(pathFile);
  std::string origRoot = File::GetRoot();
  File::SetRoot(path , "/"); // TODO change SetRoot params

  std::string objFile = StripPath(pathFile);

  // Bypass Res Manager so we don't cache the file, we want to reload it.
  // ??? This should be a flag ???
  ObjMesh* mesh = new ObjMesh;
  mesh->SetIsBinary(binary);
  bool loaded = mesh->Load(objFile);

  // After loading, revert to original file root
  File::SetRoot(origRoot, "/");

  if (!loaded)
  {
    delete mesh;
    mesh = nullptr;
  }

  return mesh;
}

void ObjMesh::CalcCollisionMesh(CollisionMesh* pCollMesh) const
{
/*
  // Clear triangle data if no coll mesh required
  if (!pCollMesh)
  {
    for (Groups::iterator it = m_groups.begin();
         it != m_groups.end();
         ++it)
    {
      Group& g = it->second;
      g.m_tris.clear();
    }
    return;
  }
*/

  // Iterate over groups once to count how many faces there are;
  // then iterate again to convert each one to a tri.
  // Don't add tris for groups whose material has 'no collide' flag set.
  pCollMesh->m_tris.clear();

  int numFaces = 0;
  for (auto it = m_groups.begin();
    it != m_groups.end();
    ++it)
  {
    const Group& g = it->second;
    if (m_materials.find(g.m_materialName) == m_materials.end())
    {
#ifdef NO_MATERIAL_DEBUG
      std::cout << "No material for group\n";
#endif
      numFaces += g.m_tris.size();
    }
    else
    {
      auto j = m_materials.find(g.m_materialName);
      if (j != m_materials.end())
      {
        const Material& mat = j->second;
        if (!(mat.m_flags & Material::AMJU_MATERIAL_NO_COLLIDE) || !g.IsCollidable())
        {
          numFaces += g.m_tris.size();
        }
      }
    }
  }
  pCollMesh->m_tris.reserve(numFaces);

  for (auto it = m_groups.begin();
    it != m_groups.end();
    ++it)
  {
    const Group& g = it->second;
   
    auto j = m_materials.find(g.m_materialName);
    if (j != m_materials.end())
    {
      const Material& mat = j->second;
      if ((mat.m_flags & Material::AMJU_MATERIAL_NO_COLLIDE) || !g.IsCollidable())
      {
        continue;
      }
    }

    unsigned int numTris = g.m_tris.size();
    for (unsigned int i = 0; i < numTris; i++)
    {
      const AmjuGL::Tri& tri = g.m_tris[i];
      Tri t(
        Vec3f(tri.m_verts[0].m_x, tri.m_verts[0].m_y, tri.m_verts[0].m_z),
        Vec3f(tri.m_verts[1].m_x, tri.m_verts[1].m_y, tri.m_verts[1].m_z),
        Vec3f(tri.m_verts[2].m_x, tri.m_verts[2].m_y, tri.m_verts[2].m_z));

      pCollMesh->m_tris.push_back(t);
    }
  }
}

bool ObjMesh::SaveBinary(const std::string& filename)
{
  return true;
}

bool ObjMesh::LoadBinary(const std::string& filename)
{
  if (ShowInfo())
  {
    std::cout << "Loading binary obj file: " << filename << "\n";
  }

  File f(File::NO_VERSION);
  if (!f.OpenRead(filename, true /* is binary */))
  {
    return false;
  }

  int n = 0;
  // Points
  f.GetInteger(&n);

  if (ShowInfo())
  {
    std::cout << "File supposedly contains " << n << " points\n";
  }

  if (n > 1000000)
  {
std::cout << "Suspiciously high number of points in obj mesh: " << n << "...\n";
    return false;
  }

  m_points.reserve(n);
  for (int i = 0; i < n; i++)
  {
    Vec3f v;
    f.GetFloat(&v.x);
    f.GetFloat(&v.y);
    f.GetFloat(&v.z);
    m_points.push_back(v);
  }

  if (ShowInfo())
  {
    std::cout << "Loaded " << n << " points\n";
  }

  // UVs
  f.GetInteger(&n);

  if (ShowInfo())
  {
    std::cout << "Contains " << n << " UVs\n";
  }

  if (n > 1000000)
  {
std::cout << "Suspiciously high number of UVs in obj mesh: " << n << "...\n";
    return false;
  }

  m_uvs.reserve(n);
  for (int i = 0; i < n; i++)
  {
    Vec2f v;
    f.GetFloat(&v.x);
    f.GetFloat(&v.y);
    m_uvs.push_back(v);
  }

  if (ShowInfo())
  {
    std::cout << "Loaded " << n << " UVs\n";
  }

  // Normals
  f.GetInteger(&n);
  if (ShowInfo())
  {
    std::cout << "Contains " << n << " normals\n";
  }

  if (n > 1000000)
  {
std::cout << "Suspiciously high number of normals in obj mesh: " << n << "...\n";
    return false;
  }

  m_normals.reserve(n);
  for (int i = 0; i < n; i++)
  {
    Vec3f v;
    f.GetFloat(&v.x);
    f.GetFloat(&v.y);
    f.GetFloat(&v.z);
    m_normals.push_back(v);
  }

  if (ShowInfo())
  {
    std::cout << "Loaded " << n << " normals\n";
  }

  // Load materials
  int numMats = 0;
  f.GetInteger(&numMats);

  if (ShowInfo())
  {
    std::cout << "Loading " << numMats << " materials..\n";
  }

  if (numMats > 1000000)
  {
std::cout << "Suspiciously high number of materials in obj mesh: " << numMats << "...\n";
    return false;
  }

  for (int i = 0; i < numMats; i++)
  {
    std::string matName;
    f.GetDataLine(&matName);
    Material& mat = m_materials[matName];
    mat.m_name = matName;

    std::string filename;
    f.GetDataLine(&filename);
    mat.m_filename = filename;

    // TODO Cube map

    for (int i = 0; i < Material::MAX_TEXTURES; i++)
    {
      std::string tex;
      f.GetDataLine(&tex);
      mat.m_texfilename[i] = tex;

      if (ShowInfo())
      {
        std::cout << "Material " << i << ": matName: " << matName << " texture filename: " << filename << " tex " << i << ": " << tex << "\n";
      }
    
      if (tex != EMPTY_STRING)
      {
        mat.m_texture[i] = (Texture*)TheResourceManager::Instance()->GetRes(tex);

        if (mat.m_texture[i])  
        {
          if (ShowInfo())
          {  
            std::cout << "Loaded texture " << tex << " OK!\n";
          }
        }
        else
        {
          // If the texture does not load, this is OK if we are just converting formats etc, 
          //  but not OK if we want to draw the mesh!
          if (RequiresTextures())
          {
            return false;
          }
          else if (ShowInfo())
          {
            std::cout << "Ignoring failed texture " << tex << "...\n";
          }
        }
      }
    }

    std::string shader;
    f.GetDataLine(&shader);
    mat.m_shaderName = shader;
    if (shader != EMPTY_STRING)
    {
      mat.m_shader = AmjuGL::LoadShader(shader);
    }

    int flags = 0;
    f.GetInteger(&flags);
    mat.m_flags = (uint32)flags;

    if (ShowInfo())
    {
      std::cout << "Flags: " << flags << "\n";
    }

    if ((flags & Material::AMJU_MATERIAL_SPHERE_MAP) && mat.m_texture[0])
    {
      mat.m_texture[0]->SetTextureType(AmjuGL::AMJU_TEXTURE_SPHERE_MAP);
    }
  }

  int numGroups = 0;
  f.GetInteger(&numGroups); 
  if (ShowInfo())
  {
    std::cout << "Supposedly " << numGroups << " groups...\n";
  }

  if (numGroups > 1000)
  {
std::cout << "Suspiciously high number of groups in obj mesh: numGroups: " << numGroups << "...\n";
    return false;
  }

  for (int ig = 0; ig < numGroups; ig++)
  {
    std::string groupName;
    f.GetDataLine(&groupName);
    if (ShowInfo())
    {
      std::cout << "Group " << ig << " name: " << groupName << "\n";
    }

    Group& g = m_groups[groupName];
    g.m_name = groupName;

    // Get flag - 1 if we have material 
    int matFlag = 0;
    f.GetInteger(&matFlag);
    if (matFlag)
    {
      std::string matFilename;
      f.GetDataLine(&matFilename);
      if (ShowInfo())
      {
        std::cout << "Material file: " << matFilename << "\n";
      }

      // TODO Do we need this ?

      std::string matName;
      f.GetDataLine(&matName);

      if (ShowInfo())
      {
        std::cout << "Material name: " << matName << "\n";
      }
      g.m_materialName = matName;
    }
    else
    {
      if (ShowInfo())
      {
        std::cout << "No material for this group.\n";
      }
    }

    // Load face info
    int numFaces = 0;
    f.GetInteger(&numFaces);
    if (numFaces > 100000)
    {
std::cout << "Suspiciously high number of faces in obj mesh: " << numFaces << "...\n";
      return false;
    }

    m_facemap[g.m_name].reserve(numFaces);
    for (int i = 0; i < numFaces; i++)
    {
      Face face;
      for (int j = 0; j < 3; j++)
      {
        f.GetInteger(&face.m_pointIndex[j]);
        f.GetInteger(&face.m_uvIndex[j]);
        f.GetInteger(&face.m_normalIndex[j]);
      }
      m_facemap[g.m_name].push_back(face);
    }

    if (ShowInfo())
    {
      std::cout << "Loaded " << numFaces << " faces\n";
    }
  }

  MungeData();
  return true;
}

void ObjMesh::SetIsBinary(bool isBinary)
{
  m_isBinary = isBinary;
}

bool ObjMesh::Load(const std::string& filename)
{
  if (m_isBinary)
  {
    return LoadBinary(filename);
  }

  if (ShowInfo())
  {
    std::cout << "Loading file " << filename << " - text, not binary\n";
  }

  File f(File::NO_VERSION);
  if (!f.OpenRead(filename))
  {
    return false;
  }

  // Start off with a default group name - using the filename makes
  //  it unique, so we can merge obj files.
  std::string currentGroup = filename;

  // Set name for default group..?
  // Bad idea, as we can end up with an empty group
  // TODO Remove any empty groups
  Group& g = m_groups[currentGroup];
  g.m_name = currentGroup;

  while (true)
  {
    std::string s;
    if (!f.GetDataLine(&s))
    {
      break;
    }
    Trim(&s);
    if (s.empty())
    {
      continue;
    }

    Strings strs = Split(s, ' ');
    Assert(!strs.empty());

    if (strs[0] == "v")
    {
      m_points.push_back(ToVec3(strs));
    }
    else if (strs[0] == "vn")
    {
      Vec3f n = ToVec3(strs);
      n.Normalise();
      m_normals.push_back(n);
    }
    else if (strs[0] == "vt")
    {
      m_uvs.push_back(ToVec2(strs));
    }
    else if (strs[0] == "f")
    {
      Group& g = m_groups[currentGroup];
      Assert(!g.m_name.empty());

      if (strs.size() > 4)
      {
        f.ReportError("Non-triangular face, taking first 3 verts only.");
      }

      Face face = ToFace(strs);
      m_facemap[g.m_name].push_back(face);
    }
    else if (strs[0] == "g")
    {
      // Switch current group - create a new group if it doesn't
      //  already exist in the map.
      if (strs.size() == 1)
      {
        f.ReportError("No group name!");
      }
      else if (strs.size() == 2)
      {
        currentGroup = strs[1];
        Group& g = m_groups[currentGroup];
        g.m_name = currentGroup;
      }
      else
      {
        f.ReportError("Info: Unexpected format for group: " + s);
        currentGroup = strs[1];
        for (unsigned int a = 2; a < strs.size(); a++)
        {
          currentGroup += strs[a];
        }
        Group& g = m_groups[currentGroup];
        g.m_name = currentGroup;
      }
    }
    else if (strs[0] == "mtllib")
    {
      if (strs.size() != 2)
      {
        f.ReportError("Unexpected format for mtllib");
        continue;
      }

      std::string mtlfilename = strs[1];
      std::string path = GetFilePath(filename);
      mtlfilename = path + (path.empty() ? "" : "/") + mtlfilename;
      MaterialVec mats;
      if (!LoadMtlFile(mtlfilename, &mats))
      {
        f.ReportError("Failed to load mtl file " + mtlfilename);
        return false;
      }
      //mat.m_filename = mtlfilename;

      for (unsigned int i = 0; i < mats.size(); i++)
      {
        Material& mat = *mats[i];

        Assert(!mat.m_name.empty());
        // Only add if texture specified 
        if (!mat.m_texfilename[0].empty())
        {
          m_materials[mat.m_name] = mat;
        }
        else
        {
          if (ShowInfo())
          {
            std::cout << "Discarding material " << mat.m_name << " as no texture\n";
          }
        }
      }
    }
    else if (strs[0] == "usemtl")
    {
      if (strs.size() != 2)
      {
        f.ReportError("Unexpected format for usemtl");
        continue;
      }

      std::string matname = strs[1];
      Group& g = m_groups[currentGroup];
      if (g.m_materialName.empty())
      {
        g.m_materialName = matname;
      }
      else
      {
        if (ShowInfo())
        {
          std::cout << "Changing material within the same group - sigh, making new group.\n";
        }

        // Make name for group
        currentGroup = matname + "_group"; // OK if group name exists
        Group& g = m_groups[currentGroup];
        g.m_name = currentGroup;
        g.m_materialName = matname;
      }
    }
  }

  if (m_points.empty())
  {
    if (ShowInfo())
    {
      std::cout << "No point data - failed!\n";
    }
    return false;
  }

  if (ShowInfo())
  {
    std::cout << "Points: " << m_points.size()
      << " UVs: " << m_uvs.size() 
      << " Norms: " << m_normals.size()
      << " Groups: " << m_groups.size()
      << " Mats: " << m_materials.size()
      << "\n";
  }

  MungeData();
  return true;
}

const AABB& ObjMesh::GetAABB()
{
  return m_aabb;
}

const AABB& ObjMesh::GetAABB(const std::string& groupname)
{
  Groups::iterator it = m_groups.find(groupname);
  Assert(it != m_groups.end());
  Group& g = it->second;
  return g.m_aabb; 
}

void ObjMesh::MungeData()
{
  if (ShowInfo())
  {
    std::cout << "Optimising .obj data...\n";
  }

  const float BIG = 99999.9f;
  m_aabb.Set(BIG, -BIG, BIG, -BIG, BIG, -BIG);

  for (Groups::iterator it = m_groups.begin();
    it != m_groups.end();
    /* increment in body */)
  {
    Group& g = it->second;
    BuildGroup(g);

    // Erase empty groups
    if (g.m_tris.empty())
    {
      if (ShowInfo())
      {
        std::cout << "Removing empty group " << g.m_name << "\n";
      }

#ifdef WIN32
      it = m_groups.erase(it);
#else
      auto jt = it;
      ++jt;
      m_groups.erase(it);
      it = jt;
#endif
    }
    else
    {
      m_aabb.Union(g.m_aabb);
      ++it;
    }
  }

  // Remove unused materials
  for (Materials::iterator it = m_materials.begin(); it != m_materials.end(); )
  {
    bool found = false;
    std::string matName = it->first;
    // Look for this name in all groups
    for (Groups::iterator jt = m_groups.begin(); jt != m_groups.end(); ++jt)
    {
      const Group& g = jt->second;
      if (g.m_materialName == matName)
      {
        found = true;
        break;
      }
    }
    if (found)
    {
      if (ShowInfo())
      {
        std::cout << "KEEPING material " << matName << "\n";
      }
      ++it;
    }
    else
    {
      if (ShowInfo())
      {
        std::cout << "Removing unused material " << matName << "\n";
      }
		
      it = m_materials.erase(it);
    }
  }

  // Remove data no longer needed (only used to create group data)
  m_points.clear();
  m_normals.clear();
  m_uvs.clear();
  m_facemap.clear();

  if (ShowInfo())
  {
    std::cout << "MungeData finished.\n";
  }
}

void ObjMesh::Draw()
{
  for (Groups::iterator it = m_groups.begin();
       it != m_groups.end();
       ++it)
  {
      Group& g = it->second;
      DrawGroup(g);
  }
}

void ObjMesh::GetMaterials(MaterialVec* vec)
{
  for (Materials::iterator it = m_materials.begin(); it != m_materials.end(); ++it)
  {
    vec->push_back(new Material(it->second));
  } 
}

Group* ObjMesh::GetGroup(const std::string& groupName)
{
  Groups::iterator it = m_groups.find(groupName);
  if (it == m_groups.end())
  {
    return 0;
  } 
  return &it->second;
}

void ObjMesh::BuildGroup(Group& g)
{
  // Only need to build Tris once - this kind of mesh is not animated!
  Faces& faces = m_facemap[g.m_name];

  unsigned int numfaces = faces.size();

  if (ShowInfo())
  {
    std::cout << "Group " << g.m_name << " has " << numfaces << " faces.\n";
    if (m_uvs.empty())
    {
      std::cout << "No UVs in this obj mesh!\n";
    }
  }

  const float BIG = 99999.9f;
  g.m_aabb.Set(BIG, -BIG, BIG, -BIG, BIG, -BIG);

  g.m_tris.reserve(numfaces);
  for (unsigned int i = 0; i < numfaces; i++)
  {
    const Face& f = faces[i];
    AmjuGL::Tri t;
    for (int j = 0; j < 3; j++)
    {
      if (!m_normals.empty())
      {
        unsigned int n = f.m_normalIndex[j];
        Assert(n < m_normals.size());
        const Vec3f& vN = m_normals[n];

        t.m_verts[j].m_nx = vN.x;
        t.m_verts[j].m_ny = vN.y;
        t.m_verts[j].m_nz = vN.z;
      }

      if (m_uvs.empty())
      {
        t.m_verts[j].m_u = 0;
        t.m_verts[j].m_v = 0;
      }
      else
      {
        unsigned int uv = f.m_uvIndex[j];
        Assert(uv < m_uvs.size());
        const Vec2f& vUV = m_uvs[uv];

        t.m_verts[j].m_u = vUV.x;
        t.m_verts[j].m_v = vUV.y;
      }

      Assert(!m_points.empty());

      unsigned int p = f.m_pointIndex[j];
      Assert(p < m_points.size());
      const Vec3f vP = m_points[p];

      t.m_verts[j].m_x = vP.x;
      t.m_verts[j].m_y = vP.y;
      t.m_verts[j].m_z = vP.z;

      g.m_aabb.SetIf(vP.x, vP.y, vP.z);
    }
    g.m_tris.push_back(t);
  }
  g.m_triList = MakeTriList(g.m_tris);
  if (g.m_triList)
  {
    g.m_triList->CalcTangents(); 
  }
}

void ObjMesh::DrawGroup(Group& g)
{
  if (!g.IsVisible())
  {
    return;
  }

  // Material can set lighting/blending flags
  AmjuGL::PushAttrib(AmjuGL::AMJU_LIGHTING | AmjuGL::AMJU_BLEND);

  // TODO Hash, not string
  Materials::iterator it = m_materials.find(g.m_materialName);
  if (it != m_materials.end())
  {
    Material& mat = it->second;
    mat.UseThisMaterial();
  }

  AmjuGL::Draw(g.m_triList);

  AmjuGL::PopAttrib();
}

void ObjMesh::Merge(const ObjMesh& om)
{
  m_materials.insert(om.m_materials.begin(), om.m_materials.end());
  m_groups.insert(om.m_groups.begin(), om.m_groups.end());
}

void ObjMesh::Transform(const Matrix& mat)
{
  // Rotate normals but don't translate
  Matrix rotMat = mat;
  rotMat.TranslateKeepRotation(Vec3f(0, 0, 0)); // overwrites any translation

  for (Groups::iterator it = m_groups.begin();
    it != m_groups.end();
    ++it)
  {
    Group& g = it->second;

    for (unsigned int i = 0; i < g.m_tris.size(); i++)
    {
      AmjuGL::Tri& tri = g.m_tris[i];
      for (int j = 0; j < 3; j++)
      {
        AmjuGL::Vert& v = tri.m_verts[j];

        Vec3f p(v.m_x, v.m_y, v.m_z);
        Vec3f vn(v.m_nx, v.m_ny, v.m_nz);
        // transform verts; write back to Tri
        p = p * mat; // sigh, TODO
        vn = vn * rotMat;
        vn.Normalise();
        v.m_x = p.x;
        v.m_y = p.y;
        v.m_z = p.z;
        v.m_nx = vn.x;
        v.m_ny = vn.y;
        v.m_nz = vn.z;
      }
    }
    g.m_triList = MakeTriList(g.m_tris);
    g.m_triList->CalcTangents();
  }
}

bool ObjMesh::Save(const std::string& filename)
{
  typedef std::map<Vec3f, int> Vec3Map;
  Vec3Map pointMap;
  Vecs points;
  Vec3Map normalMap;
  Vecs normals;
  typedef std::map<Vec2f, int> Vec2Map;
  Vec2Map uvMap;
  UVs uvs;

  typedef std::map<std::string, SaveGroup> SaveGroupMap;
  SaveGroupMap groupMap;

  for (Groups::iterator it = m_groups.begin();
    it != m_groups.end();
    ++it)
  {
    Assert(!it->first.empty()); // no group name ?!

    Group& g = it->second;
    Assert(!g.m_name.empty());
    SaveGroup& sg = groupMap[g.m_name];

    for (unsigned int i = 0; i < g.m_tris.size(); i++)
    {
      AmjuGL::Tri& tri = g.m_tris[i];
      Face face;

      for (int j = 0; j < 3; j++)
      {
        AmjuGL::Vert& v = tri.m_verts[j];

        Vec3f p(v.m_x, v.m_y, v.m_z);
        if (pointMap.find(p) == pointMap.end())
        {
          pointMap[p] = points.size() + 1; // 1-based
          points.push_back(p);
        }
        Vec3f vn(v.m_nx, v.m_ny, v.m_nz);
        if (normalMap.find(vn) == normalMap.end())
        {
          normalMap[vn] = normals.size() + 1;
          normals.push_back(vn);
        }
        Vec2f vt(v.m_u, v.m_v);
        if (uvMap.find(vt) == uvMap.end())
        {
          uvMap[vt] = uvs.size() + 1;
          uvs.push_back(vt);
        }

        face.m_pointIndex[j] = pointMap[p];
        face.m_normalIndex[j] = normalMap[vn];
        face.m_uvIndex[j] = uvMap[vt];
      }
      sg.m_faces.push_back(face);
    }
  }

  if (m_isBinary)
  {
    File of(File::NO_VERSION);
    of.OpenWrite(filename, 0, true /* is binary */);

    // Save points
    if (ShowInfo())
    {
      std::cout << "Saving " << points.size() << " points\n";
    }

    of.WriteInteger(points.size());
    for (unsigned int i = 0; i < points.size(); i++)
    {
      const Vec3f& v = points[i];
      of.WriteFloat(v.x);
      of.WriteFloat(v.y);
      of.WriteFloat(v.z);
    }

    // Save UVs
    if (ShowInfo())
    {
      std::cout << "Saving " << uvs.size() << " UVs\n";
    }
  
    of.WriteInteger(uvs.size());
    for (unsigned int i = 0; i < uvs.size(); i++)
    {
      const Vec2f& v = uvs[i];
      of.WriteFloat(v.x);
      of.WriteFloat(v.y);
    }

    // Save normals
    if (ShowInfo())
    {
      std::cout << "Saving " << normals.size() << " normals\n";
    }

    of.WriteInteger(normals.size());
    for (unsigned int i = 0; i < normals.size(); i++)
    {
      const Vec3f& v = normals[i];
      of.WriteFloat(v.x);
      of.WriteFloat(v.y);
      of.WriteFloat(v.z);
    }

    // Save materials
    if (ShowInfo())
    {
      std::cout << "Saving " << m_materials.size() << " materials\n";
    }

    of.WriteInteger(m_materials.size());
    for (Materials::iterator it = m_materials.begin(); it != m_materials.end(); ++it)
    {
      Material& mat = it->second;
      of.Write(mat.m_name);
      of.Write(mat.m_filename);
      for (int i = 0; i < Material::MAX_TEXTURES; i++)
      {
        if (mat.m_texfilename[i].empty())
          of.Write(EMPTY_STRING);
        else 
          of.Write(mat.m_texfilename[i]);
      }
      if (mat.m_shaderName.empty())
        of.Write(EMPTY_STRING);
      else
        of.Write(mat.m_shaderName);

      of.WriteInteger((int)mat.m_flags);
    }

    // Save groups
    if (ShowInfo())
    {
      std::cout << "Saving " << groupMap.size() << " groups\n";
    }

    of.WriteInteger(groupMap.size());
    for (SaveGroupMap::iterator it = groupMap.begin();
      it != groupMap.end();
      ++it)
    {
      SaveGroup& sg = it->second;
      std::string groupName = it->first;
      Group& g = m_groups[groupName];
      std::string mtlName = g.m_materialName;
      Material& m = m_materials[mtlName];

      // TODO Use ID for groups, just need to be unique
      // TODO Write all materials to this file. Use IDs instead of names.

      // Save group info
      // NB If we use names, we can go back from binary to text
      of.Write(groupName);

      if (!mtlName.empty() && !m.m_filename.empty())
      {
        of.WriteInteger(1); // means there is material for this group
        of.Write(m.m_filename);
        of.Write(mtlName); 
      }
      else
      {
        of.WriteInteger(0); // means no material for this group
      }

      // Save face info
      of.WriteInteger(sg.m_faces.size());
      if (ShowInfo())
      {
        std::cout << "Saving " << sg.m_faces.size() << " faces for group " << groupName << "\n";
      }

      for (unsigned int i = 0; i < sg.m_faces.size(); i++)
      {
        Face& face = sg.m_faces[i];
        for (int j = 0; j < 3; j++)
        {
          Assert(face.m_pointIndex[j] > 0);
          Assert(face.m_uvIndex[j] > 0);
          Assert(face.m_normalIndex[j] > 0);

          // Indices are one-based, save as zero-based in binary format
          of.WriteInteger(face.m_pointIndex[j] - 1);
          of.WriteInteger(face.m_uvIndex[j] - 1);
          of.WriteInteger(face.m_normalIndex[j] - 1);
        }
      }
    }
  }
  else
  {
    File of(File::NO_VERSION);
    of.OpenWrite(filename);
    of.Write("# Saved from Amjulib::ObjMesh");

    // Write all points
    of.Write("# points");
    for (unsigned int i = 0; i < points.size(); i++)
    {
      of.Write("v " + ToString(points[i].x) + " " + ToString(points[i].y) + " " + ToString(points[i].z));
    }
    of.Write("# UVs");
    for (unsigned int i = 0; i < uvs.size(); i++)
    {
      of.Write("vt " + ToString(uvs[i].x) + " " + ToString(uvs[i].y));
    }
    of.Write("# Normals");
    for (unsigned int i = 0; i < normals.size(); i++)
    {
      of.Write("vn " + ToString(normals[i].x) + " " + ToString(normals[i].y) + " " + ToString(normals[i].z));
    }
    for (SaveGroupMap::iterator it = groupMap.begin();
      it != groupMap.end();
      ++it)
    {
      SaveGroup& sg = it->second;
      std::string groupName = it->first;
      Group& g = m_groups[groupName];
      std::string mtlName = g.m_materialName;
      Material& m = m_materials[mtlName];

      of.Write("g " + groupName);

      // TODO This works up to a point, but only if materials are one per file.
      if (!m.m_filename.empty())
      {
        of.Write("mtllib " + m.m_filename);
      }
      if (!mtlName.empty())
      {
        of.Write("usemtl " + mtlName);
      }

      of.Write("# Faces");
      for (unsigned int i = 0; i < sg.m_faces.size(); i++)
      {
        Face& face = sg.m_faces[i];
        std::string s = "f ";
        for (int j = 0; j < 3; j++)
        {
          s += ToString(face.m_pointIndex[j]) 
            + "/" 
            + (face.m_uvIndex[j] ? ToString(face.m_uvIndex[j]) : "")
            + "/"
            + (face.m_normalIndex[j] ? ToString(face.m_normalIndex[j]) : "")
            + " ";
        }
        of.Write(s);
      }
    }

    MaterialVec matVec;
    GetMaterials(&matVec);
    if (!SaveMtlFiles(matVec))
    {
      ReportError("Failed to save materials");
      return false;
    }
  }

  return true;
}
}

