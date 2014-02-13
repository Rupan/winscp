//---------------------------------------------------------------------------
#ifndef TerminalManagerH
#define TerminalManagerH
//---------------------------------------------------------------------------
#include <Terminal.h>
#include <Queue.h>
#include <FileOperationProgress.h>
#include <WinInterface.h>
//---------------------------------------------------------------------------
class TCustomScpExplorerForm;
class TLogMemo;
class TTerminalQueue;
class TAuthenticateForm;
class ITaskbarList3;
//---------------------------------------------------------------------------
enum TTerminalPendingAction { tpNull, tpNone, tpReconnect, tpFree };
//---------------------------------------------------------------------------
class TManagedTerminal : public TTerminal
{
public:
  __fastcall TManagedTerminal(TSessionData * SessionData, TConfiguration * Configuration);
  virtual __fastcall ~TManagedTerminal();

  TColor Color;
  bool SynchronizeBrowsing;
  UnicodeString LocalDirectory;
  UnicodeString RemoteDirectory;
  TObject * LocalExplorerState;
  TObject * RemoteExplorerState;
  TDateTime ReopenStart;
  TDateTime DirectoryLoaded;
  TTerminalThread * TerminalThread;
};
//---------------------------------------------------------------------------
class TTerminalManager : public TTerminalList
{
public:
  static TTerminalManager * __fastcall Instance(bool ForceCreation = true);
  static void __fastcall DestroyInstance();

  static UnicodeString __fastcall ProgressTitle(TFileOperationProgressType * ProgressData);

  __fastcall TTerminalManager();
  __fastcall ~TTerminalManager();

  virtual TTerminal * __fastcall NewTerminal(TSessionData * Data);
  TTerminal * __fastcall NewTerminals(TList * DataList);
  virtual void __fastcall FreeTerminal(TTerminal * Terminal);
  void __fastcall Move(TTerminal * Source, TTerminal * Target);
  void __fastcall DisconnectActiveTerminal();
  void __fastcall ReconnectActiveTerminal();
  void __fastcall FreeActiveTerminal();
  void __fastcall CycleTerminals(bool Forward);
  static void ConnectTerminal(TTerminal * Terminal, bool Reopen);
  void __fastcall SetActiveTerminalWithAutoReconnect(TTerminal * value);
  void __fastcall UpdateAppTitle();
  bool __fastcall CanOpenInPutty();
  void __fastcall OpenInPutty();
  void __fastcall NewSession(bool FromSite);
  void __fastcall Idle();
  UnicodeString __fastcall TerminalTitle(TTerminal * Terminal);
  void __fastcall HandleException(Exception * E);
  void __fastcall SaveWorkspace(TList * DataList);
  void __fastcall QueueStatusUpdated();
  TTerminal * __fastcall FindActiveTerminalForSite(TSessionData * Data);
  TTerminalQueue * __fastcall FindQueueForTerminal(TTerminal * Terminal);

  __property TCustomScpExplorerForm * ScpExplorer = { read = FScpExplorer, write = SetScpExplorer };
  __property TTerminal * ActiveTerminal = { read = FActiveTerminal, write = SetActiveTerminal };
  __property TTerminalQueue * ActiveQueue = { read = GetActiveQueue };
  __property int ActiveTerminalIndex = { read = GetActiveTerminalIndex, write = SetActiveTerminalIndex };
  __property UnicodeString ActiveTerminalTitle = { read = GetActiveTerminalTitle };
  __property TStrings * TerminalList = { read = GetTerminalList };
  __property TLogMemo * LogMemo = { read = FLogMemo };
  __property TNotifyEvent OnLastTerminalClosed = { read = FOnLastTerminalClosed, write = FOnLastTerminalClosed };
  __property TNotifyEvent OnTerminalListChanged = { read = FOnTerminalListChanged, write = FOnTerminalListChanged };

protected:
  virtual TTerminal * __fastcall CreateTerminal(TSessionData * Data);

private:
  static TTerminalManager * FInstance;
  TCustomScpExplorerForm * FScpExplorer;
  TTerminal * FActiveTerminal;
  TLogMemo * FLogMemo;
  bool FDestroying;
  TTerminalPendingAction FTerminalPendingAction;
  TNotifyEvent FOnLastTerminalClosed;
  TNotifyEvent FOnTerminalListChanged;
  TStrings * FTerminalList;
  TList * FQueues;
  TStrings * FTerminationMessages;
  UnicodeString FProgressTitle;
  TDateTime FDirectoryReadingStart;
  TAuthenticateForm * FAuthenticateForm;
  TCriticalSection * FQueueSection;
  TTerminalQueue * FQueueWithEvent;
  TQueueEvent FQueueEvent;
  unsigned int FTaskbarButtonCreatedMessage;
  ITaskbarList3 * FTaskbarList;
  int FAuthenticating;
  void * FBusyToken;

