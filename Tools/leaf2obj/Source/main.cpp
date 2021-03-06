#include <iostream>
#include <ObjMesh.h>
#include <AmjuGL-Null.h>
#include "Loader.h"

#ifdef WIN32
#ifdef _DEBUG
#pragma comment(lib, "../../../../../Build/Debug/AmjuLibMsvc.lib")
#else
#pragma comment(lib, "../../../../../Build/Release/AmjuLibMsvc.lib")
#endif // _DEBUG
#endif // WIN32

using namespace Amju;

int main(int argc, char** argv)
{
  if (argc != 2)
  {
    std::cout << "Leaf2Obj converts .leaf and .comp files to .obj\nUsage: leaf2obj <input file>\n";
    return 1;
  }

  AmjuGL::SetImpl(new AmjuGLNull);

  // Resource manager was changed so we don't have to create a group for
  //  every obj we want to load.
//	TheResourceManager::Instance()->LoadResourceGroup("crate2-group");

  std::string inFilename = argv[1]; 

  if (!Load(inFilename))
  {
    std::cout << "Error loading file " << inFilename << "\n";
    return 0; 
  }

  return 0;
}
