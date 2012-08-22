#ifndef _MD3_H
#define _MD3_H

#include <iostream>
#include <string>
#include <vector>
#include <File.h>
#include <Endian.h>
#include "Md3Types.h"

// This file stores all of our structures and classes (besides the modular model ones in main.h)
// in order to read in and display a Quake3 character.  The file format is of type
// .MD3 and comes in many different files for each main body part section.  We convert
// these Quake3 structures to our own structures in Md3.cpp so that we are not dependant
// on their model data structures.  You can do what ever you want, but I like mine :P :)


//////////// *** NEW *** ////////// *** NEW *** ///////////// *** NEW *** ////////////////////

// These defines are used to pass into GetModel(), which is member function of CModelMD3

#define kLower	0			// This stores the ID for the legs model
#define kUpper	1			// This stores the ID for the torso model
#define kHead	2			// This stores the ID for the head model
#define kWeapon	3			// This stores the ID for the weapon model

// This enumeration stores all the animations in order from the config file (.cfg)
typedef enum 
{
	// If one model is set to one of the BOTH_* animations, the other one should be too,
	// otherwise it looks really bad and confusing.

	BOTH_DEATH1 = 0,		// The first twirling death animation
	BOTH_DEAD1,				// The end of the first twirling death animation
	BOTH_DEATH2,			// The second twirling death animation
	BOTH_DEAD2,				// The end of the second twirling death animation
	BOTH_DEATH3,			// The back flip death animation
	BOTH_DEAD3,				// The end of the back flip death animation

	// The next block is the animations that the upper body performs

	TORSO_GESTURE,			// The torso's gesturing animation
	
	TORSO_ATTACK,			// The torso's attack1 animation
	TORSO_ATTACK2,			// The torso's attack2 animation

	TORSO_DROP,				// The torso's weapon drop animation
	TORSO_RAISE,			// The torso's weapon pickup animation

	TORSO_STAND,			// The torso's idle stand animation
	TORSO_STAND2,			// The torso's idle stand2 animation

	// The final block is the animations that the legs perform

	LEGS_WALKCR,			// The legs's crouching walk animation
	LEGS_WALK,				// The legs's walk animation
	LEGS_RUN,				// The legs's run animation
	LEGS_BACK,				// The legs's running backwards animation
	LEGS_SWIM,				// The legs's swimming animation
	
	LEGS_JUMP,				// The legs's jumping animation
	LEGS_LAND,				// The legs's landing animation

	LEGS_JUMPB,				// The legs's jumping back animation
	LEGS_LANDB,				// The legs's landing back animation

	LEGS_IDLE,				// The legs's idle stand animation
	LEGS_IDLECR,			// The legs's idle crouching animation

	LEGS_TURN,				// The legs's turn animation

	MAX_ANIMATIONS			// The define for the maximum amount of animations
} eAnimations;

//////////// *** NEW *** ////////// *** NEW *** ///////////// *** NEW *** ////////////////////


// This holds the header information that is read in at the beginning of the file
struct tMd3Header
{ 
	char	fileID[4];					// This stores the file ID - Must be "IDP3"
	int		version;					// This stores the file version - Must be 15
	char	strFile[68];				// This stores the name of the file
	int		numFrames;					// This stores the number of animation frames
	int		numTags;					// This stores the tag count
	int		numMeshes;					// This stores the number of sub-objects in the mesh
	int		numMaxSkins;				// This stores the number of skins for the mesh
	int		headerSize;					// This stores the mesh header size
	int		tagStart;					// This stores the offset into the file for tags
	int		tagEnd;						// This stores the end offset into the file for tags
	int		fileSize;					// This stores the file size

        void Endianise()
        {
          version = Amju::Endian(version);
          numFrames = Amju::Endian(numFrames);
          numTags = Amju::Endian(numTags);
          numMeshes = Amju::Endian(numMeshes);
          numMaxSkins = Amju::Endian(numMaxSkins);
          headerSize = Amju::Endian(headerSize);
          tagStart = Amju::Endian(tagStart);
          tagEnd = Amju::Endian(tagEnd);
          fileSize = Amju::Endian(fileSize);
        }

