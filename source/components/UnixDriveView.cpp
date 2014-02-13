//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include <Common.h>

#include "UnixDriveView.h"

#ifndef DESIGN_ONLY
#include <Terminal.h>
#include <RemoteFiles.h>
#include <VCLCommon.h>
#endif

#pragma package(smart_init)
//---------------------------------------------------------------------------
static inline void ValidCtrCheck(TUnixDriveView *)
{
  new TUnixDriveView(NULL);
}
//---------------------------------------------------------------------------
namespace Unixdriveview
{
  void __fastcall PACKAGE Register()
  {
    TComponentClass classes[1] = {__classid(TUnixDriveView)};
    RegisterComponents(L"Scp", classes, 0);
  }
}
//---------------------------------------------------------------------------
__fastcall TUnixDriveView::TUnixDriveView(TComponent * Owner) :
  TCustomUnixDriveView(Owner)
{
}
//---------------------------------------------------------------------------
struct TNodeData
{
  TRemoteFileList * FileList;
  TRemoteFile * File;
  UnicodeString Directory;
};
//---------------------------------------------------------------------------
__fastcall TCustomUnixDriveView::TCustomUnixDriveView(TComponent* Owner) :
  TCustomDriveView(Owner)
{
  FTerminal = NULL;
  FRootName = Customunixdirview_SUnixDefaultRootName;
  FIgnoreChange = false;
  FPrevSelected = NULL;
  DDAllowMove = false;
  FShowInaccesibleDirectories = true;
  FDummyDragFile = NULL;
  FPendingDelete = new TList();
}
//---------------------------------------------------------------------------
__fastcall TCustomUnixDriveView::~TCustomUnixDriveView()
{
  Terminal = NULL;
  if (FDummyDragFile != NULL)
  {
    #ifndef DESIGN_ONLY
    SAFE_DESTROY(FDummyDragFile);
    #endif
  }
  SAFE_DESTROY(FPendingDelete);
}
//---------------------------------------------------------------------------
void __fastcall TCustomUnixDriveView::CreateWnd()
{
  TCustomDriveView::CreateWnd();

  FDragDropFilesEx->TargetEffects = TDropEffectSet() << deCopy << deMove;
}
//---------------------------------------------------------------------------
void __fastcall TCustomUnixDriveView::SetTerminal(TTerminal * value)
{
  if (FTerminal != value)
  {
    FTerminal = value;
    Items->Clear();
    #ifndef DESIGN_ONLY
    if (FTerminal != NULL)
    {
      TStrings * FixedPaths = FTerminal->FixedPaths;
      if (FixedPaths != NULL)
      {
        for (int i = 0; i < FixedPaths->Count; i++)
        {
          LoadPath(FixedPaths->Strings[i]);
        }
      }
    }
    #endif
  }
}
//---------------------------------------------------------------------------
void __fastcall TCustomUnixDriveView::SetDirView(TUnixDirView * Value)
{
  if (FDirView != Value)
  {
    if (FDirView != NULL)
    {
      FDirView->DriveView = NULL;
    }

    FDirView = Value;

    if (FDirView != NULL)
    {
      FDirView->DriveView = this;
    }
  }
}
//---------------------------------------------------------------------------
TCustomDirView * __fastcall TCustomUnixDriveView::GetCustomDirView()
{
  return FDirView;
}
//---------------------------------------------------------------------------
void __fastcall TCustomUnixDriveView::SetCustomDirView(TCustomDirView * Value)
{
  SetDirView(dynamic_cast<TUnixDirView *>(Value));
}
//---------------------------------------------------------------------------
bool __fastcall TCustomUnixDriveView::IsRootNameStored()
{
  return (FRootName != Customunixdirview_SUnixDefaultRootName);
}
//---------------------------------------------------------------------------
bool __fastcall TCustomUnixDriveView::NodeIsHidden(const TTreeNode * Node)
{
  #ifndef DESIGN_ONLY
  TNodeData * Data = NodeData(Node);
  TRemoteFile * File = Data->File;
  return
    ((File != NULL) && File->IsHidden) ||
    ((File == NULL) && IsUnixHiddenFile(UnixExtractFileName(Data->Directory)));
  #else
  return false;
  #endif
}
//---------------------------------------------------------------------------
bool __fastcall TCustomUnixDriveView::NodeTryDelete(TTreeNode * Node, bool RememberIfFails)
{
  bool Result =
    (Selected == NULL) ||
    ((Selected != Node) && !Selected->HasAsParent(Node));
  if (Result)
  {
    Node->Delete();
  }
  else
  {
    if (RememberIfFails)
    {
      int I = FPendingDelete->IndexOf(Node);
      if (I < 0)
      {
        FPendingDelete->Add(Node);
      }
    }
  }
  return Result;
}
//---------------------------------------------------------------------------
void __fastcall TCustomUnixDriveView::UpdatePath(TTreeNode * Node, bool Force,
  bool CanLoad)
{
  #ifndef DESIGN_ONLY
  TNodeData * Data = NodeData(Node);
  UnicodeString Path = Data->Directory;

  if (FTerminal->DirectoryFileList(Path, Data->FileList, CanLoad) ||
      ((Data->FileList != NULL) && Force))
  {
    TStringList * ChildrenDirs = new TStringList();
    try
    {
      ChildrenDirs->Duplicates = dupAccept;
      ChildrenDirs->CaseSensitive = true;

      TTreeNode * ChildNode = Node->getFirstChild();
      while (ChildNode != NULL)
      {
        TNodeData * ChildData = NodeData(ChildNode);
        ChildData->File = NULL;
        ChildrenDirs->AddObject(UnixExtractFileName(ChildData->Directory),
          ChildNode);
        ChildNode = Node->GetNextChild(ChildNode);
      }

      // sort only after adding all items.
      // should be faster then keeping the list sorted since beginning
      ChildrenDirs->Sorted = true;

      for (int i = 0; i < Data->FileList->Count; i++)
      {
        TRemoteFile * File = Data->FileList->Files[i];
        if (File->IsDirectory && !File->IsParentDirectory && !File->IsThisDirectory &&
            (ShowHiddenDirs || !File->IsHidden) &&
            (ShowInaccesibleDirectories || !File->IsInaccesibleDirectory))
        {
          int ChildIndex = ChildrenDirs->IndexOf(File->FileName);
          if (ChildIndex >= 0)
          {
            TTreeNode * ChildNode =
              dynamic_cast<TTreeNode *>(ChildrenDirs->Objects[ChildIndex]);
            TNodeData * ChildData = NodeData(ChildNode);
            ChildData->File = File;
            UpdatePath(ChildNode, Force);
          }
          else
          {
            UnicodeString ChildPath = UnixIncludeTrailingBackslash(Path) + File->FileName;
            assert(!IsUnixRootPath(ChildPath));

            LoadPathEasy(Node, ChildPath, File);
          }
        }
      }

      for (int i = 0; i < ChildrenDirs->Count; i++)
      {
        TTreeNode * ChildNode = dynamic_cast<TTreeNode *>(ChildrenDirs->Objects[i]);
        TNodeData * ChildData = NodeData(ChildNode);
        if (ChildData->File == NULL)
        {
          NodeTryDelete(ChildNode, true);
        }
      }

      Node->AlphaSort(false);
    }
    __finally
    {
      delete ChildrenDirs;
    }
  }
  else if (Force)
  {
    TTreeNode * ChildNode = Node->GetLastChild();
    while (ChildNode != NULL)
    {
      TTreeNode * PrevChildNode = Node->GetPrevChild(ChildNode);
      TRemoteFile * File = NodeFile(ChildNode);
      if (((ShowHiddenDirs || !NodeIsHidden(ChildNode)) &&
            (ShowInaccesibleDirectories || (File == NULL) || !File->IsInaccesibleDirectory)) ||
          !NodeTryDelete(ChildNode, true))
      {
        UpdatePath(ChildNode, true);
      }
      ChildNode = PrevChildNode;
    }
  }
  #else
  USEDPARAM(Node);
  USEDPARAM(Force);
  USEDPARAM(CanLoad);
  #endif
}
//---------------------------------------------------------------------------
TTreeNode * __fastcall TCustomUnixDriveView::LoadPathEasy(TTreeNode * Parent,
  UnicodeString Path, TRemoteFile * File)
{
  #ifndef DESIGN_ONLY
  assert(Path == UnixExcludeTrailingBackslash(Path));

  UnicodeString DirName;
  if (IsUnixRootPath(Path))
  {
    DirName = RootName;
  }
  else
  {
    DirName = UnixExtractFileName(Path);
  }
  TTreeNode * Node = Items->AddChild(Parent, DirName);

  TNodeData * Data = new TNodeData();
  Node->Data = Data;
  Data->Directory = Path;
  Data->File = File;
  Data->FileList = NULL;

  UpdatePath(Node, true);

  return Node;
  #else
  USEDPARAM(Parent);
  USEDPARAM(File);
  return NULL;
  #endif
}
//---------------------------------------------------------------------------
TTreeNode * __fastcall TCustomUnixDriveView::LoadPath(UnicodeString Path)
{
  #ifndef DESIGN_ONLY

  if (Path.IsEmpty())
  {
    Path = ROOTDIRECTORY;
  }

  TTreeNode * Node = FindNodeToPath(Path);
  if (Node == NULL)
  {
    Path = UnixExcludeTrailingBackslash(Path);
    TTreeNode * Parent = NULL;
    TRemoteFile * File = NULL;

    if (!IsUnixRootPath(Path))
    {
      Parent = LoadPath(UnixExtractFileDir(Path));
    }

    Node = FindNodeToPath(Path);
    if (Node == NULL)
    {
      // node still does not exist, this should happen only when
      // if the parent directory is not in cache
      if (Parent != NULL)
      {
        TRemoteFileList * ParentFileList = NodeFileList(Parent);
        if (ParentFileList != NULL)
        {
          File = ParentFileList->FindFile(UnixExtractFileName(Path));
        }
      }

      Node = LoadPathEasy(Parent, Path, File);
      if (Parent != NULL)
      {
        Parent->AlphaSort(false);
      }
    }
    else
    {
      UpdatePath(Node, false);
    }
  }
  else
  {
    UpdatePath(Node, false);
  }

  return Node;
  #else
  return NULL;
  #endif
}
//---------------------------------------------------------------------------
void __fastcall TCustomUnixDriveView::LoadDirectory()
{
  #ifndef DESIGN_ONLY
  assert(!FIgnoreChange);
  FIgnoreChange = true;
  try
  {
    Selected = LoadPath(FTerminal->Files->Directory);
    assert(Selected != NULL);
    FPrevSelected = Selected;
  }
  __finally
  {
    FIgnoreChange = false;
    FDirectoryLoaded = true;
  }
  #endif
}
//---------------------------------------------------------------------------
TNodeData * __fastcall TCustomUnixDriveView::NodeData(const TTreeNode * Node)
{
  assert(Node->Data != NULL);

  return static_cast<TNodeData*>(Node->Data);
}
//---------------------------------------------------------------------------
TRemoteFileList * __fastcall TCustomUnixDriveView::NodeFileList(const TTreeNode * Node)
{
  assert(Node->Data != NULL);

  return static_cast<TNodeData*>(Node->Data)->FileList;
}
//---------------------------------------------------------------------------
TRemoteFile * __fastcall TCustomUnixDriveView::NodeFile(const TTreeNode * Node)
{
  assert(Node->Data != NULL);

  return static_cast<TNodeData*>(Node->Data)->File;
}
//---------------------------------------------------------------------------
TRemoteFile * __fastcall TCustomUnixDriveView::NodeFileForce(TTreeNode * Node)
{
  TRemoteFile * File = NodeFile(Node);

  if (File == NULL)
  {
    #ifndef DESIGN_ONLY
    SAFE_DESTROY(FDummyDragFile);
    FDummyDragFile = new TRemoteDirectoryFile();
    FDummyDragFile->FileName = Node->Text;
    FDummyDragFile->FullFileName = NodePathName(Node);
    File = FDummyDragFile;
    #endif
  }

  return File;
}
//---------------------------------------------------------------------------
void __fastcall TCustomUnixDriveView::Delete(TTreeNode * Node)
{
  if (Node == FPrevSelected)
  {
    FPrevSelected = NULL;
  }
  // optimization check
  if (FPendingDelete->Count > 0)
  {
    int I = FPendingDelete->IndexOf(Node);
    if (I >= 0)
    {
      FPendingDelete->Delete(I);
    }
  }
  TNodeData * Data = NULL;
  if (Node != NULL)
  {
    Data = NodeData(Node);
  }
  TCustomDriveView::Delete(Node);
  // delete file list at last, when we are sure that no child nodes exist
  if (Data != NULL)
  {
    #ifndef DESIGN_ONLY
    delete Data->FileList;
    #endif
    delete Data;
  }
}
//---------------------------------------------------------------------------
void __fastcall TCustomUnixDriveView::Change(TTreeNode * Node)
{
  #ifndef DESIGN_ONLY
  bool Expand = false;
  try
  {
    // During D&D Selected is set to NULL and then back to previous selection,
    // prevent actually changing directory in such case
    if (Reading || ControlState.Contains(csRecreating) ||
        (Node == NULL) || (Node == FPrevSelected))
    {
      TCustomDriveView::Change(Node);
    }
    else
    {
      // if previous node is child to newly selected one, do not expand it.
      // it is either already expanded and it is even being collapsed.
      Expand = (FPrevSelected == NULL) || !FPrevSelected->HasAsParent(Node);
      if (FIgnoreChange)
      {
        TCustomDriveView::Change(Node);
      }
      else
      {
        if (FDirView != NULL)
        {
          // remember current directory, so it gets selected if we move to parent
          // directory
          FDirView->ContinueSession(true);
        }

        FDirectoryLoaded = false;
        try
        {
          APPLICATION_EXCEPTION_HACK_BEGIN
          {
            Terminal->ChangeDirectory(NodePathName(Node));
          }
          APPLICATION_EXCEPTION_HACK_END;
          TCustomDriveView::Change(Node);
        }
        __finally
        {
          if (!FDirectoryLoaded)
          {
            assert(!FIgnoreChange);
            Expand = false;
            FIgnoreChange = true;
            try
            {
              Selected = FPrevSelected;
            }
            __finally
            {
              FIgnoreChange = false;
            }
          }
        }
      }
    }
  }
  __finally
  {
    FPrevSelected = Selected;
    if (Expand)
    {
      Selected->Expand(false);
    }
    CheckPendingDeletes();
  }
  #else
  TCustomDriveView::Change(Node);
  #endif
}
//---------------------------------------------------------------------------
void __fastcall TCustomUnixDriveView::CheckPendingDeletes()
{
  int Index = 0;
  while (Index < FPendingDelete->Count)
  {
    TTreeNode * Node = static_cast<TTreeNode *>(FPendingDelete->Items[Index]);
    // this as well deletes node from FPendingDelete
    if (!NodeTryDelete(Node, false))
    {
      Index++;
    }
  }
}
//---------------------------------------------------------------------------
void __fastcall TCustomUnixDriveView::PerformDragDropFileOperation(
  TTreeNode * Node, int Effect)
{
  if (OnDDFileOperation)
  {
    assert(DragDropFilesEx->FileList->Count > 0);
    assert(Node != NULL);

    UnicodeString SourceDirectory;
    UnicodeString TargetDirectory;

    SourceDirectory = ExtractFilePath(DragDropFilesEx->FileList->Items[0]->Name);
    TargetDirectory = NodeData(Node)->Directory;

    bool DoFileOperation = true;
    OnDDFileOperation(this, Effect, SourceDirectory, TargetDirectory,
      DoFileOperation);
  }
}
//---------------------------------------------------------------------------
void __fastcall TCustomUnixDriveView::DDChooseEffect(int KeyState, int & Effect)
{
  // if any drop effect is allowed at all (e.g. no drop to self and drop to parent)
  if (Effect != DROPEFFECT_None)
  {
    if (DropTarget != NULL)
    {
      if ((KeyState & (MK_CONTROL | MK_SHIFT)) == 0)
      {
        Effect = DROPEFFECT_Copy;
      }
    }
    else
    {
      Effect = DROPEFFECT_None;
    }
  }

  TCustomDriveView::DDChooseEffect(KeyState, Effect);
}
//---------------------------------------------------------------------------
void __fastcall TCustomUnixDriveView::UpdateDropTarget()
{
  // should never be NULL
  if (DropTarget != NULL)
  {
    UpdatePath(DropTarget, false, true);
  }
}
//---------------------------------------------------------------------------
void __fastcall TCustomUnixDriveView::UpdateDropSource()
{
  // DragNode may be NULL if its parent directory was reloaded as result
  // of D&D operation and thus all child nodes are recreated
  if ((DragNode != NULL) && (DragNode->Parent != NULL))
  {
    UpdatePath(DragNode->Parent, false, true);
  }
}
//---------------------------------------------------------------------------
TStrings * __fastcall TCustomUnixDriveView::DragFileList()
{
  assert(DragNode != NULL);
  TStrings * FileList = new TStringList();
  try
  {
    #ifndef DESIGN_ONLY
    FileList->AddObject(ExcludeTrailingBackslash(NodePathName(DragNode)),
      NodeFileForce(DragNode));
    #endif
  }
  catch(...)
  {
    delete FileList;
    throw;
  }
  return FileList;
}
//---------------------------------------------------------------------------
bool __fastcall TCustomUnixDriveView::DragCompleteFileList()
{
  return true;
}
//---------------------------------------------------------------------------
TDropEffectSet __fastcall TCustomUnixDriveView::DDSourceEffects()
{
  TDropEffectSet Result;
  Result << deCopy;
  if (DDAllowMove)
  {
    Result << deMove;
  }
  return Result;
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TCustomUnixDriveView::NodePathName(TTreeNode * Node)
{
  // same as NodePath
  return NodeData(Node)->Directory;
}
//---------------------------------------------------------------------------
void __fastcall TCustomUnixDriveView::ClearDragFileList(TFileList * FileList)
{
  #ifndef DESIGN_ONLY
  if (FDummyDragFile != NULL)
  {
    SAFE_DESTROY(FDummyDragFile);
  }
  #endif
  TCustomDriveView::ClearDragFileList(FileList);
}
//---------------------------------------------------------------------------
void __fastcall TCustomUnixDriveView::AddToDragFileList(TFileList * FileList,
  TTreeNode * Node)
{
  UnicodeString FileName = NodePathName(Node);
  TRemoteFile * File = NodeFileForce(Node);

  if (OnDDDragFileName != NULL)
  {
    OnDDDragFileName(this, File, FileName);
  }
  FileList->AddItem(NULL, FileName);
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TCustomUnixDriveView::NodePath(TTreeNode * Node)
{
  // same as NodePathName
  return NodeData(Node)->Directory;
}
//---------------------------------------------------------------------------
bool __fastcall TCustomUnixDriveView::NodeIsRecycleBin(TTreeNode * /*Node*/)
{
  return false;
}
//---------------------------------------------------------------------------
bool __fastcall TCustomUnixDriveView::NodePathExists(TTreeNode * /*Node*/)
{
  return true;
}
//---------------------------------------------------------------------------
TColor __fastcall TCustomUnixDriveView::NodeColor(TTreeNode * Node)
{
  assert(Node != NULL);
  TColor Result = static_cast<TColor>(clDefaultItemColor);
  #ifndef DESIGN_ONLY
  if (FDimmHiddenDirs && !Node->Selected)
  {
    if (NodeIsHidden(Node))
    {
      Result = clGrayText;
    }
  }
  #else
  USEDPARAM(Node);
  #endif
  return Result;
}
//---------------------------------------------------------------------------
Word __fastcall TCustomUnixDriveView::NodeOverlayIndexes(TTreeNode * Node)
{
#ifndef DESIGN_ONLY
  Word Result = oiNoOverlay;
  // Cannot query root node for file
  if (Node->Parent != NULL)
  {
    TRemoteFile * File = NodeFile(Node);
    if ((File != NULL) && (File->IsSymLink))
    {
      // broken link cannot probably happen anyway
      // as broken links are treated as files
      Result |= File->BrokenLink ? oiBrokenLink : oiLink;
    }
  }
  return Result;
#else
  USEDPARAM(Node);
  return 0;
#endif
}
//---------------------------------------------------------------------------
void __fastcall TCustomUnixDriveView::GetImageIndex(TTreeNode * Node)
{
  TCustomDriveView::GetImageIndex(Node);
  Node->ImageIndex = StdDirIcon;
  Node->SelectedIndex = StdDirSelIcon;
}
//---------------------------------------------------------------------------
TTreeNode * __fastcall TCustomUnixDriveView::FindNodeToPath(UnicodeString Path)
{
  TTreeNode * Result;
  #ifndef DESIGN_ONLY
  if (IsUnixRootPath(Path))
  {
    Result = Items->GetFirstNode();
  }
  else
  {
    Result = NULL;
    Path = UnixExcludeTrailingBackslash(Path);
    TTreeNode * Parent = NULL;
    if (!IsUnixRootPath(Path))
    {
      Parent = FindNodeToPath(UnixExtractFileDir(Path));
    }

    if ((Parent != NULL) && (Parent->getFirstChild() != NULL))
    {
      UnicodeString DirName = UnixExtractFileName(Path);
      int StartIndex = 0;
      int EndIndex = Parent->Count - 1;

      while (true)
      {
        int Index = (StartIndex + EndIndex) / 2;
        UnicodeString NodeDir = Parent->Item[Index]->Text;
        // lstrcmp is used by AlphaSort()
        int C = lstrcmp(DirName.c_str(), NodeDir.c_str());
        if (C == 0)
        {
          Result = Parent->Item[Index];
          break;
        }
        else if (C < 0)
        {
          if (Index == StartIndex)
          {
            break;
          }
          EndIndex = Index - 1;
        }
        else
        {
          if (Index == EndIndex)
          {
            break;
          }
          StartIndex = Index + 1;
        }
      }
    }
  }
  #else
  Result = NULL;
  #endif
  return Result;
}
//---------------------------------------------------------------------------
TTreeNode * __fastcall TCustomUnixDriveView::FindPathNode(UnicodeString Path)
{
  TTreeNode * Result = NULL;

  #ifndef DESIGN_ONLY
  if (Items->GetFirstNode() != NULL)
  {
    do
    {
      Result = FindNodeToPath(Path);
      if (Result == NULL)
      {
        assert(!IsUnixRootPath(Path));
        Path = UnixExtractFileDir(UnixExcludeTrailingBackslash(Path));
      }
    }
    while (Result == NULL);
  }
  #endif

  return Result;
}
//---------------------------------------------------------------------------
void __fastcall TCustomUnixDriveView::ValidateDirectoryEx(TTreeNode * /*Node*/,
  TRecursiveScan /*Recurse*/, bool /*NewDirs*/)
{
  // nothing
}
//---------------------------------------------------------------------------
void __fastcall TCustomUnixDriveView::RebuildTree()
{
  TTreeNode * First = Items->GetFirstNode();
  if (First != NULL)
  {
    UpdatePath(First, true);
  }
}
//---------------------------------------------------------------------------
void __fastcall TCustomUnixDriveView::SetShowInaccesibleDirectories(bool value)
{
  if (FShowInaccesibleDirectories != value)
  {
    FShowInaccesibleDirectories = value;
    RebuildTree();
  }
}
//---------------------------------------------------------------------------
void __fastcall TCustomUnixDriveView::CMShowingChanged(TMessage & Message)
{
  TCustomDriveView::Dispatch(&Message);
  if (Showing && (Terminal != NULL))
  {
    LoadDirectory();
  }
}
//---------------------------------------------------------------------------
void __fastcall TCustomUnixDriveView::DisplayContextMenu(TTreeNode * /*Node*/,
  const TPoint & /*ScreenPos*/)
{
  // TODO
}
//---------------------------------------------------------------------------
void __fastcall TCustomUnixDriveView::DisplayPropertiesMenu(TTreeNode * /*Node*/)
{
  // TODO
}
