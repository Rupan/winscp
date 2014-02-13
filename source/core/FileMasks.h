//---------------------------------------------------------------------------
#ifndef FileMasksH
#define FileMasksH
//---------------------------------------------------------------------------
#include <vector>
#include <Masks.hpp>
//---------------------------------------------------------------------------
class EFileMasksException : public Exception
{
public:
  __fastcall EFileMasksException(UnicodeString Message, int ErrorStart, int ErrorLen);
  int ErrorStart;
  int ErrorLen;
};
//---------------------------------------------------------------------------
extern const wchar_t IncludeExcludeFileMasksDelimiter;
#define MASK_INDEX(DIRECTORY, INCLUDE) ((DIRECTORY ? 2 : 0) + (INCLUDE ? 0 : 1))
//---------------------------------------------------------------------------
class TFileMasks
{
public:
  struct TParams
  {
    TParams();
    __int64 Size;
    TDateTime Modification;

    UnicodeString ToString() const;
  };

  static bool __fastcall IsMask(const UnicodeString Mask);
  static UnicodeString __fastcall NormalizeMask(const UnicodeString & Mask, const UnicodeString & AnyMask = "");
  static UnicodeString __fastcall ComposeMaskStr(
    TStrings * IncludeFileMasksStr, TStrings * ExcludeFileMasksStr,
    TStrings * IncludeDirectoryMasksStr, TStrings * ExcludeDirectoryMasksStr);
  static UnicodeString __fastcall ComposeMaskStr(TStrings * MasksStr, bool Directory);

  __fastcall TFileMasks();
  __fastcall TFileMasks(int ForceDirectoryMasks);
  __fastcall TFileMasks(const TFileMasks & Source);
  __fastcall TFileMasks(const UnicodeString & AMasks);
  __fastcall ~TFileMasks();
  TFileMasks & __fastcall operator =(const TFileMasks & rhm);
  TFileMasks & __fastcall operator =(const UnicodeString & rhs);
  bool __fastcall operator ==(const TFileMasks & rhm) const;
  bool __fastcall operator ==(const UnicodeString & rhs) const;

  void __fastcall SetMask(const UnicodeString & Mask);

  bool __fastcall Matches(const UnicodeString FileName, bool Directory = false,
    const UnicodeString Path = "", const TParams * Params = NULL) const;
  bool __fastcall Matches(const UnicodeString FileName, bool Directory,
    const UnicodeString Path, const TParams * Params,
    bool & ImplicitMatch) const;
  bool __fastcall Matches(const UnicodeString FileName, bool Local, bool Directory,
    const TParams * Params = NULL) const;
  bool __fastcall Matches(const UnicodeString FileName, bool Local, bool Directory,
    const TParams * Params, bool & ImplicitMatch) const;

  __property UnicodeString Masks = { read = FStr, write = SetMasks };

  __property TStrings * IncludeFileMasksStr = { read = GetMasksStr, index = MASK_INDEX(false, true) };
  __property TStrings * ExcludeFileMasksStr = { read = GetMasksStr, index = MASK_INDEX(false, false) };
  __property TStrings * IncludeDirectoryMasksStr = { read = GetMasksStr, index = MASK_INDEX(true, true) };
  __property TStrings * ExcludeDirectoryMasksStr = { read = GetMasksStr, index = MASK_INDEX(true, false) };

private:
  int FForceDirectoryMasks;
  UnicodeString FStr;

  struct TMaskMask
  {
    enum { Any, NoExt, Regular } Kind;
    TMask * Mask;
  };

  struct TMask
  {
    TMaskMask FileNameMask;
    TMaskMask DirectoryMask;

    enum TMaskBoundary { None, Open, Close };

    TMaskBoundary HighSizeMask;
    __int64 HighSize;
    TMaskBoundary LowSizeMask;
    __int64 LowSize;

    TMaskBoundary HighModificationMask;
    TDateTime HighModification;
    TMaskBoundary LowModificationMask;
    TDateTime LowModification;

    UnicodeString MaskStr;
    UnicodeString UserStr;
  };

  typedef std::vector<TMask> TMasks;
  TMasks FMasks[4];
  mutable TStrings * FMasksStr[4];

