#pragma once

#include <Singleton.h>
#include <TriList.h>
#include <DepthOfField.h>
#include "GSBase.h"

namespace Amju 
{
class GSDepthOfField : public GSBase
{
  GSDepthOfField();
  friend class Singleton<GSDepthOfField>;

public:
  virtual void Update();
  virtual void DrawScene();
  virtual void OnActive();

private:
  RCPtr<DepthOfField> m_dof;
  Shader* m_shader;
};
typedef Singleton<GSDepthOfField> TheGSDepthOfField;
} // namespace

