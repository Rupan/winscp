//---------------------------------------------------------------------------
#ifndef ScriptH
#define ScriptH
//---------------------------------------------------------------------------
#include <time.h>
#include "Option.h"
#include "Terminal.h"
#include "FileOperationProgress.h"
//---------------------------------------------------------------------------
class TTerminal;
class TScript;
class TScriptCommands;
class TStoredSessionList;
class TTerminalList;
//---------------------------------------------------------------------------
class TScriptProgress
{
public:
  TFileOperation Operation;
  TOperationSide Side;
  UnicodeString FileName;
  UnicodeString Directory;
  unsigned int OverallProgress;
  unsigned int FileProgress;
  unsigned int CPS;
};
//---------------------------------------------------------------------------
typedef void __fastcall (__closure *TScriptPrintEvent)(TScript * Script, const UnicodeString Str);
typedef void __fastcall (__closure *TScriptSynchronizeStartStop)(TScript * Script,
  const UnicodeString LocalDirectory, const UnicodeString RemoteDirectory,
  const TCopyParamType & CopyParam, int SynchronizeParams);
typedef void __fastcall (__closure *TScriptProgressEvent)(TScript * Script, const TScriptProgress & Progress);
//---------------------------------------------------------------------------
class TScriptProcParams : public TOptions
{
public:
  friend class TManagementScript;

  __fastcall TScriptProcParams(UnicodeString ParamsStr);

  __property UnicodeString ParamsStr = { read = FParamsStr };

private:
  UnicodeString FParamsStr;
};
//---------------------------------------------------------------------------
class TScript
{
public:
  enum TBatchMode { BatchOff, BatchOn, BatchAbort, BatchContinue };

  __fastcall TScript(bool LimitedOutput);
  virtual __fastcall ~TScript();

  void __fastcall Command(UnicodeString Cmd);
  void __fastcall Log(TLogLineType Type, AnsiString Str);
  void __fastcall PrintLine(const UnicodeString Str);

  void __fastcall Synchronize(const UnicodeString LocalDirectory,
    const UnicodeString RemoteDirectory, const TCopyParamType & CopyParam,
    int SynchronizeParams, TSynchronizeChecklist ** Checklist);

  __property TScriptPrintEvent OnPrint = { read = FOnPrint, write = FOnPrint };
  __property TExtendedExceptionEvent OnShowExtendedException = { read = FOnShowExtendedException, write = FOnShowExtendedException };
  __property TSynchronizeDirectory OnTerminalSynchronizeDirectory = { read = FOnTerminalSynchronizeDirectory, write = FOnTerminalSynchronizeDirectory };
  __property TScriptSynchronizeStartStop OnSynchronizeStartStop = { read = FOnSynchronizeStartStop, write = FOnSynchronizeStartStop };
  __property TScriptProgressEvent OnProgress = { read = FOnProgress, write = FOnProgress };
  __property TCopyParamType CopyParam = { read = FCopyParam, write = SetCopyParam };
  __property int SynchronizeParams = { read = FSynchronizeParams, write = SetSynchronizeParams };
  __property TBatchMode Batch = { read = FBatch };
  __property TTerminal * Terminal = { read = FTerminal };
  __property bool Groups = { read = FGroups, write = FGroups };
  __property bool WantsProgress = { read = FWantsProgress, write = FWantsProgress };

protected:
  TTerminal * FTerminal;
  TScriptCommands * FCommands;
  TScriptPrintEvent FOnPrint;
  TExtendedExceptionEvent FOnShowExtendedException;
  TSynchronizeDirectory FOnTerminalSynchronizeDirectory;
  TScriptSynchronizeStartStop FOnSynchronizeStartStop;
  TScriptProgressEvent FOnProgress;
  TCopyParamType FCopyParam;
  bool FIncludeFileMaskOptionUsed;
  TBatchMode FBatch;
  bool FConfirm;
  bool FEcho;
  int FSynchronizeParams;
  int FSynchronizeMode;
  bool FKeepingUpToDate;
  UnicodeString FSynchronizeIntro;
  bool FLimitedOutput;
  int FSessionReopenTimeout;
  bool FGroups;
  bool FWantsProgress;
  TStrings * FPendingLogLines;

