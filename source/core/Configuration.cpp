//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include <FileInfo.h>

#include "Common.h"
#include "Exceptions.h"
#include "Configuration.h"
#include "PuttyIntf.h"
#include "TextsCore.h"
#include "Interface.h"
#include "CoreMain.h"
#include "Security.h"
#include <shlobj.h>
#include <System.IOUtils.hpp>
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
__fastcall TConfiguration::TConfiguration()
{
  FCriticalSection = new TCriticalSection();
  FUpdating = 0;
  FStorage = stDetect;
  FDontSave = false;
  FApplicationInfo = NULL;
  FUsage = new TUsage(this);
  FDefaultCollectUsage = false;

  wchar_t Buf[10];
  UnicodeString RandomSeedPath;
  if (GetEnvironmentVariable(L"APPDATA", Buf, LENOF(Buf)) > 0)
  {
    RandomSeedPath = L"%APPDATA%";
  }
  else
  {
    RandomSeedPath = GetShellFolderPath(CSIDL_LOCAL_APPDATA);
    if (RandomSeedPath.IsEmpty())
    {
      RandomSeedPath = GetShellFolderPath(CSIDL_APPDATA);
    }
  }

  FDefaultRandomSeedFile = IncludeTrailingBackslash(RandomSeedPath) + L"winscp.rnd";
}
//---------------------------------------------------------------------------
void __fastcall TConfiguration::Default()
{
  TGuard Guard(FCriticalSection);

  FDisablePasswordStoring = false;
  FForceBanners = false;
  FDisableAcceptingHostKeys = false;

  TRegistryStorage * AdminStorage;
  AdminStorage = new TRegistryStorage(RegistryStorageKey, HKEY_LOCAL_MACHINE);
  try
  {
    if (AdminStorage->OpenRootKey(false))
    {
      LoadAdmin(AdminStorage);
      AdminStorage->CloseSubKey();
    }
  }
  __finally
  {
    delete AdminStorage;
  }

  RandomSeedFile = FDefaultRandomSeedFile;
  PuttyRegistryStorageKey = OriginalPuttyRegistryStorageKey;
  FConfirmOverwriting = true;
  FConfirmResume = true;
  FAutoReadDirectoryAfterOp = true;
  FSessionReopenAuto = 5000;
  FSessionReopenBackground = 2000;
  FSessionReopenTimeout = 0;
  FSessionReopenAutoStall = 60000;
  FTunnelLocalPortNumberLow = 50000;
  FTunnelLocalPortNumberHigh = 50099;
  FCacheDirectoryChangesMaxSize = 100;
  FShowFtpWelcomeMessage = false;
  FExternalIpAddress = L"";
  FTryFtpWhenSshFails = true;
  CollectUsage = FDefaultCollectUsage;

  FLogging = false;
  FPermanentLogging = false;
  FLogFileName = DefaultLogFileName;
  FPermanentLogFileName = FLogFileName;
  FLogFileAppend = true;
  FLogWindowLines = 100;
  FLogProtocol = 0;
  UpdateActualLogProtocol();
  FLogActions = false;
  FPermanentLogActions = false;
  FActionsLogFileName = L"%TEMP%\\!S.xml";
  FPermanentActionsLogFileName = FActionsLogFileName;
  FProgramIniPathWrittable = -1;

  Changed();
}
//---------------------------------------------------------------------------
__fastcall TConfiguration::~TConfiguration()
{
  assert(!FUpdating);
  if (FApplicationInfo) FreeFileInfo(FApplicationInfo);
  delete FCriticalSection;
  delete FUsage;
}
//---------------------------------------------------------------------------
void __fastcall TConfiguration::UpdateStaticUsage()
{
  Usage->Set(L"ConfigurationIniFile", (Storage == stIniFile));

  // this is called from here, because we are guarded from calling into
  // master password handler here, see TWinConfiguration::UpdateStaticUsage
  StoredSessions->UpdateStaticUsage();
}
//---------------------------------------------------------------------------
THierarchicalStorage * TConfiguration::CreateScpStorage(bool /*SessionList*/)
{
  THierarchicalStorage * Result;
  if (Storage == stRegistry)
  {
    Result = new TRegistryStorage(RegistryStorageKey);
  }
  else if (Storage == stNul)
  {
    Result = new TIniFileStorage(L"nul");
  }
  else
  {
    Result = new TIniFileStorage(IniFileStorageName);
  }
  return Result;
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TConfiguration::PropertyToKey(const UnicodeString & Property)
{
  // no longer useful
  int P = Property.LastDelimiter(L".>");
  return Property.SubString(P + 1, Property.Length() - P);
}
//---------------------------------------------------------------------------
#define BLOCK(KEY, CANCREATE, BLOCK) \
  if (Storage->OpenSubKey(KEY, CANCREATE, true)) try { BLOCK } __finally { Storage->CloseSubKey(); }
#define KEY(TYPE, VAR) KEYEX(TYPE, VAR, PropertyToKey(TEXT(#VAR)))
#define REGCONFIG(CANCREATE) \
  BLOCK(L"Interface", CANCREATE, \
    KEY(String,   RandomSeedFile); \
    KEY(String,   PuttyRegistryStorageKey); \
    KEY(Bool,     ConfirmOverwriting); \
    KEY(Bool,     ConfirmResume); \
    KEY(Bool,     AutoReadDirectoryAfterOp); \
    KEY(Integer,  SessionReopenAuto); \
    KEY(Integer,  SessionReopenBackground); \
    KEY(Integer,  SessionReopenTimeout); \
    KEY(Integer,  SessionReopenAutoStall); \
    KEY(Integer,  TunnelLocalPortNumberLow); \
    KEY(Integer,  TunnelLocalPortNumberHigh); \
    KEY(Integer,  CacheDirectoryChangesMaxSize); \
    KEY(Bool,     ShowFtpWelcomeMessage); \
    KEY(String,   ExternalIpAddress); \
    KEY(Bool,     TryFtpWhenSshFails); \
    KEY(Bool,     CollectUsage); \
  ); \
  BLOCK(L"Logging", CANCREATE, \
    KEYEX(Bool,  PermanentLogging, L"Logging"); \
    KEYEX(String,PermanentLogFileName, L"LogFileName"); \
    KEY(Bool,    LogFileAppend); \
    KEY(Integer, LogWindowLines); \
    KEY(Integer, LogProtocol); \
    KEYEX(Bool,  PermanentLogActions, L"LogActions"); \
    KEYEX(String,PermanentActionsLogFileName, L"ActionsLogFileName"); \
  );
//---------------------------------------------------------------------------
void __fastcall TConfiguration::SaveData(THierarchicalStorage * Storage, bool /*All*/)
{
  #define KEYEX(TYPE, VAR, NAME) Storage->Write ## TYPE(NAME, VAR)
  REGCONFIG(true);
  #undef KEYEX

  if (Storage->OpenSubKey(L"Usage", true))
  {
    FUsage->Save(Storage);
    Storage->CloseSubKey();
  }
}
//---------------------------------------------------------------------------
void __fastcall TConfiguration::Save()
{
  // only modified, implicit
  DoSave(false, false);
}
//---------------------------------------------------------------------------
void __fastcall TConfiguration::SaveExplicit()
{
  // only modified, explicit
  DoSave(false, true);
}
//---------------------------------------------------------------------------
void __fastcall TConfiguration::DoSave(bool All, bool Explicit)
{
  if (FDontSave) return;

  THierarchicalStorage * AStorage = CreateScpStorage(false);
  try
  {
    AStorage->AccessMode = smReadWrite;
    AStorage->Explicit = Explicit;
    if (AStorage->OpenSubKey(ConfigurationSubKey, true))
    {
      SaveData(AStorage, All);
    }
  }
  __finally
  {
    delete AStorage;
  }

  Saved();

  if (All)
  {
    StoredSessions->Save(true, Explicit);
  }

  // clean up as last, so that if it fails (read only INI), the saving can proceed
  if (Storage == stRegistry)
  {
    CleanupIniFile();
  }
}
//---------------------------------------------------------------------------
void __fastcall TConfiguration::Export(const UnicodeString & FileName)
{
  THierarchicalStorage * Storage = NULL;
  THierarchicalStorage * ExportStorage = NULL;
  try
  {
    ExportStorage = new TIniFileStorage(FileName);
    ExportStorage->AccessMode = smReadWrite;
    ExportStorage->Explicit = true;

    Storage = CreateScpStorage(false);
    Storage->AccessMode = smRead;

    CopyData(Storage, ExportStorage);

    if (ExportStorage->OpenSubKey(ConfigurationSubKey, true))
    {
      SaveData(ExportStorage, true);
    }
  }
  __finally
  {
    delete ExportStorage;
    delete Storage;
  }

  StoredSessions->Export(FileName);
}
//---------------------------------------------------------------------------
void __fastcall TConfiguration::Import(const UnicodeString & FileName)
{
  THierarchicalStorage * Storage = NULL;
  THierarchicalStorage * ImportStorage = NULL;
  try
  {
    ImportStorage = new TIniFileStorage(FileName);
    ImportStorage->AccessMode = smRead;

    Storage = CreateScpStorage(false);
    Storage->AccessMode = smReadWrite;
    Storage->Explicit = true;

    CopyData(ImportStorage, Storage);

    Default();
    LoadFrom(ImportStorage);

    if (ImportStorage->OpenSubKey(Configuration->StoredSessionsSubKey, false))
    {
      StoredSessions->Clear();
      StoredSessions->DefaultSettings->Default();
      StoredSessions->Load(ImportStorage);
    }
  }
  __finally
  {
    delete ImportStorage;
    delete Storage;
  }

  // save all and explicit
  DoSave(true, true);
}
//---------------------------------------------------------------------------
void __fastcall TConfiguration::LoadData(THierarchicalStorage * Storage)
{
  #define KEYEX(TYPE, VAR, NAME) VAR = Storage->Read ## TYPE(NAME, VAR)
  #pragma warn -eas
  REGCONFIG(false);
  #pragma warn +eas
  #undef KEYEX

  if (Storage->OpenSubKey(L"Usage", false))
  {
    FUsage->Load(Storage);
    Storage->CloseSubKey();
  }

  if (FPermanentLogActions && FPermanentActionsLogFileName.IsEmpty() &&
      FPermanentLogging && !FPermanentLogFileName.IsEmpty())
  {
     FPermanentActionsLogFileName = FPermanentLogFileName;
     FPermanentLogging = false;
     FPermanentLogFileName = L"";
  }
}
//---------------------------------------------------------------------------
void __fastcall TConfiguration::LoadAdmin(THierarchicalStorage * Storage)
{
  FDisablePasswordStoring = Storage->ReadBool(L"DisablePasswordStoring", FDisablePasswordStoring);
  FForceBanners = Storage->ReadBool(L"ForceBanners", FForceBanners);
  FDisableAcceptingHostKeys = Storage->ReadBool(L"DisableAcceptingHostKeys", FDisableAcceptingHostKeys);
  FDefaultCollectUsage = Storage->ReadBool(L"DefaultCollectUsage", FDefaultCollectUsage);
}
//---------------------------------------------------------------------------
void __fastcall TConfiguration::LoadFrom(THierarchicalStorage * Storage)
{
  if (Storage->OpenSubKey(ConfigurationSubKey, false))
  {
    LoadData(Storage);
    Storage->CloseSubKey();
  }
}
//---------------------------------------------------------------------------
void __fastcall TConfiguration::Load()
{
  TGuard Guard(FCriticalSection);

  THierarchicalStorage * Storage = CreateScpStorage(false);
  try
  {
    Storage->AccessMode = smRead;
    LoadFrom(Storage);
  }
  __finally
  {
    delete Storage;
  }
}
//---------------------------------------------------------------------------
void __fastcall TConfiguration::CopyData(THierarchicalStorage * Source,
  THierarchicalStorage * Target)
{
  TStrings * Names = new TStringList();
  try
  {
    if (Source->OpenSubKey(ConfigurationSubKey, false))
    {
      if (Target->OpenSubKey(ConfigurationSubKey, true))
      {
        if (Source->OpenSubKey(L"CDCache", false))
        {
          if (Target->OpenSubKey(L"CDCache", true))
          {
            Names->Clear();
            Source->GetValueNames(Names);

            for (int Index = 0; Index < Names->Count; Index++)
            {
              Target->WriteBinaryData(Names->Strings[Index],
                Source->ReadBinaryData(Names->Strings[Index]));
            }

            Target->CloseSubKey();
          }
          Source->CloseSubKey();
        }

        if (Source->OpenSubKey(L"Banners", false))
        {
          if (Target->OpenSubKey(L"Banners", true))
          {
            Names->Clear();
            Source->GetValueNames(Names);

            for (int Index = 0; Index < Names->Count; Index++)
            {
              Target->WriteString(Names->Strings[Index],
                Source->ReadString(Names->Strings[Index], L""));
            }

            Target->CloseSubKey();
          }
          Source->CloseSubKey();
        }

        Target->CloseSubKey();
      }
      Source->CloseSubKey();
    }

    if (Source->OpenSubKey(SshHostKeysSubKey, false))
    {
      if (Target->OpenSubKey(SshHostKeysSubKey, true))
      {
        Names->Clear();
        Source->GetValueNames(Names);

        for (int Index = 0; Index < Names->Count; Index++)
        {
          Target->WriteStringRaw(Names->Strings[Index],
            Source->ReadStringRaw(Names->Strings[Index], L""));
        }

        Target->CloseSubKey();
      }
      Source->CloseSubKey();
    }
  }
  __finally
  {
    delete Names;
  }
}
//---------------------------------------------------------------------------
void __fastcall TConfiguration::LoadDirectoryChangesCache(const UnicodeString SessionKey,
  TRemoteDirectoryChangesCache * DirectoryChangesCache)
{
  THierarchicalStorage * Storage = CreateScpStorage(false);
  try
  {
    Storage->AccessMode = smRead;
    if (Storage->OpenSubKey(ConfigurationSubKey, false) &&
        Storage->OpenSubKey(L"CDCache", false) &&
        Storage->ValueExists(SessionKey))
    {
      DirectoryChangesCache->Deserialize(Storage->ReadBinaryData(SessionKey));
    }
  }
  __finally
  {
    delete Storage;
  }
}
//---------------------------------------------------------------------------
void __fastcall TConfiguration::SaveDirectoryChangesCache(const UnicodeString SessionKey,
  TRemoteDirectoryChangesCache * DirectoryChangesCache)
{
  THierarchicalStorage * Storage = CreateScpStorage(false);
  try
  {
    Storage->AccessMode = smReadWrite;
    if (Storage->OpenSubKey(ConfigurationSubKey, true) &&
        Storage->OpenSubKey(L"CDCache", true))
    {
      UnicodeString Data;
      DirectoryChangesCache->Serialize(Data);
      Storage->WriteBinaryData(SessionKey, Data);
    }
  }
  __finally
  {
    delete Storage;
  }
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TConfiguration::BannerHash(const UnicodeString & Banner)
{
  RawByteString Result;
  Result.SetLength(16);
  md5checksum(
    reinterpret_cast<const char*>(Banner.c_str()), Banner.Length() * sizeof(wchar_t),
    (unsigned char*)Result.c_str());
  return BytesToHex(Result);
}
//---------------------------------------------------------------------------
bool __fastcall TConfiguration::ShowBanner(const UnicodeString SessionKey,
  const UnicodeString & Banner)
{
  bool Result;
  THierarchicalStorage * Storage = CreateScpStorage(false);
  try
  {
    Storage->AccessMode = smRead;
    Result =
      !Storage->OpenSubKey(ConfigurationSubKey, false) ||
      !Storage->OpenSubKey(L"Banners", false) ||
      !Storage->ValueExists(SessionKey) ||
      (Storage->ReadString(SessionKey, L"") != BannerHash(Banner));
  }
  __finally
  {
    delete Storage;
  }

  return Result;
}
//---------------------------------------------------------------------------
void __fastcall TConfiguration::NeverShowBanner(const UnicodeString SessionKey,
  const UnicodeString & Banner)
{
  THierarchicalStorage * Storage = CreateScpStorage(false);
  try
  {
    Storage->AccessMode = smReadWrite;

    if (Storage->OpenSubKey(ConfigurationSubKey, true) &&
        Storage->OpenSubKey(L"Banners", true))
    {
      Storage->WriteString(SessionKey, BannerHash(Banner));
    }
  }
  __finally
  {
    delete Storage;
  }
}
//---------------------------------------------------------------------------
void __fastcall TConfiguration::Changed()
{
  if (FUpdating == 0)
  {
    if (OnChange)
    {
      OnChange(this);
    }
  }
  else
  {
    FChanged = true;
  }
}
//---------------------------------------------------------------------------
void __fastcall TConfiguration::BeginUpdate()
{
  if (FUpdating == 0)
  {
    FChanged = false;
  }
  FUpdating++;
  // Greater value would probably indicate some nesting problem in code
  assert(FUpdating < 6);
}
//---------------------------------------------------------------------------
void __fastcall TConfiguration::EndUpdate()
{
  assert(FUpdating > 0);
  FUpdating--;
  if ((FUpdating == 0) && FChanged)
  {
    FChanged = false;
    Changed();
  }
}
//---------------------------------------------------------------------------
void __fastcall TConfiguration::CleanupConfiguration()
{
  try
  {
    CleanupRegistry(ConfigurationSubKey);
    if (Storage == stRegistry)
    {
      FDontSave = true;
    }
  }
  catch (Exception &E)
  {
    throw ExtException(&E, LoadStr(CLEANUP_CONFIG_ERROR));
  }
}
//---------------------------------------------------------------------------
void __fastcall TConfiguration::CleanupRegistry(UnicodeString CleanupSubKey)
{
  TRegistryStorage *Registry = new TRegistryStorage(RegistryStorageKey);
  try
  {
    Registry->RecursiveDeleteSubKey(CleanupSubKey);
  }
  __finally
  {
    delete Registry;
  }
}
//---------------------------------------------------------------------------
void __fastcall TConfiguration::CleanupHostKeys()
{
  try
  {
    CleanupRegistry(SshHostKeysSubKey);
  }
  catch (Exception &E)
  {
    throw ExtException(&E, LoadStr(CLEANUP_HOSTKEYS_ERROR));
  }
}
//---------------------------------------------------------------------------
void __fastcall TConfiguration::CleanupRandomSeedFile()
{
  try
  {
    DontSaveRandomSeed();
    if (FileExists(RandomSeedFileName))
    {
      DeleteFileChecked(RandomSeedFileName);
    }
  }
  catch (Exception &E)
  {
    throw ExtException(&E, LoadStr(CLEANUP_SEEDFILE_ERROR));
  }
}
//---------------------------------------------------------------------------
void __fastcall TConfiguration::CleanupIniFile()
{
  try
  {
    if (FileExists(IniFileStorageNameForReading))
    {
      DeleteFileChecked(IniFileStorageNameForReading);
    }
    if (Storage == stIniFile)
    {
      FDontSave = true;
    }
  }
  catch (Exception &E)
  {
    throw ExtException(&E, LoadStr(CLEANUP_INIFILE_ERROR));
  }
}
//---------------------------------------------------------------------------
RawByteString __fastcall TConfiguration::EncryptPassword(UnicodeString Password, UnicodeString Key)
{
  if (Password.IsEmpty())
  {
    return RawByteString();
  }
  else
  {
    return ::EncryptPassword(Password, Key);
  }
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TConfiguration::DecryptPassword(RawByteString Password, UnicodeString Key)
{
  if (Password.IsEmpty())
  {
    return UnicodeString();
  }
  else
  {
    return ::DecryptPassword(Password, Key);
  }
}
//---------------------------------------------------------------------------
RawByteString __fastcall TConfiguration::StronglyRecryptPassword(RawByteString Password, UnicodeString /*Key*/)
{
  return Password;
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TConfiguration::GetOSVersionStr()
{
  UnicodeString Result;
  OSVERSIONINFO OSVersionInfo;
  OSVersionInfo.dwOSVersionInfoSize = sizeof(OSVersionInfo);
  if (GetVersionEx(&OSVersionInfo) != 0)
  {
    Result = FORMAT(L"%d.%d.%d", (int(OSVersionInfo.dwMajorVersion),
      int(OSVersionInfo.dwMinorVersion), int(OSVersionInfo.dwBuildNumber)));
    UnicodeString CSDVersion = OSVersionInfo.szCSDVersion;
    if (!CSDVersion.IsEmpty())
    {
      Result += L" " + CSDVersion;
    }
    UnicodeString ProductName = WindowsProductName();
    if (!ProductName.IsEmpty())
    {
      Result += L" - " + ProductName;
    }
  }
  return Result;
}
//---------------------------------------------------------------------------
TVSFixedFileInfo *__fastcall TConfiguration::GetFixedApplicationInfo()
{
  return GetFixedFileInfo(ApplicationInfo);
}
//---------------------------------------------------------------------------
int __fastcall TConfiguration::GetCompoundVersion()
{
  TVSFixedFileInfo * FileInfo = FixedApplicationInfo;
  return CalculateCompoundVersion(
    HIWORD(FileInfo->dwFileVersionMS), LOWORD(FileInfo->dwFileVersionMS),
    HIWORD(FileInfo->dwFileVersionLS), LOWORD(FileInfo->dwFileVersionLS));
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TConfiguration::ModuleFileName()
{
  return ParamStr(0);
}
//---------------------------------------------------------------------------
void * __fastcall TConfiguration::GetFileApplicationInfo(const UnicodeString FileName)
{
  void * Result;
  if (FileName.IsEmpty())
  {
    if (!FApplicationInfo)
    {
      FApplicationInfo = CreateFileInfo(ModuleFileName());
    }
    Result = FApplicationInfo;
  }
  else
  {
    Result = CreateFileInfo(FileName);
  }
  return Result;
}
//---------------------------------------------------------------------------
void * __fastcall TConfiguration::GetApplicationInfo()
{
  return GetFileApplicationInfo("");
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TConfiguration::GetFileProductName(const UnicodeString FileName)
{
  return GetFileFileInfoString(L"ProductName", FileName);
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TConfiguration::GetFileCompanyName(const UnicodeString FileName)
{
  return GetFileFileInfoString(L"CompanyName", FileName);
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TConfiguration::GetProductName()
{
  return GetFileProductName(L"");
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TConfiguration::GetCompanyName()
{
  return GetFileCompanyName(L"");
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TConfiguration::GetFileProductVersion(const UnicodeString FileName)
{
  return TrimVersion(GetFileFileInfoString(L"ProductVersion", FileName));
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TConfiguration::GetFileDescription(const UnicodeString & FileName)
{
  return GetFileFileInfoString(L"FileDescription", FileName);
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TConfiguration::GetProductVersion()
{
  return GetFileProductVersion(L"");
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TConfiguration::TrimVersion(UnicodeString Version)
{
  while ((Version.Pos(L".") != Version.LastDelimiter(L".")) &&
    (Version.SubString(Version.Length() - 1, 2) == L".0"))
  {
    Version.SetLength(Version.Length() - 2);
  }
  return Version;
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TConfiguration::GetVersionStr()
{
  TGuard Guard(FCriticalSection);
  try
  {
    TVSFixedFileInfo * Info = FixedApplicationInfo;
    return FMTLOAD(VERSION, (
      HIWORD(Info->dwFileVersionMS),
      LOWORD(Info->dwFileVersionMS),
      HIWORD(Info->dwFileVersionLS),
      LOWORD(Info->dwFileVersionLS)));
  }
  catch (Exception &E)
  {
    throw ExtException(&E, L"Can't get application version");
  }
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TConfiguration::GetVersion()
{
  TGuard Guard(FCriticalSection);
  try
  {
    TVSFixedFileInfo * Info = FixedApplicationInfo;
    UnicodeString Result;
    Result = TrimVersion(FORMAT(L"%d.%d.%d", (
      HIWORD(Info->dwFileVersionMS),
      LOWORD(Info->dwFileVersionMS),
      HIWORD(Info->dwFileVersionLS))));
    return Result;
  }
  catch (Exception &E)
  {
    throw ExtException(&E, L"Can't get application version");
  }
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TConfiguration::GetFileFileInfoString(const UnicodeString Key,
  const UnicodeString FileName)
{
  TGuard Guard(FCriticalSection);

  UnicodeString Result;
  void * Info = GetFileApplicationInfo(FileName);
  try
  {
    if ((Info != NULL) && (GetTranslationCount(Info) > 0))
    {
      TTranslation Translation;
      Translation = GetTranslation(Info, 0);
      Result = ::GetFileInfoString(Info, Translation, Key);
    }
    else
    {
      assert(!FileName.IsEmpty());
    }
  }
  __finally
  {
    if (!FileName.IsEmpty())
    {
      FreeFileInfo(Info);
    }
  }
  return Result;
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TConfiguration::GetFileInfoString(const UnicodeString Key)
{
  return GetFileFileInfoString(Key, L"");
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TConfiguration::GetRegistryStorageKey()
{
  return GetRegistryKey();
}
//---------------------------------------------------------------------------
void __fastcall TConfiguration::SetNulStorage()
{
  FStorage = stNul;
}
//---------------------------------------------------------------------------
void __fastcall TConfiguration::SetDefaultStorage()
{
  FStorage = stDetect;
}
//---------------------------------------------------------------------------
void __fastcall TConfiguration::SetIniFileStorageName(UnicodeString value)
{
  FIniFileStorageName = value;
  FStorage = stIniFile;
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TConfiguration::GetIniFileStorageNameForReading()
{
  return GetIniFileStorageName(true);
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TConfiguration::GetIniFileStorageNameForReadingWritting()
{
  return GetIniFileStorageName(false);
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TConfiguration::GetIniFileStorageName(bool ReadingOnly)
{
  if (FIniFileStorageName.IsEmpty())
  {
    UnicodeString ProgramPath = ParamStr(0);

    UnicodeString ProgramIniPath = ChangeFileExt(ProgramPath, L".ini");

    UnicodeString IniPath;
    if (FileExists(ProgramIniPath))
    {
      IniPath = ProgramIniPath;
    }
    else
    {
      UnicodeString AppDataIniPath =
        IncludeTrailingBackslash(GetShellFolderPath(CSIDL_APPDATA)) +
        ExtractFileName(ProgramIniPath);
      if (FileExists(AppDataIniPath))
      {
        IniPath = AppDataIniPath;
      }
      else
      {
        // avoid expensive test if we are interested in existing files only
        if (!ReadingOnly && (FProgramIniPathWrittable < 0))
        {
          UnicodeString ProgramDir = ExtractFilePath(ProgramPath);
          FProgramIniPathWrittable = IsDirectoryWriteable(ProgramDir) ? 1 : 0;
        }

        // does not really matter what we return when < 0
        IniPath = (FProgramIniPathWrittable == 0) ? AppDataIniPath : ProgramIniPath;
      }
    }

    // BACKWARD COMPATIBILITY with 4.x
    if (FVirtualIniFileStorageName.IsEmpty() &&
        TPath::IsDriveRooted(IniPath))
    {
      UnicodeString LocalAppDataPath = GetShellFolderPath(CSIDL_LOCAL_APPDATA);
      // virtual store for non-system drives have a different virtual store,
      // do not bother about them
      if (TPath::IsDriveRooted(LocalAppDataPath) &&
          SameText(ExtractFileDrive(IniPath), ExtractFileDrive(LocalAppDataPath)))
      {
        FVirtualIniFileStorageName =
          IncludeTrailingBackslash(LocalAppDataPath) +
          L"VirtualStore\\" +
          IniPath.SubString(4, IniPath.Length() - 3);
      }
    }

    if (!FVirtualIniFileStorageName.IsEmpty() &&
        FileExists(FVirtualIniFileStorageName))
    {
      return FVirtualIniFileStorageName;
    }
    else
    {
      return IniPath;
    }
  }
  else
  {
    return FIniFileStorageName;
  }
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TConfiguration::GetPuttySessionsKey()
{
  return PuttyRegistryStorageKey + L"\\Sessions";
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TConfiguration::GetStoredSessionsSubKey()
{
  return L"Sessions";
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TConfiguration::GetSshHostKeysSubKey()
{
  return L"SshHostKeys";
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TConfiguration::GetConfigurationSubKey()
{
  return L"Configuration";
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TConfiguration::GetRootKeyStr()
{
  return RootKeyToStr(HKEY_CURRENT_USER);
}
//---------------------------------------------------------------------------
void __fastcall TConfiguration::SetStorage(TStorage value)
{
  if (FStorage != value)
  {
    TStorage StorageBak = FStorage;
    try
    {
      THierarchicalStorage * SourceStorage = NULL;
      THierarchicalStorage * TargetStorage = NULL;

      try
      {
        SourceStorage = CreateScpStorage(false);
        SourceStorage->AccessMode = smRead;

        FStorage = value;

        TargetStorage = CreateScpStorage(false);
        TargetStorage->AccessMode = smReadWrite;
        TargetStorage->Explicit = true;

        // copy before save as it removes the ini file,
        // when switching from ini to registry
        CopyData(SourceStorage, TargetStorage);
      }
      __finally
      {
        delete SourceStorage;
        delete TargetStorage;
      }

      // save all and explicit,
      // this also removes an INI file, when switching to registry storage
      DoSave(true, true);
    }
    catch (...)
    {
      // If this fails, do not pretend that storage was switched.
      // For instance:
      // - When writting to an INI file fails (unlikely, as we fallback to user profile)
      // - When removing INI file fails, when switching to registry
      //   (possible, when the INI file is in Program Files folder)
      FStorage = StorageBak;
      throw;
    }
  }
}
//---------------------------------------------------------------------------
void __fastcall TConfiguration::Saved()
{
  // nothing
}
//---------------------------------------------------------------------------
TStorage __fastcall TConfiguration::GetStorage()
{
  if (FStorage == stDetect)
  {
    if (FileExists(IniFileStorageNameForReading))
    {
      FStorage = stIniFile;
    }
    else
    {
      FStorage = stRegistry;
    }
  }
  return FStorage;
}
//---------------------------------------------------------------------------
void __fastcall TConfiguration::SetRandomSeedFile(UnicodeString value)
{
  if (RandomSeedFile != value)
  {
    UnicodeString PrevRandomSeedFileName = RandomSeedFileName;

    FRandomSeedFile = value;

    // never allow empty seed file to avoid Putty trying to reinitialize the path
    if (RandomSeedFileName.IsEmpty())
    {
      FRandomSeedFile = FDefaultRandomSeedFile;
    }

    if (!PrevRandomSeedFileName.IsEmpty() &&
        (PrevRandomSeedFileName != RandomSeedFileName) &&
        FileExists(PrevRandomSeedFileName))
    {
      // ignore any error
      DeleteFile(PrevRandomSeedFileName);
    }
  }
}
//---------------------------------------------------------------------
UnicodeString __fastcall TConfiguration::GetRandomSeedFileName()
{
  return StripPathQuotes(ExpandEnvironmentVariables(FRandomSeedFile)).Trim();
}
//---------------------------------------------------------------------
void __fastcall TConfiguration::SetExternalIpAddress(UnicodeString value)
{
  SET_CONFIG_PROPERTY(ExternalIpAddress);
}
//---------------------------------------------------------------------
void __fastcall TConfiguration::SetTryFtpWhenSshFails(bool value)
{
  SET_CONFIG_PROPERTY(TryFtpWhenSshFails);
}
//---------------------------------------------------------------------
void __fastcall TConfiguration::SetPuttyRegistryStorageKey(UnicodeString value)
{
  SET_CONFIG_PROPERTY(PuttyRegistryStorageKey);
}
//---------------------------------------------------------------------------
TEOLType __fastcall TConfiguration::GetLocalEOLType()
{
  return eolCRLF;
}
//---------------------------------------------------------------------
bool __fastcall TConfiguration::GetCollectUsage()
{
  return FUsage->Collect;
}
//---------------------------------------------------------------------
void __fastcall TConfiguration::SetCollectUsage(bool value)
{
  FUsage->Collect = value;
}
//---------------------------------------------------------------------
void __fastcall TConfiguration::TemporaryLogging(const UnicodeString ALogFileName)
{
  if (SameText(ExtractFileExt(ALogFileName), L".xml"))
  {
    TemporaryActionsLogging(ALogFileName);
  }
  else
  {
    FLogging = true;
    FLogFileName = ALogFileName;
    UpdateActualLogProtocol();
  }
}
//---------------------------------------------------------------------
void __fastcall TConfiguration::TemporaryActionsLogging(const UnicodeString ALogFileName)
{
  FLogActions = true;
  FActionsLogFileName = ALogFileName;
}
//---------------------------------------------------------------------
void __fastcall TConfiguration::SetLogging(bool value)
{
  if (Logging != value)
  {
    FPermanentLogging = value;
    FLogging = value;
    UpdateActualLogProtocol();
    Changed();
  }
}
//---------------------------------------------------------------------
void __fastcall TConfiguration::SetLogFileName(UnicodeString value)
{
  if (LogFileName != value)
  {
    FPermanentLogFileName = value;
    FLogFileName = value;
    Changed();
  }
}
//---------------------------------------------------------------------
void __fastcall TConfiguration::SetActionsLogFileName(UnicodeString value)
{
  if (ActionsLogFileName != value)
  {
    FPermanentActionsLogFileName = value;
    FActionsLogFileName = value;
    Changed();
  }
}
//---------------------------------------------------------------------
bool __fastcall TConfiguration::GetLogToFile()
{
  return !LogFileName.IsEmpty();
}
//---------------------------------------------------------------------
void __fastcall TConfiguration::UpdateActualLogProtocol()
{
  FActualLogProtocol = FLogging ? FLogProtocol : 0;
}
//---------------------------------------------------------------------
void __fastcall TConfiguration::SetLogProtocol(int value)
{
  SET_CONFIG_PROPERTY(LogProtocol);
  UpdateActualLogProtocol();
}
//---------------------------------------------------------------------
void __fastcall TConfiguration::SetLogActions(bool value)
{
  if (LogActions != value)
  {
    FPermanentLogActions = value;
    FLogActions = value;
    Changed();
  }
}
//---------------------------------------------------------------------
void __fastcall TConfiguration::SetLogFileAppend(bool value)
{
  SET_CONFIG_PROPERTY(LogFileAppend);
}
//---------------------------------------------------------------------
void __fastcall TConfiguration::SetLogWindowLines(int value)
{
  SET_CONFIG_PROPERTY(LogWindowLines);
}
//---------------------------------------------------------------------
void __fastcall TConfiguration::SetLogWindowComplete(bool value)
{
  if (value != LogWindowComplete)
  {
    LogWindowLines = value ? 0 : 50;
    Changed();
  }
}
//---------------------------------------------------------------------
bool __fastcall TConfiguration::GetLogWindowComplete()
{
  return (bool)(LogWindowLines == 0);
}
//---------------------------------------------------------------------
UnicodeString __fastcall TConfiguration::GetDefaultLogFileName()
{
  return L"%TEMP%\\!S.log";
}
//---------------------------------------------------------------------------
void __fastcall TConfiguration::SetConfirmOverwriting(bool value)
{
  TGuard Guard(FCriticalSection);
  SET_CONFIG_PROPERTY(ConfirmOverwriting);
}
//---------------------------------------------------------------------------
bool __fastcall TConfiguration::GetConfirmOverwriting()
{
  TGuard Guard(FCriticalSection);
  return FConfirmOverwriting;
}
//---------------------------------------------------------------------------
void __fastcall TConfiguration::SetConfirmResume(bool value)
{
  TGuard Guard(FCriticalSection);
  SET_CONFIG_PROPERTY(ConfirmResume);
}
//---------------------------------------------------------------------------
bool __fastcall TConfiguration::GetConfirmResume()
{
  TGuard Guard(FCriticalSection);
  return FConfirmResume;
}
//---------------------------------------------------------------------------
void __fastcall TConfiguration::SetAutoReadDirectoryAfterOp(bool value)
{
  TGuard Guard(FCriticalSection);
  SET_CONFIG_PROPERTY(AutoReadDirectoryAfterOp);
}
//---------------------------------------------------------------------------
bool __fastcall TConfiguration::GetAutoReadDirectoryAfterOp()
{
  TGuard Guard(FCriticalSection);
  return FAutoReadDirectoryAfterOp;
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TConfiguration::GetTimeFormat()
{
  return L"h:nn:ss";
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TConfiguration::GetPartialExt() const
{
  return PARTIAL_EXT;
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TConfiguration::GetDefaultKeyFile()
{
  return L"";
}
//---------------------------------------------------------------------------
bool __fastcall TConfiguration::GetRememberPassword()
{
  return false;
}
//---------------------------------------------------------------------------
void __fastcall TConfiguration::SetSessionReopenAuto(int value)
{
  SET_CONFIG_PROPERTY(SessionReopenAuto);
}
//---------------------------------------------------------------------------
void __fastcall TConfiguration::SetSessionReopenBackground(int value)
{
  SET_CONFIG_PROPERTY(SessionReopenBackground);
}
//---------------------------------------------------------------------------
void __fastcall TConfiguration::SetSessionReopenTimeout(int value)
{
  SET_CONFIG_PROPERTY(SessionReopenTimeout);
}
//---------------------------------------------------------------------------
void __fastcall TConfiguration::SetSessionReopenAutoStall(int value)
{
  SET_CONFIG_PROPERTY(SessionReopenAutoStall);
}
//---------------------------------------------------------------------------
void __fastcall TConfiguration::SetTunnelLocalPortNumberLow(int value)
{
  SET_CONFIG_PROPERTY(TunnelLocalPortNumberLow);
}
//---------------------------------------------------------------------------
void __fastcall TConfiguration::SetTunnelLocalPortNumberHigh(int value)
{
  SET_CONFIG_PROPERTY(TunnelLocalPortNumberHigh);
}
//---------------------------------------------------------------------------
void __fastcall TConfiguration::SetCacheDirectoryChangesMaxSize(int value)
{
  SET_CONFIG_PROPERTY(CacheDirectoryChangesMaxSize);
}
//---------------------------------------------------------------------------
void __fastcall TConfiguration::SetShowFtpWelcomeMessage(bool value)
{
  SET_CONFIG_PROPERTY(ShowFtpWelcomeMessage);
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void __fastcall TShortCuts::Add(TShortCut ShortCut)
{
  FShortCuts.insert(ShortCut);
}
//---------------------------------------------------------------------------
bool __fastcall TShortCuts::Has(TShortCut ShortCut) const
{
  return (FShortCuts.count(ShortCut) != 0);
}