        friend std::ostream& operator<<(std::ostream& os, const tMd3Header& h)
        {
          os << "HEADER: \n" << h.fileID[0] << h.fileID[1] << h.fileID[2] << h.fileID[3] << "\n"
             << "Ver: " << h.version << "\n"
             << "str file: " << h.strFile << "\n"
             << "numFrames: " << h.numFrames << "\n"
             << "numTags: " << h.numTags << "\n"
             << "numMeshes: " << h.numMeshes << "\n"
             << "numMaxSkins: " << h.numMaxSkins << "\n"
             << "headerSize: " << h.headerSize << "\n"
             << "tagStart: " << h.tagStart << "\n"
             << "tagEnd: " << h.tagEnd << "\n"
             << "fileSize: " << h.fileSize << "\n";
          return os;
        }
};

// This structure is used to read in the mesh data for the .md3 models
struct tMd3MeshInfo
{
	char	meshID[4];					// This stores the mesh ID (We don't care)
	char	strName[68];				// This stores the mesh name (We do care)
	int		numMeshFrames;				// This stores the mesh aniamtion frame count
	int		numSkins;					// This stores the mesh skin count
	int     numVertices;				// This stores the mesh vertex count
	int		numTriangles;				// This stores the mesh face count
	int		triStart;					// This stores the starting offset for the triangles
	int		headerSize;					// This stores the header size for the mesh
	int     uvStart;					// This stores the starting offset for the UV coordinates
	int		vertexStart;				// This stores the starting offset for the vertex indices
	int		meshSize;					// This stores the total mesh size

  void Endianise()
  {
    numMeshFrames = Amju::Endian(numMeshFrames);
    numSkins = Amju::Endian(numSkins);
    numVertices = Amju::Endian(numVertices);
    numTriangles = Amju::Endian(numTriangles);
    triStart = Amju::Endian(triStart);
    headerSize = Amju::Endian(headerSize);
    uvStart = Amju::Endian(uvStart);
    vertexStart = Amju::Endian(vertexStart);
    meshSize = Amju::Endian(meshSize);
  }
};

// This is our tag structure for the .MD3 file format.  These are used link other
// models to and the rotate and transate the child models of that model.
struct tMd3Tag
{
	char		strName[64];			// This stores the name of the tag (I.E. "tag_torso")
	CVector3	vPosition;				// This stores the translation that should be performed
	float		rotation[3][3];			// This stores the 3x3 rotation matrix for this frame

        void Endianise()
        {
            vPosition.Endianise();
            for (int k = 0; k < 3; k++)
            {
              for (int j = 0; j < 3; j++)
              {
                rotation[j][k] = Amju::Endian(rotation[j][k]);
              }
            } 
        }
};

// This stores the bone information (useless as far as I can see...)
struct tMd3Bone
{
	float	mins[3];					// This is the min (x, y, z) value for the bone
	float	maxs[3];					// This is the max (x, y, z) value for the bone
	float	position[3];				// This supposedly stores the bone position???
	float	scale;						// This stores the scale of the bone
	char	creator[16];				// The modeler used to create the model (I.E. "3DS Max")

  void Endianise()
  {
    for (int i = 0; i < 3; i++)
    {
      mins[i] = Amju::Endian(mins[i]);
      maxs[i] = Amju::Endian(maxs[i]);
      position[i] = Amju::Endian(position[i]);
    }
  }

};


// This stores the normals and vertex indices 
struct tMd3Triangle
{
  signed short	 vertex[3];				// The vertex for this face (scale down by 64.0f)
  unsigned char normal[2];				// This stores some crazy normal values (not sure...)

  void Endianise()
  {
    for (int i = 0; i < 3; i++)
    {
      vertex[i] = Amju::Endian(vertex[i]);
    }
  }
};


// This stores the indices into the vertex and texture coordinate arrays
struct tMd3Face
{
  int vertexIndices[3];				

  void Endianise()
  {
    for (int i = 0; i < 3; i++)
    {
      vertexIndices[i] = Amju::Endian(vertexIndices[i]);
    }
  }
};


// This stores UV coordinates
struct tMd3TexCoord
{
  float textureCoord[2];