  virtual void __fastcall ResetTransfer();
  virtual void __fastcall ConnectTerminal(TTerminal * ATerminal);
  bool __fastcall EnsureCommandSessionFallback(
    TFSCapability Capability, TSessionAction & Action);
  void __fastcall Print(const UnicodeString Str);
  void __fastcall CheckSession();
  void __fastcall CheckParams(TScriptProcParams * Parameters);
  void __fastcall CopyParamParams(TCopyParamType & CopyParam, TScriptProcParams * Parameters);
  void __fastcall TransferParamParams(int & Params, TScriptProcParams * Parameters);
  enum TFileListType
  {
    fltDefault =     0x00,
    fltDirectories = 0x01,
    fltQueryServer = 0x02,
    fltMask =        0x04
  };
  TStrings * __fastcall CreateFileList(TScriptProcParams * Parameters, int Start,
    int End, TFileListType ListType = fltDefault);
  TStrings * __fastcall CreateLocalFileList(TScriptProcParams * Parameters,
    int Start, int End, TFileListType ListType);
  void __fastcall FreeFileList(TStrings * FileList);
  void __fastcall LogPendingLines(TTerminal * ATerminal);

  void __fastcall HelpProc(TScriptProcParams * Parameters);
  void __fastcall CallProc(TScriptProcParams * Parameters);
  void __fastcall PwdProc(TScriptProcParams * Parameters);
  void __fastcall CdProc(TScriptProcParams * Parameters);
  void __fastcall LsProc(TScriptProcParams * Parameters);
  void __fastcall RmProc(TScriptProcParams * Parameters);
  void __fastcall RmDirProc(TScriptProcParams * Parameters);
  void __fastcall MvProc(TScriptProcParams * Parameters);
  void __fastcall ChModProc(TScriptProcParams * Parameters);
  void __fastcall LnProc(TScriptProcParams * Parameters);
  void __fastcall MkDirProc(TScriptProcParams * Parameters);
  void __fastcall GetProc(TScriptProcParams * Parameters);
  void __fastcall PutProc(TScriptProcParams * Parameters);
  void __fastcall OptionProc(TScriptProcParams * Parameters);
  void __fastcall AsciiProc(TScriptProcParams * Parameters);
  void __fastcall BinaryProc(TScriptProcParams * Parameters);
  void __fastcall SynchronizeProc(TScriptProcParams * Parameters);
  void __fastcall KeepUpToDateProc(TScriptProcParams * Parameters);
  void __fastcall EchoProc(TScriptProcParams * Parameters);
  void __fastcall StatProc(TScriptProcParams * Parameters);

  void __fastcall OptionImpl(UnicodeString OptionName, UnicodeString ValueName);
  void __fastcall SynchronizeDirectories(TScriptProcParams * Parameters,
    UnicodeString & LocalDirectory, UnicodeString & RemoteDirectory, int FirstParam);
  virtual bool __fastcall HandleExtendedException(Exception * E,
    TTerminal * Terminal = NULL);
  void __fastcall TerminalCaptureLog(const UnicodeString & AddedLine, bool StdError);
  virtual UnicodeString __fastcall GetLogCmd(const UnicodeString & FullCommand,
    const UnicodeString & Command, const UnicodeString & Params);
  void __fastcall SynchronizePreview(
    UnicodeString LocalDirectory, UnicodeString RemoteDirectory,
    TSynchronizeChecklist * Checklist);
  UnicodeString __fastcall SynchronizeFileRecord(
    const UnicodeString & RootDirectory, const TSynchronizeChecklist::TItem * Item,
    bool Local);

private:
  void __fastcall Init();
  void __fastcall SetCopyParam(const TCopyParamType & value);
  void __fastcall SetSynchronizeParams(int value);
  TTransferMode __fastcall ParseTransferModeName(UnicodeString Name);
  inline bool __fastcall IsTerminalLogging(TTerminal * ATerminal);
};
//---------------------------------------------------------------------------
typedef void __fastcall (__closure *TScriptInputEvent)(TScript * Script, const UnicodeString Prompt, UnicodeString & Str);
typedef void __fastcall (__closure *TScriptQueryCancelEvent)(TScript * Script, bool & Cancel);
typedef void __fastcall (__closure *TScriptPrintProgressEvent)(TScript * Script, bool First, const UnicodeString Str);
//---------------------------------------------------------------------------
class TManagementScript : public TScript
{
public:
  __fastcall TManagementScript(TStoredSessionList * StoredSessions, bool LimitedOutput);
  virtual __fastcall ~TManagementScript();