  void __fastcall SetStr(const UnicodeString value, bool SingleMask);
  void __fastcall SetMasks(const UnicodeString value);
  void __fastcall CreateMaskMask(const UnicodeString & Mask, int Start, int End,
    bool Ex, TMaskMask & MaskMask);
  void __fastcall CreateMask(const UnicodeString & MaskStr, int MaskStart,
    int MaskEnd, bool Include);
  TStrings * __fastcall GetMasksStr(int Index) const;
  static UnicodeString __fastcall MakeDirectoryMask(UnicodeString Str);
  static inline void __fastcall ReleaseMaskMask(TMaskMask & MaskMask);
  inline void __fastcall Init();
  void __fastcall DoInit(bool Delete);
  void __fastcall Clear();
  static void __fastcall Clear(TMasks & Masks);
  static void __fastcall TrimEx(UnicodeString & Str, int & Start, int & End);
  static bool __fastcall MatchesMasks(const UnicodeString FileName, bool Directory,
    const UnicodeString Path, const TParams * Params, const TMasks & Masks, bool Recurse);
  static inline bool __fastcall MatchesMaskMask(const TMaskMask & MaskMask, const UnicodeString & Str);
  static inline bool __fastcall IsAnyMask(const UnicodeString & Mask);
  void __fastcall ThrowError(int Start, int End);
};
//---------------------------------------------------------------------------
UnicodeString __fastcall MaskFileName(UnicodeString FileName, const UnicodeString Mask);
bool __fastcall IsEffectiveFileNameMask(const UnicodeString & Mask);
UnicodeString __fastcall DelimitFileNameMask(UnicodeString Mask);
//---------------------------------------------------------------------------
typedef void __fastcall (__closure * TCustomCommandPatternEvent)
  (int Index, const UnicodeString Pattern, void * Arg, UnicodeString & Replacement,
   bool & LastPass);
//---------------------------------------------------------------------------
class TCustomCommand
{
friend class TInteractiveCustomCommand;

public:
  TCustomCommand();

  UnicodeString __fastcall Complete(const UnicodeString & Command, bool LastPass);
  virtual void __fastcall Validate(const UnicodeString & Command);

protected:
  static const wchar_t NoQuote;
  static const UnicodeString Quotes;
  void __fastcall GetToken(const UnicodeString & Command,
    int Index, int & Len, wchar_t & PatternCmd);
  void __fastcall CustomValidate(const UnicodeString & Command, void * Arg);
  bool __fastcall FindPattern(const UnicodeString & Command, wchar_t PatternCmd);

  virtual void __fastcall ValidatePattern(const UnicodeString & Command,
    int Index, int Len, wchar_t PatternCmd, void * Arg);

  virtual int __fastcall PatternLen(const UnicodeString & Command, int Index) = 0;
  virtual bool __fastcall PatternReplacement(const UnicodeString & Pattern,
    UnicodeString & Replacement, bool & Delimit) = 0;
  virtual void __fastcall DelimitReplacement(UnicodeString & Replacement, wchar_t Quote);
};
//---------------------------------------------------------------------------
class TInteractiveCustomCommand : public TCustomCommand
{
public:
  TInteractiveCustomCommand(TCustomCommand * ChildCustomCommand);

protected:
  virtual void __fastcall Prompt(const UnicodeString & Prompt,
    UnicodeString & Value);
  virtual void __fastcall Execute(const UnicodeString & Command,
    UnicodeString & Value);
  virtual int __fastcall PatternLen(const UnicodeString & Command, int Index);
  virtual bool __fastcall PatternReplacement(const UnicodeString & Pattern,
    UnicodeString & Replacement, bool & Delimit);

private:
  TCustomCommand * FChildCustomCommand;
};
//---------------------------------------------------------------------------
class TTerminal;
class TSessionData;
struct TCustomCommandData
{
  __fastcall TCustomCommandData();
  __fastcall TCustomCommandData(TTerminal * Terminal);
  __fastcall TCustomCommandData(
    TSessionData * SessionData, const UnicodeString & AUserName,
    const UnicodeString & Password);

  UnicodeString HostName;
  UnicodeString UserName;
  UnicodeString Password;

private:
  void __fastcall Init(
    TSessionData * SessionData, const UnicodeString & AUserName,
    const UnicodeString & Password);
};
//---------------------------------------------------------------------------
class TFileCustomCommand : public TCustomCommand
{
public:
  TFileCustomCommand();
  TFileCustomCommand(const TCustomCommandData & Data, const UnicodeString & Path);
  TFileCustomCommand(const TCustomCommandData & Data, const UnicodeString & Path,
    const UnicodeString & FileName, const UnicodeString & FileList);

  virtual void __fastcall Validate(const UnicodeString & Command);
  virtual void __fastcall ValidatePattern(const UnicodeString & Command,
    int Index, int Len, wchar_t PatternCmd, void * Arg);

  bool __fastcall IsFileListCommand(const UnicodeString & Command);
  virtual bool __fastcall IsFileCommand(const UnicodeString & Command);
  bool __fastcall IsSiteCommand(const UnicodeString & Command);
  bool __fastcall IsPasswordCommand(const UnicodeString & Command);

protected:
  virtual int __fastcall PatternLen(const UnicodeString & Command, int Index);
  virtual bool __fastcall PatternReplacement(const UnicodeString & Pattern,
    UnicodeString & Replacement, bool & Delimit);

private:
  TCustomCommandData FData;
  UnicodeString FPath;
  UnicodeString FFileName;
  UnicodeString FFileList;
};
//---------------------------------------------------------------------------
typedef TFileCustomCommand TRemoteCustomCommand;
//---------------------------------------------------------------------------
#endif
