//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "FileSystems.h"
#include "RemoteFiles.h"
#include "Common.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
__fastcall TCustomFileSystem::TCustomFileSystem(TTerminal * ATerminal):
  FTerminal(ATerminal)
{
  assert(FTerminal);
}
//---------------------------------------------------------------------------
__fastcall TCustomFileSystem::~TCustomFileSystem()
{
}
