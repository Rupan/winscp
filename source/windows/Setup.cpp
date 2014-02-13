//---------------------------------------------------------------------------
// Part of this code is
// Copyright (C) 2002-2004, Marco Barisione <marco.bari@vene.ws>
//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include <stdio.h>
#include <tchar.h>
#include <shlobj.h>
#include <tlhelp32.h>
#include <Common.h>
#include <CoreMain.h>
#include <Exceptions.h>
#include <TextsWin.h>
#include <HelpWin.h>
#include <TcpIp.hpp>
#include <CompThread.hpp>
#include <FileInfo.h>
#include "WinConfiguration.h"
#include "WinInterface.h"
#include "Tools.h"
#include "Setup.h"
#include <StrUtils.hpp>
#include "ProgParams.h"
//---------------------------------------------------------------------------
#define KEY _T("SYSTEM\\CurrentControlSet\\Control\\") \
            _T("Session Manager\\Environment")
// when the PATH registry key is over aprox 2048 characters,
// PATH as well as WINDIR variables are actually not set, breaking the system
#define MAX_PATH_LEN 2000

/* Command line options. */
UnicodeString LastPathError;
//---------------------------------------------------------------------------
// Display the error "err_msg".
void err_out(LPCTSTR err_msg)
{
  LastPathError = err_msg;
}
//---------------------------------------------------------------------------
// Display "base_err_msg" followed by the description of the system error
// identified by "sys_err".
void err_out_sys(LPCTSTR base_err_msg, LONG sys_err)
{
  LastPathError = FORMAT(L"%s %s", (base_err_msg, SysErrorMessage(sys_err)));
}
//---------------------------------------------------------------------------
// Works as "strcmp" but the comparison is not case sensitive.
int tcharicmp(LPCTSTR str1, LPCTSTR str2){
    for (; tolower(*str1) == tolower(*str2); ++str1, ++str2)
        if (*str1 == L'\0')
            return 0;
    return tolower(*str1) - tolower(*str2);
}
//---------------------------------------------------------------------------
// Returns un unquoted copy of "str" (or a copy of "str" if the quotes are
// not present). The returned value must be freed with "free".
LPTSTR unquote(LPCTSTR str){
    int last_pos;
    LPTSTR ret;
    size_t new_len;

    last_pos = _tcslen(str) - 1;
    if (last_pos != -1 && str[0] == L'"' && str[last_pos] == L'"'){
        new_len= (_tcslen(str) - 1);
        ret = (LPTSTR)malloc(new_len * sizeof(TCHAR));
        lstrcpyn(ret, &str[1], new_len);
    }
    else
        ret = _tcsdup(str);
    return ret;
}
//---------------------------------------------------------------------------
// Find "what" in the ";" separated string "str" and returns a pointer to
// the first letter of "what" in the string. If "next" is not "NULL" it
// points to the first letter after "what" (excluding the trailing ";").
// If "what" isn't find the functions returns "NULL".
LPTSTR find_reg_str(LPTSTR str, LPCTSTR what, LPTSTR * next){
    LPTSTR tok_buff;
    LPTSTR curr_tok;
    LPTSTR curr_tok_dup;
    BOOL path_eq;
    TCHAR sh_path1[MAX_PATH], sh_path2[MAX_PATH];
    int pos = -1;
    LPTSTR ret;

    tok_buff = _tcsdup(str);
    curr_tok = _tcstok(tok_buff, _T(";"));
    while (pos == -1 && curr_tok){
        curr_tok_dup = unquote(curr_tok);
        path_eq = GetShortPathName(what, sh_path1, LENOF(sh_path1)) &&
                  GetShortPathName(curr_tok_dup, sh_path2,
                                   LENOF(sh_path2)) &&
                  (tcharicmp(sh_path1, sh_path2) == 0);
        if (path_eq || tcharicmp(what, curr_tok_dup) == 0){
            pos = curr_tok - tok_buff;
        }
        free(curr_tok_dup);
        curr_tok = _tcstok(NULL, _T(";"));
        if (pos != -1 && next){
            if (curr_tok)
                *next = str + (curr_tok - tok_buff);
            else
                *next = str + _tcslen(str);
        }
    }
    free(tok_buff);
    if (pos != -1)
        ret = str + pos;
    else
        ret = NULL;
    return ret;
}
//---------------------------------------------------------------------------
void path_reg_propagate()
{
  DWORD send_message_result;
  LONG ret = SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0,
                           (LPARAM)_T("Environment"), SMTO_ABORTIFHUNG,
                           5000, &send_message_result);
  if (ret != ERROR_SUCCESS && GetLastError() != 0)
  {
    err_out_sys(_T("Cannot propagate the new enviroment to ")
                _T("other processes. The new value will be ")
                _T("avaible after a reboot."), GetLastError());
    SimpleErrorDialog(LastPathError);
    LastPathError = L"";
  }
}
//---------------------------------------------------------------------------
// Add "path" to the registry. Return "TRUE" if the path has been added or
// was already in the registry, "FALSE" otherwise.
BOOL add_path_reg(LPCTSTR path){
    HKEY key;
    LONG ret;
    DWORD data_size;
    LPTSTR reg_str;
    BOOL func_ret = TRUE;

    ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, KEY, 0,
                       KEY_WRITE | KEY_READ, &key);
    if (ret != ERROR_SUCCESS){
        err_out_sys(_T("Cannot open registry."), ret);
        return FALSE;
    }

    RegQueryValueEx(key, _T("PATH"), NULL, NULL, NULL, &data_size);
    data_size += _tcslen(path) + 3 ; /* ";" and quotes, "data_size" already
                                        includes '\0'. */
    reg_str = (LPTSTR)malloc(data_size * sizeof(TCHAR));
    ret = RegQueryValueEx(key, _T("PATH"), NULL, NULL, (LPBYTE)reg_str,
                          &data_size);
    if (ret != ERROR_SUCCESS){
        err_out_sys(_T("Cannot read \"PATH\" key."), ret);
        func_ret = FALSE;
    }
    else{
        if (!find_reg_str(reg_str, path, NULL)){
            _tcscat(reg_str, _T(";"));
            _tcscat(reg_str, path);
            size_t len = _tcslen(reg_str);
            if (len >= MAX_PATH_LEN)
            {
              err_out(LoadStr(PATH_ENV_TOO_LONG).c_str());
              func_ret = FALSE;
            }
            else
            {
              ret = RegSetValueEx(key, _T("PATH"), 0, REG_EXPAND_SZ,
                                  (LPBYTE)reg_str,
                                  (_tcslen(reg_str) + 1) * sizeof(TCHAR));
              if (ret != ERROR_SUCCESS){
                  err_out_sys(_T("Cannot write \"PATH\" key."), ret);
                  func_ret = FALSE;
              }
              /* Is this needed to make the new key avaible? */
              RegFlushKey(key);
              SetLastError(0);
              path_reg_propagate();
            }
        }
    }

    RegCloseKey(key);
    free(reg_str);
    return func_ret;
}
//---------------------------------------------------------------------------
// Removes "path" from the registry. Return "TRUE" if the path has been
// removed or it wasn't in the registry, "FALSE" otherwise.
BOOL remove_path_reg(LPCTSTR path){
    HKEY key;
    LONG ret;
    DWORD data_size;
    LPTSTR reg_str;
    LPTSTR reg_str2;
    BOOL func_ret = TRUE;
    LPTSTR next;
    LPTSTR del_part;
    int last_pos;

    ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, KEY, 0,
                       KEY_WRITE | KEY_READ, &key);
    if (ret != ERROR_SUCCESS){
        err_out_sys(_T("Cannot open registry."), ret);
        return FALSE;
    }

    RegQueryValueEx(key, _T("PATH"), NULL, NULL, NULL, &data_size);
    data_size += _tcslen(path) + 3; /* ";" and quotes,"data_size" already
                                        includes '\0'. */
    reg_str = (LPTSTR)malloc(data_size * sizeof(TCHAR));
    ret = RegQueryValueEx(key, _T("PATH"), NULL, NULL,
                          (LPBYTE)reg_str, &data_size);
    if (ret != ERROR_SUCCESS){
        err_out_sys(_T("Cannot read \"PATH\" key."), ret);
        func_ret = FALSE;
    }
    else{
        if ((del_part = find_reg_str(reg_str, path, &next)) != NULL){
            reg_str2 = (LPTSTR)malloc((_tcslen(reg_str) + 1) * sizeof(TCHAR));
            *del_part = '\0';
            _stprintf(reg_str2, _T("%s%s"), reg_str, next);
            last_pos = _tcslen(reg_str2) - 1;
            if (last_pos != -1 && reg_str2[last_pos] == ';')
                reg_str2[last_pos] = '\0';
            ret = RegSetValueEx(key, _T("PATH"), 0, REG_EXPAND_SZ,
                                (LPBYTE)reg_str2,
                                (_tcslen(reg_str2) + 1) * sizeof(TCHAR));
            if (ret != ERROR_SUCCESS){
                err_out_sys(_T("Cannot write \"PATH\" key."), ret);
                func_ret = FALSE;
            }
            free(reg_str2);
            /* Is this needed to make the new key avaible? */
            RegFlushKey(key);
            SetLastError(0);
            path_reg_propagate();
        }
    }

    RegCloseKey(key);
    free(reg_str);
    return func_ret;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void __fastcall AddSearchPath(const UnicodeString Path)
{
  if (!add_path_reg(Path.c_str()))
  {
    throw ExtException(FMTLOAD(ADD_PATH_ERROR, (Path)), LastPathError);
  }
}
//---------------------------------------------------------------------------
void __fastcall RemoveSearchPath(const UnicodeString Path)
{
  if (!remove_path_reg(Path.c_str()))
  {
    throw ExtException(FMTLOAD(REMOVE_PATH_ERROR, (Path)), LastPathError);
  }
}
//---------------------------------------------------------------------------
static const UnicodeString SoftwareClassesBaseKey = L"Software\\Classes\\";
//---------------------------------------------------------------------------
static void __fastcall DeleteKeyIfEmpty(TRegistry * Registry, const UnicodeString & Key, bool AllowRootValues)
{
  if (Registry->OpenKey(Key, false))
  {
    std::auto_ptr<TStrings> List(new TStringList());

    Registry->GetValueNames(List.get());
    bool CanDelete = true;
    for (int Index = 0; CanDelete && (Index < List->Count); Index++)
    {
      UnicodeString ValueName = List->Strings[Index];
      if (!AllowRootValues)
      {
        CanDelete = false;
      }
      if ((ValueName != L"") &&
          (ValueName != L"URL Protocol") &&
          (ValueName != L"EditFlags") &&
          (ValueName != L"BrowserFlags"))
      {
        CanDelete = false;
      }
    }

    List->Clear();
    Registry->GetKeyNames(List.get());

    Registry->CloseKey();

    if (CanDelete)
    {
      for (int Index = 0; Index < List->Count; Index++)
      {
        DeleteKeyIfEmpty(Registry, IncludeTrailingBackslash(Key) + List->Strings[Index], false);
      }

      // will fail, if not all subkeys got removed
      Registry->DeleteKey(Key);
    }
  }
}
//---------------------------------------------------------------------------
static void __fastcall RegisterProtocol(TRegistry * Registry,
  const UnicodeString & Protocol, UnicodeString Description, bool Force)
{

  if (Description.IsEmpty())
  {
    Description = FMTLOAD(PROTOCOL_URL_DESC, (Protocol));
  }

  UnicodeString ProtocolKey = SoftwareClassesBaseKey + Protocol;
  if (Force || !Registry->KeyExists(ProtocolKey))
  {
    if (Registry->OpenKey(SoftwareClassesBaseKey + Protocol, true))
    {
      Registry->WriteString(L"", Description);
      Registry->WriteString(L"URL Protocol", L"");
      Registry->WriteInteger(L"EditFlags", 0x02);
      Registry->WriteInteger(L"BrowserFlags", 0x08);
      if (Registry->OpenKey(L"DefaultIcon", true))
      {
        Registry->WriteString(L"", FORMAT(L"\"%s\",0", (Application->ExeName)));
        Registry->CloseKey();
      }
      else
      {
        Abort();
      }
    }
    else
    {
      Abort();
    }
  }
}
//---------------------------------------------------------------------------
static void __fastcall UnregisterProtocol(TRegistry * Registry,
  const UnicodeString & Protocol)
{
  DeleteKeyIfEmpty(Registry, SoftwareClassesBaseKey + Protocol, true);
}
//---------------------------------------------------------------------------
static TRegistry * __fastcall CreateRegistry(HKEY RootKey)
{
  std::auto_ptr<TRegistry> Registry(new TRegistry());

  Registry->Access = KEY_WRITE | KEY_READ;
  Registry->RootKey = RootKey;

  return Registry.release();
}
//---------------------------------------------------------------------------
static void __fastcall RegisterAsUrlHandler(HKEY RootKey,
  const UnicodeString & Protocol, UnicodeString Description = "")
{
  std::auto_ptr<TRegistry> Registry(CreateRegistry(RootKey));

  RegisterProtocol(Registry.get(), Protocol, Description, true);

  if (Registry->OpenKey(SoftwareClassesBaseKey + Protocol, false) &&
      Registry->OpenKey(L"shell", true) &&
      Registry->OpenKey(L"open", true) &&
      Registry->OpenKey(L"command", true))
  {
    Registry->WriteString(L"", FORMAT(L"\"%s\" /unsafe \"%%1\"", (Application->ExeName)));
    Registry->CloseKey();
  }
  else
  {
    Abort();
  }
}
//---------------------------------------------------------------------------
static void __fastcall RegisterAsUrlHandler(const UnicodeString & Protocol, UnicodeString Description = L"")
{

  try
  {
    RegisterAsUrlHandler(HKEY_LOCAL_MACHINE, Protocol, Description);

    // get rid of any HKCU registraction that would overrite the HKLM one
    std::auto_ptr<TRegistry> Registry(CreateRegistry(HKEY_CURRENT_USER));
    if (Registry->KeyExists(SoftwareClassesBaseKey + Protocol))
    {
      Registry->DeleteKey(SoftwareClassesBaseKey + Protocol);
    }
  }
  catch (Exception & E)
  {
    try
    {
      RegisterAsUrlHandler(HKEY_CURRENT_USER, Protocol, Description);
    }
    catch(Exception & E)
    {
      throw ExtException(&E, LoadStr(REGISTER_URL_ERROR2));
    }
  }
}
//---------------------------------------------------------------------------
static void __fastcall UnregisterAsUrlHandler(HKEY RootKey,
  const UnicodeString & Protocol, bool UnregisterProtocol, bool ForceHandlerUnregistration)
{
  std::auto_ptr<TRegistry> Registry(CreateRegistry(RootKey));

  UnicodeString DefaultIconKey = SoftwareClassesBaseKey + Protocol + L"\\DefaultIcon";
  if (Registry->OpenKey(DefaultIconKey, false))
  {
    UnicodeString Value = Registry->ReadString(L"");
    UnicodeString ExeBaseName = ExtractFileBaseName(Application->ExeName);
    if (ForceHandlerUnregistration || ContainsText(Value, ExeBaseName))
    {
      Registry->DeleteValue(L"");
    }
    Registry->CloseKey();

    DeleteKeyIfEmpty(Registry.get(), DefaultIconKey, false);
  }

  UnicodeString ShellKey = SoftwareClassesBaseKey + Protocol + L"\\shell";
  if (Registry->OpenKey(ShellKey + L"\\open\\command", false))
  {
    UnicodeString Value = Registry->ReadString(L"");
    UnicodeString ExeBaseName = ExtractFileBaseName(Application->ExeName);
    if (ForceHandlerUnregistration || ContainsText(Value, ExeBaseName))
    {
      Registry->DeleteValue(L"");
    }

    Registry->CloseKey();

    DeleteKeyIfEmpty(Registry.get(), ShellKey, false);
  }

  if (UnregisterProtocol)
  {
    ::UnregisterProtocol(Registry.get(), Protocol);
  }
}
//---------------------------------------------------------------------------
static void __fastcall UnregisterAsUrlHandler(const UnicodeString & Protocol, bool UnregisterProtocol)
{
  UnregisterAsUrlHandler(HKEY_LOCAL_MACHINE, Protocol, UnregisterProtocol, false);
  UnregisterAsUrlHandler(HKEY_CURRENT_USER, Protocol, UnregisterProtocol, false);
}
//---------------------------------------------------------------------------
static void __fastcall RegisterAsNonBrowserUrlHandler(const UnicodeString & Prefix)
{
  RegisterAsUrlHandler(Prefix + SftpProtocol.UpperCase());
  RegisterAsUrlHandler(Prefix + ScpProtocol.UpperCase());
}
//---------------------------------------------------------------------------
static void __fastcall UnregisterAsUrlHandlers(const UnicodeString & Prefix, bool UnregisterProtocol)
{
  UnregisterAsUrlHandler(Prefix + SftpProtocol, UnregisterProtocol);
  UnregisterAsUrlHandler(Prefix + ScpProtocol, UnregisterProtocol);
  // add WebDAV
}
//---------------------------------------------------------------------------
static const UnicodeString GenericUrlHandler(L"WinSCP.Url");
//---------------------------------------------------------------------------
static void __fastcall RegisterProtocolForDefaultPrograms(HKEY RootKey, const UnicodeString & Protocol)
{
  // Register protocol, if it does not exist yet.
  // Prior to Windows 8, we need to register ourselves as legacy handler to
  // become the default handler. On Windows 8, it's automatic as long as no other
  // application is registered for the protocol (i.e. RegisterProtocol would be enough)
  RegisterAsUrlHandler(RootKey, Protocol);

  // see http://msdn.microsoft.com/en-us/library/windows/desktop/cc144154.aspx#registration
  std::auto_ptr<TRegistry> Registry(CreateRegistry(RootKey));

  // create capabilities record

  // this has to be a separate branch from WinSCP one, as by its presence we
  // enforce registry storage usage, and the capabilities branch may exist
  // even if we are using INI file
  UnicodeString CapabilitiesKey = IncludeTrailingBackslash(GetCompanyRegistryKey()) + L"WinSCPCapabilities";
  if (!Registry->OpenKey(CapabilitiesKey, true))
  {
    Abort();
  }

  UnicodeString Description = LoadStr(REGISTERED_APP_DESC);
  Registry->WriteString(L"ApplicationDescription", Description);

  if (!Registry->OpenKey(L"UrlAssociations", true))
  {
    Abort();
  }

  Registry->WriteString(Protocol, GenericUrlHandler);
  Registry->CloseKey();

  // register application

  if (!Registry->OpenKey(L"Software\\RegisteredApplications", true))
  {
    Abort();
  }

  Registry->WriteString(AppNameString(), CapabilitiesKey);
  Registry->CloseKey();
}
//---------------------------------------------------------------------------
static void __fastcall UnregisterProtocolForDefaultPrograms(HKEY RootKey,
  const UnicodeString & Protocol, bool ForceHandlerUnregistration)
{
  std::auto_ptr<TRegistry> Registry(CreateRegistry(RootKey));

  // unregister the protocol
  UnregisterAsUrlHandler(RootKey, Protocol, false, ForceHandlerUnregistration);

  // remove capabilities record

  UnicodeString CapabilitiesKey = IncludeTrailingBackslash(GetCompanyRegistryKey()) + L"WinSCPCapabilities";
  UnicodeString UrlAssociationsKey = CapabilitiesKey + L"\\UrlAssociations";
  if (Registry->OpenKey(UrlAssociationsKey, false))
  {
    Registry->DeleteValue(Protocol);
    Registry->CloseKey();

    DeleteKeyIfEmpty(Registry.get(), UrlAssociationsKey, false);
  }

  if (Registry->OpenKey(CapabilitiesKey, false))
  {
    if (!Registry->HasSubKeys())
    {
      Registry->DeleteValue(L"ApplicationDescription");
    }

    Registry->CloseKey();

    DeleteKeyIfEmpty(Registry.get(), CapabilitiesKey, false);
  }

  if (!Registry->KeyExists(CapabilitiesKey))
  {
    // unregister application

    if (Registry->OpenKey(L"Software\\RegisteredApplications", false))
    {
      Registry->DeleteValue(AppNameString());
      Registry->CloseKey();
    }
  }
}
//---------------------------------------------------------------------------
static void __fastcall RegisterProtocolsForDefaultPrograms(HKEY RootKey)
{
  // register URL handler, if it does not exist yet
  RegisterAsUrlHandler(RootKey, GenericUrlHandler, L"WinSCP URL");

  RegisterProtocolForDefaultPrograms(RootKey, FtpProtocol);
  RegisterProtocolForDefaultPrograms(RootKey, FtpsProtocol);
  RegisterProtocolForDefaultPrograms(RootKey, SftpProtocol);
  RegisterProtocolForDefaultPrograms(RootKey, ScpProtocol);
}
//---------------------------------------------------------------------------
static void __fastcall UnregisterProtocolsForDefaultPrograms(HKEY RootKey, bool ForceHandlerUnregistration)
{
  UnregisterProtocolForDefaultPrograms(RootKey, FtpProtocol, ForceHandlerUnregistration);
  UnregisterProtocolForDefaultPrograms(RootKey, FtpsProtocol, ForceHandlerUnregistration);
  UnregisterProtocolForDefaultPrograms(RootKey, SftpProtocol, ForceHandlerUnregistration);
  UnregisterProtocolForDefaultPrograms(RootKey, ScpProtocol, ForceHandlerUnregistration);

  // we should not really need the "force" flag here, but why not
  UnregisterAsUrlHandler(RootKey, GenericUrlHandler, true, true);
}
//---------------------------------------------------------------------------
static void __fastcall RegisterForDefaultPrograms()
{
  try
  {
    RegisterProtocolsForDefaultPrograms(HKEY_LOCAL_MACHINE);
    // make sure we unregister any legacy protocol handler for CU,
    // this is needed for Windows Vista+7
    UnregisterProtocolsForDefaultPrograms(HKEY_CURRENT_USER, true);
  }
  catch (Exception & E)
  {
    try
    {
      RegisterProtocolsForDefaultPrograms(HKEY_CURRENT_USER);
    }
    catch (Exception & E)
    {
      throw ExtException(&E, LoadStr(REGISTER_URL_ERROR2));
    }
  }
}
//---------------------------------------------------------------------------
static void __fastcall NotifyChangedAssociations()
{
  SHChangeNotify(SHCNE_ASSOCCHANGED, 0, 0, 0);
}
//---------------------------------------------------------------------------
void __fastcall RegisterForDefaultProtocols()
{
  if (IsWinVista())
  {
    RegisterForDefaultPrograms();
  }
  else
  {
    RegisterAsNonBrowserUrlHandler(UnicodeString());
  }

  RegisterAsNonBrowserUrlHandler(WinSCPProtocolPrefix);
  RegisterAsUrlHandler(WinSCPProtocolPrefix + FtpProtocol.UpperCase());
  RegisterAsUrlHandler(WinSCPProtocolPrefix + FtpsProtocol.UpperCase());
  // add WebDAV

  NotifyChangedAssociations();
}
//---------------------------------------------------------------------------
void __fastcall UnregisterForProtocols()
{
  UnregisterAsUrlHandlers(UnicodeString(), false);
  UnregisterAsUrlHandlers(WinSCPProtocolPrefix, true);
  UnregisterAsUrlHandler(WinSCPProtocolPrefix + FtpProtocol.UpperCase(), true);
  UnregisterAsUrlHandler(WinSCPProtocolPrefix + FtpsProtocol.UpperCase(), true);

  UnregisterProtocolsForDefaultPrograms(HKEY_CURRENT_USER, false);
  UnregisterProtocolsForDefaultPrograms(HKEY_LOCAL_MACHINE, false);

  NotifyChangedAssociations();
}
//---------------------------------------------------------------------------
void __fastcall LaunchAdvancedAssociationUI()
{
  assert(IsWinVista());

  RegisterForDefaultPrograms();
  NotifyChangedAssociations();
  // sleep recommended by http://msdn.microsoft.com/en-us/library/windows/desktop/cc144154.aspx#browser
  Sleep(1000);

  IApplicationAssociationRegistrationUI * AppAssocRegUI;

  HRESULT Result =
    CoCreateInstance(CLSID_ApplicationAssociationRegistrationUI,
      NULL, CLSCTX_INPROC, __uuidof(IApplicationAssociationRegistrationUI), (void**)&AppAssocRegUI);
  if (SUCCEEDED(Result))
  {
    AppAssocRegUI->LaunchAdvancedAssociationUI(AppNameString().c_str());
    AppAssocRegUI->Release();
  }
}
//---------------------------------------------------------------------------
void __fastcall TemporaryDirectoryCleanup()
{
  bool Continue = true;
  TStrings * Folders = NULL;
  try
  {
    if (WinConfiguration->ConfirmTemporaryDirectoryCleanup)
    {
      Folders = WinConfiguration->FindTemporaryFolders();
      Continue = (Folders != NULL);

      if (Continue)
      {
        Configuration->Usage->Inc(L"TemporaryDirectoryCleanupConfirmations");

        TQueryButtonAlias Aliases[1];
        Aliases[0].Button = qaRetry;
        Aliases[0].Alias = LoadStr(OPEN_BUTTON);
        TMessageParams Params(mpNeverAskAgainCheck);
        Params.Aliases = Aliases;
        Params.AliasesCount = LENOF(Aliases);

        unsigned int Answer = MoreMessageDialog(
          FMTLOAD(CLEANTEMP_CONFIRM2, (Folders->Count)), Folders,
          qtConfirmation, qaYes | qaNo | qaRetry, HELP_CLEAN_TEMP_CONFIRM, &Params);

        if (Answer == qaNeverAskAgain)
        {
          WinConfiguration->ConfirmTemporaryDirectoryCleanup = false;
          Answer = qaYes;
        }
        else if (Answer == qaRetry)
        {
          for (int Index = 0; Index < Folders->Count; Index++)
          {
            ShellExecute(Application->Handle, NULL,
              Folders->Strings[Index].c_str(), NULL, NULL, SW_SHOWNORMAL);
          }
        }
        Continue = (Answer == qaYes);
      }
    }

    if (Continue)
    {
      try
      {
        WinConfiguration->CleanupTemporaryFolders(Folders);
      }
      catch (Exception &E)
      {
        ShowExtendedException(&E);
      }
    }
  }
  __finally
  {
    delete Folders;
  }
}
//-------------------------------------------- -------------------------------
UnicodeString __fastcall VersionStrFromCompoundVersion(int Version)
{
  int MajorVer = Version / (10000*100*100);
  int MinorVer = (Version % (10000*100*100)) / (10000*100);
  int Release = (Version % (10000*100)) / (10000);
  UnicodeString Result;
  if (Release > 0)
  {
    Result = FORMAT(L"%d.%d.%d", (MajorVer, MinorVer, Release));
  }
  else
  {
    Result = FORMAT(L"%d.%d", (MajorVer, MinorVer));
  }
  return Result;
}
//---------------------------------------------------------------------------
UnicodeString __fastcall CampaignUrl(UnicodeString AURL)
{
  // see also TWebHelpSystem::ShowHelp
  const wchar_t FragmentSeparator = L'#';
  UnicodeString URL = ::CutToChar(AURL, FragmentSeparator, false);

  if (URL.Pos(L"?") == 0)
  {
    URL += L"?";
  }
  else
  {
    URL += L"&";
  }

  int CurrentCompoundVer = Configuration->CompoundVersion;
  AnsiString Version = VersionStrFromCompoundVersion(CurrentCompoundVer);
  URL += FORMAT(L"utm_source=winscp&utm_medium=app&utm_campaign=%s", (Version));

  AddToList(URL, AURL, FragmentSeparator);

  return URL;
}
//---------------------------------------------------------------------------
UnicodeString __fastcall GetUsageData()
{
  return Configuration->Usage->Serialize();
}
//---------------------------------------------------------------------------
void __fastcall QueryUpdates()
{
  bool Complete = false;
  try
  {
    UnicodeString Response;

    TVSFixedFileInfo * FileInfo = Configuration->FixedApplicationInfo;
    int CurrentCompoundVer = Configuration->CompoundVersion;
    UnicodeString CurrentVersionStr =
      FORMAT(L"%d.%d.%d.%d",
        (HIWORD(FileInfo->dwFileVersionMS), LOWORD(FileInfo->dwFileVersionMS),
         HIWORD(FileInfo->dwFileVersionLS), LOWORD(FileInfo->dwFileVersionLS)));

    TUpdatesConfiguration Updates = WinConfiguration->Updates;
    THttp * CheckForUpdatesHTTP = new THttp(Application);
    try
    {
      UnicodeString URL = LoadStr(UPDATES_URL) +
        FORMAT(L"?v=%s&lang=%s", (CurrentVersionStr,
          IntToHex(__int64(GUIConfiguration->Locale), 4)));
      bool Beta;
      if (WinConfiguration->IsBeta)
      {
        Beta = true;
      }
      else
      {
        switch (Updates.BetaVersions)
        {
          case asAuto:
            Beta = WinConfiguration->AnyBetaInVersionHistory;
            break;

          case asOn:
            Beta = true;
            break;

          default:
            Beta = false;
            break;
        }
      }
      if (Beta)
      {
        URL += L"&beta=1";
      }
      URL += L"&dotnet=" + Updates.DotNetVersion;
      URL += L"&console=" + Updates.ConsoleVersion;
      UnicodeString Proxy;
      switch (Updates.ConnectionType)
      {
        case ctAuto:
          AutodetectProxyUrl(Proxy);
          break;

        case ctProxy:
          Proxy = FORMAT(L"%s:%d", (Updates.ProxyHost, Updates.ProxyPort));
          break;
      }
      CheckForUpdatesHTTP->Proxy = Proxy;
      CheckForUpdatesHTTP->URL = URL;
      if (Configuration->CollectUsage)
      {
        UTF8String UtfUsage = UTF8String(GetUsageData());
        CheckForUpdatesHTTP->Stream->Write(UtfUsage.c_str(), UtfUsage.Length());
        CheckForUpdatesHTTP->Post();
      }
      else
      {
        CheckForUpdatesHTTP->Action();
      }
      // sanity check
      if (CheckForUpdatesHTTP->Stream->Size > 102400)
      {
        Abort();
      }
      UTF8String UtfResponse;
      UtfResponse.SetLength(static_cast<int>(CheckForUpdatesHTTP->Stream->Size));
      CheckForUpdatesHTTP->Stream->Read(UtfResponse.c_str(), UtfResponse.Length());
      Response = UnicodeString(UtfResponse);
    }
    __finally
    {
      delete CheckForUpdatesHTTP;
    }

    bool Changed = !Updates.HaveResults;
    Updates.LastCheck = Now();
    Updates.HaveResults = true;
    TUpdatesData PrevResults = Updates.Results;
    Updates.Results.Reset();
    Updates.Results.ForVersion = CurrentCompoundVer;

    while (!Response.IsEmpty())
    {
      UnicodeString Line = ::CutToChar(Response, L'\n', false);
      UnicodeString Name = ::CutToChar(Line, L'=', false);
      if (AnsiSameText(Name, "Version"))
      {
        int MajorVer = StrToInt(::CutToChar(Line, L'.', false));
        int MinorVer = StrToInt(::CutToChar(Line, L'.', false));
        int Release = StrToInt(::CutToChar(Line, L'.', false));
        int Build = StrToInt(::CutToChar(Line, L'.', false));
        int NewVersion = CalculateCompoundVersion(MajorVer, MinorVer, Release, Build);
        Changed |= (NewVersion != PrevResults.Version);
        if (NewVersion <= CurrentCompoundVer)
        {
          NewVersion = 0;
        }
        Updates.Results.Version = NewVersion;
        Complete = true;
      }
      else if (AnsiSameText(Name, L"Message"))
      {
        Changed |= (PrevResults.Message != Line);
        Updates.Results.Message = Line;
      }
      else if (AnsiSameText(Name, L"Critical"))
      {
        bool NewCritical = (StrToIntDef(Line, 0) != 0);
        Changed |= (PrevResults.Critical != NewCritical);
        Updates.Results.Critical = NewCritical;
      }
      else if (AnsiSameText(Name, L"Release"))
      {
        Changed |= (PrevResults.Release != Line);
        Updates.Results.Release = Line;
      }
      else if (AnsiSameText(Name, L"Disabled"))
      {
        bool NewDisabled = (StrToIntDef(Line, 0) != 0);
        Changed |= (PrevResults.Disabled != NewDisabled);
        Updates.Results.Disabled = NewDisabled;
        Complete = true;
      }
      else if (AnsiSameText(Name, L"Url"))
      {
        Changed |= (PrevResults.Url != Line);
        Updates.Results.Url = Line;
      }
      else if (AnsiSameText(Name, L"UrlButton"))
      {
        Changed |= (PrevResults.UrlButton != Line);
        Updates.Results.UrlButton = Line;
      }
    }

    if (Changed)
    {
      Updates.ShownResults = false;
    }

    WinConfiguration->Updates = Updates;

    if (!Complete)
    {
      EXCEPTION;
    }

    Configuration->Usage->Reset();
    Configuration->Usage->Inc(L"UpdateChecksSucceeded");
  }
  catch(Exception & E)
  {
    Configuration->Usage->Inc(L"UpdateChecksFailed");
    throw ExtException(&E, MainInstructions(LoadStr(CHECK_FOR_UPDATES_ERROR)));
  }
}
//---------------------------------------------------------------------------
void __fastcall GetUpdatesMessage(UnicodeString & Message, bool & New,
  TQueryType & Type, bool Force)
{
  TUpdatesConfiguration Updates = WinConfiguration->Updates;
  assert(Updates.HaveResults);
  if (Updates.HaveResults)
  {
    if (Updates.Results.Disabled)
    {
      if (Force)
      {
        Message = LoadStr(UPDATE_DISABLED);
      }
    }
    else
    {
      New = (Updates.Results.Version > 0);
      if (New)
      {
        UnicodeString Version = VersionStrFromCompoundVersion(Updates.Results.Version);
        if (!Updates.Results.Release.IsEmpty())
        {
          Version = FORMAT(L"%s %s", (Version, Updates.Results.Release));
        }
        Message = FMTLOAD(NEW_VERSION4, (Version));
      }
      else
      {
        Message = LoadStr(NO_NEW_VERSION);
      }
    }

    if (!Message.IsEmpty())
    {
      Message = MainInstructions(Message);
    }

    if (!Updates.Results.Message.IsEmpty())
    {
      Message +=
        FMTLOAD(UPDATE_MESSAGE,
          (ReplaceStr(Updates.Results.Message, L"|", L"\n")));
    }
    Type = (Updates.Results.Critical ? qtWarning : qtInformation);
  }
  else
  {
    New = false;
  }
}
//---------------------------------------------------------------------------
static void __fastcall OpenHistory(void * /*Data*/, TObject * /*Sender*/)
{
  Configuration->Usage->Inc(L"UpdateHistoryOpens");
  OpenBrowser(LoadStr(HISTORY_URL));
}
//---------------------------------------------------------------------------
void __fastcall CheckForUpdates(bool CachedResults)
{
  TCustomForm * ActiveForm = Screen->ActiveCustomForm;

  TOperationVisualizer Visualizer;

  try
  {
    if (ActiveForm)
    {
      assert(ActiveForm->Enabled);
      ActiveForm->Enabled = false;
    }

    bool Again = false;
    do
    {
      TUpdatesConfiguration Updates = WinConfiguration->Updates;
      bool Cached = !Again && Updates.HaveResults &&
        (double(Updates.Period) > 0) &&
        (Updates.Results.ForVersion == Configuration->CompoundVersion) &&
        CachedResults;
      if (!Cached)
      {
        QueryUpdates();
        // reread new data
        Updates = WinConfiguration->Updates;
      }
      Again = false;

      if (!Updates.ShownResults)
      {
        Updates.ShownResults = true;
        WinConfiguration->Updates = Updates;
      }
      assert(Updates.HaveResults);

      UnicodeString Message;
      bool New;
      TQueryType Type;
      GetUpdatesMessage(Message, New, Type, true);

      Configuration->Usage->Inc(L"UpdateDisplays");
      if (New)
      {
        Configuration->Usage->Inc(L"UpdateDisplaysNew");
      }

      if (Updates.HaveResults)
      {
        bool ShowLast = Cached;
        bool ShowNext = (double(Updates.Period) > 0);

        if (ShowLast || ShowNext)
        {
          Message += L"\n";

          if (ShowLast)
          {
            Message += L"\n" +
              FMTLOAD(UPDATE_LAST,
                (FormatDateTime("ddddd", Updates.LastCheck)));
          }

          if (ShowNext)
          {
            Message += L"\n" +
              FMTLOAD(UPDATE_NEXT, (FormatDateTime("ddddd", Updates.LastCheck + Updates.Period)));
          }
        }
      }

      // add FLAGMASK(Cached, qaRetry) to enable "check again" button
      // for cached results
      int Answers = qaOK |
        FLAGMASK(New, qaCancel | qaAll) |
        FLAGMASK(!Updates.Results.Url.IsEmpty(), qaYes);
      TQueryButtonAlias Aliases[4];
      Aliases[0].Button = qaRetry;
      Aliases[0].Alias = LoadStr(CHECK_AGAIN_BUTTON);
      Aliases[1].Button = qaYes;
      if (Updates.Results.UrlButton.IsEmpty())
      {
        Aliases[1].Alias = LoadStr(UPDATE_URL_BUTTON);
      }
      else
      {
        Aliases[1].Alias = Updates.Results.UrlButton;
      }
      Aliases[2].Button = qaAll;
      Aliases[2].Alias = LoadStr(WHATS_NEW_BUTTON);
      Aliases[2].OnClick = MakeMethod<TNotifyEvent>(NULL, OpenHistory);
      Aliases[3].Button = qaOK;
      Aliases[3].Alias = LoadStr(DOWNLOAD_BUTTON);

      TMessageParams Params;
      Params.Aliases = Aliases;
      // alias "ok" button to "download" only if we have new version
      Params.AliasesCount = (New ? 4 : 3);
      unsigned int Answer =
        MessageDialog(Message, Type,
          Answers, HELP_UPDATES, &Params);
      switch (Answer)
      {
        case qaOK:
          if (New)
          {
            Configuration->Usage->Inc(L"UpdateDownloadOpens");
            OpenBrowser(LoadStr(DOWNLOAD_URL));
          }
          break;

        case qaYes:
          OpenBrowser(Updates.Results.Url);
          break;

        case qaAll:
          FAIL;
          break;

        case qaRetry:
          Again = true;
          break;
      }
    }
    while (Again);
  }
  __finally
  {
    if (ActiveForm)
    {
      ActiveForm->Enabled = true;
    }
  }
}
//---------------------------------------------------------------------------
class TUpdateThread : public TCompThread
{
public:
  __fastcall TUpdateThread(TThreadMethod OnUpdatesChecked);
protected:
  virtual void __fastcall Execute();
  TThreadMethod FOnUpdatesChecked;
};
//---------------------------------------------------------------------------
TUpdateThread * UpdateThread = NULL;
//---------------------------------------------------------------------------
__fastcall TUpdateThread::TUpdateThread(TThreadMethod OnUpdatesChecked) :
  TCompThread(false),
  FOnUpdatesChecked(OnUpdatesChecked)
{
}
//---------------------------------------------------------------------------
void __fastcall TUpdateThread::Execute()
{
  try
  {
    QueryUpdates();
    if (FOnUpdatesChecked != NULL)
    {
      Synchronize(FOnUpdatesChecked);
    }
  }
  catch(...)
  {
    // ignore errors
  }
}
//---------------------------------------------------------------------------
void __fastcall StartUpdateThread(TThreadMethod OnUpdatesChecked)
{
  assert(UpdateThread == NULL);
  UpdateThread = new TUpdateThread(OnUpdatesChecked);
}
//---------------------------------------------------------------------------
void __fastcall StopUpdateThread()
{
  if (UpdateThread != NULL)
  {
    SAFE_DESTROY(UpdateThread);
  }
}
//---------------------------------------------------------------------------
void __fastcall SetupInitialize()
{
  WinConfiguration->UpdateJumpList();
}
//---------------------------------------------------------------------------
static bool __fastcall AddJumpListCategory(TStrings * Names,
  UnicodeString AdditionalParams, TStringList * Removed,
  ICustomDestinationList * DestinationList, UnicodeString CategoryName,
  int IconIndex)
{
  bool Result = false;
  IObjectCollection * Collection = NULL;
  if (SUCCEEDED(CoCreateInstance(CLSID_EnumerableObjectCollection, NULL,
        CLSCTX_INPROC_SERVER, IID_IObjectCollection, (void**)&Collection)))
  {
    try
    {
      AddToList(AdditionalParams, TProgramParams::FormatSwitch(JUMPLIST_SWITCH), L" ");

      int Count = 0;
      for (int Index = 0; Index < Names->Count; Index++)
      {
        IShellLink * Link =
          CreateDesktopSessionShortCut(
            Names->Strings[Index], L"", AdditionalParams, -1, IconIndex, true);

        wchar_t Desc[2048];
        if (SUCCEEDED(Link->GetDescription(Desc, sizeof(Desc) - 1)))
        {
          if (Removed->IndexOf(Desc) < 0)
          {
            try
            {
              CHECK(SUCCEEDED(Collection->AddObject(Link)));
              Count++;
            }
            __finally
            {
              Link->Release();
            }
          }
          else
          {
            Names->Delete(Index);
            Index--;
          }
        }
      }

      if (Count > 0)
      {
        IObjectArray * Array;
        if (SUCCEEDED(Collection->QueryInterface(IID_IObjectArray, (void**)&Array)))
        {
          try
          {
            Result = SUCCEEDED(
              DestinationList->AppendCategory(CategoryName.c_str(), Array));
          }
          __finally
          {
            Array->Release();
          }
        }
      }
    }
    __finally
    {
      Collection->Release();
    }
  }
  return Result;
}
//---------------------------------------------------------------------------
void __fastcall UpdateJumpList(TStrings * SessionNames, TStrings * WorkspaceNames)
{
  ICustomDestinationList * DestinationList = NULL;
  IObjectArray * RemovedArray = NULL;
  TStringList * Removed = NULL;
  int OldErrMode = SetErrorMode(SEM_FAILCRITICALERRORS);

  try
  {
    if (SUCCEEDED(CoCreateInstance(CLSID_DestinationList, NULL,
          CLSCTX_INPROC_SERVER, IID_ICustomDestinationList, (void**)&DestinationList)))
    {

      unsigned int MinSlots;
      unsigned int * PMinSlots = &MinSlots;
      void ** PRemovedArray = (void**)&RemovedArray;
      HRESULT Result = DestinationList->BeginList(PMinSlots, IID_IObjectArray, PRemovedArray);
      if (SUCCEEDED(Result) && ALWAYS_TRUE(RemovedArray != NULL))
      {
        Removed = new TStringList();

        unsigned int RemovedCount;
        if (FAILED(RemovedArray->GetCount(&RemovedCount)))
        {
          RemovedCount = 0;
        }

        for (unsigned int Index = 0; Index < RemovedCount; Index++)
        {
          IShellLink * Link;
          wchar_t Desc[2048];
          if (SUCCEEDED(RemovedArray->GetAt(Index, IID_IShellLink, (void**)&Link)) &&
              SUCCEEDED(Link->GetDescription(Desc, sizeof(Desc) - 1)))
          {
            Removed->Add(Desc);
          }
        }

        AddJumpListCategory(
          WorkspaceNames, L"", Removed, DestinationList,
          LoadStr(JUMPLIST_WORKSPACES), WORKSPACE_ICON);

        AddJumpListCategory(
          SessionNames, TProgramParams::FormatSwitch(UPLOAD_IF_ANY_SWITCH), Removed, DestinationList,
          LoadStr(JUMPLIST_RECENT), SITE_ICON);

        if (DestinationList != NULL)
        {
          DestinationList->CommitList();
        }
      }
    }
  }
  __finally
  {
    SetErrorMode(OldErrMode);
    if (RemovedArray != NULL)
    {
      RemovedArray->Release();
    }
    if (DestinationList != NULL)
    {
      DestinationList->Release();
    }
    delete Removed;
  }
}
//---------------------------------------------------------------------------
bool __fastcall AnyOtherInstanceOfSelf()
{

  HANDLE Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

  bool Result = false;

  try
  {
    unsigned int Process = GetCurrentProcessId();
    UnicodeString ExeBaseName = ExtractFileBaseName(Application->ExeName);

    PROCESSENTRY32 ProcessEntry;
    ProcessEntry.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(Snapshot, &ProcessEntry))
    {
      while (!Result && Process32Next(Snapshot, &ProcessEntry))
      {
        // we should check if the process is running in the same session,
        // but for that we probably need some special priviledges
        if ((Process != ProcessEntry.th32ProcessID) &&
            SameText(ExtractFileBaseName(ProcessEntry.szExeFile), ExeBaseName))
        {
          Result = true;
        }
      }
    }
  }
  __finally
  {
    CloseHandle(Snapshot);
  }

  return Result;
}