  void __fastcall Connect(const UnicodeString Session, TOptions * Options, bool CheckParams);
  void __fastcall ReflectSettings();

  __property TScriptInputEvent OnInput = { read = FOnInput, write = FOnInput };
  __property TScriptQueryCancelEvent OnQueryCancel = { read = FOnQueryCancel, write = FOnQueryCancel };
  __property TPromptUserEvent OnTerminalPromptUser = { read = FOnTerminalPromptUser, write = FOnTerminalPromptUser };
  __property TQueryUserEvent OnTerminalQueryUser = { read = FOnTerminalQueryUser, write = FOnTerminalQueryUser };
  __property TScriptPrintProgressEvent OnPrintProgress = { read = FOnPrintProgress, write = FOnPrintProgress };
  __property bool Continue = { read = FContinue };

protected:
  TScriptInputEvent FOnInput;
  TScriptQueryCancelEvent FOnQueryCancel;
  TPromptUserEvent FOnTerminalPromptUser;
  TQueryUserEvent FOnTerminalQueryUser;
  TScriptPrintProgressEvent FOnPrintProgress;
  TStoredSessionList * FStoredSessions;
  TTerminalList * FTerminalList;
  UnicodeString FLastProgressFile;
  UnicodeString FLastProgressMessage;
  time_t FLastProgressTime;
  time_t FLastProgressEventTime;
  bool FContinue;

  virtual void __fastcall ResetTransfer();
  void __fastcall Input(const UnicodeString Prompt, UnicodeString & Str, bool AllowEmpty);
  void __fastcall TerminalInformation(TTerminal * Terminal, const UnicodeString & Str,
    bool Status, int Phase);
  void __fastcall TerminalOperationProgress(TFileOperationProgressType & ProgressData,
    TCancelStatus & Cancel);
  void __fastcall TerminalOperationFinished(TFileOperation Operation, TOperationSide Side,
    bool Temp, const UnicodeString & FileName, Boolean Success,
    TOnceDoneOperation & OnceDoneOperation);

  void __fastcall PrintActiveSession();
  TTerminal * __fastcall FindSession(const UnicodeString Index);
  void __fastcall FreeTerminal(TTerminal * Terminal);
  void __fastcall PrintProgress(bool First, const UnicodeString Str);
  bool __fastcall QueryCancel();
  void __fastcall TerminalSynchronizeDirectory(const UnicodeString LocalDirectory,
    const UnicodeString RemoteDirectory, bool & Continue, bool Collect);
  void __fastcall DoChangeLocalDirectory(UnicodeString Directory);
  void __fastcall DoClose(TTerminal * Terminal);
  virtual bool __fastcall HandleExtendedException(Exception * E,
    TTerminal * Terminal = NULL);
  void __fastcall TerminalPromptUser(TTerminal * Terminal, TPromptKind Kind,
    UnicodeString Name, UnicodeString Instructions, TStrings * Prompts,
    TStrings * Results, bool & Result, void * Arg);
  void __fastcall TerminalInitializeLog(TObject * Sender);
  inline bool __fastcall Synchronizing();
  inline void __fastcall ShowPendingProgress();
  virtual UnicodeString __fastcall GetLogCmd(const UnicodeString & FullCommand,
    const UnicodeString & Command, const UnicodeString & Params);

  void __fastcall ExitProc(TScriptProcParams * Parameters);
  void __fastcall OpenProc(TScriptProcParams * Parameters);
  void __fastcall CloseProc(TScriptProcParams * Parameters);
  void __fastcall SessionProc(TScriptProcParams * Parameters);
  void __fastcall LPwdProc(TScriptProcParams * Parameters);
  void __fastcall LCdProc(TScriptProcParams * Parameters);
  void __fastcall LLsProc(TScriptProcParams * Parameters);
};
//---------------------------------------------------------------------------
#endif
