//---------------------------------------------------------------------------
#ifndef CopyParamH
#define CopyParamH

#include "FileMasks.h"
#include "RemoteFiles.h"
//---------------------------------------------------------------------------
// When adding new options, mind TCopyParamType::GetLogStr()
enum TOperationSide { osLocal, osRemote, osCurrent };
enum TFileNameCase { ncNoChange, ncUpperCase, ncLowerCase, ncFirstUpperCase, ncLowerCaseShort };
// TScript::OptionProc depend on the order
enum TTransferMode { tmBinary, tmAscii, tmAutomatic };
enum TResumeSupport { rsOn, rsSmart, rsOff };
class THierarchicalStorage;
const int cpaIncludeMaskOnly = 0x01;
const int cpaNoTransferMode =  0x02;
const int cpaNoIncludeMask =   0x04;
const int cpaNoClearArchive =  0x08;
const int cpaNoPreserveTime =  0x10;
const int cpaNoRights =        0x20;
const int cpaNoPreserveReadOnly = 0x40;
const int cpaNoIgnorePermErrors = 0x80;
const int cpaNoNewerOnly        = 0x100;
const int cpaNoRemoveCtrlZ      = 0x200;
const int cpaNoRemoveBOM        = 0x400;
//---------------------------------------------------------------------------
struct TUsableCopyParamAttrs
{
  int General;
  int Upload;
  int Download;
};
//---------------------------------------------------------------------------
class TCopyParamType
{
private:
  TFileMasks FAsciiFileMask;
  TFileNameCase FFileNameCase;
  bool FPreserveReadOnly;
  bool FPreserveTime;
  TRights FRights;
  TTransferMode FTransferMode;
  bool FAddXToDirectories;
  bool FPreserveRights;
  bool FIgnorePermErrors;
  TResumeSupport FResumeSupport;
  __int64 FResumeThreshold;
  UnicodeString __fastcall GetLogStr() const;
  wchar_t FInvalidCharsReplacement;
  UnicodeString FLocalInvalidChars;
  UnicodeString FTokenizibleChars;
  bool FCalculateSize;
  UnicodeString FFileMask;
  TFileMasks FIncludeFileMask;
  bool FClearArchive;
  bool FRemoveCtrlZ;
  bool FRemoveBOM;
  unsigned long FCPSLimit;
  bool FNewerOnly;
  static const wchar_t TokenPrefix = L'%';
  static const wchar_t NoReplacement = wchar_t(false);
  static const wchar_t TokenReplacement = wchar_t(true);

  void __fastcall SetLocalInvalidChars(UnicodeString value);
  bool __fastcall GetReplaceInvalidChars() const;
  void __fastcall SetReplaceInvalidChars(bool value);
  UnicodeString __fastcall RestoreChars(UnicodeString FileName) const;
  void __fastcall DoGetInfoStr(UnicodeString Separator, int Attrs,
    UnicodeString & Result, bool & SomeAttrIncluded) const;

public:
  __fastcall TCopyParamType();
  __fastcall TCopyParamType(const TCopyParamType & Source);
  virtual __fastcall ~TCopyParamType();
  TCopyParamType & __fastcall operator =(const TCopyParamType & rhp);
  virtual void __fastcall Assign(const TCopyParamType * Source);
  virtual void __fastcall Default();
  UnicodeString __fastcall ChangeFileName(UnicodeString FileName,
    TOperationSide Side, bool FirstLevel) const;
  int __fastcall LocalFileAttrs(const TRights & Rights) const;
  TRights __fastcall RemoteFileRights(int Attrs) const;
  bool __fastcall UseAsciiTransfer(UnicodeString FileName, TOperationSide Side,
    const TFileMasks::TParams & Params) const;
  bool __fastcall AllowResume(__int64 Size) const;
  UnicodeString __fastcall ValidLocalFileName(UnicodeString FileName) const;
  UnicodeString __fastcall ValidLocalPath(UnicodeString Path) const;
  bool __fastcall AllowAnyTransfer() const;
  bool __fastcall AllowTransfer(UnicodeString FileName, TOperationSide Side,
    bool Directory, const TFileMasks::TParams & Params) const;

  void __fastcall Load(THierarchicalStorage * Storage);
  void __fastcall Save(THierarchicalStorage * Storage) const;
  UnicodeString __fastcall GetInfoStr(UnicodeString Separator, int Attrs) const;
  bool __fastcall AnyUsableCopyParam(int Attrs) const;

  bool __fastcall operator==(const TCopyParamType & rhp) const;

  __property TFileMasks AsciiFileMask = { read = FAsciiFileMask, write = FAsciiFileMask };
  __property TFileNameCase FileNameCase = { read = FFileNameCase, write = FFileNameCase };
  __property bool PreserveReadOnly = { read = FPreserveReadOnly, write = FPreserveReadOnly };
  __property bool PreserveTime = { read = FPreserveTime, write = FPreserveTime };
  __property TRights Rights = { read = FRights, write = FRights };
  __property TTransferMode TransferMode = { read = FTransferMode, write = FTransferMode };
  __property UnicodeString LogStr  = { read=GetLogStr };
  __property bool AddXToDirectories  = { read=FAddXToDirectories, write=FAddXToDirectories };
  __property bool PreserveRights = { read = FPreserveRights, write = FPreserveRights };
  __property bool IgnorePermErrors = { read = FIgnorePermErrors, write = FIgnorePermErrors };
  __property TResumeSupport ResumeSupport = { read = FResumeSupport, write = FResumeSupport };
  __property __int64 ResumeThreshold = { read = FResumeThreshold, write = FResumeThreshold };
  __property wchar_t InvalidCharsReplacement = { read = FInvalidCharsReplacement, write = FInvalidCharsReplacement };
  __property bool ReplaceInvalidChars = { read = GetReplaceInvalidChars, write = SetReplaceInvalidChars };
  __property UnicodeString LocalInvalidChars = { read = FLocalInvalidChars, write = SetLocalInvalidChars };
  __property bool CalculateSize = { read = FCalculateSize, write = FCalculateSize };
  __property UnicodeString FileMask = { read = FFileMask, write = FFileMask };
  __property TFileMasks IncludeFileMask = { read = FIncludeFileMask, write = FIncludeFileMask };
  __property bool ClearArchive = { read = FClearArchive, write = FClearArchive };
  __property bool RemoveCtrlZ = { read = FRemoveCtrlZ, write = FRemoveCtrlZ };
  __property bool RemoveBOM = { read = FRemoveBOM, write = FRemoveBOM };
  __property unsigned long CPSLimit = { read = FCPSLimit, write = FCPSLimit };
  __property bool NewerOnly = { read = FNewerOnly, write = FNewerOnly };
};
//---------------------------------------------------------------------------
unsigned long __fastcall GetSpeedLimit(const UnicodeString & Text);
UnicodeString __fastcall SetSpeedLimit(unsigned long Limit);
//---------------------------------------------------------------------------
#endif
