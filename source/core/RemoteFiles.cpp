//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "Common.h"
#include "RemoteFiles.h"

#include <SysUtils.hpp>
#include <StrUtils.hpp>

#include "Exceptions.h"
#include "Interface.h"
#include "Terminal.h"
#include "TextsCore.h"
#include "HelpCore.h"
/* TODO 1 : Path class instead of UnicodeString (handle relativity...) */
//---------------------------------------------------------------------------
bool __fastcall IsUnixStyleWindowsPath(const UnicodeString & Path)
{
  return (Path.Length() >= 3) && IsLetter(Path[1]) && (Path[2] == L':') && (Path[3] == L'/');
}
//---------------------------------------------------------------------------
bool __fastcall UnixIsAbsolutePath(const UnicodeString & Path)
{
  return
    ((Path.Length() >= 1) && (Path[1] == L'/')) ||
    // we need this for FTP only, but this is unfortunately used in a static context
    IsUnixStyleWindowsPath(Path);
}
//---------------------------------------------------------------------------
UnicodeString __fastcall UnixIncludeTrailingBackslash(const UnicodeString Path)
{
  // it used to return "/" when input path was empty
  if (!Path.IsEmpty() && !Path.IsDelimiter(L"/", Path.Length()))
  {
    return Path + L"/";
  }
  else
  {
    return Path;
  }
}
//---------------------------------------------------------------------------
// Keeps "/" for root path
UnicodeString __fastcall UnixExcludeTrailingBackslash(const UnicodeString Path, bool Simple)
{
  if (Path.IsEmpty() ||
      (Path == L"/") ||
      !Path.IsDelimiter(L"/", Path.Length()) ||
      (!Simple && ((Path.Length() == 3) && IsUnixStyleWindowsPath(Path))))
  {
    return Path;
  }
  else
  {
    return Path.SubString(1, Path.Length() - 1);
  }
}
//---------------------------------------------------------------------------
UnicodeString __fastcall SimpleUnixExcludeTrailingBackslash(const UnicodeString Path)
{
  return UnixExcludeTrailingBackslash(Path, true);
}
//---------------------------------------------------------------------------
Boolean __fastcall UnixComparePaths(const UnicodeString Path1, const UnicodeString Path2)
{
  return (UnixIncludeTrailingBackslash(Path1) == UnixIncludeTrailingBackslash(Path2));
}
//---------------------------------------------------------------------------
bool __fastcall UnixIsChildPath(UnicodeString Parent, UnicodeString Child)
{
  Parent = UnixIncludeTrailingBackslash(Parent);
  Child = UnixIncludeTrailingBackslash(Child);
  return (Child.SubString(1, Parent.Length()) == Parent);
}
//---------------------------------------------------------------------------
UnicodeString __fastcall UnixExtractFileDir(const UnicodeString Path)
{
  int Pos = Path.LastDelimiter(L'/');
  // it used to return Path when no slash was found
  if (Pos > 1)
  {
    return Path.SubString(1, Pos - 1);
  }
  else
  {
    return (Pos == 1) ? UnicodeString(L"/") : UnicodeString();
  }
}
//---------------------------------------------------------------------------
// must return trailing backslash
UnicodeString __fastcall UnixExtractFilePath(const UnicodeString Path)
{
  int Pos = Path.LastDelimiter(L'/');
  // it used to return Path when no slash was found
  return (Pos > 0) ? Path.SubString(1, Pos) : UnicodeString();
}
//---------------------------------------------------------------------------
UnicodeString __fastcall UnixExtractFileName(const UnicodeString Path)
{
  int Pos = Path.LastDelimiter(L'/');
  UnicodeString Result;
  if (Pos > 0)
  {
    Result = Path.SubString(Pos + 1, Path.Length() - Pos);
  }
  else
  {
    Result = Path;
  }
  return Result;
}
//---------------------------------------------------------------------------
UnicodeString __fastcall UnixExtractFileExt(const UnicodeString Path)
{
  UnicodeString FileName = UnixExtractFileName(Path);
  int Pos = FileName.LastDelimiter(L".");
  return (Pos > 0) ? Path.SubString(Pos, Path.Length() - Pos + 1) : UnicodeString();
}
//---------------------------------------------------------------------------
UnicodeString __fastcall ExtractFileName(const UnicodeString & Path, bool Unix)
{
  if (Unix)
  {
    return UnixExtractFileName(Path);
  }
  else
  {
    return ExtractFileName(Path);
  }
}
//---------------------------------------------------------------------------
bool __fastcall ExtractCommonPath(TStrings * Files, UnicodeString & Path)
{
  assert(Files->Count > 0);

  Path = ExtractFilePath(Files->Strings[0]);
  bool Result = !Path.IsEmpty();
  if (Result)
  {
    for (int Index = 1; Index < Files->Count; Index++)
    {
      while (!Path.IsEmpty() &&
        (Files->Strings[Index].SubString(1, Path.Length()) != Path))
      {
        int PrevLen = Path.Length();
        Path = ExtractFilePath(ExcludeTrailingBackslash(Path));
        if (Path.Length() == PrevLen)
        {
          Path = L"";
          Result = false;
        }
      }
    }
  }

  return Result;
}
//---------------------------------------------------------------------------
bool __fastcall UnixExtractCommonPath(TStrings * Files, UnicodeString & Path)
{
  assert(Files->Count > 0);

  Path = UnixExtractFilePath(Files->Strings[0]);
  bool Result = !Path.IsEmpty();
  if (Result)
  {
    for (int Index = 1; Index < Files->Count; Index++)
    {
      while (!Path.IsEmpty() &&
        (Files->Strings[Index].SubString(1, Path.Length()) != Path))
      {
        int PrevLen = Path.Length();
        Path = UnixExtractFilePath(UnixExcludeTrailingBackslash(Path));
        if (Path.Length() == PrevLen)
        {
          Path = L"";
          Result = false;
        }
      }
    }
  }

  return Result;
}
//---------------------------------------------------------------------------
bool __fastcall IsUnixRootPath(const UnicodeString Path)
{
  return Path.IsEmpty() || (Path == ROOTDIRECTORY);
}
//---------------------------------------------------------------------------
bool __fastcall IsUnixHiddenFile(const UnicodeString FileName)
{
  return (FileName != ROOTDIRECTORY) && (FileName != PARENTDIRECTORY) &&
    !FileName.IsEmpty() && (FileName[1] == L'.');
}
//---------------------------------------------------------------------------
UnicodeString __fastcall AbsolutePath(const UnicodeString & Base, const UnicodeString & Path)
{
  // There's a duplicate implementation in TTerminal::ExpandFileName()
  UnicodeString Result;
  if (Path.IsEmpty())
  {
    Result = Base;
  }
  else if (Path[1] == L'/')
  {
    Result = UnixExcludeTrailingBackslash(Path);
  }
  else
  {
    Result = UnixIncludeTrailingBackslash(
      UnixIncludeTrailingBackslash(Base) + Path);
    int P;
    while ((P = Result.Pos(L"/../")) > 0)
    {
      int P2 = Result.SubString(1, P-1).LastDelimiter(L"/");
      assert(P2 > 0);
      Result.Delete(P2, P - P2 + 3);
    }
    while ((P = Result.Pos(L"/./")) > 0)
    {
      Result.Delete(P, 2);
    }
    Result = UnixExcludeTrailingBackslash(Result);
  }
  return Result;
}
//---------------------------------------------------------------------------
UnicodeString __fastcall FromUnixPath(const UnicodeString Path)
{
  return ReplaceStr(Path, L"/", L"\\");
}
//---------------------------------------------------------------------------
UnicodeString __fastcall ToUnixPath(const UnicodeString Path)
{
  return ReplaceStr(Path, L"\\", L"/");
}
//---------------------------------------------------------------------------
static void __fastcall CutFirstDirectory(UnicodeString & S, bool Unix)
{
  bool Root;
  int P;
  UnicodeString Sep = Unix ? L"/" : L"\\";
  if (S == Sep)
  {
    S = L"";
  }
  else
  {
    if (S[1] == Sep[1])
    {
      Root = true;
      S.Delete(1, 1);
    }
    else
    {
      Root = false;
    }
    if (S[1] == L'.')
    {
      S.Delete(1, 4);
    }
    P = S.Pos(Sep[1]);
    if (P)
    {
      S.Delete(1, P);
      S = L"..." + Sep + S;
    }
    else
    {
      S = L"";
    }
    if (Root)
    {
      S = Sep + S;
    }
  }
}
//---------------------------------------------------------------------------
UnicodeString __fastcall MinimizeName(const UnicodeString FileName, int MaxLen, bool Unix)
{
  UnicodeString Drive, Dir, Name, Result;
  UnicodeString Sep = Unix ? L"/" : L"\\";

  Result = FileName;
  if (Unix)
  {
    int P = Result.LastDelimiter(L"/");
    if (P)
    {
      Dir = Result.SubString(1, P);
      Name = Result.SubString(P + 1, Result.Length() - P);
    }
    else
    {
      Dir = L"";
      Name = Result;
    }
  }
  else
  {
    Dir = ExtractFilePath(Result);
    Name = ExtractFileName(Result);

    if (Dir.Length() >= 2 && Dir[2] == L':')
    {
      Drive = Dir.SubString(1, 2);
      Dir.Delete(1, 2);
    }
  }

  while ((!Dir.IsEmpty() || !Drive.IsEmpty()) && (Result.Length() > MaxLen))
  {
    if (Dir == Sep + L"..." + Sep)
    {
      Dir = L"..." + Sep;
    }
    else if (Dir == L"")
    {
      Drive = L"";
    }
    else
    {
      CutFirstDirectory(Dir, Unix);
    }
    Result = Drive + Dir + Name;
  }

  if (Result.Length() > MaxLen)
  {
    Result = Result.SubString(1, MaxLen);
  }
  return Result;
}
//---------------------------------------------------------------------------
UnicodeString __fastcall MakeFileList(TStrings * FileList)
{
  UnicodeString Result;
  for (int Index = 0; Index < FileList->Count; Index++)
  {
    if (!Result.IsEmpty())
    {
      Result += L" ";
    }

    UnicodeString FileName = FileList->Strings[Index];
    // currently this is used for local file only, so no delimiting is done
    if (FileName.Pos(L" ") > 0)
    {
      Result += L"\"" + FileName + L"\"";
    }
    else
    {
      Result += FileName;
    }
  }
  return Result;
}
//---------------------------------------------------------------------------
// copy from BaseUtils.pas
TDateTime __fastcall ReduceDateTimePrecision(TDateTime DateTime,
  TModificationFmt Precision)
{
  if (Precision == mfNone)
  {
    DateTime = double(0);
  }
  else if (Precision != mfFull)
  {
    unsigned short Y, M, D, H, N, S, MS;

    DecodeDate(DateTime, Y, M, D);
    DecodeTime(DateTime, H, N, S, MS);
    switch (Precision)
    {
      case mfMDHM:
        S = 0;
        MS = 0;
        break;

      case mfMDY:
        H = 0;
        N = 0;
        S = 0;
        MS = 0;
        break;

      default:
        assert(false);
    }

    DateTime = EncodeDateVerbose(Y, M, D) + EncodeTimeVerbose(H, N, S, MS);
  }
  return DateTime;
}
//---------------------------------------------------------------------------
TModificationFmt __fastcall LessDateTimePrecision(
  TModificationFmt Precision1, TModificationFmt Precision2)
{
  return (Precision1 < Precision2) ? Precision1 : Precision2;
}
//---------------------------------------------------------------------------
UnicodeString __fastcall UserModificationStr(TDateTime DateTime,
  TModificationFmt Precision)
{
  switch (Precision)
  {
    case mfNone:
      return L"";
    case mfMDY:
      return FormatDateTime(L"ddddd", DateTime);
    case mfMDHM:
      return FormatDateTime(L"ddddd t", DateTime);
    case mfFull:
    default:
      return FormatDateTime(L"ddddd tt", DateTime);
  }
}
//---------------------------------------------------------------------------
UnicodeString __fastcall ModificationStr(TDateTime DateTime,
  TModificationFmt Precision)
{
  Word Year, Month, Day, Hour, Min, Sec, MSec;
  DateTime.DecodeDate(&Year, &Month, &Day);
  DateTime.DecodeTime(&Hour, &Min, &Sec, &MSec);
  switch (Precision)
  {
    case mfNone:
      return L"";

    case mfMDY:
      return FORMAT(L"%3s %2d %2d", (EngShortMonthNames[Month-1], Day, Year));

    case mfMDHM:
      return FORMAT(L"%3s %2d %2d:%2.2d",
        (EngShortMonthNames[Month-1], Day, Hour, Min));

    default:
      assert(false);
      // fall thru

    case mfFull:
      return FORMAT(L"%3s %2d %2d:%2.2d:%2.2d %4d",
        (EngShortMonthNames[Month-1], Day, Hour, Min, Sec, Year));
  }
}
//---------------------------------------------------------------------------
int __fastcall FakeFileImageIndex(UnicodeString FileName, unsigned long Attrs,
  UnicodeString * TypeName)
{
  Attrs |= FILE_ATTRIBUTE_NORMAL;

  TSHFileInfoW SHFileInfo;
  // On Win2k we get icon of "ZIP drive" for ".." (parent directory)
  if ((FileName == L"..") ||
      ((FileName.Length() == 2) && (FileName[2] == L':') && IsLetter(FileName[1])) ||
      IsReservedName(FileName))
  {
    FileName = L"dumb";
  }
  // this should be somewhere else, probably in TUnixDirView,
  // as the "partial" overlay is added there too
  if (AnsiSameText(UnixExtractFileExt(FileName), PARTIAL_EXT))
  {
    static const size_t PartialExtLen = LENOF(PARTIAL_EXT) - 1;
    FileName.SetLength(FileName.Length() - PartialExtLen);
  }

  int Icon;
  if (SHGetFileInfo(UnicodeString(FileName).c_str(),
        Attrs, &SHFileInfo, sizeof(SHFileInfo),
        SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES | SHGFI_TYPENAME) != 0)
  {
    if (TypeName != NULL)
    {
      *TypeName = SHFileInfo.szTypeName;
    }
    Icon = SHFileInfo.iIcon;
  }
  else
  {
    if (TypeName != NULL)
    {
      *TypeName = L"";
    }
    Icon = -1;
  }

  return Icon;
}
//---------------------------------------------------------------------------
__fastcall TRemoteToken::TRemoteToken() :
  FID(0),
  FIDValid(false)
{
}
//---------------------------------------------------------------------------
__fastcall TRemoteToken::TRemoteToken(const UnicodeString & Name) :
  FName(Name),
  FID(0),
  FIDValid(false)
{
}
//---------------------------------------------------------------------------
void __fastcall TRemoteToken::Clear()
{
  FID = 0;
  FIDValid = false;
}
//---------------------------------------------------------------------------
bool __fastcall TRemoteToken::operator ==(const TRemoteToken & rht) const
{
  return
    (FName == rht.FName) &&
    (FIDValid == rht.FIDValid) &&
    (!FIDValid || (FID == rht.FID));
}
//---------------------------------------------------------------------------
bool __fastcall TRemoteToken::operator !=(const TRemoteToken & rht) const
{
  return !(*this == rht);
}
//---------------------------------------------------------------------------
TRemoteToken & __fastcall TRemoteToken::operator =(const TRemoteToken & rht)
{
  if (this != &rht)
  {
    FName = rht.FName;
    FIDValid = rht.FIDValid;
    FID = rht.FID;
  }
  return *this;
}
//---------------------------------------------------------------------------
int __fastcall TRemoteToken::Compare(const TRemoteToken & rht) const
{
  int Result;
  if (!FName.IsEmpty())
  {
    if (!rht.FName.IsEmpty())
    {
      Result = AnsiCompareText(FName, rht.FName);
    }
    else
    {
      Result = -1;
    }
  }
  else
  {
    if (!rht.FName.IsEmpty())
    {
      Result = 1;
    }
    else
    {
      if (FIDValid)
      {
        if (rht.FIDValid)
        {
          Result = (FID < rht.FID) ? -1 : ((FID > rht.FID) ? 1 : 0);
        }
        else
        {
          Result = -1;
        }
      }
      else
      {
        if (rht.FIDValid)
        {
          Result = 1;
        }
        else
        {
          Result = 0;
        }
      }
    }
  }
  return Result;
}
//---------------------------------------------------------------------------
void __fastcall TRemoteToken::SetID(unsigned int value)
{
  FID = value;
  FIDValid = true;
}
//---------------------------------------------------------------------------
bool __fastcall TRemoteToken::GetNameValid() const
{
  return !FName.IsEmpty();
}
//---------------------------------------------------------------------------
bool __fastcall TRemoteToken::GetIsSet() const
{
  return !FName.IsEmpty() || FIDValid;
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TRemoteToken::GetDisplayText() const
{
  if (!FName.IsEmpty())
  {
    return FName;
  }
  else if (FIDValid)
  {
    return IntToStr(int(FID));
  }
  else
  {
    return UnicodeString();
  }
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TRemoteToken::GetLogText() const
{
  return FORMAT(L"\"%s\" [%d]", (FName, int(FID)));
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
TRemoteTokenList * __fastcall TRemoteTokenList::Duplicate() const
{
  TRemoteTokenList * Result = new TRemoteTokenList();
  try
  {
    TTokens::const_iterator I = FTokens.begin();
    while (I != FTokens.end())
    {
      Result->Add(*I);
      ++I;
    }
  }
  catch(...)
  {
    delete Result;
    throw;
  }
  return Result;
}
//---------------------------------------------------------------------------
void __fastcall TRemoteTokenList::Clear()
{
  FTokens.clear();
  FNameMap.clear();
  FIDMap.clear();
}
//---------------------------------------------------------------------------
void __fastcall TRemoteTokenList::Add(const TRemoteToken & Token)
{
  FTokens.push_back(Token);
  if (Token.IDValid)
  {
    std::pair<TIDMap::iterator, bool> Position =
      FIDMap.insert(TIDMap::value_type(Token.ID, FTokens.size() - 1));
  }
  if (Token.NameValid)
  {
    std::pair<TNameMap::iterator, bool> Position =
      FNameMap.insert(TNameMap::value_type(Token.Name, FTokens.size() - 1));
  }
}
//---------------------------------------------------------------------------
void __fastcall TRemoteTokenList::AddUnique(const TRemoteToken & Token)
{
  if (Token.IDValid)
  {
    TIDMap::const_iterator I = FIDMap.find(Token.ID);
    if (I != FIDMap.end())
    {
      // is present already.
      // may have different name (should not),
      // but what can we do about it anyway?
    }
    else
    {
      Add(Token);
    }
  }
  else if (Token.NameValid)
  {
    TNameMap::const_iterator I = FNameMap.find(Token.Name);
    if (I != FNameMap.end())
    {
      // is present already.
    }
    else
    {
      Add(Token);
    }
  }
  else
  {
    // can happen, e.g. with winsshd/SFTP
  }
}
//---------------------------------------------------------------------------
bool __fastcall TRemoteTokenList::Exists(const UnicodeString & Name) const
{
  return (FNameMap.find(Name) != FNameMap.end());
}
//---------------------------------------------------------------------------
const TRemoteToken * TRemoteTokenList::Find(unsigned int ID) const
{
  TIDMap::const_iterator I = FIDMap.find(ID);
  const TRemoteToken * Result;
  if (I != FIDMap.end())
  {
    Result = &FTokens[(*I).second];
  }
  else
  {
    Result = NULL;
  }
  return Result;
}
//---------------------------------------------------------------------------
const TRemoteToken * TRemoteTokenList::Find(const UnicodeString & Name) const
{
  TNameMap::const_iterator I = FNameMap.find(Name);
  const TRemoteToken * Result;
  if (I != FNameMap.end())
  {
    Result = &FTokens[(*I).second];
  }
  else
  {
    Result = NULL;
  }
  return Result;
}
//---------------------------------------------------------------------------
void __fastcall TRemoteTokenList::Log(TTerminal * Terminal, const wchar_t * Title)
{
  if (!FTokens.empty())
  {
    Terminal->LogEvent(FORMAT(L"Following %s found:", (Title)));
    for (size_t Index = 0; Index < FTokens.size(); Index++)
    {
      Terminal->LogEvent(UnicodeString(L"  ") + FTokens[Index].LogText);
    }
  }
  else
  {
    Terminal->LogEvent(FORMAT(L"No %s found.", (Title)));
  }
}
//---------------------------------------------------------------------------
int __fastcall TRemoteTokenList::Count() const
{
  return (int)FTokens.size();
}
//---------------------------------------------------------------------------
const TRemoteToken * __fastcall TRemoteTokenList::Token(int Index) const
{
  return &FTokens[Index];
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
__fastcall TRemoteFile::TRemoteFile(TRemoteFile * ALinkedByFile):
  TPersistent()
{
  FLinkedFile = NULL;
  FRights = new TRights();
  FIconIndex = -1;
  FCyclicLink = false;
  FModificationFmt = mfFull;
  FLinkedByFile = ALinkedByFile;
  FTerminal = NULL;
  FDirectory = NULL;
  FIsHidden = -1;
}
//---------------------------------------------------------------------------
__fastcall TRemoteFile::~TRemoteFile()
{
  delete FRights;
  delete FLinkedFile;
}
//---------------------------------------------------------------------------
TRemoteFile * __fastcall TRemoteFile::Duplicate(bool Standalone) const
{
  TRemoteFile * Result;
  Result = new TRemoteFile();
  try
  {
    if (FLinkedFile)
    {
      Result->FLinkedFile = FLinkedFile->Duplicate(true);
      Result->FLinkedFile->FLinkedByFile = Result;
    }
    *Result->Rights = *FRights;
    #define COPY_FP(PROP) Result->F ## PROP = F ## PROP;
    COPY_FP(Terminal);
    COPY_FP(Owner);
    COPY_FP(ModificationFmt);
    COPY_FP(Size);
    COPY_FP(FileName);
    COPY_FP(INodeBlocks);
    COPY_FP(Modification);
    COPY_FP(LastAccess);
    COPY_FP(Group);
    COPY_FP(IconIndex);
    COPY_FP(TypeName);
    COPY_FP(IsSymLink);
    COPY_FP(LinkTo);
    COPY_FP(Type);
    COPY_FP(Selected);
    COPY_FP(CyclicLink);
    #undef COPY_FP
    if (Standalone && (!FFullFileName.IsEmpty() || (Directory != NULL)))
    {
      Result->FFullFileName = FullFileName;
    }
  }
  catch(...)
  {
    delete Result;
    throw;
  }
  return Result;
}
//---------------------------------------------------------------------------
void __fastcall TRemoteFile::LoadTypeInfo()
{
  /* TODO : If file is link: Should be attributes taken from linked file? */
  unsigned long Attrs = 0;
  if (IsDirectory) Attrs |= FILE_ATTRIBUTE_DIRECTORY;
  if (IsHidden) Attrs |= FILE_ATTRIBUTE_HIDDEN;

  UnicodeString DumbFileName = (IsSymLink && !LinkTo.IsEmpty() ? LinkTo : FileName);

  FIconIndex = FakeFileImageIndex(DumbFileName, Attrs, &FTypeName);
}
//---------------------------------------------------------------------------
Integer __fastcall TRemoteFile::GetIconIndex() const
{
  if (FIconIndex == -1)
  {
    const_cast<TRemoteFile *>(this)->LoadTypeInfo();
  }
  return FIconIndex;
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TRemoteFile::GetTypeName()
{
  // check availability of type info by icon index, because type name can be empty
  if (FIconIndex < 0)
  {
    LoadTypeInfo();
  }
  return FTypeName;
}
//---------------------------------------------------------------------------
Boolean __fastcall TRemoteFile::GetIsHidden()
{
  bool Result;
  switch (FIsHidden)
  {
    case 0:
      Result = false;
      break;

    case 1:
      Result = true;
      break;

    default:
      Result = IsUnixHiddenFile(FileName);
      break;
  }

  return Result;
}
//---------------------------------------------------------------------------
void __fastcall TRemoteFile::SetIsHidden(bool value)
{
  FIsHidden = value ? 1 : 0;
}
//---------------------------------------------------------------------------
Boolean __fastcall TRemoteFile::GetIsDirectory() const
{
  return (toupper(Type) == FILETYPE_DIRECTORY);
}
//---------------------------------------------------------------------------
Boolean __fastcall TRemoteFile::GetIsParentDirectory() const
{
  return (FileName == PARENTDIRECTORY);
}
//---------------------------------------------------------------------------
Boolean __fastcall TRemoteFile::GetIsThisDirectory() const
{
  return (FileName == THISDIRECTORY);
}
//---------------------------------------------------------------------------
Boolean __fastcall TRemoteFile::GetIsInaccesibleDirectory() const
{
  Boolean Result;
  if (IsDirectory)
  {
    assert(Terminal);
    Result = !
       (((Rights->RightUndef[TRights::rrOtherExec] != TRights::rsNo)) ||
        ((Rights->Right[TRights::rrGroupExec] != TRights::rsNo) &&
         Terminal->Membership->Exists(Group.Name)) ||
        ((Rights->Right[TRights::rrUserExec] != TRights::rsNo) &&
         (AnsiCompareText(Terminal->UserName, Owner.Name) == 0)));
  }
    else Result = False;
  return Result;
}
//---------------------------------------------------------------------------
wchar_t __fastcall TRemoteFile::GetType() const
{
  if (IsSymLink && FLinkedFile) return FLinkedFile->Type;
    else return FType;
}
//---------------------------------------------------------------------------
void __fastcall TRemoteFile::SetType(wchar_t AType)
{
  FType = AType;
  FIsSymLink = ((wchar_t)towupper(FType) == FILETYPE_SYMLINK);
}
//---------------------------------------------------------------------------
TRemoteFile * __fastcall TRemoteFile::GetLinkedFile()
{
  // do not call FindLinkedFile as it would be called repeatedly for broken symlinks
  return FLinkedFile;
}
//---------------------------------------------------------------------------
void __fastcall TRemoteFile::SetLinkedFile(TRemoteFile * value)
{
  if (FLinkedFile != value)
  {
    if (FLinkedFile) delete FLinkedFile;
    FLinkedFile = value;
  }
}
//---------------------------------------------------------------------------
bool __fastcall TRemoteFile::GetBrokenLink()
{
  assert(Terminal);
  // If file is symlink but we couldn't find linked file we assume broken link
  return (IsSymLink && (FCyclicLink || !FLinkedFile) &&
    Terminal->ResolvingSymlinks);
  // "!FLinkTo.IsEmpty()" removed because it does not work with SFTP
}
//---------------------------------------------------------------------------
void __fastcall TRemoteFile::ShiftTime(const TDateTime & Difference)
{
  double D = double(Difference);
  if ((D != 0) && (FModificationFmt != mfMDY))
  {
    assert(int(FModification) != 0);
    FModification = double(FModification) + D;
    assert(int(FLastAccess) != 0);
    FLastAccess = double(FLastAccess) + D;
  }
}
//---------------------------------------------------------------------------
void __fastcall TRemoteFile::SetModification(const TDateTime & value)
{
  if (FModification != value)
  {
    FModificationFmt = mfFull;
    FModification = value;
  }
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TRemoteFile::GetUserModificationStr()
{
  return ::UserModificationStr(Modification, FModificationFmt);
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TRemoteFile::GetModificationStr()
{
  return ::ModificationStr(Modification, FModificationFmt);
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TRemoteFile::GetExtension()
{
  return UnixExtractFileExt(FFileName);
}
//---------------------------------------------------------------------------
void __fastcall TRemoteFile::SetRights(TRights * value)
{
  FRights->Assign(value);
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TRemoteFile::GetRightsStr()
{
  return FRights->Unknown ? UnicodeString() : FRights->Text;
}
//---------------------------------------------------------------------------
void __fastcall TRemoteFile::SetListingStr(UnicodeString value)
{
  // Value stored in 'value' can be used for error message
  UnicodeString Line = value;
  FIconIndex = -1;
  try
  {
    UnicodeString Col;

    // Do we need to do this (is ever TAB is LS output)?
    Line = ReplaceChar(Line, L'\t', L' ');

    Type = Line[1];
    Line.Delete(1, 1);

    #define GETNCOL  \
      { if (Line.IsEmpty()) throw Exception(L""); \
        Integer P = Line.Pos(L' '); \
        if (P) { Col = Line.SubString(1, P-1); Line.Delete(1, P); } \
          else { Col = Line; Line = L""; } \
      }
    #define GETCOL { GETNCOL; Line = TrimLeft(Line); }

    // Rights string may contain special permission attributes (S,t, ...)
    // (TODO: maybe no longer necessary, once we can handle the special permissions)
    Rights->AllowUndef = True;
    // On some system there is no space between permissions and node blocks count columns
    // so we get only first 9 characters and trim all following spaces (if any)
    Rights->Text = Line.SubString(1, 9);
    Line.Delete(1, 9);
    // Rights column maybe followed by '+', '@' or '.' signs, we ignore them
    // (On MacOS, there may be a space in between)
    if (!Line.IsEmpty() && ((Line[1] == L'+') || (Line[1] == L'@') || (Line[1] == L'.')))
    {
      Line.Delete(1, 1);
    }
    else if ((Line.Length() >= 2) && (Line[1] == L' ') &&
             ((Line[2] == L'+') || (Line[2] == L'@') || (Line[2] == L'.')))
    {
      Line.Delete(1, 2);
    }
    Line = Line.TrimLeft();

    GETCOL;
    if (!TryStrToInt(Col, FINodeBlocks))
    {
      // if the column is not an integer, suppose it's owner
      // (Android BusyBox)
      FINodeBlocks = 0;
    }
    else
    {
      GETCOL;
    }

    FOwner.Name = Col;

    // #60 17.10.01: group name can contain space
    FGroup.Name = L"";
    GETCOL;
    __int64 ASize;
    do
    {
      FGroup.Name = FGroup.Name + Col;
      GETCOL;
      assert(!Col.IsEmpty());
      // for devices etc.. there is additional column ending by comma, we ignore it
      if (Col[Col.Length()] == L',') GETCOL;
      ASize = StrToInt64Def(Col, -1);
      // if it's not a number (file size) we take it as part of group name
      // (at least on CygWin, there can be group with space in its name)
      if (ASize < 0) Col = L" " + Col;
    }
    while (ASize < 0);

    // do not read modification time and filename if it is already set
    if (double(FModification) == 0 && FileName.IsEmpty())
    {
      FSize = ASize;

      bool FullTime = false;
      bool DayMonthFormat = false;
      Word Day, Month, Year, Hour, Min, Sec, P;

      GETCOL;
      // format dd mmm or mmm dd ?
      Day = (Word)StrToIntDef(Col, 0);
      if (Day > 0)
      {
        DayMonthFormat = true;
        GETCOL;
      }
      Month = 0;
      #define COL2MONTH \
        for (Word IMonth = 0; IMonth < 12; IMonth++) \
          if (!Col.CompareIC(EngShortMonthNames[IMonth])) { Month = IMonth; Month++; break; }
      COL2MONTH;
      // if the column is not known month name, it may have been "yyyy-mm-dd"
      // for --full-time format
      if ((Month == 0) && (Col.Length() == 10) && (Col[5] == L'-') && (Col[8] == L'-'))
      {
        Year = (Word)Col.SubString(1, 4).ToInt();
        Month = (Word)Col.SubString(6, 2).ToInt();
        Day = (Word)Col.SubString(9, 2).ToInt();
        GETCOL;
        Hour = (Word)Col.SubString(1, 2).ToInt();
        Min = (Word)Col.SubString(4, 2).ToInt();
        if (Col.Length() >= 8)
        {
          Sec = (Word)StrToInt(Col.SubString(7, 2));
        }
        else
        {
          Sec = 0;
        }
        FModificationFmt = mfFull;
        // skip TZ (TODO)
        // do not trim leading space of filename
        GETNCOL;
      }
      else
      {
        // or it may have been day name for another format of --full-time
        if (Month == 0)
        {
          GETCOL;
          COL2MONTH;
          // neither standard, not --full-time format
          if (Month == 0)
          {
            Abort();
          }
          else
          {
            FullTime = true;
          }
        }
        #undef COL2MONTH

        if (Day == 0)
        {
          GETNCOL;
          Day = (Word)StrToInt(Col);
        }
        if ((Day < 1) || (Day > 31)) Abort();

        // second full-time format
        // ddd mmm dd hh:nn:ss yyyy
        if (FullTime)
        {
          GETCOL;
          if (Col.Length() != 8)
          {
            Abort();
          }
          Hour = (Word)StrToInt(Col.SubString(1, 2));
          Min = (Word)StrToInt(Col.SubString(4, 2));
          Sec = (Word)StrToInt(Col.SubString(7, 2));
          FModificationFmt = mfFull;
          // do not trim leading space of filename
          GETNCOL;
          Year = (Word)StrToInt(Col);
        }
        else
        {
          // for format dd mmm the below description seems not to be true,
          // the year is not aligned to 5 characters
          if (DayMonthFormat)
          {
            GETCOL;
          }
          else
          {
            // Time/Year indicator is always 5 characters long (???), on most
            // systems year is aligned to right (_YYYY), but on some to left (YYYY_),
            // we must ensure that trailing space is also deleted, so real
            // separator space is not treated as part of file name
            Col = Line.SubString(1, 6).Trim();
            Line.Delete(1, 6);
          }
          // GETNCOL; // We don't want to trim input strings (name with space at beginning???)
          // Check if we got time (contains :) or year
          if ((P = (Word)Col.Pos(L':')) > 0)
          {
            Word CurrMonth, CurrDay;
            Hour = (Word)StrToInt(Col.SubString(1, P-1));
            Min = (Word)StrToInt(Col.SubString(P+1, Col.Length() - P));
            if (Hour > 23 || Min > 59) Abort();
            // When we don't got year, we assume current year
            // with exception that the date would be in future
            // in this case we assume last year.
            DecodeDate(Date(), Year, CurrMonth, CurrDay);
            if ((Month > CurrMonth) ||
                (Month == CurrMonth && Day > CurrDay)) Year--;
            Sec = 0;
            FModificationFmt = mfMDHM;
          }
            else
          {
            Year = (Word)StrToInt(Col);
            if (Year > 10000) Abort();
            // When we don't got time we assume midnight
            Hour = 0; Min = 0; Sec = 0;
            FModificationFmt = mfMDY;
          }
        }
      }

      FModification = EncodeDateVerbose(Year, Month, Day) + EncodeTimeVerbose(Hour, Min, Sec, 0);
      // adjust only when time is known,
      // adjusting default "midnight" time makes no sense
      if ((FModificationFmt == mfMDHM) || (FModificationFmt == mfFull))
      {
        assert(Terminal != NULL);
        FModification = AdjustDateTimeFromUnix(FModification,
          Terminal->SessionData->DSTMode);
      }

      if (double(FLastAccess) == 0)
      {
        FLastAccess = FModification;
      }

      // separating space is already deleted, other spaces are treated as part of name

      {
        int P;

        FLinkTo = L"";
        if (IsSymLink)
        {
          P = Line.Pos(SYMLINKSTR);
          if (P)
          {
            FLinkTo = Line.SubString(
              P + wcslen(SYMLINKSTR), Line.Length() - P + wcslen(SYMLINKSTR) + 1);
            Line.SetLength(P - 1);
          }
          else
          {
            Abort();
          }
        }
        FFileName = UnixExtractFileName(Line);
      }
    }

    #undef GETNCOL
    #undef GETCOL
  }
  catch (Exception &E)
  {
    throw ETerminal(&E, FmtLoadStr(LIST_LINE_ERROR, ARRAYOFCONST((value))), HELP_LIST_LINE_ERROR);
  }
}
//---------------------------------------------------------------------------
void __fastcall TRemoteFile::Complete()
{
  assert(Terminal != NULL);
  if (IsSymLink && Terminal->ResolvingSymlinks)
  {
    FindLinkedFile();
  }
}
//---------------------------------------------------------------------------
void __fastcall TRemoteFile::FindLinkedFile()
{
  assert(Terminal && IsSymLink);

  if (FLinkedFile) delete FLinkedFile;
  FLinkedFile = NULL;

  FCyclicLink = false;
  if (!LinkTo.IsEmpty())
  {
    // check for cyclic link
    TRemoteFile * LinkedBy = FLinkedByFile;
    while (LinkedBy)
    {
      if (LinkedBy->LinkTo == LinkTo)
      {
        // this is currenly redundant information, because it is used only to
        // detect broken symlink, which would be otherwise detected
        // by FLinkedFile == NULL
        FCyclicLink = true;
        break;
      }
      LinkedBy = LinkedBy->FLinkedByFile;
    }
  }

  if (FCyclicLink)
  {
    TRemoteFile * LinkedBy = FLinkedByFile;
    while (LinkedBy)
    {
      LinkedBy->FCyclicLink = true;
      LinkedBy = LinkedBy->FLinkedByFile;
    }
  }
  else
  {
    assert(Terminal->ResolvingSymlinks);
    Terminal->ExceptionOnFail = true;
    try
    {
      try
      {
        Terminal->ReadSymlink(this, FLinkedFile);
      }
      __finally
      {
        Terminal->ExceptionOnFail = false;
      }
    }
    catch (Exception &E)
    {
      if (E.InheritsFrom(__classid(EFatal))) throw;
      else
      {
        Terminal->Log->AddException(&E);
      }
    }
  }
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TRemoteFile::GetListingStr()
{
  // note that ModificationStr is longer than 12 for mfFull
  UnicodeString LinkPart;
  // expanded from ?: to avoid memory leaks
  if (IsSymLink)
  {
    LinkPart = UnicodeString(SYMLINKSTR) + LinkTo;
  }
  return Format(L"%s%s %3s %-8s %-8s %9s %-12s %s%s", ARRAYOFCONST((
    Type, Rights->Text, IntToStr(INodeBlocks), Owner.Name,
    Group.Name, IntToStr(Size), ModificationStr, FileName,
    LinkPart)));
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TRemoteFile::GetFullFileName() const
{
  if (FFullFileName.IsEmpty())
  {
    assert(Terminal);
    assert(Directory != NULL);
    UnicodeString Path;
    if (IsParentDirectory) Path = Directory->ParentPath;
    else if (IsDirectory) Path = UnixIncludeTrailingBackslash(Directory->FullDirectory + FileName);
    else Path = Directory->FullDirectory + FileName;
    return Terminal->TranslateLockedPath(Path, true);
  }
  else
  {
    return FFullFileName;
  }
}
//---------------------------------------------------------------------------
bool __fastcall TRemoteFile::GetHaveFullFileName() const
{
  return !FFullFileName.IsEmpty() || (Directory != NULL);
}
//---------------------------------------------------------------------------
Integer __fastcall TRemoteFile::GetAttr()
{
  Integer Result = 0;
  if (Rights->ReadOnly) Result |= faReadOnly;
  if (IsHidden) Result |= faHidden;
  return Result;
}
//---------------------------------------------------------------------------
void __fastcall TRemoteFile::SetTerminal(TTerminal * value)
{
  FTerminal = value;
  if (FLinkedFile)
  {
    FLinkedFile->Terminal = value;
  }
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
__fastcall TRemoteDirectoryFile::TRemoteDirectoryFile() : TRemoteFile()
{
  Modification = double(0);
  ModificationFmt = mfNone;
  LastAccess = Modification;
  Type = L'D';
  Size = 0;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
__fastcall TRemoteParentDirectory::TRemoteParentDirectory(TTerminal * ATerminal)
  : TRemoteDirectoryFile()
{
  FileName = PARENTDIRECTORY;
  Terminal = ATerminal;
}
//=== TRemoteFileList ------------------------------------------------------
__fastcall TRemoteFileList::TRemoteFileList():
  TObjectList()
{
  FTimestamp = Now();
}
//---------------------------------------------------------------------------
void __fastcall TRemoteFileList::AddFile(TRemoteFile * File)
{
  Add(File);
  File->Directory = this;
}
//---------------------------------------------------------------------------
void __fastcall TRemoteFileList::DuplicateTo(TRemoteFileList * Copy)
{
  Copy->Reset();
  for (int Index = 0; Index < Count; Index++)
  {
    TRemoteFile * File = Files[Index];
    Copy->AddFile(File->Duplicate(false));
  }
  Copy->FDirectory = Directory;
  Copy->FTimestamp = FTimestamp;
}
//---------------------------------------------------------------------------
void __fastcall TRemoteFileList::Reset()
{
  FTimestamp = Now();
  Clear();
}
//---------------------------------------------------------------------------
void __fastcall TRemoteFileList::SetDirectory(UnicodeString value)
{
  FDirectory = UnixExcludeTrailingBackslash(value);
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TRemoteFileList::GetFullDirectory()
{
  return UnixIncludeTrailingBackslash(Directory);
}
//---------------------------------------------------------------------------
TRemoteFile * __fastcall TRemoteFileList::GetFiles(Integer Index)
{
  return (TRemoteFile *)Items[Index];
}
//---------------------------------------------------------------------------
Boolean __fastcall TRemoteFileList::GetIsRoot()
{
  return (Directory == ROOTDIRECTORY);
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TRemoteFileList::GetParentPath()
{
  return UnixExtractFilePath(Directory);
}
//---------------------------------------------------------------------------
__int64 __fastcall TRemoteFileList::GetTotalSize()
{
  __int64 Result = 0;
  for (Integer Index = 0; Index < Count; Index++)
    if (!Files[Index]->IsDirectory) Result += Files[Index]->Size;
  return Result;
}
//---------------------------------------------------------------------------
TRemoteFile * __fastcall TRemoteFileList::FindFile(const UnicodeString &FileName)
{
  for (Integer Index = 0; Index < Count; Index++)
    if (Files[Index]->FileName == FileName) return Files[Index];
  return NULL;
}
//=== TRemoteDirectory ------------------------------------------------------
__fastcall TRemoteDirectory::TRemoteDirectory(TTerminal * aTerminal, TRemoteDirectory * Template) :
  TRemoteFileList(), FTerminal(aTerminal)
{
  FSelectedFiles = NULL;
  FThisDirectory = NULL;
  FParentDirectory = NULL;
  if (Template == NULL)
  {
    FIncludeThisDirectory = false;
    FIncludeParentDirectory = true;
  }
  else
  {
    FIncludeThisDirectory = Template->FIncludeThisDirectory;
    FIncludeParentDirectory = Template->FIncludeParentDirectory;
  }
}
//---------------------------------------------------------------------------
__fastcall TRemoteDirectory::~TRemoteDirectory()
{
  ReleaseRelativeDirectories();
}
//---------------------------------------------------------------------------
void __fastcall TRemoteDirectory::ReleaseRelativeDirectories()
{
  if ((ThisDirectory != NULL) && !IncludeThisDirectory)
  {
    delete FThisDirectory;
    FThisDirectory = NULL;
  }
  if ((ParentDirectory != NULL) && !IncludeParentDirectory)
  {
    delete FParentDirectory;
    FParentDirectory = NULL;
  }
}
//---------------------------------------------------------------------------
void __fastcall TRemoteDirectory::Reset()
{
  ReleaseRelativeDirectories();
  TRemoteFileList::Reset();
}
//---------------------------------------------------------------------------
void __fastcall TRemoteDirectory::SetDirectory(UnicodeString value)
{
  TRemoteFileList::SetDirectory(value);
}
//---------------------------------------------------------------------------
void __fastcall TRemoteDirectory::AddFile(TRemoteFile * File)
{
  if (File->IsThisDirectory) FThisDirectory = File;
  if (File->IsParentDirectory) FParentDirectory = File;

  if ((!File->IsThisDirectory || IncludeThisDirectory) &&
      (!File->IsParentDirectory || IncludeParentDirectory))
  {
    TRemoteFileList::AddFile(File);
  }
  File->Terminal = Terminal;
}
//---------------------------------------------------------------------------
void __fastcall TRemoteDirectory::DuplicateTo(TRemoteFileList * Copy)
{
  TRemoteFileList::DuplicateTo(Copy);
  if (ThisDirectory && !IncludeThisDirectory)
  {
    Copy->AddFile(ThisDirectory->Duplicate(false));
  }
  if (ParentDirectory && !IncludeParentDirectory)
  {
    Copy->AddFile(ParentDirectory->Duplicate(false));
  }
}
//---------------------------------------------------------------------------
bool __fastcall TRemoteDirectory::GetLoaded()
{
  return ((Terminal != NULL) && Terminal->Active && !Directory.IsEmpty());
}
//---------------------------------------------------------------------------
TStrings * __fastcall TRemoteDirectory::GetSelectedFiles()
{
  if (!FSelectedFiles)
  {
    FSelectedFiles = new TStringList();
  }
  else
  {
    FSelectedFiles->Clear();
  }

  for (int Index = 0; Index < Count; Index ++)
  {
    if (Files[Index]->Selected)
    {
      FSelectedFiles->Add(Files[Index]->FullFileName);
    }
  }

  return FSelectedFiles;
}
//---------------------------------------------------------------------------
void __fastcall TRemoteDirectory::SetIncludeParentDirectory(Boolean value)
{
  if (IncludeParentDirectory != value)
  {
    FIncludeParentDirectory = value;
    if (value && ParentDirectory)
    {
      assert(IndexOf(ParentDirectory) < 0);
      Add(ParentDirectory);
    }
    else if (!value && ParentDirectory)
    {
      assert(IndexOf(ParentDirectory) >= 0);
      Extract(ParentDirectory);
    }
  }
}
//---------------------------------------------------------------------------
void __fastcall TRemoteDirectory::SetIncludeThisDirectory(Boolean value)
{
  if (IncludeThisDirectory != value)
  {
    FIncludeThisDirectory = value;
    if (value && ThisDirectory)
    {
      assert(IndexOf(ThisDirectory) < 0);
      Add(ThisDirectory);
    }
    else if (!value && ThisDirectory)
    {
      assert(IndexOf(ThisDirectory) >= 0);
      Extract(ThisDirectory);
    }
  }
}
//===========================================================================
__fastcall TRemoteDirectoryCache::TRemoteDirectoryCache(): TStringList()
{
  FSection = new TCriticalSection();
  Sorted = true;
  Duplicates = dupError;
  CaseSensitive = true;
}
//---------------------------------------------------------------------------
__fastcall TRemoteDirectoryCache::~TRemoteDirectoryCache()
{
  Clear();
  delete FSection;
}
//---------------------------------------------------------------------------
void __fastcall TRemoteDirectoryCache::Clear()
{
  TGuard Guard(FSection);

  try
  {
    for (int Index = 0; Index < Count; Index++)
    {
      delete (TRemoteFileList *)Objects[Index];
      Objects[Index] = NULL;
    }
  }
  __finally
  {
    TStringList::Clear();
  }
}
//---------------------------------------------------------------------------
bool __fastcall TRemoteDirectoryCache::GetIsEmpty() const
{
  TGuard Guard(FSection);

  return (const_cast<TRemoteDirectoryCache*>(this)->Count == 0);
}
//---------------------------------------------------------------------------
bool __fastcall TRemoteDirectoryCache::HasFileList(const UnicodeString Directory)
{
  TGuard Guard(FSection);

  int Index = IndexOf(UnixExcludeTrailingBackslash(Directory));
  return (Index >= 0);
}
//---------------------------------------------------------------------------
bool __fastcall TRemoteDirectoryCache::HasNewerFileList(const UnicodeString Directory,
  TDateTime Timestamp)
{
  TGuard Guard(FSection);

  int Index = IndexOf(UnixExcludeTrailingBackslash(Directory));
  if (Index >= 0)
  {
    TRemoteFileList * FileList = dynamic_cast<TRemoteFileList *>(Objects[Index]);
    if (FileList->Timestamp <= Timestamp)
    {
      Index = -1;
    }
  }
  return (Index >= 0);
}
//---------------------------------------------------------------------------
bool __fastcall TRemoteDirectoryCache::GetFileList(const UnicodeString Directory,
  TRemoteFileList * FileList)
{
  TGuard Guard(FSection);

  int Index = IndexOf(UnixExcludeTrailingBackslash(Directory));
  bool Result = (Index >= 0);
  if (Result)
  {
    assert(Objects[Index] != NULL);
    dynamic_cast<TRemoteFileList *>(Objects[Index])->DuplicateTo(FileList);
  }
  return Result;
}
//---------------------------------------------------------------------------
void __fastcall TRemoteDirectoryCache::AddFileList(TRemoteFileList * FileList)
{
  assert(FileList);
  TRemoteFileList * Copy = new TRemoteFileList();
  FileList->DuplicateTo(Copy);

  {
    TGuard Guard(FSection);

    // file list cannot be cached already with only one thread, but it can be
    // when directory is loaded by secondary terminal
    DoClearFileList(FileList->Directory, false);
    AddObject(Copy->Directory, Copy);
  }
}
//---------------------------------------------------------------------------
void __fastcall TRemoteDirectoryCache::ClearFileList(UnicodeString Directory, bool SubDirs)
{
  TGuard Guard(FSection);
  DoClearFileList(Directory, SubDirs);
}
//---------------------------------------------------------------------------
void __fastcall TRemoteDirectoryCache::DoClearFileList(UnicodeString Directory, bool SubDirs)
{
  Directory = UnixExcludeTrailingBackslash(Directory);
  int Index = IndexOf(Directory);
  if (Index >= 0)
  {
    Delete(Index);
  }
  if (SubDirs)
  {
    Directory = UnixIncludeTrailingBackslash(Directory);
    Index = Count-1;
    while (Index >= 0)
    {
      if (Strings[Index].SubString(1, Directory.Length()) == Directory)
      {
        Delete(Index);
      }
      Index--;
    }
  }
}
//---------------------------------------------------------------------------
void __fastcall TRemoteDirectoryCache::Delete(int Index)
{
  delete dynamic_cast<TRemoteFileList *>(Objects[Index]);
  TStringList::Delete(Index);
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
__fastcall TRemoteDirectoryChangesCache::TRemoteDirectoryChangesCache(int MaxSize) :
  TStringList(),
  FMaxSize(MaxSize)
{
}
//---------------------------------------------------------------------------
void __fastcall TRemoteDirectoryChangesCache::Clear()
{
  TStringList::Clear();
}
//---------------------------------------------------------------------------
bool __fastcall TRemoteDirectoryChangesCache::GetIsEmpty() const
{
  return (const_cast<TRemoteDirectoryChangesCache*>(this)->Count == 0);
}
//---------------------------------------------------------------------------
void __fastcall TRemoteDirectoryChangesCache::SetValue(const UnicodeString & Name,
  const UnicodeString & Value)
{
  int Index = IndexOfName(Name);
  if (Index > 0)
  {
    Delete(Index);
  }
  Values[Name] = Value;
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TRemoteDirectoryChangesCache::GetValue(const UnicodeString & Name)
{
  UnicodeString Value = Values[Name];
  SetValue(Name, Value);
  return Value;
}
//---------------------------------------------------------------------------
void __fastcall TRemoteDirectoryChangesCache::AddDirectoryChange(
  const UnicodeString SourceDir, const UnicodeString Change,
  const UnicodeString TargetDir)
{
  assert(!TargetDir.IsEmpty());
  SetValue(TargetDir, L"//");
  if (TTerminal::ExpandFileName(Change, SourceDir) != TargetDir)
  {
    UnicodeString Key;
    if (DirectoryChangeKey(SourceDir, Change, Key))
    {
      SetValue(Key, TargetDir);
    }
  }
}
//---------------------------------------------------------------------------
void __fastcall TRemoteDirectoryChangesCache::ClearDirectoryChange(
  UnicodeString SourceDir)
{
  for (int Index = 0; Index < Count; Index++)
  {
    if (Names[Index].SubString(1, SourceDir.Length()) == SourceDir)
    {
      Delete(Index);
      Index--;
    }
  }
}
//---------------------------------------------------------------------------
void __fastcall TRemoteDirectoryChangesCache::ClearDirectoryChangeTarget(
  UnicodeString TargetDir)
{
  UnicodeString Key;
  // hack to clear at least local sym-link change in case symlink is deleted
  DirectoryChangeKey(UnixExcludeTrailingBackslash(UnixExtractFilePath(TargetDir)),
    UnixExtractFileName(TargetDir), Key);

  for (int Index = 0; Index < Count; Index++)
  {
    UnicodeString Name = Names[Index];
    if ((Name.SubString(1, TargetDir.Length()) == TargetDir) ||
        (Values[Name].SubString(1, TargetDir.Length()) == TargetDir) ||
        (!Key.IsEmpty() && (Name == Key)))
    {
      Delete(Index);
      Index--;
    }
  }
}
//---------------------------------------------------------------------------
bool __fastcall TRemoteDirectoryChangesCache::GetDirectoryChange(
  const UnicodeString SourceDir, const UnicodeString Change, UnicodeString & TargetDir)
{
  UnicodeString Key;
  bool Result;
  Key = TTerminal::ExpandFileName(Change, SourceDir);
  Result = (IndexOfName(Key) >= 0);
  if (Result)
  {
    TargetDir = GetValue(Key);
    // TargetDir is not "//" here only when Change is full path to symbolic link
    if (TargetDir == L"//")
    {
      TargetDir = Key;
    }
  }
  else
  {
    Result = DirectoryChangeKey(SourceDir, Change, Key);
    if (Result)
    {
      UnicodeString Directory = GetValue(Key);
      Result = !Directory.IsEmpty();
      if (Result)
      {
        TargetDir = Directory;
      }
    }
  }
  return Result;
}
//---------------------------------------------------------------------------
void __fastcall TRemoteDirectoryChangesCache::Serialize(UnicodeString & Data)
{
  Data = L"A";
  int ACount = Count;
  if (ACount > FMaxSize)
  {
    TStrings * Limited = new TStringList();
    try
    {
      int Index = ACount - FMaxSize;
      while (Index < ACount)
      {
        Limited->Add(Strings[Index]);
        Index++;
      }
      Data += Limited->Text;
    }
    __finally
    {
      delete Limited;
    }
  }
  else
  {
    Data += Text;
  }
}
//---------------------------------------------------------------------------
void __fastcall TRemoteDirectoryChangesCache::Deserialize(const UnicodeString Data)
{
  if (Data.IsEmpty())
  {
    Text = L"";
  }
  else
  {
    Text = Data.c_str() + 1;
  }
}
//---------------------------------------------------------------------------
bool __fastcall TRemoteDirectoryChangesCache::DirectoryChangeKey(
  const UnicodeString SourceDir, const UnicodeString Change, UnicodeString & Key)
{
  bool Result = !Change.IsEmpty();
  if (Result)
  {
    bool Absolute = UnixIsAbsolutePath(Change);
    Result = !SourceDir.IsEmpty() || Absolute;
    if (Result)
    {
      // expanded from ?: to avoid memory leaks
      if (Absolute)
      {
        Key = Change;
      }
      else
      {
        Key = SourceDir + L"," + Change;
      }
    }
  }
  return Result;
}
//=== TRights ---------------------------------------------------------------
const wchar_t TRights::BasicSymbols[] = L"rwxrwxrwx";
const wchar_t TRights::CombinedSymbols[] = L"--s--s--t";
const wchar_t TRights::ExtendedSymbols[] = L"--S--S--T";
const wchar_t TRights::ModeGroups[] = L"ugo";
//---------------------------------------------------------------------------
__fastcall TRights::TRights()
{
  FAllowUndef = false;
  FSet = 0;
  FUnset = 0;
  Number = 0;
  FUnknown = true;
}
//---------------------------------------------------------------------------
__fastcall TRights::TRights(unsigned short ANumber)
{
  FAllowUndef = false;
  FSet = 0;
  FUnset = 0;
  Number = ANumber;
}
//---------------------------------------------------------------------------
__fastcall TRights::TRights(const TRights & Source)
{
  Assign(&Source);
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void __fastcall TRights::Assign(const TRights * Source)
{
  FAllowUndef = Source->AllowUndef;
  FSet = Source->FSet;
  FUnset = Source->FUnset;
  FText = Source->FText;
  FUnknown = Source->FUnknown;
}
//---------------------------------------------------------------------------
TRights::TFlag __fastcall TRights::RightToFlag(TRights::TRight Right)
{
  return static_cast<TFlag>(1 << (rrLast - Right));
}
//---------------------------------------------------------------------------
bool __fastcall TRights::operator ==(const TRights & rhr) const
{
  if (AllowUndef || rhr.AllowUndef)
  {
    for (int Right = rrFirst; Right <= rrLast; Right++)
    {
      if (RightUndef[static_cast<TRight>(Right)] !=
            rhr.RightUndef[static_cast<TRight>(Right)])
      {
        return false;
      }
    }
    return true;
  }
  else
  {
    return (Number == rhr.Number);
  }
}
//---------------------------------------------------------------------------
bool __fastcall TRights::operator ==(unsigned short rhr) const
{
  return (Number == rhr);
}
//---------------------------------------------------------------------------
bool __fastcall TRights::operator !=(const TRights & rhr) const
{
  return !(*this == rhr);
}
//---------------------------------------------------------------------------
TRights & __fastcall TRights::operator =(unsigned short rhr)
{
  Number = rhr;
  return *this;
}
//---------------------------------------------------------------------------
TRights & __fastcall TRights::operator =(const TRights & rhr)
{
  Assign(&rhr);
  return *this;
}
//---------------------------------------------------------------------------
TRights __fastcall TRights::operator ~() const
{
  TRights Result(static_cast<unsigned short>(~Number));
  return Result;
}
//---------------------------------------------------------------------------
TRights __fastcall TRights::operator &(const TRights & rhr) const
{
  TRights Result(*this);
  Result &= rhr;
  return Result;
}
//---------------------------------------------------------------------------
TRights __fastcall TRights::operator &(unsigned short rhr) const
{
  TRights Result(*this);
  Result &= rhr;
  return Result;
}
//---------------------------------------------------------------------------
TRights & __fastcall TRights::operator &=(const TRights & rhr)
{
  if (AllowUndef || rhr.AllowUndef)
  {
    for (int Right = rrFirst; Right <= rrLast; Right++)
    {
      if (RightUndef[static_cast<TRight>(Right)] !=
            rhr.RightUndef[static_cast<TRight>(Right)])
      {
        RightUndef[static_cast<TRight>(Right)] = rsUndef;
      }
    }
  }
  else
  {
    Number &= rhr.Number;
  }
  return *this;
}
//---------------------------------------------------------------------------
TRights & __fastcall TRights::operator &=(unsigned short rhr)
{
  Number &= rhr;
  return *this;
}
//---------------------------------------------------------------------------
TRights __fastcall TRights::operator |(const TRights & rhr) const
{
  TRights Result(*this);
  Result |= rhr;
  return Result;
}
//---------------------------------------------------------------------------
TRights __fastcall TRights::operator |(unsigned short rhr) const
{
  TRights Result(*this);
  Result |= rhr;
  return Result;
}
//---------------------------------------------------------------------------
TRights & __fastcall TRights::operator |=(const TRights & rhr)
{
  Number |= rhr.Number;
  return *this;
}
//---------------------------------------------------------------------------
TRights & __fastcall TRights::operator |=(unsigned short rhr)
{
  Number |= rhr;
  return *this;
}
//---------------------------------------------------------------------------
void __fastcall TRights::SetAllowUndef(bool value)
{
  if (FAllowUndef != value)
  {
    assert(!value || ((FSet | FUnset) == rfAllSpecials));
    FAllowUndef = value;
  }
}
//---------------------------------------------------------------------------
void __fastcall TRights::SetText(const UnicodeString & value)
{
  if (value != Text)
  {
    if ((value.Length() != TextLen) ||
        (!AllowUndef && (value.Pos(UndefSymbol) > 0)) ||
        (value.Pos(L" ") > 0))
    {
      throw Exception(FMTLOAD(RIGHTS_ERROR, (value)));
    }

    FSet = 0;
    FUnset = 0;
    int Flag = 00001;
    int ExtendedFlag = 01000;
    bool KeepText = false;
    for (int i = TextLen; i >= 1; i--)
    {
      if (value[i] == UnsetSymbol)
      {
        FUnset |= static_cast<unsigned short>(Flag | ExtendedFlag);
      }
      else if (value[i] == UndefSymbol)
      {
        // do nothing
      }
      else if (value[i] == CombinedSymbols[i - 1])
      {
        FSet |= static_cast<unsigned short>(Flag | ExtendedFlag);
      }
      else if (value[i] == ExtendedSymbols[i - 1])
      {
        FSet |= static_cast<unsigned short>(ExtendedFlag);
        FUnset |= static_cast<unsigned short>(Flag);
      }
      else
      {
        if (value[i] != BasicSymbols[i - 1])
        {
          KeepText = true;
        }
        FSet |= static_cast<unsigned short>(Flag);
        if (i % 3 == 0)
        {
          FUnset |= static_cast<unsigned short>(ExtendedFlag);
        }
      }

      Flag <<= 1;
      if (i % 3 == 1)
      {
        ExtendedFlag <<= 1;
      }
    }

    FText = KeepText ? value : UnicodeString();
  }
  FUnknown = false;
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TRights::GetText() const
{
  if (!FText.IsEmpty())
  {
    return FText;
  }
  else
  {
    UnicodeString Result;
    Result.SetLength(TextLen);

    int Flag = 00001;
    int ExtendedFlag = 01000;
    bool ExtendedPos = true;
    wchar_t Symbol;
    int i = TextLen;
    while (i >= 1)
    {
      if (ExtendedPos &&
          ((FSet & (Flag | ExtendedFlag)) == (Flag | ExtendedFlag)))
      {
        Symbol = CombinedSymbols[i - 1];
      }
      else if ((FSet & Flag) != 0)
      {
        Symbol = BasicSymbols[i - 1];
      }
      else if (ExtendedPos && ((FSet & ExtendedFlag) != 0))
      {
        Symbol = ExtendedSymbols[i - 1];
      }
      else if ((!ExtendedPos && ((FUnset & Flag) == Flag)) ||
        (ExtendedPos && ((FUnset & (Flag | ExtendedFlag)) == (Flag | ExtendedFlag))))
      {
        Symbol = UnsetSymbol;
      }
      else
      {
        Symbol = UndefSymbol;
      }

      Result[i] = Symbol;

      Flag <<= 1;
      i--;
      ExtendedPos = ((i % 3) == 0);
      if (ExtendedPos)
      {
        ExtendedFlag <<= 1;
      }
    }
    return Result;
  }
}
//---------------------------------------------------------------------------
void __fastcall TRights::SetOctal(UnicodeString value)
{
  UnicodeString AValue(value);
  if (AValue.Length() == 3)
  {
    AValue = L"0" + AValue;
  }

  if (Octal != AValue)
  {
    bool Correct = (AValue.Length() == 4);
    if (Correct)
    {
      for (int i = 1; (i <= AValue.Length()) && Correct; i++)
      {
        Correct = (AValue[i] >= L'0') && (AValue[i] <= L'7');
      }
    }

    if (!Correct)
    {
      throw Exception(FMTLOAD(INVALID_OCTAL_PERMISSIONS, (value)));
    }

    Number = static_cast<unsigned short>(
      ((AValue[1] - L'0') << 9) +
      ((AValue[2] - L'0') << 6) +
      ((AValue[3] - L'0') << 3) +
      ((AValue[4] - L'0') << 0));
  }
  FUnknown = false;
}
//---------------------------------------------------------------------------
unsigned long __fastcall TRights::GetNumberDecadic() const
{
  unsigned long N = NumberSet; // used to be "Number"
  unsigned long Result =
      ((N & 07000) / 01000 * 1000) +
      ((N & 00700) /  0100 *  100) +
      ((N & 00070) /   010 *   10) +
      ((N & 00007) /    01 *    1);

  return Result;
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TRights::GetOctal() const
{
  UnicodeString Result;
  unsigned short N = NumberSet; // used to be "Number"
  Result.SetLength(4);
  Result[1] = static_cast<wchar_t>(L'0' + ((N & 07000) >> 9));
  Result[2] = static_cast<wchar_t>(L'0' + ((N & 00700) >> 6));
  Result[3] = static_cast<wchar_t>(L'0' + ((N & 00070) >> 3));
  Result[4] = static_cast<wchar_t>(L'0' + ((N & 00007) >> 0));

  return Result;
}
//---------------------------------------------------------------------------
void __fastcall TRights::SetNumber(unsigned short value)
{
  if ((FSet != value) || ((FSet | FUnset) != rfAllSpecials))
  {
    FSet = value;
    FUnset = static_cast<unsigned short>(rfAllSpecials & ~FSet);
    FText = L"";
  }
  FUnknown = false;
}
//---------------------------------------------------------------------------
unsigned short __fastcall TRights::GetNumber() const
{
  assert(!IsUndef);
  return FSet;
}
//---------------------------------------------------------------------------
void __fastcall TRights::SetRight(TRight Right, bool value)
{
  RightUndef[Right] = (value ? rsYes : rsNo);
}
//---------------------------------------------------------------------------
bool __fastcall TRights::GetRight(TRight Right) const
{
  TState State = RightUndef[Right];
  assert(State != rsUndef);
  return (State == rsYes);
}
//---------------------------------------------------------------------------
void __fastcall TRights::SetRightUndef(TRight Right, TState value)
{
  if (value != RightUndef[Right])
  {
    assert((value != rsUndef) || AllowUndef);

    TFlag Flag = RightToFlag(Right);

    switch (value)
    {
      case rsYes:
        FSet |= static_cast<unsigned short>(Flag);
        FUnset &= static_cast<unsigned short>(~Flag);
        break;

      case rsNo:
        FSet &= static_cast<unsigned short>(~Flag);
        FUnset |= static_cast<unsigned short>(Flag);
        break;

      case rsUndef:
      default:
        FSet &= static_cast<unsigned short>(~Flag);
        FUnset &= static_cast<unsigned short>(~Flag);
        break;
    }

    FText = L"";
  }
  FUnknown = false;
}
//---------------------------------------------------------------------------
TRights::TState __fastcall TRights::GetRightUndef(TRight Right) const
{
  TFlag Flag = RightToFlag(Right);
  TState Result;

  if ((FSet & Flag) != 0)
  {
    Result = rsYes;
  }
  else if ((FUnset & Flag) != 0)
  {
    Result = rsNo;
  }
  else
  {
    Result = rsUndef;
  }
  return Result;
}
//---------------------------------------------------------------------------
void __fastcall TRights::SetReadOnly(bool value)
{
  Right[rrUserWrite] = !value;
  Right[rrGroupWrite] = !value;
  Right[rrOtherWrite] = !value;
}
//---------------------------------------------------------------------------
bool  __fastcall TRights::GetReadOnly()
{
  return Right[rrUserWrite] && Right[rrGroupWrite] && Right[rrOtherWrite];
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TRights::GetSimplestStr() const
{
  if (IsUndef)
  {
    return ModeStr;
  }
  else
  {
    return Octal;
  }
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TRights::GetModeStr() const
{
  UnicodeString Result;
  UnicodeString SetModeStr, UnsetModeStr;
  TRight Right;
  int Index;

  for (int Group = 0; Group < 3; Group++)
  {
    SetModeStr = L"";
    UnsetModeStr = L"";
    for (int Mode = 0; Mode < 3; Mode++)
    {
      Index = (Group * 3) + Mode;
      Right = static_cast<TRight>(rrUserRead + Index);
      switch (RightUndef[Right])
      {
        case rsYes:
          SetModeStr += BasicSymbols[Index];
          break;

        case rsNo:
          UnsetModeStr += BasicSymbols[Index];
          break;
      }
    }

    Right = static_cast<TRight>(rrUserIDExec + Group);
    Index = (Group * 3) + 2;
    switch (RightUndef[Right])
    {
      case rsYes:
        SetModeStr += CombinedSymbols[Index];
        break;

      case rsNo:
        UnsetModeStr += CombinedSymbols[Index];
        break;
    }

    if (!SetModeStr.IsEmpty() || !UnsetModeStr.IsEmpty())
    {
      if (!Result.IsEmpty())
      {
        Result += L',';
      }
      Result += ModeGroups[Group];
      if (!SetModeStr.IsEmpty())
      {
        Result += L"+" + SetModeStr;
      }
      if (!UnsetModeStr.IsEmpty())
      {
        Result += L"-" + UnsetModeStr;
      }
    }
  }
  return Result;
}
//---------------------------------------------------------------------------
void __fastcall TRights::AddExecute()
{
  for (int Group = 0; Group < 3; Group++)
  {
    if ((RightUndef[static_cast<TRight>(rrUserRead + (Group * 3))] == rsYes) ||
        (RightUndef[static_cast<TRight>(rrUserWrite + (Group * 3))] == rsYes))
    {
      Right[static_cast<TRight>(rrUserExec + (Group * 3))] = true;
    }
  }
  FUnknown = false;
}
//---------------------------------------------------------------------------
void __fastcall TRights::AllUndef()
{
  if ((FSet != 0) || (FUnset != 0))
  {
    FSet = 0;
    FUnset = 0;
    FText = L"";
  }
  FUnknown = false;
}
//---------------------------------------------------------------------------
bool __fastcall TRights::GetIsUndef() const
{
  return ((FSet | FUnset) != rfAllSpecials);
}
//---------------------------------------------------------------------------
__fastcall TRights::operator unsigned short() const
{
  return Number;
}
//---------------------------------------------------------------------------
__fastcall TRights::operator unsigned long() const
{
  return Number;
}
//=== TRemoteProperties -------------------------------------------------------
__fastcall TRemoteProperties::TRemoteProperties()
{
  Default();
}
//---------------------------------------------------------------------------
__fastcall TRemoteProperties::TRemoteProperties(const TRemoteProperties & rhp) :
  Valid(rhp.Valid),
  Recursive(rhp.Recursive),
  Rights(rhp.Rights),
  AddXToDirectories(rhp.AddXToDirectories),
  Group(rhp.Group),
  Owner(rhp.Owner),
  Modification(rhp.Modification),
  LastAccess(rhp.Modification)
{
}
//---------------------------------------------------------------------------
void __fastcall TRemoteProperties::Default()
{
  Valid.Clear();
  AddXToDirectories = false;
  Rights.AllowUndef = false;
  Rights.Number = 0;
  Group.Clear();
  Owner.Clear();
  Recursive = false;
}
//---------------------------------------------------------------------------
bool __fastcall TRemoteProperties::operator ==(const TRemoteProperties & rhp) const
{
  bool Result = (Valid == rhp.Valid && Recursive == rhp.Recursive);

  if (Result)
  {
    if ((Valid.Contains(vpRights) &&
          (Rights != rhp.Rights || AddXToDirectories != rhp.AddXToDirectories)) ||
        (Valid.Contains(vpOwner) && (Owner != rhp.Owner)) ||
        (Valid.Contains(vpGroup) && (Group != rhp.Group)) ||
        (Valid.Contains(vpModification) && (Modification != rhp.Modification)) ||
        (Valid.Contains(vpLastAccess) && (LastAccess != rhp.LastAccess)))
    {
      Result = false;
    }
  }
  return Result;
}
//---------------------------------------------------------------------------
bool __fastcall TRemoteProperties::operator !=(const TRemoteProperties & rhp) const
{
  return !(*this == rhp);
}
//---------------------------------------------------------------------------
TRemoteProperties __fastcall TRemoteProperties::CommonProperties(TStrings * FileList)
{
  // TODO: Modification and LastAccess
  TRemoteProperties CommonProperties;
  for (int Index = 0; Index < FileList->Count; Index++)
  {
    TRemoteFile * File = (TRemoteFile *)(FileList->Objects[Index]);
    assert(File);
    if (!Index)
    {
      CommonProperties.Rights = *(File->Rights);
      // previously we allowed undef implicitly for directories,
      // now we do it explicitly in properties dialog and only in combination
      // with "recursive" option
      CommonProperties.Rights.AllowUndef = File->Rights->IsUndef;
      CommonProperties.Valid << vpRights;
      if (File->Owner.IsSet)
      {
        CommonProperties.Owner = File->Owner;
        CommonProperties.Valid << vpOwner;
      }
      if (File->Group.IsSet)
      {
        CommonProperties.Group = File->Group;
        CommonProperties.Valid << vpGroup;
      }
    }
    else
    {
      CommonProperties.Rights.AllowUndef = True;
      CommonProperties.Rights &= *File->Rights;
      if (CommonProperties.Owner != File->Owner)
      {
        CommonProperties.Owner.Clear();
        CommonProperties.Valid >> vpOwner;
      }
      if (CommonProperties.Group != File->Group)
      {
        CommonProperties.Group.Clear();
        CommonProperties.Valid >> vpGroup;
      }
    }
  }
  return CommonProperties;
}
//---------------------------------------------------------------------------
TRemoteProperties __fastcall TRemoteProperties::ChangedProperties(
  const TRemoteProperties & OriginalProperties, TRemoteProperties NewProperties)
{
  // TODO: Modification and LastAccess
  if (!NewProperties.Recursive)
  {
    if (NewProperties.Rights == OriginalProperties.Rights &&
        !NewProperties.AddXToDirectories)
    {
      NewProperties.Valid >> vpRights;
    }

    if (NewProperties.Group == OriginalProperties.Group)
    {
      NewProperties.Valid >> vpGroup;
    }

    if (NewProperties.Owner == OriginalProperties.Owner)
    {
      NewProperties.Valid >> vpOwner;
    }
  }
  return NewProperties;
}
//---------------------------------------------------------------------------
void __fastcall TRemoteProperties::Load(THierarchicalStorage * Storage)
{
  unsigned char Buf[sizeof(Valid)];
  if (static_cast<size_t>(Storage->ReadBinaryData(L"Valid", &Buf, sizeof(Buf))) == sizeof(Buf))
  {
    memmove(&Valid, Buf, sizeof(Valid));
  }

  if (Valid.Contains(vpRights))
  {
    Rights.Text = Storage->ReadString(L"Rights", Rights.Text);
  }

  // TODO
}
//---------------------------------------------------------------------------
void __fastcall TRemoteProperties::Save(THierarchicalStorage * Storage) const
{
  Storage->WriteBinaryData(UnicodeString(L"Valid"),
    static_cast<const void *>(&Valid), sizeof(Valid));

  if (Valid.Contains(vpRights))
  {
    Storage->WriteString(L"Rights", Rights.Text);
  }

  // TODO
}
