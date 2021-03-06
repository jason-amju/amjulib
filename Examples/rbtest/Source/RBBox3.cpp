#include "RBBox3.h"
#include "DrawOBB3.h"
#include "Timer.h"
#include <Sphere.h>
#include <DrawSphere.h>
#include <AmjuGL.h>
#include <iostream>
#include "RBManager.h"

//#define RBBOX3_DEBUG

namespace Amju
{
const RBBox3::TypeName RBBox3::TYPENAME = "rbbox3";

RBBox3::RBBox3()
{
  SetSize(Vec3f(1, 1, 1));
}

void RBBox3::SetSize(const Vec3f& s)
{
  m_obb3.SetExtents(s);
}

bool FindContact(const RBBox3& box1, const RBBox3& box2, Contact3* c)
{
  // Check for collision: SAT, ignores "tunnelling" for now, i.e. ignores velocities!
  if (!Intersects(box1.m_obb3, box2.m_obb3))
  {
    return false;
  }

#ifdef RBBOX3_DEBUG
std::cout << "Got intersection, box " << box1.GetId() << " and " << box2.GetId() << " - looking for contact info...\n";
#endif

  // Vertex of box1 may intersect a face of box2, or vice versa. So we call FindContact
  //  twice, swapping the parameters.
  Contact3 c1, c2;
  // Vert(s) from box1 inside box2 ?
  bool box1InBox2 = box1.FindContact(box2, &c1);

#ifdef RBBOX3_DEBUG
if (box1InBox2)
{
  std::cout << "C1 Contact normal: x: " << c1.m_contactNormal.x << " y: " << c1.m_contactNormal.y << " z: " << c1.m_contactNormal.z << "\n";
}
#endif

  // Vert(s) from box2 inside box1 ?
  bool box2InBox1 = box2.FindContact(box1, &c2);

#ifdef RBBOX3_DEBUG
if (box2InBox1)
{
  std::cout << "C2 Contact normal: x: " << c2.m_contactNormal.x << " y: " << c2.m_contactNormal.y << " z: " << c2.m_contactNormal.z << "\n";
}
#endif

  if (box1InBox2 && box2InBox1)
  {
    // Intersecting each other. Get avg contact point. Make contact normal always w.r.t. box2
    *c = c1;
    c->m_penetrationDepth = std::max(c1.m_penetrationDepth, c2.m_penetrationDepth); // so we detach fully
    c->m_pos = (c1.m_pos + c2.m_pos) * 0.5f; // mid point of contact points
    return true;
  }
  else if (box1InBox2)
  {
    *c = c1;
    return true;
  }
  else if (box2InBox1)
  {
    *c = c2;
    c->m_contactNormal = -c->m_contactNormal;
    return true;
  }

#ifdef RBBOX3_DEBUG
std::cout << "SAT: box3 collision! But no contact info!?\n";
//  Assert(0);
#endif

  return false;
}

bool RBBox3::FindContact(const RBBox3& b, Contact3* c) const
{
  // Find vertex-face contacts
  // For each vert, check if it intersects other box (behind all planes).
  // We collect contact normal and pen depth for all intersecting verts, then get the avg.
  // This is so if, say, two of our verts intersect the same face of b, we get the avg 
  // pen depth and avg contact normal.

  Vec3f corners[8];
  m_obb3.GetCorners(corners);

  float avgPd = 0;
  Vec3f avgPos;
  int numPenetratingVerts = 0;
  Vec3f contactNormal;

  for (int i = 0; i < 8; i++)
  {
#ifdef RBBOX3_DEBUG
std::cout << "Checking vert " << i << "\n";
#endif

    float pd = 0;
    Vec3f cn;
    // Intersection test, also gets contact normal and penetration depth (i.e. dist behind plane) 
    if (b.m_obb3.Intersects(corners[i], &cn, &pd))
    {
      numPenetratingVerts++;
      avgPd += pd;

#ifdef RBBOX3_DEBUG
std::cout << " yes, cn: x: " << cn.x << " y: " << cn.y << " z: " << cn.z << "\n";
#endif

      // If we average the contact normal we can end up with the Zero vector.
      // TODO How do we choose the contact normal ??
      contactNormal += cn;

#ifdef RBBOX3_DEBUG
std::cout << "CN: x: " << contactNormal.x << " y: " << contactNormal.y << " z: " << contactNormal.z << "\n";
#endif

      avgPos += corners[i];
    }
  }

  static const float NO_NORMAL = 0.00010f;
  // Ignore if contact normal is too small
  if (numPenetratingVerts > 0)
  {
    if (contactNormal.SqLen() > NO_NORMAL)
    {
      avgPd /= (float)numPenetratingVerts;
      // Get average of positions of penetrating verts, i.e. the centre of the points
      avgPos *= (1.0f / (float)numPenetratingVerts);
      contactNormal.Normalise();

      c->m_pos = avgPos;
      c->m_contactNormal = contactNormal;
      c->m_penetrationDepth = avgPd;
      return true;
    }
    else
    {
#ifdef RBBOX3_DEBUG
std::cout << "BAD CONTACT NORMAL! (Vertex in box)\n";
std::cout << "CN: x: " << contactNormal.x << " y: " << contactNormal.y << " z: " << contactNormal.z << "\n";
#endif
    }
  }
  
  // No vertex-face intersection. Check for edge-edge intersection.

#ifdef RBBOX3_DEBUG
std::cout << "No vertex-face intersection. Check for edge-edge intersection.\n";
#endif

  // Get each edge of this box. Test against box b 
  //bool Intersects(const LineSeg& e, LineSeg* clip, Vec3f* contactNormal, float* penDepth) const;

  LineSeg e[12];
  m_obb3.GetEdges(e);
  int numPenEdges = 0;
  for (int i = 0; i < 12; i++)
  {
    float pd = 0;
    Vec3f cn;
    LineSeg clip;

#ifdef RBBOX3_DEBUG
std::cout << "Edge-box: checking edge " << i << "\n";
#endif

    if (b.m_obb3.Intersects(e[i], &clip, &cn, &pd))
    {
      numPenEdges++;
      avgPd += pd;
#ifdef RBBOX3_DEBUG
std::cout << " yes, cn: x: " << cn.x << " y: " << cn.y << " z: " << cn.z << "\n";
#endif

      // If we average the contact normal we can end up with the Zero vector.
      // TODO How do we choose the contact normal ??
      contactNormal += cn;

#ifdef RBBOX3_DEBUG
std::cout << "CN: x: " << contactNormal.x << " y: " << contactNormal.y << " z: " << contactNormal.z << "\n";
#endif

      Vec3f mid = (clip.p0 + clip.p1) * 0.5f;
      avgPos += mid;
    }
  }

  if (numPenEdges > 0)
  {
    if (contactNormal.SqLen() > NO_NORMAL)
    {
      float oneOver = 1.0f / (float)numPenEdges;
      avgPd *= oneOver;
      avgPos *= oneOver; 
      contactNormal.Normalise();

      c->m_pos = avgPos;
      c->m_contactNormal = contactNormal;
      c->m_penetrationDepth = avgPd;
      return true;
    }
    else
    {
#ifdef RBBOX3_DEBUG
std::cout << "BAD CONTACT NORMAL! (edge in box)\n";
std::cout << "CN: x: " << contactNormal.x << " y: " << contactNormal.y << " z: " << contactNormal.z << "\n";
#endif
    }
  }

#ifdef RBBOX3_DEBUG
std::cout << "Nope, no edge intersections!\n";
#endif

  return false;
}

void RBBox3::Update()
{
  // damping
  m_angVel *= 0.99f;
  m_torques *= 0.99f;
  m_forces *= 0.99f;

  RigidBody::Update();

  // Update OBB
  m_obb3.SetCentre(m_pos);
  m_obb3.SetOrientation(m_rot);
}

void RBBox3::Draw()
{
  DrawOBB3(m_obb3);

  DrawSolidOBB3(m_obb3);

/*
  Vec3f corners[8];
  m_obb3.GetCorners(corners);

  const float RADIUS = 0.1f;

  for (int i = 0; i < 8; i++)
  { 
    PushColour();
    if (0)
    {
      MultColour(Colour(1, 0, 0, 1));
    }
    Sphere s(corners[i], RADIUS);
    DrawSphere(s);
    PopColour();
  }
*/
}

void RBBox3Collision(RBBox3* box1, RBBox3* box2)
{
  Contact3 c;

  if (FindContact(*box1, *box2, &c))
  {
#ifdef RBBOX3_DEBUG
std::cout << "Checking boxes: " << box1->GetId() << " and " << box2->GetId() << "\n";
std::cout << "Box3-box3 Contact!\n";
#endif

    // Got penetration. Move away in dir of contact normal.
    Vec3f contactNormal = c.m_contactNormal;

#ifdef RBBOX3_DEBUG
std::cout << "Contact normal: x: " << contactNormal.x << " y: " << contactNormal.y << " z: " << contactNormal.z << "\n";
std::cout << "Contact point: x: " << c.m_pos.x << " y: " << c.m_pos.y << " z: " << c.m_pos.z << "\n";
#endif

    // "Resolve" penetration: bodge it by moving away in direction of
    //  contact normal, by distance <penetration depth>
    // Don't call SetPos, this will zero pos diff for Verlet???

    // Move away, proportional to relative mass
    float totalInvMass = box1->m_invMass + box2->m_invMass;
    float frac1 = box1->m_invMass / totalInvMass;
    float frac2 = 1.0f - frac1;

    box1->m_pos += contactNormal * (c.m_penetrationDepth * frac1);
    box2->m_pos -= contactNormal * (c.m_penetrationDepth * frac2);

    // vel gets velChange added. If other object is immovable, v gets 2*velChange
    // and we get the reflection vector.
    // TODO!!!!
    Vec3f relvel = box1->m_vel - box2->m_vel;
    float relSpeed = sqrt(relvel.SqLen());

    // Each box has proj of rel velocity onto contact normal added to vel.
    // This gives reflect vector if one box is immovable
    float dp = DotProduct(relvel, contactNormal);

    box1->m_vel -= frac1 * 2.0f * dp * contactNormal;
    box2->m_vel += frac2 * 2.0f * dp * contactNormal;

    // TODO TEMP TEST
    // Dampen the response - this happens once per collision, so not multiplied by dt etc
    box1->m_vel *= 0.5f;
    box2->m_vel *= 0.5f;

    // Apply Torque
    // Mag of torque depends on how fast the contact point was moving
    //  relative to the face it penetrated.
    // i.e. Linear vel + rotational vel

    float relSpeedMult = 0.1f; 
    float angVelMult = 0.02f; 
    float mag = relSpeed * relSpeedMult; // TODO + totalAngVel * angVelMult;

    box1->AddTorque(-mag * contactNormal, c.m_pos);
    box2->AddTorque( mag * contactNormal, c.m_pos);


    // This seems useless, as we are explicitly changing vel above, it doesn't
    //  make any difference -- except that it seems to have an effect on how
    //  things settle down.
    //AddForce(mag * contactNormal);
  }
}
static bool regRBBox3Collision = TheRBManager::Instance()->AddRBFunc(
  RBBox3::TYPENAME, RBBox3::TYPENAME, (RBManager::RBFunc)RBBox3Collision);

}