  void Endianise()
  {
    for (int i = 0; i < 2; i++)
    {
      textureCoord[i] = Amju::Endian(textureCoord[i]);
    }
  }
};


// This stores a skin name (We don't use this, just the name of the model to get the texture)
struct tMd3Skin 
{
	char strName[68];
  void Endianise() {}
};


// (* NEW NOTE *) The CLoadMD3 class does not change in this tutorial (* NEW NOTE *)

// This class handles all of the main loading code
class CLoadMD3
{

public:

	// This inits the data members
	CLoadMD3();								

	// This is the function that you call to load the MD3 model
	bool ImportMD3(t3DModel *pModel, const char * strFileName);

	// This loads a model's .skin file
	bool LoadSkin(t3DModel *pModel, const char *  strSkin);

	// This loads a weapon's .shader file
	bool LoadShader(t3DModel *pModel, const char *  strShader);

private:


	// This reads in the data from the MD3 file and stores it in the member variables,
	// later to be converted to our cool structures so we don't depend on Quake3 stuff.
	void ReadMD3Data(t3DModel *pModel);

	// This converts the member variables to our pModel structure, and takes the model
	// to be loaded and the mesh header to get the mesh info.
	void ConvertDataStructures(t3DModel *pModel, tMd3MeshInfo meshHeader);

	// This frees memory and closes the file
	void CleanUp();
	
	// Member Variables		

	// The file pointer
	FILE *m_FilePointer;

        Amju::SharedPtr<Amju::File> m_pFile; // Amju File
        unsigned int m_bytesRead; // bytes read so far from above file

	tMd3Header				m_Header;			// The header data

	tMd3Skin				*m_pSkins;			// The skin name data (not used)
	tMd3TexCoord			*m_pTexCoords;		// The texture coordinates
	tMd3Face				*m_pTriangles;		// Face/Triangle data
	tMd3Triangle			*m_pVertices;		// Vertex/UV indices
	tMd3Bone				*m_pBones;			// This stores the bone data (not used)
};

// This is our model class that we use to load and draw and free the Quake3 characters
class CModelMD3
{

public:

	// These our our init and deinit() C++ functions (Constructor/Deconstructor)
	CModelMD3();
	~CModelMD3();
	
	// This loads the model from a path and name prefix.   It takes the path and
	// model name prefix to be added to _upper.md3, _lower.md3 or _head.md3.
	bool LoadModel(const char *  strPath, const char *  strModel);

	// This loads the weapon and takes the same path and model name to be added to .md3
	bool LoadWeapon(const char * strPath, const char * strModel);

	// This links a model to another model (pLink) so that it's the parent of that child.
	// The strTagName is the tag, or joint, that they will be linked at (I.E. "tag_torso").
	void LinkModel(t3DModel *pModel, t3DModel *pLink, const char * strTagName);


//////////// *** NEW *** ////////// *** NEW *** ///////////// *** NEW *** ////////////////////

	// This takes a string of an animation and sets the torso animation accordingly
	void SetTorsoAnimation(const char * strAnimation);

	// This takes a string of an animation and sets the legs animation accordingly
	void SetLegsAnimation(const char * strAnimation);

//////////// *** NEW *** ////////// *** NEW *** ///////////// *** NEW *** ////////////////////


	// This renders the character to the screen
	void DrawModel();

	// This frees the character's data
	void DestroyModel(t3DModel *pModel);


//////////// *** NEW *** ////////// *** NEW *** ///////////// *** NEW *** ////////////////////

	// This returns a pointer to a .md3 model in the character (kLower, kUpper, kHead, kWeapon)
	t3DModel *GetModel(int whichPart);

//////////// *** NEW *** ////////// *** NEW *** ///////////// *** NEW *** ////////////////////
	
private:

	// This loads the models textures with a given path
	bool LoadModelTextures(t3DModel *pModel, const char * strPath);


//////////// *** NEW *** ////////// *** NEW *** ///////////// *** NEW *** ////////////////////

	// This loads the animation config file (.cfg) for the character
	bool LoadAnimations(const char * strConfigFile);

	// This updates the models current frame of animation, and calls SetCurrentTime()
	void UpdateModel(t3DModel *pModel);

