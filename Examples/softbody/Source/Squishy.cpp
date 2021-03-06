#include <CollisionMesh.h>
#include "Squishy.h"
#include "SquishLoader.h"
#include "IntersectLine3Line3.h"

#ifdef AMJU_USE_GLUT
#ifdef WIN32
#include <gl/glut.h>
#endif
#ifdef MACOSX
#include <GLUT/glut.h>
#endif
#endif

namespace Amju
{
/*
Ray-Tri Intersection
Adapted from http://www.lighthouse3d.com/tutorials/maths/ray-triangle-intersection/
Moller-Haines algorithm
*/
int rayIntersectsTriangle(
  const Vec3f& p, const Vec3f& d, 
  const Vec3f& v0, const Vec3f& v1, const Vec3f& v2,
  Vec3f* intersectionPoint)
{
  Vec3f e1 = v1 - v0;
  Vec3f e2 = v2 - v0;

  Vec3f h = CrossProduct(d, e2);

  float a = DotProduct(e1, h);

  if (a > -0.00001 && a < 0.00001)
    return(false);

  float f = 1.0f / a;
  Vec3f s = p - v0;

  float u = f * DotProduct(s, h);

  if (u < 0.0 || u > 1.0)
    return(false);

  Vec3f q = CrossProduct(s, e1);

  float v = f * DotProduct(d, q);

  if (v < 0.0 || u + v > 1.0)
    return(false);

  // at this stage we can compute t to find out where
  // the intersection point is on the line
  float t = f * DotProduct(e2, q);

  if (t > 0.00001) // ray intersection
  {
    *intersectionPoint = p + t * d; 
    return(true);
  }
  // this means that there is a line intersection but not a ray intersection
  return (false);
}


Squishy::Squishy()
{
  m_volume = 0;

 // m_mesh = nullptr; // TODO TEMP TEST
  m_drawSpringSystem = true;
  m_drawTris = true;

  m_numVerts = 0;
}

struct CutPointSorter
{
  CutPointSorter(const Vec3f& p) : m_pos(p) {}
  bool operator()(const Squishy::CutPoint& cp1, const Squishy::CutPoint& cp2)
  {
    float sqdist1 = (cp1.m_pos - m_pos).SqLen();
    float sqdist2 = (cp2.m_pos - m_pos).SqLen();
    return sqdist1 < sqdist2;
  }
  Vec3f m_pos;
};

void Squishy::GetCutPoints(const LineSeg& seg, CutPoints* cutpoints)
{
  // Check each tri, does it intersect.
  // NB Could be more than one if we cut through the squishy
  // Get distance along line seg and intersection point for each tri

  // points where line seg intersects squishy - once we have tested all tris,
  //  find closest

  for (auto it = m_trilist.begin(); it != m_trilist.end(); ++it)
  {
    Tri* tri = *it;

    Particle* particle[3] = 
    {
      GetParticle(tri->m_particles[0]),
      GetParticle(tri->m_particles[1]),
      GetParticle(tri->m_particles[2])
    };
    Vec3f pos[3] = 
    {
      particle[0]->GetPos(),
      particle[1]->GetPos(),
      particle[2]->GetPos()
    };

    Vec3f intersect;
    int r = rayIntersectsTriangle(seg.p0, seg.p1 - seg.p0, pos[0], pos[1], pos[2], &intersect);
    tri->m_selected = (r != 0);
    if (r)
    {
      //std::cout << "Got intersection!: " << intersect.x << " " << intersect.y << " " << intersect.z << "\n";
      CutPoint cp;
      cp.m_pos = intersect;
      cp.m_tri = tri;
      cutpoints->push_back(cp);
    }
  }

  // Sort by distance from lineseg start
  std::sort(cutpoints->begin(), cutpoints->end(), CutPointSorter(seg.p0));
}

void Squishy::StartCut(const LineSeg& seg, float cutDepth)
{
  m_cutLines.clear();

  float sqDepth = cutDepth * cutDepth;

  CutPoints cutpoints;
  GetCutPoints(seg, &cutpoints);
  if (cutpoints.empty())
  {
    return;
  }
 
  // Order the tris by distance. Taking the closest tri as cut depth 0, do we 
  //  reach the next tri? If so, we are cutting through.

  const CutPoint& closest = cutpoints[0];

  for (int i = 0; i < (int)cutpoints.size(); i++)
  {
    CutPoint& cp = cutpoints[i];
 
    if (i > 0)
    {
      // Test distance from cut point 0 to this one
      float sqdist = (closest.m_pos - cp.m_pos).SqLen();
      if (sqdist > sqDepth)
      {
        break;
      }
    }
    // Create a new cutline for this depth
    CutLine cl;
    cl.push_back(cp);
    m_cutLines.push_back(cl);
  }
}

void Squishy::ContinueCut(const LineSeg& seg, float cutDepth)
{
  CutPoints cutpoints;
  GetCutPoints(seg, &cutpoints); 

  int size = std::min(cutpoints.size(), m_cutLines.size());

  for (int i = 0; i < size; i++)
  {
    CutPoint& cp = cutpoints[i];

    CutLine& cl = m_cutLines[i];
    cl.push_back(cp);
  }
}

std::ostream& operator<<(std::ostream& os, const Vec3f& v)
{
  return os << v.x << ", " << v.y << ", " << v.z;
}

std::ostream& operator<<(std::ostream& os, const Squishy::Tri& tri)
{
  return os <<
    tri.m_particles[0] << "/" << 
    tri.m_particles[1] << "/" <<
    tri.m_particles[2];
}

void PrintTri(const Squishy::Tri& tri)
{
  std::cout << tri;
}

int ThirdVert(const Squishy::Tri& tr, int e1, int e2)
{ 
  if (e1 != tr.m_particles[0] && e2 != tr.m_particles[0]) return tr.m_particles[0];
  if (e1 != tr.m_particles[1] && e2 != tr.m_particles[1]) return tr.m_particles[1];
  Assert(e1 != tr.m_particles[2]);
  Assert(e2 != tr.m_particles[2]);
  return tr.m_particles[2];
}

bool CommonEdge(const Squishy::Tri& tr1, const Squishy::Tri& tr2, int* v1, int* v2)
{
  // Find edge shared by tris
  // Get the number of times each of the 3 tri verts in tr1 appears in the
  //  list of verts for tr2.
  int p1 = std::count(tr2.m_particles, tr2.m_particles + 3, tr1.m_particles[0]);
  int p2 = std::count(tr2.m_particles, tr2.m_particles + 3, tr1.m_particles[1]);
  int p3 = std::count(tr2.m_particles, tr2.m_particles + 3, tr1.m_particles[2]);

  // Number of verts shared between tr1 and tr2 -- should be 2 if they are neighbours
  int num = p1 + p2 + p3; 
std::cout << "Found " << num << " matching verts, "; 
  if (num == 2)
  {
    int e[2] = { 0, 1 };
    if (p1 == 0)
    {
      e[0] = 2;
    }
    else if (p2 == 0)
    {
     e[1] = 2;
    }
   
    *v1 = tr1.m_particles[e[0]];
    *v2 = tr1.m_particles[e[1]];
    // Put in ascending order
    if (*v1 > *v2)
    {
      std::swap(*v1, *v2);
    } 
    std::cout << "Edge " << e[0] << "-" << e[1] << ": " << *v1 << " - " << *v2 << "\n";
    return true;
  }
std::cout << "\n";
  return false;
}

void Squishy::FillTri1(const Vec3f& normal, int centre, int p1, int p2, int e1, int e2)
{
  // p1 and p2 are tri verts. If there are springs connecting these 2 verts and the centre,
  //  fill in with a triangle.
  // If no edge from p1 to p2, there is an edge between p1-e1 and p2-e2 OR p1-e2 and p2-e1.

  Assert(GetSpring(centre, p1));
  Assert(GetSpring(centre, p2));
  Assert(GetSpring(centre, e1));
  Assert(GetSpring(centre, e2));

  // TODO GetSpring needs to be FAST
  if (GetSpring(p1, p2))
  {
    AddTriWithWinding(normal, Tri(centre, p1, p2)); 
  }
  else if (GetSpring(p1, e1))
  {
    Assert(GetSpring(p2, e2));
    AddTriWithWinding(normal, Tri(centre, p1, e1)); 
    AddTriWithWinding(normal, Tri(centre, p2, e2)); 
  }
  else
  {
    Assert(GetSpring(p1, e2));
    Assert(GetSpring(p2, e1));
    AddTriWithWinding(normal, Tri(centre, p1, e2)); 
    AddTriWithWinding(normal, Tri(centre, p2, e1));
  }
}

void Squishy::FillTriHoles(const Vec3f& normal, const Squishy::Tri& tr, int e1, int e2, int centre)
{
  // Fill holes in tr, when tri has a triangle chopped out of it, from centre to e1 to e2 back to centre. 
  FillTri1(normal, centre, tr.m_particles[0], tr.m_particles[1], e1, e2);
  FillTri1(normal, centre, tr.m_particles[1], tr.m_particles[2], e1, e2);
  FillTri1(normal, centre, tr.m_particles[2], tr.m_particles[0], e1, e2);
}

int Squishy::AddNewParticle(const Vec3f& pos)
{
  Particle* p = CreateParticle();
  m_numVerts++;
  int id = p->GetId(); 
  p->SetPos(pos);
  return id;
}

void Squishy::CutInto(const CutLine& cutline)
{
std::cout << "Cutting...\n";

  m_edgePoints.clear();

  // Create a new squishy or change the existing data?
  // Changes to make:
  // Add new particles, add new springs
  // Remove some springs ?
  // Remove tris, replace with more as tris tesselated

  // Presumably would not be OK to just take first and last point and make a 
  //  straight cut..
  // But could remove points on the same tri, just ending up with 1 or 2 points per tri?
  // Otherwise, we would be pointing to tris which would no longer exist after being
  //  tesselated.
  // Cases are:
  //  - all points on the same tri - assume we can just cut straight from first to last?

  if (cutline.empty())
  {
    return;
  }

  int size = cutline.size();
  CutPoint start = cutline[0];

  m_edgePoints.push_back(EdgePoint(start.m_pos, start.m_tri, Edge(-1, -1)));

  // True until we create the first 'v'-shaped cut in the first tri
  bool isFirstSeg = true; 

  // IDs of particles along edge we have created as the cut goes through
  //  the edge.
  int edgep1 = -1;
  int edgep2 = -1;

  for (int i = 1; i < size;  i++)
  {
    const CutPoint& cp = cutline[i];

    // Check point i and i+1. Same tri or different?
    if (cp.m_tri == start.m_tri)
    {
      // Same tri. Skip this cut point unless the last point
      if (i == (size - 1))
      {
std::cout << "The final point!?!\n";
        if (edgep1 != -1)
        {
          // Cut spans at least 2 tris - connect last 2 points on shared edge
          //  to the final point.
          Particle* p = CreateParticle();
          m_numVerts++;
          int id = p->GetId();
          p->SetPos(cp.m_pos);

          // Connect new particle to verts of this tri, and to the points on
          //  the last shared edge.
          const Tri& tr = *(cp.m_tri);
          CreateSpring(id, tr.m_particles[0]);
          CreateSpring(id, tr.m_particles[1]);
          CreateSpring(id, tr.m_particles[2]);
          CreateSpring(id, edgep1);
          CreateSpring(id, edgep2);

          // Which edge of the original tri has been cut ? Don't add a tri along that edge!
          FillTriHoles(tr.m_normal, tr, edgep1, edgep2, id);
        }
        else
        {
          // Whole cut is on one tri
std::cout << "Whole cut is on one tri..?!?\n";
        }
      }      
    }
    else
    {
      // start and cp are on different tris - they should be neighbours sharing an edge ?
      // Find point on edge.
      Tri* tr1 = start.m_tri;
      Tri* tr2 = cp.m_tri;

std::cout << "Finding edge between 2 tris: TR1: " << *tr1 << " TR2: " << *tr2 << "\n";
      // Find common edge (if exists)
      int e1 = 0;
      int e2 = 0;
      if (CommonEdge(*tr1, *tr2, &e1, &e2))
      {
        // Same edge as last time? Skip if so
        // TODO

        // Find the point on the edge where the cut line crosses it 
        LineSeg triEdge(GetParticle(e1)->GetPos(), GetParticle(e2)->GetPos());
        LineSeg cut(start.m_pos, cp.m_pos);
        LineSeg closest; // shortest line seg connecting the tri edge and cut segment
        float mua, mub; // t values along segments triEdge and cut
        if (Intersects(triEdge, cut, &closest, &mua, &mub))
        {
          std::cout << "Found point on tri edge!\n";
          m_edgePoints.push_back(EdgePoint(closest.p0, cp.m_tri, Edge(e1, e2)));
          // Make two points, close to each other
          Vec3f dir = triEdge.p1 - triEdge.p0;
          Vec3f p1 = triEdge.p0 + dir * (0.95f * mua); // TODO
          Vec3f p2 = triEdge.p0 + dir * (1.05f * mua);
          // If this is the first segment in the cut line, this is the start 
          //  of the cut - v shaped.
          // Connect the start point to p1 and p2. 
          // Remove triEdge. 
          // Tesselate the tri. 
          // Remove old tri.
          if (isFirstSeg)
          {
            isFirstSeg = false;
std::cout << "Creating first v-shaped cut from start to first edge:\n";
std::cout << "  Remove edge " << e1 << " - " << e2 << "\n";
            EraseSpring(e1, e2);
 
            Particle* p = CreateParticle();
            m_numVerts++;
            int startid = p->GetId();

            // TODO Push the new particles in the direction of the tri normal,
            //  or the avg normal of the tri verts, scaled by some factor.
            // This factor depends on the vert normals, as these give the
            //  curvature of the surface we are approximating.
            // If vert norms are same as tri norm, it's flat, so scale is zero.
            // As vert norms diverge from tri norm, the triangle should be more 
            //  and more curved. At 90 degs this breaks down, which could 
            //  happen for a cut wall.

            // TODO Calc normals for new verts. This is the weighted avg of
            //  the vert normals, weighted by distance to each.
            // (Use barycentric coords ?)

            p->SetPos(start.m_pos);
std::cout << "  Add new particle for start of cut: " << start.m_pos << ": " << startid << "\n";
            // Connect edges from this new particle to the tri verts.
            // This anchors the new particle and tesselates the cut tri as well.
std::cout << "  Create edge " << e1 << " - " << startid << "\n";
             CreateSpring(e1, startid);
std::cout << "  Create edge " << e2 << " - " << startid << "\n";
             CreateSpring(e2, startid);
            // Connect the new cut point and the third tri vert (the one opposite
            //  the edge we removed).
            int e3 = ThirdVert(*tr1, e1, e2);
std::cout << "  Create edge " << e3 << " - " << startid << "\n";
             CreateSpring(e3, startid);
 
            p = CreateParticle();
            m_numVerts++;
            int p1id = p->GetId(); 
            p->SetPos(p1);

            p = CreateParticle();
            m_numVerts++;
            int p2id = p->GetId(); 
            p->SetPos(p2);

            // store the new points on the edge. Connect to the next pair of
            //  (or single) particle we create
            edgep1 = p1id;
            edgep2 = p2id;

std::cout << "  Add new particles for new verts on edge: " << p1id << " and " << p2id << "\n";

std::cout << "  Create edge " << startid << " - " << p1id << "\n";
             CreateSpring(startid, p1id);
std::cout << "  Create edge " << startid << " - " << p2id << "\n";
             CreateSpring(startid, p2id);

            // Rebuild the cut edge that we removed: connect e1 to p1,
            //  and e2 to p2.
std::cout << "  Create edge " << e1 << " - " << p1id << "\n";
             CreateSpring(e1, p1id);
std::cout << "  Create edge " << e2 << " - " << p2id << "\n";
             CreateSpring(e2, p2id);

             FillTriHoles(tr1->m_normal, *tr1, edgep1, edgep2, startid);
          } 
          else
          {
            // This is a 'mid-cut' segment, i.e. between 2 tri edges.
            //  Join old p1 to new p1, and old p2 to new p2. These may cross, so uncross if necessary.
            // start is in the prev tri, cp is in the new tri.
            Particle* p = CreateParticle();
            m_numVerts++;
            int p1id = p->GetId(); 
            p->SetPos(p1);

            p = CreateParticle();
            m_numVerts++;
            int p2id = p->GetId(); 
            p->SetPos(p2);

            // Remove old edge. Create new edge segments. Tesselate the quad.
            EraseSpring(e1, e2);
            // Rebuild the cut edge that we removed: connect e1 to p1,
            //  and e2 to p2.
std::cout << "  Create edge " << e1 << " - " << p1id << "\n";
             CreateSpring(e1, p1id);
std::cout << "  Create edge " << e2 << " - " << p2id << "\n";
             CreateSpring(e2, p2id);

            // Create edges across tri from prev edge to new edge
            // Swap verts if the edges cross
            LineSeg closest;
            float mua, mub;
            const Vec3f& edge1 = GetParticle(edgep1)->GetPos();
            const Vec3f& edge2 = GetParticle(edgep2)->GetPos();
            if (Intersects(LineSeg(edge1, p1), LineSeg(edge2, p2), &closest, &mua, &mub) &&
                mua >= 0 && mua <= 1.0f && mub >= 0 && mub <= 1.0f)
            {
std::cout << "Found crossing edges, mua: " << mua << " mub: " << mub << "\n";
              std::swap(p1id, p2id);
            }

            CreateSpring(edgep1, p1id);
            CreateSpring(edgep2, p2id);

            // Fill holes: 
            // Find out which edge is on the triangle, which is on the quad.
            const Tri& tr = *(start.m_tri);
            if (GetSpring(edgep1, tr.m_particles[0]) && GetSpring(p1id, tr.m_particles[0]))
            {
              // edgep1 - p1id - tr.m_particles[0] is the triangle.
              AddTriWithWinding(tr.m_normal, Tri(edgep1, p1id, tr.m_particles[0]));
              // edgep2 - p2id -   tr.m_particles[1] - tr.m_particles[2] is a quad.
              AddQuad(tr.m_normal, edgep2, p2id, tr.m_particles[1], tr.m_particles[2]);
            }
            else if (GetSpring(edgep1, tr.m_particles[1]) && GetSpring(p1id, tr.m_particles[1]))
            {
              AddTriWithWinding(tr.m_normal, Tri(edgep1, p1id, tr.m_particles[1]));
              AddQuad(tr.m_normal, edgep2, p2id, tr.m_particles[0], tr.m_particles[2]);
            }
            else if (GetSpring(edgep1, tr.m_particles[2]) && GetSpring(p1id, tr.m_particles[2]))
            {
              AddTriWithWinding(tr.m_normal, Tri(edgep1, p1id, tr.m_particles[2]));
              AddQuad(tr.m_normal, edgep2, p2id, tr.m_particles[0], tr.m_particles[1]);
            }
            else // edgep1 - p1id is an edge on the quad
            if (GetSpring(edgep2, tr.m_particles[0]) && GetSpring(p2id, tr.m_particles[0]))
            {
              AddTriWithWinding(tr.m_normal, Tri(edgep2, p2id, tr.m_particles[0]));
              AddQuad(tr.m_normal, edgep1, p1id, tr.m_particles[1], tr.m_particles[2]);
            }
            else if (GetSpring(edgep2, tr.m_particles[1]) && GetSpring(p2id, tr.m_particles[1]))
            {
              AddTriWithWinding(tr.m_normal, Tri(edgep2, p2id, tr.m_particles[1]));              
              AddQuad(tr.m_normal, edgep1, p1id, tr.m_particles[0], tr.m_particles[2]);
            }
            else
            {
              Assert(GetSpring(edgep2, tr.m_particles[2]) && GetSpring(p2id, tr.m_particles[2]));
              AddTriWithWinding(tr.m_normal, Tri(edgep2, p2id, tr.m_particles[2]));
              AddQuad(tr.m_normal, edgep1, p1id, tr.m_particles[1], tr.m_particles[0]);
            }

            // store the new points on the edge. Connect to the next pair of
            //  (or single) particle we create
            edgep1 = p1id;
            edgep2 = p2id;
          }
        }
      }

      start = cp;
    }  
  }
  const CutPoint& end = cutline.back();
  m_edgePoints.push_back(EdgePoint(end.m_pos, end.m_tri, Edge(-1, -1)));

  for (auto it = m_edgePoints.begin(); it != m_edgePoints.end(); ++it)
  {
    EdgePoint& ep = *it;
    if (m_trilist.count(ep.m_tri))
    {
      m_trilist.erase(ep.m_tri);
    }
    ep.m_tri = nullptr;
  }
}

void Squishy::EndCut(const LineSeg&, float cutDepth)
{
  int numLines = m_cutLines.size();
  // TODO Could be one or more lines
  switch (numLines)
  {
  case 0:
    return;

  case 1:
    // Cutting into the squishy but not all the way through to the back
    CutInto(m_cutLines[0]);

    break;

  case 2:
    // Cutting at least partially all the way through. 
    // This wil make a hole where points along the front line are matched by points
    //  along the line at the back.
    break;

  default:
    // Squishy is folded, and we are cutting through the layers
    break;
  }
}

void Squishy::SetNumVerts(int n)
{
  m_numVerts = n;
}

bool Squishy::Init(const std::string& objFilename, float k)
{
  if (!SquishyLoadObj(this, objFilename))
  {
    return false;
  }

  return true; 
}

void Squishy::Update()
{
  // TODO do nothing if inert

  SpringSystem::Update();

  // Recalc normals
  m_normals.clear();
  m_normals.resize(m_numVerts);

  for (auto it = m_trilist.begin(); it != m_trilist.end(); ++it)
  {
    Tri* tri = *it;

    Particle* particle[3] = 
    {
      GetParticle(tri->m_particles[0]),
      GetParticle(tri->m_particles[1]),
      GetParticle(tri->m_particles[2])
    };
    Vec3f pos[3] = 
    {
      particle[0]->GetPos(),
      particle[1]->GetPos(),
      particle[2]->GetPos()
    };
    Vec3f v1 = pos[1] - pos[0];
    Vec3f v2 = pos[2] - pos[0];
    Vec3f n = CrossProduct(v1, v2);
    n.Normalise();
    tri->m_normal = n;
  
    // Smooth normals
    Assert(tri->m_particles[0] < m_numVerts);
    Assert(tri->m_particles[1] < m_numVerts);
    Assert(tri->m_particles[2] < m_numVerts);

    m_normals[tri->m_particles[0]] += n; 
    m_normals[tri->m_particles[1]] += n; 
    m_normals[tri->m_particles[2]] += n; 
  }
  for (int i = 0; i < m_numVerts; i++)
  {
    if (m_normals[i].SqLen() > 0)
    {
      m_normals[i].Normalise();
    }
  }
}

void Squishy::Draw()
{
  if (m_drawSpringSystem)
  {
    SpringSystem::Draw();
  }

  if (m_drawTris)
  {
    AmjuGL::Disable(AmjuGL::AMJU_TEXTURE_2D);
    AmjuGL::Enable(AmjuGL::AMJU_BLEND);
    PushColour();
    MultColour(Colour(1, 1, 1, 0.8f));

    glEnable(GL_CULL_FACE); 

    glBegin(GL_TRIANGLES);
    for (auto it = m_trilist.begin(); it != m_trilist.end(); ++it)
    {
      Tri* tri = *it;

      Particle* particle[3] = 
      {
        GetParticle(tri->m_particles[0]),
        GetParticle(tri->m_particles[1]),
        GetParticle(tri->m_particles[2])
      };
      Vec3f pos[3] = 
      {
        particle[0]->GetPos(),
        particle[1]->GetPos(),
        particle[2]->GetPos()
      };
      Vec3f normal[3] = 
      {
        m_normals[tri->m_particles[0]],
        m_normals[tri->m_particles[1]],
        m_normals[tri->m_particles[2]],
      };

      Colour col(1, 1, 1, 1);
      if (tri->m_selected)
      {
        col = Colour(1, 0, 0, 1);
      }
      PushColour();
      MultColour(col);

      glNormal3f(normal[0].x, normal[0].y, normal[0].z);
      glVertex3f(pos[0].x, pos[0].y, pos[0].z);

      glNormal3f(normal[1].x, normal[1].y, normal[1].z);
      glVertex3f(pos[1].x, pos[1].y, pos[1].z);

      glNormal3f(normal[2].x, normal[2].y, normal[2].z);
      glVertex3f(pos[2].x, pos[2].y, pos[2].z);

      PopColour();
    }

    glEnd(); 

    PopColour();
  }


  AmjuGL::Disable(AmjuGL::AMJU_BLEND);
  AmjuGL::Disable(AmjuGL::AMJU_LIGHTING);
  AmjuGL::Disable(AmjuGL::AMJU_DEPTH_READ);

  glLineWidth(3);
  glPointSize(3);
  PushColour();
  MultColour(Colour(0, 1, 0, 1));
  for (int i = 0; i < (int)m_cutLines.size(); i++)
  {
    glBegin(GL_POINTS);
    CutLine& cl = m_cutLines[i];
    for (int j = 0; j < (int)cl.size(); j++)
    {
      CutPoint& cp = cl[j];
      glVertex3f(cp.m_pos.x, cp.m_pos.y, cp.m_pos.z);
    } 
    glEnd();  
  }

  glColor3f(1, 1, 1);
  // Draw simplified line ?
  /*
  glBegin(GL_LINE_STRIP);
  for (int i = 0; i < m_edgePoints.size(); i++)
  {
    Vec3f& v = m_edgePoints[i].m_pos; 
    glVertex3f(v.x, v.y, v.z);
  }
  glEnd();
  */

  PopColour();
}

void Squishy::AddForce(const Vec3f& pos, const Vec3f& dir)
{
}
  
// p1 - p4 are verts on a quad but not necessarily in either winding order.
// Find the topology and fill in one of the two diagonals, then fill in the holes with tris.
void Squishy::AddQuad(const Vec3f& normal, int p1, int p2, int p3, int p4)
{
  // Tesselate quad and add triangles to fill the holes.
  // p1 and p2 should be connected (asserts). p3 and p4 are the other 2 verts of a quad, but we
  //  don't know about the other edges. I.e. the quad could be p1-p2-p3-p4-p1 OR p1-p2-p4-p3-p1.

  Assert(GetSpring(p1, p2));
  Assert(GetSpring(p3, p4)); // the other two verts must be connected, right?

  if (GetSpring(p2, p3))
  {
    Assert(GetSpring(p4, p1)); 

    CreateSpring(p1, p3); // or (p2, p4) would work. TODO Decide on best one - shortest length?
    
    // p1 and p2 are connected; p2 and p3 are connected; p3 and p1 are connected
    AddTriWithWinding(normal, Tri(p1, p2, p3)); 

    // p1 and p3 are connected;  p3 and p4 are connected; p4 and p1 are connected
    AddTriWithWinding(normal, Tri(p1, p3, p4)); 
  }
  else
  {
    Assert(GetSpring(p2, p4));

    Assert(GetSpring(p3, p1));

    CreateSpring(p1, p4); // or (p2, p3) would work. TODO Decide on best one - shortest length?

    AddTriWithWinding(normal, Tri(p1, p2, p4)); 
    AddTriWithWinding(normal, Tri(p1, p3, p4)); 
  }
}

void Squishy::AddTri(const Tri& tri)
{
  m_trilist.insert(new Tri(tri));
}

void Squishy::AddTriWithWinding(const Vec3f& normal, Tri tri)
{
  Assert(GetParticle(tri.m_particles[0]));
  Assert(GetParticle(tri.m_particles[1]));
  Assert(GetParticle(tri.m_particles[2]));

  const Vec3f& p0 = GetParticle(tri.m_particles[0])->GetPos();
  const Vec3f& p1 = GetParticle(tri.m_particles[1])->GetPos();
  const Vec3f& p2 = GetParticle(tri.m_particles[2])->GetPos();

  Vec3f triNorm = CrossProduct(p1 - p0, p2 - p0);
  triNorm.Normalise(); // ? necessary ?
  if (DotProduct(normal, triNorm) < 0)
  {
    std::swap(tri.m_particles[0], tri.m_particles[1]); // flip winding order
  }
  AddTri(tri);
}

}

