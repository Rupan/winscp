//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include <Common.h>
#include <CoreMain.h>
#include <TextsWin.h>
#include <SessionData.h>
#include "WinConfiguration.h"
#include "EditorManager.h"
#include <algorithm>
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
TEditedFileData::TEditedFileData()
{
  ForceText = false;
  Terminal = NULL;
  SessionData = NULL;
  Queue = NULL;
}
//---------------------------------------------------------------------------
TEditedFileData::~TEditedFileData()
{
  delete SessionData;
}
//---------------------------------------------------------------------------
__fastcall TEditorManager::TEditorManager()
{
  FOnFileChange = NULL;
  FOnFileReload = NULL;
  FOnFileEarlyClosed = NULL;
}
//---------------------------------------------------------------------------
__fastcall TEditorManager::~TEditorManager()
{
  for (unsigned int i = FFiles.size(); i > 0; i--)
  {
    int Index = i - 1;
    TFileData * FileData = &FFiles[Index];

    // pending should be only external editors and files being uploaded
    assert(FileData->Closed || FileData->External);

    if (!FileData->Closed)
    {
      if (!CloseFile(Index, true, true))
      {
        ReleaseFile(Index);
      }
    }
  }
}
//---------------------------------------------------------------------------
bool __fastcall TEditorManager::Empty(bool IgnoreClosed)
{
  bool Result;

  if (!IgnoreClosed)
  {
    Result = (FFiles.size() == 0);
  }
  else
  {
    Result = true;
    for (unsigned int i = 0; i < FFiles.size(); i++)
    {
      if (!FFiles[i].Closed)
      {
        Result = false;
        break;
      }
    }
  }

  return Result;
}
//---------------------------------------------------------------------------
bool __fastcall TEditorManager::CanAddFile(const UnicodeString RemoteDirectory,
  const UnicodeString OriginalFileName, const UnicodeString SessionName,
  TObject *& Token, UnicodeString & ExistingLocalRootDirectory,
  UnicodeString & ExistingLocalDirectory)
{
  bool Result = true;

  Token = NULL;
  for (unsigned int i = 0; i < FFiles.size(); i++)
  {
    TFileData * FileData = &FFiles[i];

    // include even "closed" (=being uploaded) files as it is nonsense
    // to download file being uploaded
    if ((FileData->Data->RemoteDirectory == RemoteDirectory) &&
        (FileData->Data->OriginalFileName == OriginalFileName) &&
        (FileData->Data->SessionName == SessionName))
    {
      if (!FileData->External)
      {
        Result = false;
        if (!FileData->Closed && (FileData->Token != NULL))
        {
          Token = FileData->Token;
        }
      }
      else
      {
        // MDI editor?
        if (FileData->Process == INVALID_HANDLE_VALUE)
        {
          // file is just being uploaded, do not allow new editor instance
          if (FileData->Closed)
          {
            Result = false;
          }
          else
          {
            // get directory where the file already is so we download it there again
            ExistingLocalRootDirectory = FileData->Data->LocalRootDirectory;
            ExistingLocalDirectory = ExtractFilePath(FileData->FileName);
            CloseFile(i, false, false); // do not delete file
            Result = true;
          }
        }
        else
        {
          Result = false;
        }
      }
      break;
    }
  }

  if (Result)
  {
    if (FFiles.size() >= WinConfiguration->Editor.MaxEditors)
    {
      throw Exception(LoadStr(TOO_MANY_EDITORS));
    }
  }

  return Result;
}
//---------------------------------------------------------------------------
void __fastcall TEditorManager::ProcessFiles(TEditedFileProcessEvent Callback, void * Arg)
{
  for (unsigned int i = 0; i < FFiles.size(); i++)
  {
    TFileData * FileData = &FFiles[i];
    Callback(FileData->FileName, FileData->Data,
      (FileData->Closed ? NULL : FileData->Token), Arg);
  }
}
//---------------------------------------------------------------------------
bool __fastcall TEditorManager::CloseInternalEditors(TNotifyEvent CloseCallback)
{
  // Traverse from end, as closing internal editor causes deletion of
  // respective file vector element.
  TObject * PrevToken = NULL;
  for (unsigned int i = FFiles.size(); i > 0; i--)
  {
    // Note that element may be deleted by external cause (like if external editor
    // is closed while "save confirmation" message is displayed).
    if (i <= FFiles.size())
    {
      TFileData * FileData = &FFiles[i - 1];
      // PrevToken is simple check not to ask same editor twice, however
      // it does not solve all posibilities
      if (!FileData->Closed && (FileData->Token != NULL) &&
          (FileData->Token != PrevToken))
      {
        CloseCallback(FileData->Token);
      }
    }
  }

  bool Result = true;
  for (unsigned int i = 0; i < FFiles.size(); i++)
  {
    TFileData * FileData = &FFiles[i];

    if (!FileData->Closed && (FileData->Token != NULL))
    {
      Result = false;
      break;
    }
  }

  return Result;
}
//---------------------------------------------------------------------------
bool __fastcall TEditorManager::CloseExternalFilesWithoutProcess()
{
  for (unsigned int i = FFiles.size(); i > 0; i--)
  {
    TFileData * FileData = &FFiles[i - 1];

    if (!FileData->Closed && FileData->External &&
        (FileData->Process == INVALID_HANDLE_VALUE))
    {
      CloseFile(i - 1, true, true);
    }
  }
  return true;
}
//---------------------------------------------------------------------------
void __fastcall TEditorManager::AddFileInternal(const UnicodeString FileName,
  TEditedFileData * AData, TObject * Token)
{
  std::unique_ptr<TEditedFileData> Data(AData);
  TFileData FileData;
  FileData.FileName = FileName;
  FileData.External = false;
  FileData.Process = INVALID_HANDLE_VALUE;
  FileData.Token = Token;
  FileData.Monitor = INVALID_HANDLE_VALUE;

  AddFile(FileData, Data.release());
}
//---------------------------------------------------------------------------
void __fastcall TEditorManager::AddFileExternal(const UnicodeString FileName,
  TEditedFileData * AData, HANDLE Process)
{
  std::unique_ptr<TEditedFileData> Data(AData);
  TFileData FileData;
  FileData.FileName = FileName;
  FileData.External = true;
  FileData.Process = Process;
  FileData.Token = NULL;
  UnicodeString FilePath = ExtractFilePath(FileData.FileName);
  FileData.Monitor =
    FindFirstChangeNotification(
      FilePath.c_str(), false, FILE_NOTIFY_CHANGE_LAST_WRITE);
  if (FileData.Monitor == INVALID_HANDLE_VALUE)
  {
    throw Exception(FMTLOAD(FILE_WATCH_ERROR, (FileData.FileName)));
  }

  FMonitors.push_back(FileData.Monitor);
  if (Process != INVALID_HANDLE_VALUE)
  {
    FProcesses.push_back(Process);
  }
  AddFile(FileData, Data.release());
}
//---------------------------------------------------------------------------
void __fastcall TEditorManager::Check()
{
  int Index;

  if (FMonitors.size() > 0)
  {
    do
    {
      Index = WaitFor(FMonitors.size(), &(FMonitors[0]), MONITOR);

      if (Index >= 0)
      {
        // let the editor finish writting to the file
        // (first to avoid uploading partially saved file, second
        // because the timestamp may change more than once during saving)
        Sleep(GUIConfiguration->KeepUpToDateChangeDelay);

        TFileData * FileData = &FFiles[Index];
        FindNextChangeNotification(FileData->Monitor);

        CheckFileChange(Index, false);
      }
    }
    while (Index >= 0);
  }

  if (FProcesses.size() > 0)
  {
    do
    {
      Index = WaitFor(FProcesses.size(), &(FProcesses[0]), PROCESS);

      if (Index >= 0)
      {
        try
        {
          CheckFileChange(Index, false);
        }
        __finally
        {
          if (!EarlyClose(Index))
          {
            // CheckFileChange may fail (file is already being uploaded),
            // but we want to close handles anyway
            CloseFile(Index, false, true);
          }
        }
      }
    }
    while ((Index >= 0) && (FProcesses.size() > 0));
  }

  if (FUploadCompleteEvents.size() > 0)
  {
    do
    {
      Index = WaitFor(FUploadCompleteEvents.size(), &(FUploadCompleteEvents[0]),
        EVENT);

      if (Index >= 0)
      {
        UploadComplete(Index);
      }
    }
    while ((Index >= 0) && (FUploadCompleteEvents.size() > 0));
  }
}
//---------------------------------------------------------------------------
bool __fastcall TEditorManager::EarlyClose(int Index)
{
  TFileData * FileData = &FFiles[Index];

  bool Result =
    (FileData->Process != INVALID_HANDLE_VALUE) &&
    (Now() - FileData->Opened <=
      TDateTime(0, 0, static_cast<unsigned short>(WinConfiguration->Editor.EarlyClose), 0)) &&
    (FOnFileEarlyClosed != NULL);

  if (Result)
  {
    Result = false;
    FOnFileEarlyClosed(FileData->Data, Result);
    if (Result)
    {
      // forget the associated process
      CloseProcess(Index);
    }
  }
  return Result;
}
//---------------------------------------------------------------------------
void __fastcall TEditorManager::FileChanged(TObject * Token)
{
  int Index = FindFile(Token);

  assert(Index >= 0);
  assert(!FFiles[Index].External);

  CheckFileChange(Index, true);
}
//---------------------------------------------------------------------------
void __fastcall TEditorManager::FileReload(TObject * Token)
{
  int Index = FindFile(Token);

  assert(Index >= 0);
  TFileData * FileData = &FFiles[Index];
  assert(!FileData->External);

  OnFileReload(FileData->FileName, FileData->Data);
  FileAge(FileData->FileName, FileData->Timestamp);
}
//---------------------------------------------------------------------------
void __fastcall TEditorManager::FileClosed(TObject * Token, bool Forced)
{
  int Index = FindFile(Token);

  assert(Index >= 0);
  assert(!FFiles[Index].External);

  CheckFileChange(Index, false);
  CloseFile(Index, false, !Forced);
}
//---------------------------------------------------------------------------
void __fastcall TEditorManager::AddFile(TFileData & FileData, TEditedFileData * AData)
{
  std::unique_ptr<TEditedFileData> Data(AData);

  FileAge(FileData.FileName, FileData.Timestamp);
  FileData.Closed = false;
  FileData.UploadCompleteEvent = INVALID_HANDLE_VALUE;
  FileData.Opened = Now();
  FileData.Reupload = false;
  FileData.Saves = 0;
  FileData.Data = Data.get();

  FFiles.push_back(FileData);

  Data.release(); // ownership passed
}
//---------------------------------------------------------------------------
void __fastcall TEditorManager::UploadComplete(int Index)
{
  TFileData * FileData = &FFiles[Index];

  assert(FileData->UploadCompleteEvent != INVALID_HANDLE_VALUE);

  CloseHandle(FileData->UploadCompleteEvent);
  FUploadCompleteEvents.erase(std::find(FUploadCompleteEvents.begin(),
    FUploadCompleteEvents.end(), FileData->UploadCompleteEvent));
  FileData->UploadCompleteEvent = INVALID_HANDLE_VALUE;

  if (FileData->Closed)
  {
    CloseFile(Index, false, true);
  }
  else if (FileData->Reupload)
  {
    FileData->Reupload = false;
    CheckFileChange(Index, true);
  }
}
//---------------------------------------------------------------------------
void __fastcall TEditorManager::CloseProcess(int Index)
{
  TFileData * FileData = &FFiles[Index];

  FProcesses.erase(std::find(FProcesses.begin(), FProcesses.end(), FileData->Process));
  CHECK(CloseHandle(FileData->Process));
  FileData->Process = INVALID_HANDLE_VALUE;
}
//---------------------------------------------------------------------------
void __fastcall TEditorManager::ReleaseFile(int Index)
{
  TFileData * FileData = &FFiles[Index];
  delete FileData->Data;
  FileData->Data = NULL;
}
//---------------------------------------------------------------------------
bool __fastcall TEditorManager::CloseFile(int Index, bool IgnoreErrors, bool Delete)
{
  bool Result = false;
  TFileData * FileData = &FFiles[Index];

  if (FileData->Process != INVALID_HANDLE_VALUE)
  {
    CloseProcess(Index);
  }

  if (FileData->Monitor != INVALID_HANDLE_VALUE)
  {
    CHECK(FindCloseChangeNotification(FileData->Monitor));
    FMonitors.erase(std::find(FMonitors.begin(), FMonitors.end(), FileData->Monitor));
    FileData->Monitor = INVALID_HANDLE_VALUE;
  }

  if (FileData->UploadCompleteEvent != INVALID_HANDLE_VALUE)
  {
    FileData->Closed = true;
  }
  else
  {
    UnicodeString FileName = FileData->FileName;
    UnicodeString LocalRootDirectory = FileData->Data->LocalRootDirectory;

    ReleaseFile(Index);
    FFiles.erase(FFiles.begin() + Index);
    Result = true;

    if (Delete)
    {
      if (!RecursiveDeleteFile(ExcludeTrailingBackslash(LocalRootDirectory), false) &&
          !IgnoreErrors)
      {
        throw Exception(FMTLOAD(DELETE_TEMP_EXECUTE_FILE_ERROR, (LocalRootDirectory)));
      }
    }
  }
  return Result;
}
//---------------------------------------------------------------------------
void __fastcall TEditorManager::CheckFileChange(int Index, bool Force)
{
  TFileData * FileData = &FFiles[Index];

  TDateTime NewTimestamp;
  FileAge(FileData->FileName, NewTimestamp);
  if (Force || (FileData->Timestamp != NewTimestamp))
  {
    if (FileData->UploadCompleteEvent != INVALID_HANDLE_VALUE)
    {
      FileData->Reupload = true;
      Abort();
    }
    FileData->UploadCompleteEvent = CreateEvent(NULL, false, false, NULL);
    FUploadCompleteEvents.push_back(FileData->UploadCompleteEvent);

    FileData->Timestamp = NewTimestamp;
    FileData->Saves++;
    if (FileData->Saves == 1)
    {
      Configuration->Usage->Inc(L"RemoteFilesSaved");
    }
    Configuration->Usage->Inc(L"RemoteFileSaves");

    try
    {
      assert(OnFileChange != NULL);
      OnFileChange(FileData->FileName, FileData->Data,
        FileData->UploadCompleteEvent);
    }
    catch(...)
    {
      // upload failed (was not even started)
      if (FileData->UploadCompleteEvent != INVALID_HANDLE_VALUE)
      {
        UploadComplete(Index);
      }
      throw;
    }
  }
}
//---------------------------------------------------------------------------
int __fastcall TEditorManager::FindFile(const TObject * Token)
{
  int Index = -1;
  for (unsigned int i = 0; i < FFiles.size(); i++)
  {
    if (!FFiles[i].Closed && (FFiles[i].Token == Token))
    {
      Index = i;
      break;
    }
  }

  return Index;
}
//---------------------------------------------------------------------------
int __fastcall TEditorManager::WaitFor(unsigned int Count, const HANDLE * Handles,
  TWaitHandle WaitFor)
{
  static const unsigned int Offset = MAXIMUM_WAIT_OBJECTS;
  int Result = -1;
  unsigned int Start = 0;
  while ((Result < 0) && (Start < Count))
  {
    unsigned int C = (Count - Start > Offset ? Offset : Count - Start);
    unsigned int WaitResult = WaitForMultipleObjects(C, Handles + Start, false, 0);

    if (WaitResult == WAIT_FAILED)
    {
      throw Exception(LoadStr(WATCH_ERROR_GENERAL));
    }
    else if (WaitResult != WAIT_TIMEOUT)
    {
      // WAIT_OBJECT_0 is zero
      assert(WaitResult < WAIT_OBJECT_0 + Count);

      HANDLE Handle = Handles[WaitResult - WAIT_OBJECT_0];

      for (unsigned int i = 0; i < FFiles.size(); i++)
      {
        TFileData * Data = &FFiles[i];
        HANDLE FHandle;
        switch (WaitFor)
        {
          case MONITOR:
            FHandle = Data->Monitor;
            break;

          case PROCESS:
            FHandle = Data->Process;
            break;

          case EVENT:
            FHandle = Data->UploadCompleteEvent;
            break;
        };

        if (FHandle == Handle)
        {
          Result = Start + i;
          break;
        }
      }
      assert(Result >= 0);
    }

    Start += C;
  }

  return Result;
}
//---------------------------------------------------------------------------
