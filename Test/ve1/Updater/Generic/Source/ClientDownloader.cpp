#include "ClientDownloader.h"
#include "Updater.h"
#include <iostream>

namespace Amju
{
ClientDownloader::ClientDownloader(Updater* u, const std::string& filename, const std::string& url) : 
  DownloadReq(filename, url, HttpClient::GET, "client download"), m_updater(u)
{ 
}

void ClientDownloader::OnDownloaded()
{
std::cout << "Download succeeded!\n";

  m_updater->OnDownloadSuccess();
}

void ClientDownloader::OnDownloadFailed()
{
std::cout << "Download failed!\n";

  m_updater->Unwait();
}

}

