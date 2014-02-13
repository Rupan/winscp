//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "Common.h"
#include "Exceptions.h"
#include "CopyParam.h"
#include "HierarchicalStorage.h"
#include "TextsCore.h"
//---------------------------------------------------------------------------
__fastcall TCopyParamType::TCopyParamType()
{
  Default();
}
//---------------------------------------------------------------------------
__fastcall TCopyParamType::TCopyParamType(const TCopyParamType & Source)
{
  Assign(&Source);
}
//---------------------------------------------------------------------------
__fastcall TCopyParamType::~TCopyParamType()
{
}
//---------------------------------------------------------------------------
void __fastcall TCopyParamType::Default()
{
  // when changing defaults, make sure GetInfoStr() can handle it
  FileNameCase = ncNoChange;
  PreserveReadOnly = false;
  PreserveTime = true;
  Rights.Number = TRights::rfDefault;
  PreserveRights = false; // Was True until #106
  IgnorePermErrors = false;
  AsciiFileMask.Masks = L"*.*html; *.htm; *.txt; *.php; *.php3; *.cgi; *.c; *.cpp; *.h; *.pas; "
    "*.bas; *.tex; *.pl; *.js; .htaccess; *.xtml; *.css; *.cfg; *.ini; *.sh; *.xml";
  TransferMode = tmBinary;
  AddXToDirectories = true;
  ResumeSupport = rsSmart;
  ResumeThreshold = 100 * 1024; // (100 KiB)
  InvalidCharsReplacement = TokenReplacement;
  LocalInvalidChars = ::LocalInvalidChars;
  CalculateSize = true;
  FileMask = L"*.*";
  IncludeFileMask.Masks = L"";
  ClearArchive = false;
  RemoveCtrlZ = false;
  RemoveBOM = false;
  CPSLimit = 0;
  NewerOnly = false;
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TCopyParamType::GetInfoStr(
  UnicodeString Separator, int Attrs) const
{
  UnicodeString Result;
  bool SomeAttrIncluded;
  DoGetInfoStr(Separator, Attrs, Result, SomeAttrIncluded);
  return Result;
}
//---------------------------------------------------------------------------
bool __fastcall TCopyParamType::AnyUsableCopyParam(int Attrs) const
{
  UnicodeString Result;
  bool SomeAttrIncluded;
  DoGetInfoStr(L";", Attrs, Result, SomeAttrIncluded);
  return SomeAttrIncluded;
}
//---------------------------------------------------------------------------
void __fastcall TCopyParamType::DoGetInfoStr(
  UnicodeString Separator, int Options,
  UnicodeString & Result, bool & SomeAttrIncluded) const
{
  TCopyParamType Defaults;

  bool SomeAttrExcluded = false;
  SomeAttrIncluded = false;
  #define ADD(STR, EXCEPT) \
    if (FLAGCLEAR(Options, EXCEPT)) \
    { \
      AddToList(Result, (STR), Separator); \
      SomeAttrIncluded = true; \
    } \
    else \
    { \
      SomeAttrExcluded = true; \
    }

  bool TransferModeDiffers =
    ((TransferMode != Defaults.TransferMode) ||
     ((TransferMode == tmAutomatic) && !(AsciiFileMask == Defaults.AsciiFileMask)));

  if (FLAGCLEAR(Options, cpaIncludeMaskOnly | cpaNoTransferMode))
  {
    // Adding Transfer type unconditionally
    bool FormatMask;
    int Ident;
    switch (TransferMode)
    {
      case tmBinary:
        FormatMask = false;
        Ident = 2;
        break;
      case tmAscii:
        FormatMask = false;
        Ident = 3;
        break;
      case tmAutomatic:
      default:
        FormatMask = !(AsciiFileMask == Defaults.AsciiFileMask);
        Ident = FormatMask ? 4 : 5;
        break;
    }
    UnicodeString S = FORMAT(LoadStrPart(COPY_INFO_TRANSFER_TYPE2, 1),
      (LoadStrPart(COPY_INFO_TRANSFER_TYPE2, Ident)));
    if (FormatMask)
    {
      S = FORMAT(S, (AsciiFileMask.Masks));
    }
    AddToList(Result, S, Separator);

    if (TransferModeDiffers)
    {
      ADD("", cpaIncludeMaskOnly | cpaNoTransferMode);
    }
  }
  else
  {
    if (TransferModeDiffers)
    {
      SomeAttrExcluded = true;
    }
  }

  if (FileNameCase != Defaults.FileNameCase)
  {
    ADD(FORMAT(LoadStrPart(COPY_INFO_FILENAME, 1),
      (LoadStrPart(COPY_INFO_FILENAME, FileNameCase + 2))),
      cpaIncludeMaskOnly);
  }

  if ((InvalidCharsReplacement == NoReplacement) !=
        (Defaults.InvalidCharsReplacement == NoReplacement))
  {
    assert(InvalidCharsReplacement == NoReplacement);
    if (InvalidCharsReplacement == NoReplacement)
    {
      ADD(LoadStr(COPY_INFO_DONT_REPLACE_INV_CHARS), cpaIncludeMaskOnly);
    }
  }

  if ((PreserveRights != Defaults.PreserveRights) ||
      (PreserveRights &&
       ((Rights != Defaults.Rights) || (AddXToDirectories != Defaults.AddXToDirectories))))
  {
    assert(PreserveRights);

    if (PreserveRights)
    {
      UnicodeString RightsStr = Rights.Text;
      if (AddXToDirectories)
      {
        RightsStr += L", " + LoadStr(COPY_INFO_ADD_X_TO_DIRS);
      }
      ADD(FORMAT(LoadStr(COPY_INFO_PERMISSIONS), (RightsStr)),
        cpaIncludeMaskOnly | cpaNoRights);
    }
  }

  if (PreserveTime != Defaults.PreserveTime)
  {
    ADD(LoadStr(PreserveTime ? COPY_INFO_TIMESTAMP : COPY_INFO_DONT_PRESERVE_TIME),
      cpaIncludeMaskOnly | cpaNoPreserveTime);
  }

  if ((PreserveRights || PreserveTime) &&
      (IgnorePermErrors != Defaults.IgnorePermErrors))
  {
    assert(IgnorePermErrors);

    if (IgnorePermErrors)
    {
      ADD(LoadStr(COPY_INFO_IGNORE_PERM_ERRORS),
        cpaIncludeMaskOnly | cpaNoIgnorePermErrors);
    }
  }

  if (PreserveReadOnly != Defaults.PreserveReadOnly)
  {
    assert(PreserveReadOnly);
    if (PreserveReadOnly)
    {
      ADD(LoadStr(COPY_INFO_PRESERVE_READONLY),
        cpaIncludeMaskOnly | cpaNoPreserveReadOnly);
    }
  }

  if (CalculateSize != Defaults.CalculateSize)
  {
    assert(!CalculateSize);
    if (!CalculateSize)
    {
      ADD(LoadStr(COPY_INFO_DONT_CALCULATE_SIZE), cpaIncludeMaskOnly);
    }
  }

  if (ClearArchive != Defaults.ClearArchive)
  {
    assert(ClearArchive);
    if (ClearArchive)
    {
      ADD(LoadStr(COPY_INFO_CLEAR_ARCHIVE),
        cpaIncludeMaskOnly | cpaNoClearArchive);
    }
  }

  if ((TransferMode == tmAscii) || (TransferMode == tmAutomatic))
  {
    if (RemoveBOM != Defaults.RemoveBOM)
    {
      if (ALWAYS_TRUE(RemoveBOM))
      {
        ADD(LoadStr(COPY_INFO_REMOVE_BOM),
          cpaIncludeMaskOnly | cpaNoRemoveBOM | cpaNoTransferMode);
      }
    }

    if (RemoveCtrlZ != Defaults.RemoveCtrlZ)
    {
      if (ALWAYS_TRUE(RemoveCtrlZ))
      {
        ADD(LoadStr(COPY_INFO_REMOVE_CTRLZ),
          cpaIncludeMaskOnly | cpaNoRemoveCtrlZ | cpaNoTransferMode);
      }
    }
  }

  if (!(IncludeFileMask == Defaults.IncludeFileMask))
  {
    ADD(FORMAT(LoadStr(COPY_INFO_FILE_MASK), (IncludeFileMask.Masks)),
      cpaNoIncludeMask);
  }

  if (CPSLimit > 0)
  {
    ADD(FMTLOAD(COPY_INFO_CPS_LIMIT, (int(CPSLimit / 1024))), cpaIncludeMaskOnly);
  }

  if (NewerOnly != Defaults.NewerOnly)
  {
    if (ALWAYS_TRUE(NewerOnly))
    {
      ADD(StripHotkey(LoadStr(COPY_PARAM_NEWER_ONLY)), cpaIncludeMaskOnly | cpaNoNewerOnly);
    }
  }

  if (SomeAttrExcluded)
  {
    Result += (Result.IsEmpty() ? UnicodeString() : Separator) +
      FORMAT(LoadStrPart(COPY_INFO_NOT_USABLE, 1),
        (LoadStrPart(COPY_INFO_NOT_USABLE, (SomeAttrIncluded ? 2 : 3))));
  }
  else if (Result.IsEmpty())
  {
    Result = LoadStr(COPY_INFO_DEFAULT);
  }
  #undef ADD
}
//---------------------------------------------------------------------------
void __fastcall TCopyParamType::Assign(const TCopyParamType * Source)
{
  assert(Source != NULL);
  #define COPY(Prop) Prop = Source->Prop
  COPY(FileNameCase);
  COPY(PreserveReadOnly);
  COPY(PreserveTime);
  COPY(Rights);
  COPY(AsciiFileMask);
  COPY(TransferMode);
  COPY(AddXToDirectories);
  COPY(PreserveRights);
  COPY(IgnorePermErrors);
  COPY(ResumeSupport);
  COPY(ResumeThreshold);
  COPY(InvalidCharsReplacement);
  COPY(LocalInvalidChars);
  COPY(CalculateSize);
  COPY(FileMask);
  COPY(IncludeFileMask);
  COPY(ClearArchive);
  COPY(RemoveCtrlZ);
  COPY(RemoveBOM);
  COPY(CPSLimit);
  COPY(NewerOnly);
  #undef COPY
}
//---------------------------------------------------------------------------
TCopyParamType & __fastcall TCopyParamType::operator =(const TCopyParamType & rhp)
{
  Assign(&rhp);
  return *this;
}
//---------------------------------------------------------------------------
void __fastcall TCopyParamType::SetLocalInvalidChars(UnicodeString value)
{
  if (value != LocalInvalidChars)
  {
    FLocalInvalidChars = value;
    FTokenizibleChars = FLocalInvalidChars + TokenPrefix;
  }
}
//---------------------------------------------------------------------------
bool __fastcall TCopyParamType::GetReplaceInvalidChars() const
{
  return (InvalidCharsReplacement != NoReplacement);
}
//---------------------------------------------------------------------------
void __fastcall TCopyParamType::SetReplaceInvalidChars(bool value)
{
  if (ReplaceInvalidChars != value)
  {
    InvalidCharsReplacement = (value ? TokenReplacement : NoReplacement);
  }
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TCopyParamType::ValidLocalFileName(UnicodeString FileName) const
{
  return ::ValidLocalFileName(FileName, InvalidCharsReplacement, FTokenizibleChars, LocalInvalidChars);
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TCopyParamType::RestoreChars(UnicodeString FileName) const
{
  if (InvalidCharsReplacement == TokenReplacement)
  {
    wchar_t * InvalidChar = FileName.c_str();
    while ((InvalidChar = wcschr(InvalidChar, TokenPrefix)) != NULL)
    {
      int Index = InvalidChar - FileName.c_str() + 1;
      if (FileName.Length() >= Index + 2)
      {
        UnicodeString Hex = FileName.SubString(Index + 1, 2);
        wchar_t Char = static_cast<wchar_t>(HexToByte(Hex));
        if ((Char != L'\0') &&
            ((FTokenizibleChars.Pos(Char) > 0) ||
             (((Char == L' ') || (Char == L'.')) && (Index == FileName.Length() - 2))))
        {
          FileName[Index] = Char;
          FileName.Delete(Index + 1, 2);
          InvalidChar = FileName.c_str() + Index;
        }
        else if ((Hex == L"00") &&
                 ((Index == FileName.Length() - 2) || (FileName[Index + 3] == L'.')) &&
                 IsReservedName(FileName.SubString(1, Index - 1) + FileName.SubString(Index + 3, FileName.Length() - Index - 3 + 1)))
        {
          FileName.Delete(Index, 3);
          InvalidChar = FileName.c_str() + Index - 1;
        }
        else
        {
          InvalidChar++;
        }
      }
      else
      {
        InvalidChar++;
      }
    }
  }
  return FileName;
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TCopyParamType::ValidLocalPath(UnicodeString Path) const
{
  UnicodeString Result;
  while (!Path.IsEmpty())
  {
    if (!Result.IsEmpty())
    {
      Result += L"\\";
    }
    Result += ValidLocalFileName(CutToChar(Path, L'\\', false));
  }
  return Result;
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TCopyParamType::ChangeFileName(UnicodeString FileName,
  TOperationSide Side, bool FirstLevel) const
{
  if (FirstLevel)
  {
    FileName = MaskFileName(FileName, FileMask);
  }
  switch (FileNameCase) {
    case ncUpperCase: FileName = FileName.UpperCase(); break;
    case ncLowerCase: FileName = FileName.LowerCase(); break;
    case ncFirstUpperCase: FileName = FileName.SubString(1, 1).UpperCase() +
      FileName.SubString(2, FileName.Length()-1).LowerCase(); break;
    case ncLowerCaseShort:
      if ((FileName.Length() <= 12) && (FileName.Pos(L".") <= 9) &&
          (FileName == FileName.UpperCase()))
      {
        FileName = FileName.LowerCase();
      }
      break;
    case ncNoChange:
    default:
      /*nothing*/
      break;
  }
  if (Side == osRemote)
  {
    FileName = ValidLocalFileName(FileName);
  }
  else
  {
    FileName = RestoreChars(FileName);
  }
  return FileName;
}
//---------------------------------------------------------------------------
bool __fastcall TCopyParamType::UseAsciiTransfer(UnicodeString FileName,
  TOperationSide Side, const TFileMasks::TParams & Params) const
{
  switch (TransferMode)
  {
    case tmBinary: return false;
    case tmAscii: return true;
    case tmAutomatic: return AsciiFileMask.Matches(FileName, (Side == osLocal),
      false, &Params);
    default: assert(false); return false;
  }
}
//---------------------------------------------------------------------------
TRights __fastcall TCopyParamType::RemoteFileRights(Integer Attrs) const
{
  TRights R = Rights;
  if ((Attrs & faDirectory) && AddXToDirectories)
    R.AddExecute();
  return R;
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TCopyParamType::GetLogStr() const
{
  wchar_t CaseC[] = L"NULFS";
  wchar_t ModeC[] = L"BAM";
  wchar_t ResumeC[] = L"YSN";
  return FORMAT(
    L"  PrTime: %s; PrRO: %s; Rght: %s; PrR: %s (%s); FnCs: %s; RIC: %s; "
       "Resume: %s (%d); CalcS: %s; Mask: %s\n"
     "  TM: %s; ClAr: %s; RemEOF: %s; RemBOM: %s; CPS: %u; NewerOnly: %s; InclM: %s\n"
     "  AscM: %s\n",
    (BooleanToEngStr(PreserveTime),
     BooleanToEngStr(PreserveReadOnly),
     Rights.Text,
     BooleanToEngStr(PreserveRights),
     BooleanToEngStr(IgnorePermErrors),
     CaseC[FileNameCase],
     CharToHex(InvalidCharsReplacement),
     ResumeC[ResumeSupport],
     (int)ResumeThreshold,
     BooleanToEngStr(CalculateSize),
     FileMask,
     ModeC[TransferMode],
     BooleanToEngStr(ClearArchive),
     BooleanToEngStr(RemoveCtrlZ),
     BooleanToEngStr(RemoveBOM),
     int(CPSLimit),
     BooleanToEngStr(NewerOnly),
     IncludeFileMask.Masks,
     AsciiFileMask.Masks));
}
//---------------------------------------------------------------------------
int __fastcall TCopyParamType::LocalFileAttrs(const TRights & Rights) const
{
  int Result = 0;
  if (PreserveReadOnly && !Rights.Right[TRights::rrUserWrite])
  {
    Result |= faReadOnly;
  }
  return Result;
}
//---------------------------------------------------------------------------
bool __fastcall TCopyParamType::AllowResume(__int64 Size) const
{
  switch (ResumeSupport)
  {
    case rsOn: return true;
    case rsOff: return false;
    case rsSmart: return (Size >= ResumeThreshold);
    default: assert(false); return false;
  }
}
//---------------------------------------------------------------------------
bool __fastcall TCopyParamType::AllowAnyTransfer() const
{
  return IncludeFileMask.Masks.IsEmpty();
}
//---------------------------------------------------------------------------
bool __fastcall TCopyParamType::AllowTransfer(UnicodeString FileName,
  TOperationSide Side, bool Directory, const TFileMasks::TParams & Params) const
{
  bool Result = true;
  if (!IncludeFileMask.Masks.IsEmpty())
  {
    Result = IncludeFileMask.Matches(FileName, (Side == osLocal),
      Directory, &Params);
  }
  return Result;
}
//---------------------------------------------------------------------------
void __fastcall TCopyParamType::Load(THierarchicalStorage * Storage)
{
  AddXToDirectories = Storage->ReadBool(L"AddXToDirectories", AddXToDirectories);
  AsciiFileMask.Masks = Storage->ReadString(L"Masks", AsciiFileMask.Masks);
  FileNameCase = (TFileNameCase)Storage->ReadInteger(L"FileNameCase", FileNameCase);
  PreserveReadOnly = Storage->ReadBool(L"PreserveReadOnly", PreserveReadOnly);
  PreserveTime = Storage->ReadBool(L"PreserveTime", PreserveTime);
  PreserveRights = Storage->ReadBool(L"PreserveRights", PreserveRights);
  IgnorePermErrors = Storage->ReadBool(L"IgnorePermErrors", IgnorePermErrors);
  Rights.Text = Storage->ReadString(L"Text", Rights.Text);
  TransferMode = (TTransferMode)Storage->ReadInteger(L"TransferMode", TransferMode);
  ResumeSupport = (TResumeSupport)Storage->ReadInteger(L"ResumeSupport", ResumeSupport);
  ResumeThreshold = Storage->ReadInt64(L"ResumeThreshold", ResumeThreshold);
  InvalidCharsReplacement = (wchar_t)Storage->ReadInteger(L"ReplaceInvalidChars",
    (unsigned int)InvalidCharsReplacement);
  LocalInvalidChars = Storage->ReadString(L"LocalInvalidChars", LocalInvalidChars);
  CalculateSize = Storage->ReadBool(L"CalculateSize", CalculateSize);
  if (Storage->ValueExists(L"IncludeFileMask"))
  {
    IncludeFileMask.Masks = Storage->ReadString(L"IncludeFileMask", IncludeFileMask.Masks);
  }
  else if (Storage->ValueExists(L"ExcludeFileMask"))
  {
    UnicodeString ExcludeFileMask = Storage->ReadString(L"ExcludeFileMask", L"");
    if (!ExcludeFileMask.IsEmpty())
    {
      bool NegativeExclude = Storage->ReadBool(L"NegativeExclude", false);
      if (NegativeExclude)
      {
        IncludeFileMask.Masks = ExcludeFileMask;
      }
      // convert at least simple cases to new format
      else if (ExcludeFileMask.Pos(IncludeExcludeFileMasksDelimiter) == 0)
      {
        IncludeFileMask.Masks = UnicodeString(IncludeExcludeFileMasksDelimiter) + ExcludeFileMask;
      }
    }
  }
  ClearArchive = Storage->ReadBool(L"ClearArchive", ClearArchive);
  RemoveCtrlZ = Storage->ReadBool(L"RemoveCtrlZ", RemoveCtrlZ);
  RemoveBOM = Storage->ReadBool(L"RemoveBOM", RemoveBOM);
  CPSLimit = Storage->ReadInteger(L"CPSLimit", CPSLimit);
  NewerOnly = Storage->ReadBool(L"NewerOnly", NewerOnly);
}
//---------------------------------------------------------------------------
void __fastcall TCopyParamType::Save(THierarchicalStorage * Storage) const
{
  Storage->WriteBool(L"AddXToDirectories", AddXToDirectories);
  Storage->WriteString(L"Masks", AsciiFileMask.Masks);
  Storage->WriteInteger(L"FileNameCase", FileNameCase);
  Storage->WriteBool(L"PreserveReadOnly", PreserveReadOnly);
  Storage->WriteBool(L"PreserveTime", PreserveTime);
  Storage->WriteBool(L"PreserveRights", PreserveRights);
  Storage->WriteBool(L"IgnorePermErrors", IgnorePermErrors);
  Storage->WriteString(L"Text", Rights.Text);
  Storage->WriteInteger(L"TransferMode", TransferMode);
  Storage->WriteInteger(L"ResumeSupport", ResumeSupport);
  Storage->WriteInt64(L"ResumeThreshold", ResumeThreshold);
  Storage->WriteInteger(L"ReplaceInvalidChars", (unsigned int)InvalidCharsReplacement);
  Storage->WriteString(L"LocalInvalidChars", LocalInvalidChars);
  Storage->WriteBool(L"CalculateSize", CalculateSize);
  Storage->WriteString(L"IncludeFileMask", IncludeFileMask.Masks);
  Storage->DeleteValue(L"ExcludeFileMask"); // obsolete
  Storage->DeleteValue(L"NegativeExclude"); // obsolete
  Storage->WriteBool(L"ClearArchive", ClearArchive);
  Storage->WriteBool(L"RemoveCtrlZ", RemoveCtrlZ);
  Storage->WriteBool(L"RemoveBOM", RemoveBOM);
  Storage->WriteInteger(L"CPSLimit", CPSLimit);
  Storage->WriteBool(L"NewerOnly", NewerOnly);
}
//---------------------------------------------------------------------------
#define C(Property) (Property == rhp.Property)
bool __fastcall TCopyParamType::operator==(const TCopyParamType & rhp) const
{
  return
    C(AddXToDirectories) &&
    C(AsciiFileMask) &&
    C(FileNameCase) &&
    C(PreserveReadOnly) &&
    C(PreserveTime) &&
    C(PreserveRights) &&
    C(IgnorePermErrors) &&
    C(Rights) &&
    C(TransferMode) &&
    C(ResumeSupport) &&
    C(ResumeThreshold) &&
    C(InvalidCharsReplacement) &&
    C(LocalInvalidChars) &&
    C(CalculateSize) &&
    C(IncludeFileMask) &&
    C(ClearArchive) &&
    C(RemoveCtrlZ) &&
    C(RemoveBOM) &&
    C(CPSLimit) &&
    C(NewerOnly) &&
    true;
}
#undef C
//---------------------------------------------------------------------------
unsigned long __fastcall GetSpeedLimit(const UnicodeString & Text)
{
  unsigned long Speed;
  if (AnsiSameText(Text, LoadStr(SPEED_UNLIMITED)))
  {
    Speed = 0;
  }
  else
  {
    int SSpeed;
    if (!TryStrToInt(Text, SSpeed) ||
        (SSpeed < 0))
    {
      throw Exception(FMTLOAD(SPEED_INVALID, (Text)));
    }
    Speed = SSpeed;
  }
  return Speed * 1024;
}
//---------------------------------------------------------------------------
UnicodeString __fastcall SetSpeedLimit(unsigned long Limit)
{
  UnicodeString Text;
  if (Limit == 0)
  {
    Text = LoadStr(SPEED_UNLIMITED);
  }
  else
  {
    Text = IntToStr(int(Limit / 1024));
  }
  return Text;
}
