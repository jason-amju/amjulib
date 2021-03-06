#include <AmjuFirst.h>
#include <AmjuGL.h>
#include <Game.h>
#include <ConfigFile.h>
#include <GuiTextEdit.h>
#include <UrlUtils.h>
#include <GuiButton.h>
#include <GuiFileListBox.h>
#include <GuiFileDialog.h>
#include "GSObjMesh.h"
#include "GSEdit.h"
#include "SaveConfig.h"
#include "UploadNewContent.h"
#include "GSWaitForUpload.h"
#include "ReqSetObjectFilenames.h"
#include <AmjuFinal.h>

namespace Amju
{
static const char* MESH_PATH_KEY = "last_mesh_path";

static void OnOKButton(GuiElement*)
{
  TheGSObjMesh::Instance()->OnOKButton();
}

/*
static void OnDoubleClick(const std::string& filename)
{
  TheGSObjMesh::Instance()->SetFile(filename);
  TheGSObjMesh::Instance()->OnOKButton();
}

static void OnSingleClick(const std::string& filename)
{
  TheGSObjMesh::Instance()->SetFile(filename);
}
*/

static void OnCancelButton(GuiElement*)
{
  TheGame::Instance()->SetCurrentState(TheGSEdit::Instance());
}

static void OnFinishedUpload()
{
  TheGSObjMesh::Instance()->OnFinishedUpload();
}


GSObjMesh::GSObjMesh()
{
  m_objId = 0;
  m_uploadedFiles = 0;
  m_totalFiles = 0;
  // TODO MODE
}

void GSObjMesh::SetId(int iid)
{
  m_objId = iid;
  std::string id = ToString(m_objId); 
  m_assetFilename = "assets_obj_" + id + ".txt";
  m_dataFilename = "data_obj_" + id + ".txt";
  m_assetFileContents = 
    "// Asset file for obj " + id + ", ver\r\n1\r\n";
  m_dataFileContents = 
    "// Data file for obj " + id + ", ver\r\n1\r\n";
}

void GSObjMesh::SetError(const std::string& str)
{
  if (!m_gui)
  {
std::cout << "ERROR: Attempt to set GSObjMesh error message, but is not active. Race condition!?!\n";
std::cout << "The msg was:" << str << "\n";
    return;
  }

  GuiText* text = dynamic_cast<GuiText*>(m_gui->GetElementByName("error"));
  Assert(text);
  text->SetText(str);
}

void GSObjMesh::OnFinishedUpload()
{
  // Make sure the GUI is loaded ?? Race condition ??
  TheGame::Instance()->SetCurrentState(this);

  // Now send DB query to update row in Object table - we must set asset file and data file.
  // Can't SetError here, we are not active yet!!

std::cout << "Finished uploading, creating new object on server...\n";

  SendReqSetObjectFilenames(m_objId, m_assetFilename, m_dataFilename);

//  // Send req to make new game object.
//  std::string id = ToString(m_objId); 
//  std::string url = TheVe1ReqManager::Instance()->MakeUrl(CREATE_OBJECT);
//  url += "&obj_id=" + id + "&asset_file=" + m_assetFilename + "&data_file=" + m_dataFilename;
//  url = ToUrlFormat(url);
//  TheVe1ReqManager::Instance()->AddReq(new CreateNewObjectReq(url), m_totalFiles);
}

// Naw, lots of objects could use the same mesh.
//std::string GSObjMesh::MakeDir() const
//{
//  return "Obj_" + m_objId;
//}

void GSObjMesh::SetFile(const std::string& pathAndFile)
{
  GuiText* text = dynamic_cast<GuiText*>(m_gui->GetElementByName("fd-path-text"));
  Assert(text);
  text->SetText(pathAndFile);
}

void GSObjMesh::OnOKButton()
{
  // Get path + file name
  GuiText* text = dynamic_cast<GuiText*>(m_gui->GetElementByName("fd-path-text"));
  Assert(text);
  std::string pathFile = text->GetText();

  // Remember this path
  static GameConfigFile* config = TheGameConfigFile::Instance();
  config->Set(MESH_PATH_KEY, pathFile);
  SaveConfig();

  if (CheckCanLoadObj(pathFile))
  {
    SetError("Loaded obj file ok!");    

    std::string id = ToString(m_objId);

    bool ok = UploadObjFile(
      pathFile,
      "", // Server dir - check empty works ok
      m_assetFilename,
      m_assetFileContents, 
      m_dataFilename,
      m_dataFileContents +  "obj mesh\r\n" + StripPath(pathFile) + "\r\n",
      &m_totalFiles);

    if (ok)
    {
      // Hide file dialog
      GuiElement* dlg = (m_gui->GetElementByName("my-file-dialog"));
      Assert(dlg);
      dlg->SetVisible(false);

      static GSWaitForUpload* wfu = TheGSWaitForUpload::Instance();
      wfu->SetOnFinishedFunc(Amju::OnFinishedUpload);
      wfu->SetTotalFiles(m_totalFiles);
      TheGame::Instance()->SetCurrentState(wfu);
    }
    else
    {
      SetError("Uploading FAILED!");
    } 
  }
  else
  {
    SetError("Failed to load obj file. Is filename correct ?");    
  }
}

void GSObjMesh::Update()
{
  GSGui::Update();

}

void GSObjMesh::Draw()
{
  GSGui::Draw();

}

void GSObjMesh::Draw2d()
{
  GSGui::Draw2d();
}

void GSObjMesh::OnActive()
{
  GSGui::OnActive();

std::cout << "Testing new file browser GUI...\n";

  m_gui = LoadGui("gui-choose-obj-file.txt");
  Assert(m_gui);

  GuiButton* ok = (GuiButton*)GetElementByName(m_gui, "ok-button");
  Assert(ok);
  ok->SetCommand(Amju::OnOKButton);
  ok->SetHasFocus(true);

  GuiButton* cancel = (GuiButton*)GetElementByName(m_gui, "cancel-button");
  Assert(cancel);
  cancel->SetCommand(Amju::OnCancelButton);
  cancel->SetIsCancelButton(true);

  // Text edit control needs focus to accept kb input ??
  //GuiTextEdit* edit = (GuiTextEdit*)GetElementByName(m_gui, "fd-path-text");
  //Assert(edit);
  //edit->SetHasFocus(true);
  static GameConfigFile* config = TheGameConfigFile::Instance();
  std::string pathFile = "/";
  if (config->Exists(MESH_PATH_KEY))
  {
    pathFile = config->GetValue(MESH_PATH_KEY);
  } 

  GuiFileDialog* dlg = (GuiFileDialog*)(m_gui->GetElementByName("my-file-dialog"));
  Assert(dlg);
  dlg->SetPathAndFile(pathFile);

  //edit->SetText(pathFile);
  //GuiFileListBox* flb = (GuiFileListBox*)GetElementByName(m_gui, "my-file-browser");
  //flb->SetDoubleClickCallback(Amju::OnDoubleClick);
  //flb->SetSingleClickCallback(Amju::OnSingleClick);
  //flb->SetDir(GetFilePath(pathFile));

}

} // namespace