	// This sets the lastTime, t, and the currentFrame of the models animation when needed
	void SetCurrentTime(t3DModel *pModel);

//////////// *** NEW *** ////////// *** NEW *** ///////////// *** NEW *** ////////////////////


	// This recursively draws the character models, starting with the lower.md3 model
	void DrawLink(t3DModel *pModel);

	// This a md3 model to the screen (not the whole character)
	void RenderModel(t3DModel *pModel);

	// Member Variables

	// This stores the texture array for each of the textures assigned to this model
	unsigned int /*UINT*/ m_Textures[MAX_TEXTURES];	

	// This stores a list of all the names of the textures that have been loaded.  
	// This was created so that we could check to see if a texture that is assigned
	// to a mesh has already been loaded.  If so, then we don't need to load it again
	// and we can assign the textureID to the same textureID as the first loaded texture.
	// You could get rid of this variable by doing something tricky in the texture loading
	// function, but I didn't want to make it too confusing to load the textures.
	std::vector<std::string> strTextures;

	// These are are models for the character's head and upper and lower body parts
	t3DModel m_Head;
	t3DModel m_Upper;
	t3DModel m_Lower;

	// This store the players weapon model (optional load)
	t3DModel m_Weapon;
};


/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

// This is our quaternion class
class CQuaternion 
{

public:

	// This is our default constructor, which initializes everything to an identity
	// quaternion.  An identity quaternion has x, y, z as 0 and w as 1.
	CQuaternion() { x = y = z = 0.0f;   w = 1.0f;  }
	
	// Creates a constructor that will allow us to initialize the quaternion when creating it
	CQuaternion(float X, float Y, float Z, float W) 
	{ 
		x = X;		y = Y;		z = Z;		w = W;
	}

	// This takes in an array of 16 floats to fill in a 4x4 homogeneous matrix from a quaternion
	void CreateMatrix(float *pMatrix);

	// This takes a 3x3 or 4x4 matrix and converts it to a quaternion, depending on rowColumnCount
	void CreateFromMatrix(float *pMatrix, int rowColumnCount);

	// This returns a spherical linear interpolated quaternion between q1 and q2, according to t
	CQuaternion Slerp(CQuaternion &q1, CQuaternion &q2, float t);

private:

	// This stores the 4D values for the quaternion
	float x, y, z, w;
};

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *


#endif


/////////////////////////////////////////////////////////////////////////////////
//
// * QUICK NOTES * 
// 
// There were a bunch of functions added to this file, since the last MD3 tutorial.
// We added an enum for the animation indices, along with some defines for GetModel().
//
// The CLoadMD3 class stayed the same, but these functions were added to our CLoadMD3 class:
//
//	This takes a string of an animation and sets the torso animation accordingly
//	void SetTorsoAnimation(LPSTR strAnimation);
//
//	This takes a string of an animation and sets the legs animation accordingly
//	void SetLegsAnimation(LPSTR strAnimation);
//
//	This returns a pointer to a .md3 model in the character (kLower, kUpper, kHead, kWeapon)
//	t3DModel *GetModel(int whichPart);
//
//	This loads the animation config file (.cfg) for the character
//	bool LoadAnimations(LPSTR strConfigFile);
//
//	This updates the models current frame of animation, and calls SetCurrentTime()
//	void UpdateModel(t3DModel *pModel);
//
//	This sets the lastTime, t, and the currentFrame of the models animation when needed
//	void SetCurrentTime(t3DModel *pModel);
//
// We also added a CQuaternion class to help with our key frame rotation interpolation.
//
//
//
// Ben Humphrey (DigiBen)
// Game Programmer
// DigiBen@GameTutorials.com
// Co-Web Host of www.GameTutorials.com
//
// The Quake3 .Md3 file format is owned by ID Software.  This tutorial is being used 
// as a teaching tool to help understand model loading and animation.  This should
// not be sold or used under any way for commercial use with out written consent
// from ID Software.
//
// Quake, Quake2 and Quake3 are trademarks of ID Software.
// Lara Croft is a trademark of Eidos and should not be used for any commercial gain.
// All trademarks used are properties of their respective owners. 
//
//