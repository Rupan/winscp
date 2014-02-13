//---------------------------------------------------------------------------
#ifndef ExceptionsH
#define ExceptionsH

#include <Classes.hpp>
#include <SysUtils.hpp>
#include <SysInit.hpp>
#include <System.hpp>
//---------------------------------------------------------------------------
bool __fastcall ShouldDisplayException(Exception * E);
bool __fastcall ExceptionMessage(Exception * E, UnicodeString & Message);
bool __fastcall ExceptionMessageFormatted(Exception * E, UnicodeString & Message);
UnicodeString __fastcall LastSysErrorMessage();
TStrings * __fastcall ExceptionToMoreMessages(Exception * E);
//---------------------------------------------------------------------------
enum TOnceDoneOperation { odoIdle, odoDisconnect, odoShutDown };
//---------------------------------------------------------------------------
class ExtException : public Sysutils::Exception
{
public:
  __fastcall ExtException(Exception* E);
  __fastcall ExtException(Exception* E, UnicodeString Msg, UnicodeString HelpKeyword = L"");
  // "copy the exception", just append message to the end
  __fastcall ExtException(UnicodeString Msg, Exception* E, UnicodeString HelpKeyword = L"");
  __fastcall ExtException(UnicodeString Msg, UnicodeString MoreMessages, UnicodeString HelpKeyword = "");
  __fastcall ExtException(UnicodeString Msg, TStrings* MoreMessages, bool Own, UnicodeString HelpKeyword = "");
  __fastcall virtual ~ExtException(void);
  __property TStrings* MoreMessages = {read=FMoreMessages};
  __property UnicodeString HelpKeyword = {read=FHelpKeyword};

  inline __fastcall ExtException(const UnicodeString Msg, const TVarRec * Args, const int Args_Size) :
    Sysutils::Exception(Msg, Args, Args_Size)
  {
  }
  inline __fastcall ExtException(int Ident, const TVarRec * Args, const int Args_Size)/* overload */ :
    Sysutils::Exception(Ident, Args, Args_Size)
  {
  }
  inline __fastcall ExtException(const UnicodeString Msg, int AHelpContext) :
    Sysutils::Exception(Msg, AHelpContext)
  {
  }
  inline __fastcall ExtException(const UnicodeString Msg, const TVarRec * Args, const int Args_Size, int AHelpContext) :
    Sysutils::Exception(Msg, Args, Args_Size, AHelpContext)
  {
  }
  inline __fastcall ExtException(int Ident, int AHelpContext)/* overload */ :
    Exception(Ident, AHelpContext)
  {
  }
  inline __fastcall ExtException(PResStringRec ResStringRec, const TVarRec * Args, const int Args_Size, int AHelpContext)/* overload */ :
    Sysutils::Exception(ResStringRec, Args, Args_Size, AHelpContext)
  {
  }

  virtual ExtException * __fastcall Clone();

protected:
  void __fastcall AddMoreMessages(Exception* E);

private:
  Classes::TStrings* FMoreMessages;
  UnicodeString FHelpKeyword;
};
//---------------------------------------------------------------------------
#define DERIVE_EXT_EXCEPTION(NAME, BASE) \
  class NAME : public BASE \
  { \
  public: \
    inline __fastcall NAME(Exception* E, UnicodeString Msg, UnicodeString HelpKeyword = L"") : \
      BASE(E, Msg, HelpKeyword) \
    { \
    } \
    inline __fastcall NAME(Exception* E, int Ident) : \
      BASE(E, Ident) \
    { \
    } \
    inline __fastcall virtual ~NAME(void) \
    { \
    } \
    inline __fastcall NAME(const UnicodeString Msg, const TVarRec * Args, const int Args_Size) : \
      BASE(Msg, Args, Args_Size) \
    { \
    } \
    inline __fastcall NAME(int Ident, const TVarRec * Args, const int Args_Size) : \
      BASE(Ident, Args, Args_Size) \
    { \
    } \
    inline __fastcall NAME(const UnicodeString Msg, int AHelpContext) : \
      BASE(Msg, AHelpContext) \
    { \
    } \
    inline __fastcall NAME(const UnicodeString Msg, const TVarRec * Args, const int Args_Size, int AHelpContext) : \
      BASE(Msg, Args, Args_Size, AHelpContext) \
    { \
    } \
    inline __fastcall NAME(int Ident, int AHelpContext) : \
      BASE(Ident, AHelpContext) \
    { \
    } \
    inline __fastcall NAME(PResStringRec ResStringRec, const TVarRec * Args, const int Args_Size, int AHelpContext) : \
      BASE(ResStringRec, Args, Args_Size, AHelpContext) \
    { \
    } \
    virtual ExtException * __fastcall Clone() \
    { \
      return new NAME(this, L""); \
    } \
  };
//---------------------------------------------------------------------------
DERIVE_EXT_EXCEPTION(ESsh, ExtException);
DERIVE_EXT_EXCEPTION(ETerminal, ExtException);
DERIVE_EXT_EXCEPTION(ECommand, ExtException);
DERIVE_EXT_EXCEPTION(EScp, ExtException); // SCP protocol fatal error (non-fatal in application context)
DERIVE_EXT_EXCEPTION(EScpSkipFile, ExtException);
DERIVE_EXT_EXCEPTION(EScpFileSkipped, EScpSkipFile);
//---------------------------------------------------------------------------
class EOSExtException : public ExtException
{
public:
  __fastcall EOSExtException();
  __fastcall EOSExtException(UnicodeString Msg);
};
//---------------------------------------------------------------------------
class EFatal : public ExtException
{
public:
  // fatal errors are always copied, new message is only appended
  __fastcall EFatal(Exception* E, UnicodeString Msg, UnicodeString HelpKeyword = "");

  __property bool ReopenQueried = { read = FReopenQueried, write = FReopenQueried };

  virtual ExtException * __fastcall Clone();

private:
  bool FReopenQueried;
};
//---------------------------------------------------------------------------
#define DERIVE_FATAL_EXCEPTION(NAME, BASE) \
  class NAME : public BASE \
  { \
  public: \
    inline __fastcall NAME(Exception* E, UnicodeString Msg, UnicodeString HelpKeyword = "") : \
      BASE(E, Msg, HelpKeyword) \
    { \
    } \
    virtual ExtException * __fastcall Clone() \
    { \
      return new NAME(this, L""); \
    } \
  };
//---------------------------------------------------------------------------
DERIVE_FATAL_EXCEPTION(ESshFatal, EFatal);
//---------------------------------------------------------------------------
// exception that closes application, but displays info message (not error message)
// = close on completion
class ESshTerminate : public EFatal
{
public:
  inline __fastcall ESshTerminate(Exception* E, UnicodeString Msg, TOnceDoneOperation AOperation) :
    EFatal(E, Msg),
    Operation(AOperation)
  {
  }

  virtual ExtException * __fastcall Clone();

  TOnceDoneOperation Operation;
};
//---------------------------------------------------------------------------
class ECallbackGuardAbort : public EAbort
{
public:
  __fastcall ECallbackGuardAbort();
};
//---------------------------------------------------------------------------
Exception * __fastcall CloneException(Exception * Exception);
void __fastcall RethrowException(Exception * E);
UnicodeString __fastcall GetExceptionHelpKeyword(Exception * E);
UnicodeString __fastcall MergeHelpKeyword(const UnicodeString & PrimaryHelpKeyword, const UnicodeString & SecondaryHelpKeyword);
bool __fastcall IsInternalErrorHelpKeyword(const UnicodeString & HelpKeyword);
//---------------------------------------------------------------------------
#endif  // Exceptions
