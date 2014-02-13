//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "Common.h"
#include "Exceptions.h"
#include "TextsCore.h"
#include "HelpCore.h"
#include "Configuration.h"
#include "CoreMain.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
static bool __fastcall WellKnownException(
  Exception * E, UnicodeString * AMessage, const wchar_t ** ACounterName, Exception ** AClone, bool Rethrow)
{
  UnicodeString Message;
  const wchar_t * CounterName;
  std::unique_ptr<Exception> Clone;

  bool Result = true;

  // EAccessViolation is EExternal
  if (dynamic_cast<EAccessViolation*>(E) != NULL)
  {
    if (Rethrow)
    {
      throw EAccessViolation(E->Message);
    }
    Message = MainInstructions(LoadStr(ACCESS_VIOLATION_ERROR3));
    CounterName = L"AccessViolations";
    Clone.reset(new EAccessViolation(E->Message));
  }
  // EIntError and EMathError are EExternal
  else if ((dynamic_cast<EListError*>(E) != NULL) ||
           (dynamic_cast<EStringListError*>(E) != NULL) ||
           (dynamic_cast<EIntError*>(E) != NULL) ||
           (dynamic_cast<EMathError*>(E) != NULL) ||
           (dynamic_cast<EVariantError*>(E) != NULL))
  {
    if (Rethrow)
    {
      throw EIntError(E->Message);
    }
    Message = MainInstructions(E->Message);
    CounterName = L"InternalExceptions";
    Clone.reset(new EIntError(E->Message));
  }
  else if (dynamic_cast<EExternal*>(E) != NULL)
  {
    if (Rethrow)
    {
      throw EExternal(E->Message);
    }
    Message = MainInstructions(E->Message);
    CounterName = L"ExternalExceptions";
    Clone.reset(new EExternal(E->Message));
  }
  else if (dynamic_cast<EHeapException*>(E) != NULL)
  {
    if (Rethrow)
    {
      throw EHeapException(E->Message);
    }
    Message = MainInstructions(E->Message);
    CounterName = L"HeapExceptions";
    Clone.reset(new EHeapException(E->Message));
  }
  else
  {
    Result = false;
  }

  if (Result)
  {
    if (AMessage != NULL)
    {
      (*AMessage) = Message;
    }
    if (ACounterName != NULL)
    {
      (*ACounterName) = CounterName;
    }
    if (AClone != NULL)
    {
      (*AClone) = NOT_NULL(Clone.release());
    }
  }

  return Result;
}
//---------------------------------------------------------------------------
static bool __fastcall ExceptionMessage(Exception * E, bool Count,
  bool Formatted, UnicodeString & Message, bool & InternalError)
{
  bool Result = true;
  const wchar_t * CounterName = NULL;
  InternalError = false;

  // this list has to be in sync with CloneException
  if (dynamic_cast<EAbort *>(E) != NULL)
  {
    Result = false;
  }
  else if (WellKnownException(E, &Message, &CounterName, NULL, false))
  {
    InternalError = true;
  }
  else if (E->Message.IsEmpty())
  {
    Result = false;
  }
  else
  {
    Message = E->Message;
  }

  if (!Formatted)
  {
    Message = UnformatMessage(Message);
  }

  if (InternalError)
  {
    Message = FMTLOAD(REPORT_ERROR, (Message));
  }

  if (Count && (CounterName != NULL) && (Configuration->Usage != NULL))
  {
    Configuration->Usage->Inc(CounterName);
  }

  return Result;
}
//---------------------------------------------------------------------------
bool __fastcall ExceptionMessage(Exception * E, UnicodeString & Message)
{
  bool InternalError;
  return ExceptionMessage(E, true, false, Message, InternalError);
}
//---------------------------------------------------------------------------
bool __fastcall ExceptionMessageFormatted(Exception * E, UnicodeString & Message)
{
  bool InternalError;
  return ExceptionMessage(E, true, true, Message, InternalError);
}
//---------------------------------------------------------------------------
bool __fastcall ShouldDisplayException(Exception * E)
{
  UnicodeString Message;
  return ExceptionMessageFormatted(E, Message);
}
//---------------------------------------------------------------------------
TStrings * __fastcall ExceptionToMoreMessages(Exception * E)
{
  TStrings * Result = NULL;
  UnicodeString Message;
  if (ExceptionMessage(E, Message))
  {
    Result = new TStringList();
    Result->Add(Message);
    ExtException * ExtE = dynamic_cast<ExtException *>(E);
    if ((ExtE != NULL) && (ExtE->MoreMessages != NULL))
    {
      Result->AddStrings(ExtE->MoreMessages);
    }
  }
  return Result;
}
//---------------------------------------------------------------------------
UnicodeString __fastcall GetExceptionHelpKeyword(Exception * E)
{
  UnicodeString HelpKeyword;
  ExtException * ExtE = dynamic_cast<ExtException *>(E);
  UnicodeString Message; // not used
  bool InternalError = false;
  if (ExtE != NULL)
  {
    HelpKeyword = ExtE->HelpKeyword;
  }
  else if ((E != NULL) && ExceptionMessage(E, false, false, Message, InternalError) &&
           InternalError)
  {
    HelpKeyword = HELP_INTERNAL_ERROR;
  }
  return HelpKeyword;
}
//---------------------------------------------------------------------------
UnicodeString __fastcall MergeHelpKeyword(const UnicodeString & PrimaryHelpKeyword, const UnicodeString & SecondaryHelpKeyword)
{
  if (!PrimaryHelpKeyword.IsEmpty() &&
      !IsInternalErrorHelpKeyword(SecondaryHelpKeyword))
  {
    // we have to yet decide what we have both
    // PrimaryHelpKeyword and SecondaryHelpKeyword
    return PrimaryHelpKeyword;
  }
  else
  {
    return SecondaryHelpKeyword;
  }
}
//---------------------------------------------------------------------------
bool __fastcall IsInternalErrorHelpKeyword(const UnicodeString & HelpKeyword)
{
  return
    (HelpKeyword == HELP_INTERNAL_ERROR);
}
//---------------------------------------------------------------------------
__fastcall ExtException::ExtException(Exception * E) :
  Exception("")
{
  AddMoreMessages(E);
  FHelpKeyword = GetExceptionHelpKeyword(E);
}
//---------------------------------------------------------------------------
__fastcall ExtException::ExtException(Exception* E, UnicodeString Msg, UnicodeString HelpKeyword):
  Exception(Msg)
{
  AddMoreMessages(E);
  FHelpKeyword = MergeHelpKeyword(HelpKeyword, GetExceptionHelpKeyword(E));
}
//---------------------------------------------------------------------------
__fastcall ExtException::ExtException(UnicodeString Msg, Exception* E, UnicodeString HelpKeyword) :
  Exception("")
{
  // "copy exception"
  AddMoreMessages(E);
  // and append message to the end to more messages
  if (!Msg.IsEmpty())
  {
    if (Message.IsEmpty())
    {
      Message = Msg;
    }
    else
    {
      if (FMoreMessages == NULL)
      {
        FMoreMessages = new TStringList();
      }
      FMoreMessages->Append(UnformatMessage(Msg));
    }
  }
  FHelpKeyword = MergeHelpKeyword(GetExceptionHelpKeyword(E), HelpKeyword);
}
//---------------------------------------------------------------------------
__fastcall ExtException::ExtException(UnicodeString Msg, UnicodeString MoreMessages,
    UnicodeString HelpKeyword) :
  Exception(Msg),
  FHelpKeyword(HelpKeyword)
{
  if (!MoreMessages.IsEmpty())
  {
    FMoreMessages = new TStringList();
    FMoreMessages->Text = MoreMessages;
  }
}
//---------------------------------------------------------------------------
__fastcall ExtException::ExtException(UnicodeString Msg, TStrings* MoreMessages,
  bool Own, UnicodeString HelpKeyword) :
  Exception(Msg),
  FHelpKeyword(HelpKeyword)
{
  if (Own)
  {
    FMoreMessages = MoreMessages;
  }
  else
  {
    FMoreMessages = new TStringList();
    FMoreMessages->Assign(MoreMessages);
  }
}
//---------------------------------------------------------------------------
void __fastcall ExtException::AddMoreMessages(Exception* E)
{
  if (E != NULL)
  {
    if (FMoreMessages == NULL)
    {
      FMoreMessages = new TStringList();
    }

    ExtException * ExtE = dynamic_cast<ExtException *>(E);
    if (ExtE != NULL)
    {
      if (ExtE->MoreMessages != NULL)
      {
        FMoreMessages->Assign(ExtE->MoreMessages);
      }
    }

    UnicodeString Msg;
    ExceptionMessageFormatted(E, Msg);

    // new exception does not have own message, this is in fact duplication of
    // the exception data, but the exception class may being changed
    if (Message.IsEmpty())
    {
      Message = Msg;
    }
    else if (!Msg.IsEmpty())
    {
      FMoreMessages->Insert(0, UnformatMessage(Msg));
    }

    if (FMoreMessages->Count == 0)
    {
      delete FMoreMessages;
      FMoreMessages = NULL;
    }
  }
}
//---------------------------------------------------------------------------
__fastcall ExtException::~ExtException()
{
  delete FMoreMessages;
}
//---------------------------------------------------------------------------
ExtException * __fastcall ExtException::Clone()
{
  return new ExtException(this, L"");
}
//---------------------------------------------------------------------------
UnicodeString __fastcall LastSysErrorMessage()
{
  int LastError = GetLastError();
  UnicodeString Result;
  if (LastError != 0)
  {
    Result = FORMAT(System_Sysconst_SOSError, (LastError, SysErrorMessage(LastError)));
  }
  return Result;
}
//---------------------------------------------------------------------------
__fastcall EOSExtException::EOSExtException(UnicodeString Msg) :
  ExtException(Msg, LastSysErrorMessage())
{
}
//---------------------------------------------------------------------------
__fastcall EFatal::EFatal(Exception* E, UnicodeString Msg, UnicodeString HelpKeyword) :
  ExtException(Msg, E, HelpKeyword),
  FReopenQueried(false)
{
  EFatal * F = dynamic_cast<EFatal *>(E);
  if (F != NULL)
  {
    FReopenQueried = F->ReopenQueried;
  }
}
//---------------------------------------------------------------------------
ExtException * __fastcall EFatal::Clone()
{
  return new EFatal(this, L"");
}
//---------------------------------------------------------------------------
ExtException * __fastcall ESshTerminate::Clone()
{
  return new ESshTerminate(this, L"", Operation);
}
//---------------------------------------------------------------------------
__fastcall ECallbackGuardAbort::ECallbackGuardAbort() : EAbort(L"callback abort")
{
}
//---------------------------------------------------------------------------
Exception * __fastcall CloneException(Exception * E)
{
  Exception * Result;
  // this list has to be in sync with ExceptionMessage
  ExtException * Ext = dynamic_cast<ExtException *>(E);
  if (Ext != NULL)
  {
    Result = Ext->Clone();
  }
  else if (dynamic_cast<ECallbackGuardAbort *>(E) != NULL)
  {
    Result = new ECallbackGuardAbort();
  }
  else if (dynamic_cast<EAbort *>(E) != NULL)
  {
    Result = new EAbort(E->Message);
  }
  else if (WellKnownException(E, NULL, NULL, &Result, false))
  {
    // noop
  }
  else
  {
    Result = new Exception(E->Message);
  }
  return Result;
}
//---------------------------------------------------------------------------
void __fastcall RethrowException(Exception * E)
{
   // this list has to be in sync with ExceptionMessage
 if (dynamic_cast<EFatal *>(E) != NULL)
  {
    throw EFatal(E, L"");
  }
  else if (dynamic_cast<ECallbackGuardAbort *>(E) != NULL)
  {
    throw ECallbackGuardAbort();
  }
  else if (dynamic_cast<EAbort *>(E) != NULL)
  {
    throw EAbort(E->Message);
  }
  else if (WellKnownException(E, NULL, NULL, NULL, true))
  {
    // noop, should never get here
  }
  else
  {
    throw ExtException(E, L"");
  }
}