  bool __fastcall ConnectActiveTerminalImpl(bool Reopen);
  bool __fastcall ConnectActiveTerminal();
  TTerminalQueue * __fastcall NewQueue(TTerminal * Terminal);
  void __fastcall CreateLogMemo();
  void __fastcall FreeLogMemo();
  void __fastcall SetScpExplorer(TCustomScpExplorerForm * value);
  void __fastcall DoSetActiveTerminal(TTerminal * value, bool AutoReconnect);
  void __fastcall SetActiveTerminal(TTerminal * value);
  void __fastcall SetLogMemo(TLogMemo * value);
  void __fastcall UpdateAll();
  void __fastcall ApplicationException(TObject * Sender, Exception * E);
  void __fastcall ApplicationShowHint(UnicodeString & HintStr, bool & CanShow,
    THintInfo & HintInfo);
  void __fastcall ApplicationMessage(TMsg & Msg, bool & Handled);
  void __fastcall ConfigurationChange(TObject * Sender);
  void __fastcall TerminalUpdateStatus(TTerminal * Terminal, bool Active);
  void __fastcall TerminalQueryUser(TObject * Sender,
    const UnicodeString Query, TStrings * MoreMessages, unsigned int Answers,
    const TQueryParams * Params, unsigned int & Answer, TQueryType Type, void * Arg);
  void __fastcall TerminalPromptUser(TTerminal * Terminal,
    TPromptKind Kind, UnicodeString Name, UnicodeString Instructions, TStrings * Prompt,
    TStrings * Results, bool & Result, void * Arg);
  void __fastcall TerminalDisplayBanner(TTerminal * Terminal,
    UnicodeString SessionName, const UnicodeString & Banner, bool & NeverShowAgain,
    int Options);
  void __fastcall TerminalShowExtendedException(TTerminal * Terminal,
    Exception * E, void * Arg);
  void __fastcall TerminalReadDirectoryProgress(TObject * Sender, int Progress,
    bool & Cancel);
  void __fastcall TerminalInformation(TTerminal * Terminal, const UnicodeString & Str,
    bool Status, int Phase);
  void __fastcall FreeAll();
  void __fastcall TerminalReady();
  TStrings * __fastcall GetTerminalList();
  int __fastcall GetActiveTerminalIndex();
  UnicodeString __fastcall GetActiveTerminalTitle();
  TTerminalQueue * __fastcall GetActiveQueue();
  void __fastcall SaveTerminal(TTerminal * Terminal);
  void __fastcall SetActiveTerminalIndex(int value);
  void __fastcall OperationFinished(::TFileOperation Operation, TOperationSide Side,
    bool Temp, const UnicodeString & FileName, bool Success,
    TOnceDoneOperation & OnceDoneOperation);
  void __fastcall OperationProgress(TFileOperationProgressType & ProgressData,
    TCancelStatus & Cancel);
  void __fastcall DeleteLocalFile(const UnicodeString FileName, bool Alternative);
  void __fastcall QueueEvent(TTerminalQueue * Queue, TQueueEvent Event);
  TAuthenticateForm * __fastcall MakeAuthenticateForm(TTerminal * Terminal);
  void __fastcall MasterPasswordPrompt();
  void __fastcall FileNameInputDialogInitializeRenameBaseName(
    TObject * Sender, TInputDialogData * Data);
  void __fastcall InitTaskbarButtonCreatedMessage();
  void __fastcall ReleaseTaskbarList();
  void __fastcall CreateTaskbarList();
  void __fastcall UpdateTaskbarList();
  void __fastcall AuthenticateFormCancel(TObject * Sender);
  void __fastcall DoTerminalListChanged();
  TTerminal * __fastcall DoNewTerminal(TSessionData * Data);
  static void __fastcall TerminalThreadIdle(void * Data, TObject * Sender);
  void __fastcall SetQueueConfiguration(TTerminalQueue * Queue);
  void __fastcall ApplicationModalBegin(TObject * Sender);
  void __fastcall ApplicationModalEnd(TObject * Sender);
};
//---------------------------------------------------------------------------
#endif
