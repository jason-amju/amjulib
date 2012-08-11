#include "ROConfig.h"

namespace Amju
{
static const char* RO_CONFIG_FILENAME = "roconfig.txt";
void SaveROConfig()
{
  ROConfig()->Save(RO_CONFIG_FILENAME);
}

ConfigFile* ROConfig()
{
  static ConfigFile* cf = 0;
  if (!cf)
  {
    cf = new ConfigFile;
	  cf->Load(RO_CONFIG_FILENAME);
	  atexit(SaveROConfig);
  }
  return cf;
}

}