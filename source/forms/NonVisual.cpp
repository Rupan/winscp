//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "NonVisual.h"

#include <Common.h>
#include <CoreMain.h>
#include <TextsWin.h>
#include <Tools.h>
#include <Setup.h>

#include <Log.h>
#include <Interface.h>
#include "WinConfiguration.h"
#include "TerminalManager.h"
#include "TBX.hpp"
#include "VCLCommon.h"
#include <HistoryComboBox.hpp>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "TB2Item"
#pragma link "TBX"
#pragma link "TB2ExtItems"
#pragma link "TBXExtItems"
#pragma link "TBXToolPals"
#ifndef NO_RESOURCES
#pragma resource "*.dfm"
#endif
TNonVisualDataModule *NonVisualDataModule;
//---------------------------------------------------------------------------
#define SCPCOMMANDER ((TScpCommanderForm *)ScpExplorer)
#define UPDEX(HandleAction, Condition, OtherEnabled, OtherDisabled) if (Action == HandleAction) { \
  ((TCustomAction *)Action)->Enabled = (Condition); \
  if (((TCustomAction *)Action)->Enabled) { OtherEnabled; } else { OtherDisabled; }; \
  Handled = true; } else
#define UPDEX1(HandleAction, Condition, Other) UPDEX(HandleAction, Condition, Other, Other)
#define UPD(HandleAction, Condition) if (Action == HandleAction) { \
  ((TCustomAction *)Action)->Enabled = (Condition); Handled = true; } else
#define UPDFUNC(HandleAction, Function) if (Action == HandleAction) { Function; Handled = true; } else
#define EXE(HandleAction, Command) if (Action == HandleAction) { \
  Command; Handled = true; } else
#define UPDACT(HandleAction, Command) \
  EXE(HandleAction, ((TCustomAction *)Action)->Enabled = true; Command)
#define UPDCOMP(COMP) if (Action == COMP ## Action) { COMP ## Action->Enabled = true; \
  COMP ## Action->Checked = ScpExplorer->ComponentVisible[fc ## COMP]; Handled = true; } else
#define EXECOMP(COMP) EXE(COMP ## Action, \
  ScpExplorer->ComponentVisible[fc ## COMP] = !ScpExplorer->ComponentVisible[fc ## COMP] )
#define COLPROPS(SIDE) \
  ((TCustomDirViewColProperties*)ScpExplorer->DirView(os ## SIDE)->ColProperties)
#define UPDSORT(SIDE, PREFIX, COL) if (Action == SIDE ## SortBy ## COL ## Action) { \
  SIDE ## SortBy ## COL ## Action->Enabled = true; Handled = true; \
  SIDE ## SortBy ## COL ## Action->Checked = (COLPROPS(SIDE)->SortColumn == PREFIX ## COL); } else
#define EXESORT(SIDE, PREFIX, COL) EXE(SIDE ## SortBy ## COL ## Action, \
    if (COLPROPS(SIDE)->SortColumn == PREFIX ## COL) \
      COLPROPS(SIDE)->SortAscending = !COLPROPS(SIDE)->SortAscending; \
    else COLPROPS(SIDE)->SortColumn = PREFIX ## COL )
#define UPDSORTA(SIDE) if (Action == SIDE ## SortAscendingAction) { \
  SIDE ## SortAscendingAction->Enabled = true; Handled = true; \
  SIDE ## SortAscendingAction->Checked = COLPROPS(SIDE)->SortAscending; } else
#define EXESORTA(SIDE) EXE(SIDE ## SortAscendingAction, \
  COLPROPS(SIDE)->SortAscending = !COLPROPS(SIDE)->SortAscending; )
#define UPDSORTC(LPREFIX, LCOL, RPREFIX, RCOL) if (Action == CurrentSortBy ## RCOL ## Action) { \
  CurrentSortBy ## RCOL ## Action->Enabled = ScpExplorer->AllowedAction((TAction *)Action, aaShortCut); \
  if (CurrentSortBy ## RCOL ## Action->Enabled) { \
    if (ScpExplorer->DirView(osCurrent) == ScpExplorer->DirView(osRemote)) \
         CurrentSortBy ## RCOL ## Action->Checked = (COLPROPS(Current)->SortColumn == RPREFIX ## RCOL); \
    else CurrentSortBy ## RCOL ## Action->Checked = (COLPROPS(Current)->SortColumn == LPREFIX ## LCOL); \
  } else CurrentSortBy ## RCOL ## Action->Checked =  false; Handled = true; } else
#define EXESORTC(COL, LCOL, RCOL) \
  EXE(CurrentSortBy ## COL ## Action, \
    Integer NewSortCol = \
      ((ScpExplorer->DirView(osCurrent) == ScpExplorer->DirView(osRemote)) ? RCOL : LCOL); \
    if (COLPROPS(Current)->SortColumn == NewSortCol) \
      COLPROPS(Current)->SortAscending = !COLPROPS(Current)->SortAscending; \
    else COLPROPS(Current)->SortColumn = NewSortCol \
  )
#define UPDSHCOL(SIDE, PREFIX, COL) \
  EXE(ShowHide ## SIDE ## COL ## ColumnAction, \
    ShowHide ## SIDE ## COL ## ColumnAction->Checked = COLPROPS(SIDE)->Visible[PREFIX ## COL])
#define EXESHCOL(SIDE, PREFIX, COL) \
  EXE(ShowHide ## SIDE ## COL ## ColumnAction, \
    COLPROPS(SIDE)->Visible[PREFIX ## COL] = !COLPROPS(SIDE)->Visible[PREFIX ## COL])
//---------------------------------------------------------------------------
__fastcall TNonVisualDataModule::TNonVisualDataModule(TComponent* Owner)
        : TDataModule(Owner)
{
  FListColumn = NULL;
  FSessionIdleTimerExecuting = false;
  FBusy = 0;

  QueueSpeedComboBoxItem(QueuePopupSpeedComboBoxItem);
}
//---------------------------------------------------------------------------
__fastcall TNonVisualDataModule::~TNonVisualDataModule()
{
}
//---------------------------------------------------------------------------
void __fastcall TNonVisualDataModule::LogActionsUpdate(
      TBasicAction *Action, bool &Handled)
{
  TLogMemo * LogMemo = TTerminalManager::Instance()->LogMemo;
  bool ValidLogMemo = LogMemo && LogMemo->Parent;
  UPD(LogClearAction, ValidLogMemo && LogMemo->Lines->Count)
  // removed potentially CPU intensive check for "all selected already"
  UPD(LogSelectAllAction, ValidLogMemo && LogMemo->Lines->Count)
  UPD(LogCopyAction, ValidLogMemo && LogMemo->SelLength)

  UPD(LogPreferencesAction, true)
  ;
}
//---------------------------------------------------------------------------
void __fastcall TNonVisualDataModule::LogActionsExecute(
      TBasicAction *Action, bool &Handled)
{

  {
    TAutoNestingCounter Counter(FBusy);

    TLogMemo * LogMemo = TTerminalManager::Instance()->LogMemo;
    assert(LogMemo && LogMemo->Parent);
    EXE(LogClearAction, LogMemo->SessionLog->Clear())
    EXE(LogSelectAllAction, LogMemo->SelectAll())
    EXE(LogCopyAction, LogMemo->CopyToClipboard())

    EXE(LogPreferencesAction, PreferencesDialog(pmLogging));
    ;
  }

  DoIdle();
}
//---------------------------------------------------------------------------
void __fastcall TNonVisualDataModule::ExplorerActionsUpdate(
      TBasicAction *Action, bool &Handled)
{
  if (!ScpExplorer || !ScpExplorer->AllowedAction((TAction *)Action, aaUpdate))
  {
    ((TAction *)Action)->Enabled = false;
    Handled = true;
    return;
  }
  void * Param;
  // CURRENT DIRVIEW
  #define EnabledSelectedOperation (ScpExplorer->EnableSelectedOperation[osCurrent])
  #define EnabledFocusedOperation (ScpExplorer->EnableFocusedOperation[osCurrent])
  #define EnabledSelectedFileOperation (ScpExplorer->EnableSelectedFileOperation[osCurrent])
  #define EnabledFocusedFileOperation (ScpExplorer->EnableFocusedFileOperation[osCurrent])
  #define EnabledLocalSelectedOperation (ScpExplorer->HasDirView[osLocal] && ScpExplorer->EnableSelectedOperation[osLocal])
  #define EnabledLocalFocusedOperation (ScpExplorer->HasDirView[osLocal] && ScpExplorer->EnableFocusedOperation[osLocal])
  #define EnabledLocalSelectedFileOperation (ScpExplorer->HasDirView[osLocal] && ScpExplorer->EnableSelectedFileOperation[osLocal])
  #define EnabledRemoteSelectedOperation (ScpExplorer->EnableSelectedOperation[osRemote])
  #define EnabledRemoteFocusedOperation (ScpExplorer->EnableFocusedOperation[osRemote])
  #define EnabledRemoteSelectedFileOperation (ScpExplorer->EnableSelectedFileOperation[osRemote])
  // focused operation
  UPD(CurrentDeleteFocusedAction, EnabledFocusedOperation)
  UPD(CurrentPropertiesFocusedAction, EnabledFocusedOperation)
  UPD(CurrentEditFocusedAction, EnabledFocusedFileOperation &&
    !WinConfiguration->DisableOpenEdit)
  UPD(CurrentSystemMenuFocusedAction, EnabledFocusedOperation)
  // file operation
  UPD(CurrentRenameAction, EnabledFocusedOperation &&
    ((ScpExplorer->HasDirView[osLocal] && DirView(osLocal) == DirView(osCurrent)) ||
      ScpExplorer->Terminal->IsCapable[fcRename]))
  UPD(CurrentEditAction, EnabledSelectedFileOperation &&
    !WinConfiguration->DisableOpenEdit)
  UPD(CurrentEditAlternativeAction, EnabledSelectedFileOperation &&
    !WinConfiguration->DisableOpenEdit)
  UPD(CurrentEditInternalAction, EnabledSelectedFileOperation &&
    !WinConfiguration->DisableOpenEdit)
  UPD(CurrentEditWithAction, EnabledSelectedFileOperation &&
    !WinConfiguration->DisableOpenEdit)
  UPD(CurrentOpenAction, EnabledFocusedOperation &&
    !WinConfiguration->DisableOpenEdit &&
    !DirView(osCurrent)->ItemIsDirectory(DirView(osCurrent)->ItemFocused))
  UPDEX1(CurrentAddEditLinkContextAction, ScpExplorer->CanAddEditLink(osCurrent),
    ((TAction *)Action)->Visible = ScpExplorer->LinkFocused())
  UPD(NewLinkAction, ScpExplorer->CanAddEditLink(osCurrent))
  // selected operation
  UPD(CurrentDeleteAction, EnabledSelectedOperation)
  UPD(CurrentDeleteAlternativeAction, EnabledSelectedOperation)
  UPD(CurrentPropertiesAction, EnabledSelectedOperation)
  UPD(RemoteMoveToAction, EnabledSelectedOperation &&
    (DirView(osRemote) == DirView(osCurrent)) &&
    ScpExplorer->Terminal->IsCapable[fcRemoteMove])
  UPD(RemoteCopyToAction, EnabledSelectedOperation &&
    (DirView(osRemote) == DirView(osCurrent)))
  UPD(FileListToCommandLineAction, EnabledSelectedOperation)
  UPD(FileListToClipboardAction, EnabledSelectedOperation)
  UPD(FullFileListToClipboardAction, EnabledSelectedOperation)
  UPD(UrlToClipboardAction, EnabledSelectedOperation && (DirView(osRemote) == DirView(osCurrent)))
  UPD(FileListFromClipboardAction, IsFormatInClipboard(CF_TEXT));
  UPD(CurrentAddEditLinkAction, ScpExplorer->CanAddEditLink(osCurrent))
  // local selected operation
  UPD(LocalCopyAction, EnabledLocalSelectedOperation)
  UPD(LocalRenameAction, EnabledLocalSelectedOperation)
  UPD(LocalEditAction, EnabledLocalSelectedFileOperation && !WinConfiguration->DisableOpenEdit)
  UPD(LocalMoveAction, EnabledLocalSelectedOperation)
  UPD(LocalCreateDirAction, true)
  UPD(LocalDeleteAction, EnabledLocalSelectedOperation)
  UPD(LocalPropertiesAction, EnabledLocalSelectedOperation)
  UPD(LocalAddEditLinkAction, ScpExplorer->CanAddEditLink(osLocal))
  // local focused operation
  UPD(LocalCopyFocusedAction, EnabledLocalFocusedOperation)
  UPD(LocalMoveFocusedAction, EnabledLocalFocusedOperation)
  // remote selected operation
  UPD(RemoteCopyAction, EnabledRemoteSelectedOperation)
  UPD(RemoteRenameAction, EnabledRemoteSelectedOperation &&
    ScpExplorer->Terminal->IsCapable[fcRename])
  UPD(RemoteEditAction, EnabledRemoteSelectedFileOperation && !WinConfiguration->DisableOpenEdit)
  UPD(RemoteMoveAction, EnabledRemoteSelectedOperation)
  UPD(RemoteCreateDirAction, true)
  UPD(RemoteDeleteAction, EnabledRemoteSelectedOperation)
  UPD(RemotePropertiesAction, EnabledRemoteSelectedOperation)
  UPD(RemoteAddEditLinkAction, ScpExplorer->CanAddEditLink(osRemote))
  // remote focused operation
  UPD(RemoteCopyFocusedAction, EnabledRemoteFocusedOperation)
  UPD(RemoteMoveFocusedAction, EnabledRemoteFocusedOperation)
  UPD(RemoteMoveToFocusedAction, EnabledFocusedOperation &&
    (DirView(osRemote) == DirView(osCurrent)) &&
    ScpExplorer->Terminal->IsCapable[fcRemoteMove])
  UPD(RemoteCopyToFocusedAction, EnabledFocusedOperation &&
    DirView(osRemote) == DirView(osCurrent))
  // directory
  UPD(CurrentCreateDirAction, true)
  UPD(NewDirAction, true)
  UPD(RemoteFindFilesAction, true)
  // selection
  UPD(SelectOneAction, DirView(osCurrent)->FilesCount)
  UPD(SelectAction, DirView(osCurrent)->FilesCount)
  UPD(UnselectAction, DirView(osCurrent)->SelCount)
  UPD(SelectAllAction, DirView(osCurrent)->FilesCount)
  UPD(InvertSelectionAction, DirView(osCurrent)->FilesCount)
  UPD(ClearSelectionAction, DirView(osCurrent)->SelCount)
  UPD(RestoreSelectionAction, DirView(osCurrent)->SelectedNamesSaved)
  UPD(PasteAction, ScpExplorer->CanPasteFromClipBoard())
  UPD(LocalSelectAction, ScpExplorer->HasDirView[osLocal] && DirView(osLocal)->FilesCount)
  UPD(LocalUnselectAction, ScpExplorer->HasDirView[osLocal] && DirView(osLocal)->SelCount)
  UPD(LocalSelectAllAction, ScpExplorer->HasDirView[osLocal] && DirView(osLocal)->FilesCount)
  UPD(RemoteSelectAction, DirView(osRemote)->FilesCount)
  UPD(RemoteUnselectAction, DirView(osRemote)->SelCount)
  UPD(RemoteSelectAllAction, DirView(osRemote)->FilesCount)

  //style
  UPDACT(CurrentCycleStyleAction,
    CurrentCycleStyleAction->ImageIndex = 8 + (DirView(osCurrent)->ViewStyle + 1) % 4)
  #define STYLEACTION(Style) UPDACT(Current ## Style ## Action, \
    Current ## Style ## Action->Checked = (DirView(osCurrent)->ViewStyle == vs ## Style))
  STYLEACTION(Icon)
  STYLEACTION(SmallIcon)
  STYLEACTION(List)
  STYLEACTION(Report)
  #undef STYLEACTION

  // REMOTE+LOCAL
  // back/forward
  #define HISTORYACTION(SIDE, DIRECTION, HINTFMT, DELTA) \
    UPDEX(SIDE ## DIRECTION ## Action, (DirView(os ## SIDE)->DIRECTION ## Count > 0), \
    SIDE ## DIRECTION ## Action->Hint = FMTLOAD(HINTFMT, (DirView(os ## SIDE)->HistoryPath[DELTA])), \
    SIDE ## DIRECTION ## Action->Hint = L"")
  HISTORYACTION(Local, Back, EXPLORER_BACK_HINT, -1)
  HISTORYACTION(Local, Forward, EXPLORER_FORWARD_HINT, 1)
  HISTORYACTION(Remote, Back, EXPLORER_BACK_HINT, -1)
  HISTORYACTION(Remote, Forward, EXPLORER_FORWARD_HINT, 1)
  #undef HISTORYACTION
  #define PANEL_ACTIONS(SIDE) \
    UPD(SIDE ## ParentDirAction, !DirView(os ## SIDE)->IsRoot) \
    UPD(SIDE ## RootDirAction, !DirView(os ## SIDE)->IsRoot) \
    UPD(SIDE ## HomeDirAction, true) \
    UPD(SIDE ## RefreshAction, DirView(os ## SIDE)->DirOK) \
    UPD(SIDE ## OpenDirAction, true) \
    UPD(SIDE ## ChangePathAction, true) \
    UPD(SIDE ## AddBookmarkAction, true) \
    UPD(SIDE ## PathToClipboardAction, true) \
    UPDEX1(SIDE ## FilterAction, true, ((TAction *)Action)->Checked = !DirView(os ## SIDE)->Mask.IsEmpty())
  PANEL_ACTIONS(Local)
  PANEL_ACTIONS(Remote)
  #undef PANEL_ACTIONS
  UPD(LocalExploreDirectoryAction, true)

  // HELP
  UPD(AboutAction, true)
  UPD(HomepageAction, true)
  UPD(HistoryPageAction, true)
  UPD(TableOfContentsAction, true)
  UPD(ForumPageAction, true)
  UPD(CheckForUpdatesAction, true)
  UPDACT(ShowUpdatesAction, ShowUpdatesUpdate())
  UPD(UpdatesPreferencesAction, true)
  UPD(DonatePageAction, true)
  UPD(DownloadPageAction, true)

  // VIEW
  UPDCOMP(SessionsTabs)
  UPDCOMP(StatusBar)
  UPDCOMP(ToolBar2)
  UPDCOMP(LocalStatusBar)
  UPDCOMP(RemoteStatusBar)
  UPDCOMP(CommandLinePanel)
  UPDCOMP(RemoteTree)
  UPDCOMP(LocalTree)
  UPDCOMP(ExplorerMenuBand)
  UPDCOMP(ExplorerAddressBand)
  UPDCOMP(ExplorerToolbarBand)
  UPDCOMP(ExplorerSelectionBand)
  UPDCOMP(ExplorerSessionBand)
  UPDCOMP(ExplorerPreferencesBand)
  UPDCOMP(ExplorerSortBand)
  UPDCOMP(ExplorerUpdatesBand)
  UPDCOMP(ExplorerTransferBand)
  UPDCOMP(ExplorerCustomCommandsBand)
  UPDCOMP(CommanderMenuBand)
  UPDCOMP(CommanderSessionBand)
  UPDCOMP(CommanderPreferencesBand)
  UPDCOMP(CommanderSortBand)
  UPDCOMP(CommanderCommandsBand)
  UPDCOMP(CommanderUpdatesBand)
  UPDCOMP(CommanderTransferBand)
  UPDCOMP(CommanderCustomCommandsBand)
  UPDCOMP(CommanderLocalHistoryBand)
  UPDCOMP(CommanderLocalNavigationBand)
  UPDCOMP(CommanderLocalFileBand)
  UPDCOMP(CommanderLocalSelectionBand)
  UPDCOMP(CommanderRemoteHistoryBand)
  UPDCOMP(CommanderRemoteNavigationBand)
  UPDCOMP(CommanderRemoteFileBand)
  UPDCOMP(CommanderRemoteSelectionBand)

  UPD(GoToCommandLineAction, true)
  UPD(GoToTreeAction, true)
  UPDEX(ViewLogAction, Configuration->Logging,
    ViewLogAction->Checked = (WinConfiguration->LogView == lvWindow),
    ViewLogAction->Checked = false )
  UPDEX(ShowHiddenFilesAction, true,
    ShowHiddenFilesAction->Checked = WinConfiguration->ShowHiddenFiles, )
  UPDEX(FormatSizeBytesAction, true,
    FormatSizeBytesAction->Checked = WinConfiguration->FormatSizeBytes, )
  UPDEX(AutoReadDirectoryAfterOpAction, true,
    AutoReadDirectoryAfterOpAction->Checked = Configuration->AutoReadDirectoryAfterOp, )
  UPD(PreferencesAction, true)
  UPD(PresetsPreferencesAction, true)
  UPDEX(LockToolbarsAction, true,
    LockToolbarsAction->Checked = WinConfiguration->LockToolbars, )
  UPDEX(SelectiveToolbarTextAction, true,
    SelectiveToolbarTextAction->Checked = WinConfiguration->SelectiveToolbarText, )
  UPDCOMP(CustomCommandsBand)
  UPD(ColorMenuAction, true)

  // SORT
  UPDSORTA(Local)
  UPDSORT(Local, dv, Name)
  UPDSORT(Local, dv, Ext)
  UPDSORT(Local, dv, Size)
  UPDSORT(Local, dv, Type)
  UPDSORT(Local, dv, Changed)
  UPDSORT(Local, dv, Attr)
  UPDSORTA(Remote)
  UPDSORT(Remote, uv, Name)
  UPDSORT(Remote, uv, Ext)
  UPDSORT(Remote, uv, Size)
  UPDSORT(Remote, uv, Changed)
  UPDSORT(Remote, uv, Rights)
  UPDSORT(Remote, uv, Owner)
  UPDSORT(Remote, uv, Group)
  UPDSORT(Remote, uv, Type)
  UPDSORTA(Current)
  UPDSORTC(dv, Name, uv, Name)
  UPDSORTC(dv, Ext, uv, Ext)
  UPDSORTC(dv, Size, uv, Size)
  UPDSORTC(dv, Type, uv, Type)
  UPDSORTC(dv, Changed, uv, Changed)
  UPDSORTC(dv, Attr, uv, Rights)
  UPDSORTC(dv, Name, uv, Owner)
  UPDSORTC(dv, Name, uv, Group)
  #define COLVIEWPROPS ((TCustomDirViewColProperties*)(((TCustomDirView*)(((TListColumns*)(ListColumn->Collection))->Owner()))->ColProperties))
  UPDEX(SortColumnAscendingAction, (ListColumn != NULL), SortColumnAscendingAction->Checked =
    (COLVIEWPROPS->SortColumn == ListColumn->Index) && COLVIEWPROPS->SortAscending, /*assert(false)*/  )
  UPDEX(SortColumnDescendingAction, (ListColumn != NULL), SortColumnDescendingAction->Checked =
    (COLVIEWPROPS->SortColumn == ListColumn->Index) && !COLVIEWPROPS->SortAscending, /*assert(false)*/ )
  #undef COLVIEWPROPS

  // SHOW/HIDE COLUMN
  UPDSHCOL(Local, dv, Name)
  UPDSHCOL(Local, dv, Ext)
  UPDSHCOL(Local, dv, Size)
  UPDSHCOL(Local, dv, Type)
  UPDSHCOL(Local, dv, Changed)
  UPDSHCOL(Local, dv, Attr)
  UPDSHCOL(Remote, uv, Name)
  UPDSHCOL(Remote, uv, Ext)
  UPDSHCOL(Remote, uv, Size)
  UPDSHCOL(Remote, uv, Changed)
  UPDSHCOL(Remote, uv, Rights)
  UPDSHCOL(Remote, uv, Owner)
  UPDSHCOL(Remote, uv, Group)
  UPDSHCOL(Remote, uv, LinkTarget)
  UPDSHCOL(Remote, uv, Type)
  UPD(HideColumnAction, (ListColumn != NULL))
  UPD(BestFitColumnAction, (ListColumn != NULL))

  // SESSION
  UPD(NewSessionAction, true)
  UPD(SiteManagerAction, true)
  UPD(DuplicateSessionAction, true)
  UPD(CloseSessionAction, true)
  UPD(SavedSessionsAction2, true)
  UPD(WorkspacesAction, StoredSessions->HasAnyWorkspace())
  UPD(OpenedSessionsAction, true)
  UPD(SaveCurrentSessionAction2, true)
  UPD(SaveWorkspaceAction, true)

  // COMMAND
  UPD(CompareDirectoriesAction, true)
  UPD(SynchronizeAction, true)
  UPD(FullSynchronizeAction, true)
  UPD(ConsoleAction, true)
  UPD(PuttyAction, TTerminalManager::Instance()->CanOpenInPutty())
  UPD(SynchronizeBrowsingAction, true)
  UPD(CloseApplicationAction, true)
  UPD(FileSystemInfoAction, true)
  UPD(ClearCachesAction, (ScpExplorer->Terminal != NULL) && !ScpExplorer->Terminal->AreCachesEmpty)
  UPD(NewFileAction, !WinConfiguration->DisableOpenEdit)
  UPD(EditorListCustomizeAction, true)

  // CUSTOM COMMANDS
  UPD(CustomCommandsAction, true)
  UPD(CustomCommandsEnterAction, true)
  UPD(CustomCommandsEnterFocusedAction, true)
  UPDFUNC(CustomCommandsLastAction, CustomCommandsLastUpdate(CustomCommandsLastAction))
  UPDFUNC(CustomCommandsLastFocusedAction, CustomCommandsLastUpdate(CustomCommandsLastFocusedAction))
  UPD(CustomCommandsCustomizeAction, true)

  // QUEUE
  UPDEX(QueueEnableAction, true, ((TAction *)Action)->Checked = ScpExplorer->GetQueueEnabled(), )
  #define UPDQUEUE(OPERATION) UPD(Queue ## OPERATION ## Action, \
    ScpExplorer->AllowQueueOperation(qo ## OPERATION))
  UPDQUEUE(GoTo)
  UPDQUEUE(Preferences)
  UPDEX(QueueItemQueryAction, ScpExplorer->AllowQueueOperation(qoItemQuery),
    ((TAction *)Action)->Visible = true, ((TAction *)Action)->Visible = false)
  UPDEX(QueueItemErrorAction, ScpExplorer->AllowQueueOperation(qoItemError),
    ((TAction *)Action)->Visible = true, ((TAction *)Action)->Visible = false)
  UPDEX(QueueItemPromptAction, ScpExplorer->AllowQueueOperation(qoItemPrompt),
    ((TAction *)Action)->Visible = true, ((TAction *)Action)->Visible = false)
  UPDQUEUE(ItemDelete)
  UPDEX(QueueItemExecuteAction, ScpExplorer->AllowQueueOperation(qoItemExecute),
    ((TAction *)Action)->Visible = true, ((TAction *)Action)->Visible =
      !ScpExplorer->AllowQueueOperation(qoItemPause) &&
      !ScpExplorer->AllowQueueOperation(qoItemResume))
  UPDEX(QueueItemPauseAction, ScpExplorer->AllowQueueOperation(qoItemPause),
    ((TAction *)Action)->Visible = true, ((TAction *)Action)->Visible = false)
  UPDEX(QueueItemResumeAction, ScpExplorer->AllowQueueOperation(qoItemResume),
    ((TAction *)Action)->Visible = true, ((TAction *)Action)->Visible = false)
  UPDQUEUE(ItemUp)
  UPDQUEUE(ItemDown)
  UPDQUEUE(PauseAll)
  UPDQUEUE(ResumeAll)
  UPDQUEUE(DeleteAllDone)
  #undef UPDQUEUE
  UPDEX(QueueItemSpeedAction, ScpExplorer->AllowQueueOperation(qoItemSpeed, &Param),
    QueueItemSpeedAction->Text = SetSpeedLimit(reinterpret_cast<unsigned long>(Param)),
    QueueItemSpeedAction->Text = L"")
  UPDACT(QueueToggleShowAction,
    ((TAction *)Action)->Checked = ScpExplorer->ComponentVisible[fcQueueView])
  #define QUEUEACTION(SHOW) UPDACT(Queue ## SHOW ## Action, \
    ((TAction *)Action)->Checked = WinConfiguration->QueueView.Show == qv ## SHOW)
  QUEUEACTION(Show)
  QUEUEACTION(HideWhenEmpty)
  QUEUEACTION(Hide)
  #undef QUEUEACTION
  UPDEX1(QueueCycleOnceEmptyAction, ScpExplorer->AllowQueueOperation(qoOnceEmpty),
    QueueCycleOnceEmptyAction->ImageIndex = CurrentQueueOnceEmptyAction()->ImageIndex;
    QueueCycleOnceEmptyAction->Checked = !QueueIdleOnceEmptyAction->Checked)
  UPD(QueueIdleOnceEmptyAction, ScpExplorer->AllowQueueOperation(qoOnceEmpty))
  UPD(QueueDisconnectOnceEmptyAction, ScpExplorer->AllowQueueOperation(qoOnceEmpty))
  UPD(QueueShutDownOnceEmptyAction, ScpExplorer->AllowQueueOperation(qoOnceEmpty))
  UPDCOMP(CommanderPreferencesBand)
  UPDACT(QueueToolbarAction,
    ((TAction *)Action)->Enabled = ScpExplorer->ComponentVisible[fcQueueView];
    ((TAction *)Action)->Checked = ScpExplorer->ComponentVisible[fcQueueToolbar])
  ;
}
//---------------------------------------------------------------------------
void __fastcall TNonVisualDataModule::ExplorerActionsExecute(
      TBasicAction *Action, bool &Handled)
{
  APPLICATION_EXCEPTION_HACK_BEGIN
  {
    assert(ScpExplorer);
    if (!ScpExplorer->AllowedAction((TAction *)Action, aaExecute))
    {
      Handled = true;
      return;
    }
    ScpExplorer->BeforeAction();


    {
      TAutoNestingCounter Counter(FBusy);
      // focused operation
      EXE(CurrentDeleteFocusedAction, ScpExplorer->ExecuteFileOperationCommand(foDelete, osCurrent, true))
      EXE(CurrentPropertiesFocusedAction, ScpExplorer->ExecuteFileOperationCommand(foSetProperties, osCurrent, true))
      EXE(CurrentEditFocusedAction, ScpExplorer->ExecuteFile(osCurrent, efDefaultEditor, NULL, true, true))
      EXE(CurrentSystemMenuFocusedAction, ScpExplorer->DisplaySystemContextMenu())
      // operation
      EXE(CurrentEditAction, ScpExplorer->ExecuteFile(osCurrent, efDefaultEditor, NULL, true, false))
      EXE(CurrentEditAlternativeAction, CreateEditorListMenu(CurrentEditAlternativeAction))
      EXE(CurrentEditInternalAction, ScpExplorer->ExecuteFile(osCurrent, efInternalEditor, NULL, true, false))
      EXE(CurrentEditWithAction, ScpExplorer->ExecuteCurrentFileWith())
      EXE(CurrentOpenAction, ScpExplorer->ExecuteCurrentFile())
      EXE(CurrentAddEditLinkAction, ScpExplorer->AddEditLink(osCurrent, false))
      EXE(CurrentAddEditLinkContextAction, ScpExplorer->AddEditLink(osCurrent, false))
      EXE(NewLinkAction, ScpExplorer->AddEditLink(osCurrent, true))
      EXE(CurrentRenameAction, ScpExplorer->ExecuteFileOperationCommand(foRename, osCurrent, false))
      EXE(CurrentDeleteAction, ScpExplorer->ExecuteFileOperationCommand(foDelete, osCurrent, false))
      EXE(CurrentDeleteAlternativeAction, ScpExplorer->ExecuteFileOperationCommand(foDelete, osCurrent, false, false, (void*)true))
      EXE(CurrentPropertiesAction, ScpExplorer->ExecuteFileOperationCommand(foSetProperties, osCurrent, false))
      EXE(FileListToCommandLineAction, ScpExplorer->PanelExport(osCurrent, peFileList, pedCommandLine))
      EXE(FileListToClipboardAction, ScpExplorer->PanelExport(osCurrent, peFileList, pedClipboard))
      EXE(FullFileListToClipboardAction, ScpExplorer->PanelExport(osCurrent, peFullFileList, pedClipboard))
      EXE(UrlToClipboardAction, ScpExplorer->PanelExport(osCurrent, peUrl, pedClipboard))
      EXE(FileListFromClipboardAction, ScpExplorer->FileListFromClipboard())
      // local selected operation
      EXE(LocalCopyAction, ScpExplorer->ExecuteFileOperationCommand(foCopy, osLocal, false))
      EXE(LocalRenameAction, ScpExplorer->ExecuteFileOperationCommand(foRename, osLocal, false))
      EXE(LocalEditAction, ScpExplorer->ExecuteFile(osLocal, efDefaultEditor, NULL, true, false))
      EXE(LocalMoveAction, ScpExplorer->ExecuteFileOperationCommand(foMove, osLocal, false))
      EXE(LocalCreateDirAction, ScpExplorer->CreateDirectory(osLocal))
      EXE(LocalDeleteAction, ScpExplorer->ExecuteFileOperationCommand(foDelete, osLocal, false))
      EXE(LocalPropertiesAction, ScpExplorer->ExecuteFileOperationCommand(foSetProperties, osLocal, false))
      EXE(LocalAddEditLinkAction, ScpExplorer->AddEditLink(osLocal, false))
      // local focused operation
      EXE(LocalCopyFocusedAction, ScpExplorer->ExecuteFileOperationCommand(foCopy, osLocal, true))
      EXE(LocalMoveFocusedAction, ScpExplorer->ExecuteFileOperationCommand(foMove, osLocal, true))
      // remote selected operation
      EXE(RemoteCopyAction, ScpExplorer->ExecuteFileOperationCommand(foCopy, osRemote, false))
      EXE(RemoteRenameAction, ScpExplorer->ExecuteFileOperationCommand(foRename, osRemote, false))
      EXE(RemoteEditAction, ScpExplorer->ExecuteFile(osRemote, efDefaultEditor, NULL, true, false))
      EXE(RemoteMoveAction, ScpExplorer->ExecuteFileOperationCommand(foMove, osRemote, false))
      EXE(RemoteCreateDirAction, ScpExplorer->CreateDirectory(osRemote))
      EXE(RemoteDeleteAction, ScpExplorer->ExecuteFileOperationCommand(foDelete, osRemote, false))
      EXE(RemotePropertiesAction, ScpExplorer->ExecuteFileOperationCommand(foSetProperties, osRemote, false))
      EXE(RemoteMoveToAction, ScpExplorer->ExecuteFileOperationCommand(foRemoteMove, osCurrent, false))
      EXE(RemoteCopyToAction, ScpExplorer->ExecuteFileOperationCommand(foRemoteCopy, osCurrent, false))
      EXE(RemoteAddEditLinkAction, ScpExplorer->AddEditLink(osRemote, false))
      // remote focused operation
      EXE(RemoteCopyFocusedAction, ScpExplorer->ExecuteFileOperationCommand(foCopy, osRemote, true))
      EXE(RemoteMoveFocusedAction, ScpExplorer->ExecuteFileOperationCommand(foMove, osRemote, true))
      EXE(RemoteMoveToFocusedAction, ScpExplorer->ExecuteFileOperationCommand(foRemoteMove, osCurrent, true))
      EXE(RemoteCopyToFocusedAction, ScpExplorer->ExecuteFileOperationCommand(foRemoteCopy, osCurrent, true))
      // directory
      EXE(CurrentCreateDirAction, ScpExplorer->CreateDirectory(osCurrent))
      EXE(NewDirAction, ScpExplorer->CreateDirectory(osCurrent))
      EXE(RemoteFindFilesAction, ScpExplorer->RemoteFindFiles())
      //selection
      EXE(SelectOneAction, DirView(osCurrent)->SelectCurrentItem(DirView(osCurrent)->NortonLike))
      EXE(SelectAction, DirView(osCurrent)->DoSelectByMask(true))
      EXE(UnselectAction, DirView(osCurrent)->DoSelectByMask(false))
      EXE(SelectAllAction, DirView(osCurrent)->SelectAll(smAll))
      EXE(InvertSelectionAction, DirView(osCurrent)->SelectAll(smInvert))
      EXE(ClearSelectionAction, DirView(osCurrent)->SelectAll(smNone))
      EXE(RestoreSelectionAction, DirView(osCurrent)->RestoreSelectedNames())
      EXE(LocalSelectAction, DirView(osLocal)->DoSelectByMask(true))
      EXE(LocalUnselectAction, DirView(osLocal)->DoSelectByMask(false))
      EXE(LocalSelectAllAction, DirView(osLocal)->SelectAll(smAll))
      EXE(RemoteSelectAction, DirView(osRemote)->DoSelectByMask(true))
      EXE(RemoteUnselectAction, DirView(osRemote)->DoSelectByMask(false))
      EXE(RemoteSelectAllAction, DirView(osRemote)->SelectAll(smAll))
      EXE(PasteAction, ScpExplorer->PasteFromClipBoard())

      // style
      EXE(CurrentCycleStyleAction,
        if (DirView(osCurrent)->ViewStyle == vsReport) DirView(osCurrent)->ViewStyle = vsIcon;
          else DirView(osCurrent)->ViewStyle = (TViewStyle)(DirView(osCurrent)->ViewStyle + 1);
      )
      #define STYLEACTION(Style) EXE(Current ## Style ## Action, \
        DirView(osCurrent)->ViewStyle = vs ## Style)
      STYLEACTION(Icon)
      STYLEACTION(SmallIcon)
      STYLEACTION(List)
      STYLEACTION(Report)
      #undef STYLEACTION

      #define PANEL_ACTIONS(SIDE) \
        EXE(SIDE ## BackAction, ScpExplorer->HistoryGo(os ## SIDE, -1)) \
        EXE(SIDE ## ForwardAction, ScpExplorer->HistoryGo(os ## SIDE, 1)) \
        EXE(SIDE ## ParentDirAction, DirView(os ## SIDE)->ExecuteParentDirectory()) \
        EXE(SIDE ## RootDirAction, DirView(os ## SIDE)->ExecuteRootDirectory()) \
        EXE(SIDE ## HomeDirAction, ScpExplorer->HomeDirectory(os ## SIDE)) \
        EXE(SIDE ## RefreshAction, DirView(os ## SIDE)->ReloadDirectory()) \
        EXE(SIDE ## OpenDirAction, ScpExplorer->OpenDirectory(os ## SIDE)) \
        EXE(SIDE ## ChangePathAction, ScpExplorer->ChangePath(os ## SIDE)) \
        EXE(SIDE ## AddBookmarkAction, ScpExplorer->AddBookmark(os ## SIDE)) \
        EXE(SIDE ## PathToClipboardAction, ScpExplorer->PanelExport(os ## SIDE, pePath, pedClipboard)) \
        EXE(SIDE ## FilterAction, ScpExplorer->Filter(os ## SIDE))
      PANEL_ACTIONS(Local)
      PANEL_ACTIONS(Remote)
      #undef PANEL_ACTIONS
      EXE(LocalExploreDirectoryAction, ScpExplorer->ExploreLocalDirectory())

      //HELP
      EXE(AboutAction, DoAboutDialog(Configuration))
      EXE(HomepageAction, OpenBrowser(LoadStr(HOMEPAGE_URL)))
      EXE(HistoryPageAction, OpenBrowser(LoadStr(HISTORY_URL)))
      EXE(TableOfContentsAction, Application->HelpSystem->ShowTableOfContents())
      EXE(ForumPageAction, OpenBrowser(LoadStr(FORUM_URL)))
      EXE(CheckForUpdatesAction, CheckForUpdates(false))
      EXE(ShowUpdatesAction, CheckForUpdates(true))
      EXE(UpdatesPreferencesAction, PreferencesDialog(pmUpdates))
      EXE(DonatePageAction, OpenBrowser(LoadStr(DONATE_URL)))
      EXE(DownloadPageAction, OpenBrowser(LoadStr(DOWNLOAD_URL)))

      // VIEW
      EXECOMP(SessionsTabs)
      EXECOMP(StatusBar)
      EXECOMP(ToolBar2)
      EXECOMP(LocalStatusBar)
      EXECOMP(RemoteStatusBar)
      EXECOMP(ExplorerMenuBand)
      EXECOMP(ExplorerAddressBand)
      EXECOMP(ExplorerToolbarBand)
      EXECOMP(ExplorerSelectionBand)
      EXECOMP(ExplorerSessionBand)
      EXECOMP(ExplorerPreferencesBand)
      EXECOMP(ExplorerSortBand)
      EXECOMP(ExplorerUpdatesBand)
      EXECOMP(ExplorerTransferBand)
      EXECOMP(ExplorerCustomCommandsBand)
      EXECOMP(CommanderMenuBand)
      EXECOMP(CommanderSessionBand)
      EXECOMP(CommanderPreferencesBand)
      EXECOMP(CommanderSortBand)
      EXECOMP(CommanderCommandsBand)
      EXECOMP(CommanderUpdatesBand)
      EXECOMP(CommanderTransferBand)
      EXECOMP(CommanderCustomCommandsBand)
      EXECOMP(CommanderLocalHistoryBand)
      EXECOMP(CommanderLocalNavigationBand)
      EXECOMP(CommanderLocalFileBand)
      EXECOMP(CommanderLocalSelectionBand)
      EXECOMP(CommanderRemoteHistoryBand)
      EXECOMP(CommanderRemoteNavigationBand)
      EXECOMP(CommanderRemoteFileBand)
      EXECOMP(CommanderRemoteSelectionBand)
      EXECOMP(CommandLinePanel)
      EXECOMP(RemoteTree)
      EXECOMP(LocalTree)
      EXE(GoToCommandLineAction, ScpExplorer->GoToCommandLine())
      EXE(GoToTreeAction, ScpExplorer->GoToTree())

      EXE(ViewLogAction, WinConfiguration->LogView =
        (WinConfiguration->LogView == lvNone ? lvWindow : lvNone) )
      EXE(ShowHiddenFilesAction, ScpExplorer->ToggleShowHiddenFiles())
      EXE(FormatSizeBytesAction, ScpExplorer->ToggleFormatSizeBytes())
      EXE(AutoReadDirectoryAfterOpAction, ScpExplorer->ToggleAutoReadDirectoryAfterOp())
      EXE(PreferencesAction, PreferencesDialog(::pmDefault) )
      EXE(PresetsPreferencesAction, PreferencesDialog(pmPresets) )
      EXE(LockToolbarsAction, WinConfiguration->LockToolbars = !WinConfiguration->LockToolbars)
      EXE(SelectiveToolbarTextAction, WinConfiguration->SelectiveToolbarText = !WinConfiguration->SelectiveToolbarText)
      EXECOMP(CustomCommandsBand)
      EXE(ColorMenuAction, CreateSessionColorMenu(ColorMenuAction));

      #define COLVIEWPROPS ((TCustomDirViewColProperties*)(((TCustomDirView*)(((TListColumns*)(ListColumn->Collection))->Owner()))->ColProperties))
      // SORT
      EXESORTA(Local)
      EXESORT(Local, dv, Name)
      EXESORT(Local, dv, Ext)
      EXESORT(Local, dv, Size)
      EXESORT(Local, dv, Type)
      EXESORT(Local, dv, Changed)
      EXESORT(Local, dv, Attr)
      EXESORTA(Remote)
      EXESORT(Remote, uv, Name)
      EXESORT(Remote, uv, Ext)
      EXESORT(Remote, uv, Size)
      EXESORT(Remote, uv, Changed)
      EXESORT(Remote, uv, Rights)
      EXESORT(Remote, uv, Owner)
      EXESORT(Remote, uv, Group)
      EXESORT(Remote, uv, Type)
      EXESORTA(Current)
      EXESORTC(Name, dvName, uvName)
      EXESORTC(Ext, dvExt, uvExt)
      EXESORTC(Size, dvSize, uvSize)
      EXESORTC(Type, dvType, uvType)
      EXESORTC(Changed, dvChanged, uvChanged)
      EXESORTC(Rights, dvAttr, uvRights)
      EXESORTC(Owner, dvName, uvOwner)
      EXESORTC(Group, dvName, uvGroup)
      EXE(SortColumnAscendingAction, assert(ListColumn);
        COLVIEWPROPS->SortColumn = ListColumn->Index; COLVIEWPROPS->SortAscending = true; ListColumn = NULL )
      EXE(SortColumnDescendingAction, assert(ListColumn);
        COLVIEWPROPS->SortColumn = ListColumn->Index; COLVIEWPROPS->SortAscending = false; ListColumn = NULL )

      // SHOW/HIDE COLUMN
      EXESHCOL(Local, dv, Name)
      EXESHCOL(Local, dv, Ext)
      EXESHCOL(Local, dv, Size)
      EXESHCOL(Local, dv, Type)
      EXESHCOL(Local, dv, Changed)
      EXESHCOL(Local, dv, Attr)
      EXESHCOL(Remote, uv, Name)
      EXESHCOL(Remote, uv, Ext)
      EXESHCOL(Remote, uv, Size)
      EXESHCOL(Remote, uv, Changed)
      EXESHCOL(Remote, uv, Rights)
      EXESHCOL(Remote, uv, Owner)
      EXESHCOL(Remote, uv, Group)
      EXESHCOL(Remote, uv, LinkTarget)
      EXESHCOL(Remote, uv, Type)
      EXE(HideColumnAction, assert(ListColumn);
        COLVIEWPROPS->Visible[ListColumn->Index] = false; ListColumn = NULL )
      EXE(BestFitColumnAction, assert(ListColumn); ListColumn = NULL ) // TODO
      #undef COLVIEWPROPS

      // SESSION
      EXE(NewSessionAction, ScpExplorer->NewSession(false))
      EXE(SiteManagerAction, ScpExplorer->NewSession(true))
      EXE(DuplicateSessionAction, ScpExplorer->DuplicateSession())
      EXE(CloseSessionAction, ScpExplorer->CloseSession())
      EXE(SavedSessionsAction2, CreateSessionListMenu(SavedSessionsAction2))
      EXE(WorkspacesAction, CreateWorkspacesMenu(WorkspacesAction))
      EXE(OpenedSessionsAction, CreateOpenedSessionListMenu(OpenedSessionsAction))
      EXE(SaveCurrentSessionAction2, ScpExplorer->SaveCurrentSession())
      EXE(SaveWorkspaceAction, ScpExplorer->SaveWorkspace(false))

      // COMMAND
      EXE(CompareDirectoriesAction, ScpExplorer->CompareDirectories())
      EXE(SynchronizeAction, ScpExplorer->SynchronizeDirectories())
      EXE(FullSynchronizeAction, ScpExplorer->FullSynchronizeDirectories())
      EXE(ConsoleAction, ScpExplorer->OpenConsole())
      EXE(PuttyAction, TTerminalManager::Instance()->OpenInPutty())
      EXE(SynchronizeBrowsingAction, ScpExplorer->SynchronizeBrowsingChanged())
      EXE(CloseApplicationAction, ScpExplorer->Close())
      EXE(FileSystemInfoAction, ScpExplorer->FileSystemInfo())
      EXE(ClearCachesAction, ScpExplorer->Terminal->ClearCaches())
      EXE(NewFileAction, ScpExplorer->EditNew(osCurrent))
      EXE(EditorListCustomizeAction, PreferencesDialog(pmEditor))

      // CUSTOM COMMANDS
      EXE(CustomCommandsAction, CreateCustomCommandsMenu(CustomCommandsAction))
      EXE(CustomCommandsEnterAction, ScpExplorer->AdHocCustomCommand(false))
      EXE(CustomCommandsEnterFocusedAction, ScpExplorer->AdHocCustomCommand(true))
      EXE(CustomCommandsLastAction, ScpExplorer->LastCustomCommand(false))
      EXE(CustomCommandsLastFocusedAction, ScpExplorer->LastCustomCommand(true))
      EXE(CustomCommandsCustomizeAction, PreferencesDialog(pmCustomCommands))

      // QUEUE
      EXE(QueueEnableAction, ScpExplorer->ToggleQueueEnabled())
      #define EXEQUEUE(OPERATION) EXE(Queue ## OPERATION ## Action, \
        ScpExplorer->ExecuteQueueOperation(qo ## OPERATION))
      EXEQUEUE(GoTo)
      EXEQUEUE(Preferences)
      EXEQUEUE(ItemQuery)
      EXEQUEUE(ItemError)
      EXEQUEUE(ItemPrompt)
      EXEQUEUE(ItemDelete)
      EXEQUEUE(ItemExecute)
      EXEQUEUE(ItemPause)
      EXEQUEUE(ItemResume)
      EXEQUEUE(ItemUp)
      EXEQUEUE(ItemDown)
      EXEQUEUE(PauseAll)
      EXEQUEUE(ResumeAll)
      EXEQUEUE(DeleteAllDone)
      #undef EXEQUEUE
      EXE(QueueToggleShowAction, ScpExplorer->ToggleQueueVisibility())
      #define QUEUEACTION(SHOW) EXE(Queue ## SHOW ## Action, \
        TQueueViewConfiguration Config = WinConfiguration->QueueView; \
        if (Config.Show != qvShow) Config.LastHideShow = Config.Show; \
        Config.Show = qv ## SHOW; \
        WinConfiguration->QueueView = Config)
      QUEUEACTION(Show)
      QUEUEACTION(HideWhenEmpty)
      QUEUEACTION(Hide)
      #undef QUEUEACTION
      EXE(QueueCycleOnceEmptyAction, CycleQueueOnceEmptyAction());
      EXE(QueueIdleOnceEmptyAction, SetQueueOnceEmptyAction(QueueIdleOnceEmptyAction))
      EXE(QueueDisconnectOnceEmptyAction, SetQueueOnceEmptyAction(QueueDisconnectOnceEmptyAction))
      EXE(QueueShutDownOnceEmptyAction, SetQueueOnceEmptyAction(QueueShutDownOnceEmptyAction))
      EXECOMP(QueueToolbar);
      EXE(QueueItemSpeedAction, )
      ;
    }

    DoIdle();
  }
  APPLICATION_EXCEPTION_HACK_END;
}
//---------------------------------------------------------------------------
void __fastcall TNonVisualDataModule::UpdateNonVisibleActions()
{
  // following actions needs to be updated even when all clients
  // are invisible, so the queue list toolbar button can be shown
  NonVisualDataModule->QueueItemQueryAction->Update();
  NonVisualDataModule->QueueItemErrorAction->Update();
  NonVisualDataModule->QueueItemPromptAction->Update();
}
//---------------------------------------------------------------------------
#define CTRL TShiftState() << ssCtrl
#define ALT TShiftState() << ssAlt
#define SHIFT TShiftState() << ssShift
#define CTRLSHIFT TShiftState() << ssCtrl << ssShift
#define CTRLALT TShiftState() << ssCtrl << ssAlt
#define NONE TShiftState()
void __fastcall TNonVisualDataModule::ExplorerShortcuts()
{
  // Directory
  CurrentCreateDirAction->ShortCut = ShortCut(L'D', CTRL);
  // File operation
  CurrentRenameAction->ShortCut = ShortCut(VK_F2, NONE);
  CurrentEditAction->ShortCut = ShortCut(L'E', CTRL);
  CurrentAddEditLinkAction->ShortCut = ShortCut(L'L', CTRLALT);
  CurrentEditInternalAction->ShortCut = 0;
  // Focused operation
  RemoteCopyAction->ShortCut = ShortCut(L'C', CTRL);
  RemoteMoveAction->ShortCut = ShortCut(L'M', CTRL);
  CurrentDeleteFocusedAction->ShortCut = ShortCut(VK_DELETE, NONE);
  CurrentPropertiesFocusedAction->ShortCut = ShortCut(VK_RETURN, ALT);
  RemoteMoveToFocusedAction->ShortCut = ShortCut(L'M', CTRLALT);
  // remote directory
  RemoteOpenDirAction->ShortCut = ShortCut(L'O', CTRL);
  RemoteRefreshAction->ShortCut = ShortCut(VK_F5, NONE);
  RemoteHomeDirAction->ShortCut = ShortCut(L'H', CTRL);
  RemotePathToClipboardAction->ShortCut = ShortCut(L'P', CTRLSHIFT);
  // selected operation
  CurrentDeleteAlternativeAction->ShortCut = ShortCut(VK_DELETE, SHIFT);
  RemoteMoveToAction->ShortCut = ShortCut(L'M', CTRLALT);
  // selection
  SelectAction->ShortCut = ShortCut(VK_ADD, NONE);
  UnselectAction->ShortCut = ShortCut(VK_SUBTRACT, NONE);
  SelectAllAction->ShortCut = ShortCut(L'A', CTRL);
  InvertSelectionAction->ShortCut = ShortCut(VK_MULTIPLY, NONE);
  ClearSelectionAction->ShortCut = ShortCut(L'L', CTRL);
  RestoreSelectionAction->ShortCut = ShortCut(L'R', CTRLSHIFT);
  // commands
  NewFileAction->ShortCut = ShortCut(L'E', CTRLSHIFT);
  RemoteFindFilesAction->ShortCut = ShortCut(VK_F3, NONE);

  CloseApplicationAction->ShortCut = ShortCut(VK_F4, ALT);

  CloneShortcuts();
}
//---------------------------------------------------------------------------
void __fastcall TNonVisualDataModule::CommanderShortcuts()
{
  bool ExplorerKeyboardShortcuts = WinConfiguration->ScpCommander.ExplorerKeyboardShortcuts;
  // Directory
  CurrentCreateDirAction->ShortCut = ShortCut(VK_F7, NONE);
  // File operation
  CurrentRenameAction->ShortCut = ShortCut(VK_F2, NONE);
  CurrentEditAction->ShortCut = ShortCut(VK_F4, NONE);
  CurrentAddEditLinkAction->ShortCut = ShortCut(VK_F6, ALT);
  CurrentEditInternalAction->ShortCut = ShortCut(VK_F4, CTRLALT);
  // Focused operation
  RemoteCopyAction->ShortCut =
    ExplorerKeyboardShortcuts ? ShortCut(L'K', CTRL) : ShortCut(VK_F5, NONE);
  RemoteMoveAction->ShortCut = ShortCut(VK_F6, NONE);
  CurrentDeleteFocusedAction->ShortCut = ShortCut(VK_F8, NONE);
  CurrentPropertiesFocusedAction->ShortCut = ShortCut(VK_F9, NONE);
  RemoteMoveToFocusedAction->ShortCut = ShortCut(VK_F6, SHIFT);
  RemoteCopyToFocusedAction->ShortCut = ShortCut(VK_F5, SHIFT);
  // remote directory
  RemoteOpenDirAction->ShortCut = ShortCut(L'O', CTRL);
  RemoteRefreshAction->ShortCut =
    ExplorerKeyboardShortcuts ? ShortCut(VK_F5, NONE) : ShortCut(L'R', CTRL);
  RemoteHomeDirAction->ShortCut = ShortCut(L'H', CTRL);
  RemotePathToClipboardAction->ShortCut = ShortCut(VK_OEM_6 /* ] */, CTRL);
  // local directory
  LocalPathToClipboardAction->ShortCut = ShortCut(VK_OEM_4 /* [ */, CTRL);
  // selected operation
  CurrentDeleteAction->SecondaryShortCuts->Clear();
  CurrentDeleteAction->SecondaryShortCuts->Add(ShortCutToText(ShortCut(VK_DELETE, NONE)));
  CurrentDeleteAlternativeAction->ShortCut = ShortCut(VK_F8, SHIFT);
  CurrentDeleteAlternativeAction->SecondaryShortCuts->Clear();
  CurrentDeleteAlternativeAction->SecondaryShortCuts->Add(ShortCutToText(ShortCut(VK_DELETE, SHIFT)));
  RemoteMoveToAction->ShortCut = ShortCut(VK_F6, SHIFT);
  RemoteCopyToAction->ShortCut = ShortCut(VK_F5, SHIFT);
  // selection
  SelectOneAction->ShortCut = VK_INSERT;
  SelectAction->ShortCut = ShortCut(VK_ADD, NONE);
  UnselectAction->ShortCut = ShortCut(VK_SUBTRACT, NONE);
  SelectAllAction->ShortCut = ShortCut(L'A', CTRL);
  InvertSelectionAction->ShortCut = ShortCut(VK_MULTIPLY, NONE);
  ClearSelectionAction->ShortCut = ShortCut(L'L', CTRL);
  RestoreSelectionAction->ShortCut = ShortCut(L'R', CTRLSHIFT);
  // commands
  NewFileAction->ShortCut = ShortCut(VK_F4, SHIFT);
  RemoteFindFilesAction->ShortCut =
    ExplorerKeyboardShortcuts ? ShortCut(VK_F3, NONE) : ShortCut(VK_F7, ALT);
  // legacy shortcut (can be removed when necessary)
  NewFileAction->SecondaryShortCuts->Add(ShortCutToText(ShortCut(VK_F4, CTRLSHIFT)));

  CloseApplicationAction->ShortCut = ShortCut(VK_F10, NONE);

  CloneShortcuts();
}
#undef CTRL
#undef ALT
#undef NONE
//---------------------------------------------------------------------------
void __fastcall TNonVisualDataModule::CloneShortcuts()
{
  // Directory
  NewDirAction->ShortCut = CurrentCreateDirAction->ShortCut;
  // File operation
  CurrentAddEditLinkContextAction->ShortCut = CurrentAddEditLinkAction->ShortCut;
  LocalAddEditLinkAction->ShortCut = CurrentAddEditLinkAction->ShortCut;
  RemoteAddEditLinkAction->ShortCut = CurrentAddEditLinkAction->ShortCut;
  // local directory
  LocalOpenDirAction->ShortCut = RemoteOpenDirAction->ShortCut;
  LocalRefreshAction->ShortCut = RemoteRefreshAction->ShortCut;
  LocalHomeDirAction->ShortCut = RemoteHomeDirAction->ShortCut;
  // selected operation
  CurrentDeleteAction->ShortCut = CurrentDeleteFocusedAction->ShortCut;
  CurrentPropertiesAction->ShortCut = CurrentPropertiesFocusedAction->ShortCut;
  // local selected operation
  LocalCopyAction->ShortCut = RemoteCopyAction->ShortCut;
  LocalRenameAction->ShortCut = CurrentRenameAction->ShortCut;
  LocalEditAction->ShortCut = CurrentEditAction->ShortCut;
  LocalMoveAction->ShortCut = RemoteMoveAction->ShortCut;
  LocalCreateDirAction->ShortCut = CurrentCreateDirAction->ShortCut;
  LocalDeleteAction->ShortCut = CurrentDeleteAction->ShortCut;
  LocalPropertiesAction->ShortCut = CurrentPropertiesAction->ShortCut;
  // local focused operation
  LocalCopyFocusedAction->ShortCut = LocalCopyAction->ShortCut;
  LocalMoveFocusedAction->ShortCut = LocalMoveAction->ShortCut;
  // remote selected operation
  RemoteRenameAction->ShortCut = CurrentRenameAction->ShortCut;
  RemoteEditAction->ShortCut = CurrentEditAction->ShortCut;
  RemoteCreateDirAction->ShortCut = CurrentCreateDirAction->ShortCut;
  RemoteDeleteAction->ShortCut = CurrentDeleteAction->ShortCut;
  RemotePropertiesAction->ShortCut = CurrentPropertiesAction->ShortCut;
  // remote focused operation
  RemoteCopyFocusedAction->ShortCut = RemoteCopyAction->ShortCut;
  RemoteMoveFocusedAction->ShortCut = RemoteMoveAction->ShortCut;
  // selection
  LocalSelectAction->ShortCut = SelectAction->ShortCut;
  LocalUnselectAction->ShortCut = UnselectAction->ShortCut;
  LocalSelectAllAction->ShortCut = SelectAllAction->ShortCut;
  RemoteSelectAction->ShortCut = SelectAction->ShortCut;
  RemoteUnselectAction->ShortCut = UnselectAction->ShortCut;
  RemoteSelectAllAction->ShortCut = SelectAllAction->ShortCut;
}
//---------------------------------------------------------------------------
void __fastcall TNonVisualDataModule::SetScpExplorer(TCustomScpExplorerForm * value)
{
  FScpExplorer = value;
  SessionIdleTimer->Enabled = (FScpExplorer != NULL);
}
//---------------------------------------------------------------------------
void __fastcall TNonVisualDataModule::SessionIdleTimerTimer(
      TObject */*Sender*/)
{
  APPLICATION_EXCEPTION_HACK_BEGIN
  {
    DoIdle();
  }
  APPLICATION_EXCEPTION_HACK_END;
}
//---------------------------------------------------------------------------
void __fastcall TNonVisualDataModule::DoIdle()
{
  if (!FSessionIdleTimerExecuting)
  {
    FSessionIdleTimerExecuting = true;
    try
    {
      assert(ScpExplorer);
      ScpExplorer->Idle();
    }
    __finally
    {
      FSessionIdleTimerExecuting = false;
    }
  }
}
//---------------------------------------------------------------------------
void __fastcall TNonVisualDataModule::CreateCustomCommandsMenu(
  TTBCustomItem * Menu, bool OnFocused, bool Toolbar, bool Both)
{
  for (int Index = 0; Index < WinConfiguration->CustomCommandList->Count; Index++)
  {
    const TCustomCommandType * Command = WinConfiguration->CustomCommandList->Commands[Index];
    int State;

    if (!Both)
    {
      State = ScpExplorer->CustomCommandState(*Command, OnFocused);
    }
    else
    {
      State = ScpExplorer->BothCustomCommandState(*Command);
    }

    if (State >= 0)
    {
      TTBCustomItem * Item = new TTBXItem(Owner);
      Item->Caption = Command->Name;
      if (Toolbar)
      {
        Item->Caption = EscapeHotkey(StripHotkey(Item->Caption));
      }
      Item->Tag = Index;
      Item->Enabled = (State > 0);
      if (OnFocused)
      {
        Item->Tag = Item->Tag | 0x0100;
      }
      if (Both)
      {
        Item->Tag = Item->Tag | 0x0200;
      }
      Item->Hint = FMTLOAD(CUSTOM_COMMAND_HINT, (StripHotkey(Command->Name)));
      if (!Both)
      {
        Item->ShortCut = Command->ShortCut;
      }
      Item->OnClick = CustomCommandClick;

      Menu->Add(Item);
    }
  }

  TTBCustomItem * Item;

  if (!Both)
  {
    Item = new TTBXItem(Menu);
    Item->Action = OnFocused ? CustomCommandsEnterFocusedAction : CustomCommandsEnterAction;
    Menu->Add(Item);

    Item = new TTBXItem(Menu);
    Item->Action = OnFocused ? CustomCommandsLastFocusedAction : CustomCommandsLastAction;
    if (Toolbar)
    {
      Item->Caption = EscapeHotkey(StripHotkey(LoadStr(CUSTOM_COMMAND_LAST_SHORT)));
    }
    Menu->Add(Item);
  }

  AddMenuSeparator(Menu);

  if (!Toolbar && !Both)
  {
    Item = new TTBXItem(Menu);
    Item->Action = CustomCommandsBandAction;
    Menu->Add(Item);
  }

  Item = new TTBXItem(Menu);
  Item->Action = CustomCommandsCustomizeAction;
  Menu->Add(Item);
}
//---------------------------------------------------------------------------
void __fastcall TNonVisualDataModule::CreateCustomCommandsMenu(TAction * Action)
{
  assert(Action);
  TTBCustomItem * Menu = dynamic_cast<TTBCustomItem *>(Action->ActionComponent);
  if (Menu)
  {
    int PrevCount = Menu->Count;
    bool OnFocused = (Menu == RemoteDirViewCustomCommandsMenu);

    CreateCustomCommandsMenu(Menu, OnFocused, false, false);

    for (int Index = 0; Index < PrevCount; Index++)
    {
      Menu->Delete(0);
    }
  }
}
//---------------------------------------------------------------------------
void __fastcall TNonVisualDataModule::UpdateCustomCommandsToolbar(TTBXToolbar * Toolbar)
{
  // can be called while explorer is being created
  if (ScpExplorer == NULL)
  {
    return;
  }

  int AdditionalCommands = 4;
  TCustomCommandList * CustomCommandList = WinConfiguration->CustomCommandList;
  bool Changed = (CustomCommandList->Count != (Toolbar->Items->Count - AdditionalCommands));
  if (!Changed)
  {
    int Index = 0;
    while (!Changed && (Index < CustomCommandList->Count))
    {
      Changed =
        (Toolbar->Items->Items[Index]->Caption !=
          EscapeHotkey(StripHotkey(CustomCommandList->Commands[Index]->Name)));
      Index++;
    }
  }

  if (Changed)
  {
    Toolbar->BeginUpdate();
    try
    {
      Toolbar->Items->Clear();
      CreateCustomCommandsMenu(Toolbar->Items, false, true, false);
      assert(CustomCommandList->Count == (Toolbar->Items->Count - AdditionalCommands));
    }
    __finally
    {
      Toolbar->EndUpdate();
    }
  }
  else
  {
    for (int Index = 0; Index < Toolbar->Items->Count - AdditionalCommands; Index++)
    {
      TTBCustomItem * Item = Toolbar->Items->Items[Index];
      int CommandIndex = (Item->Tag & 0x00FF);
      assert(CommandIndex == Index);
      int State = ScpExplorer->CustomCommandState(
        *CustomCommandList->Commands[CommandIndex], false);
      assert(State >= 0);
      Item->Enabled = (State > 0);
    }
  }
}
//---------------------------------------------------------------------------
void __fastcall TNonVisualDataModule::CustomCommandClick(TObject * Sender)
{
  TTBCustomItem * Item = dynamic_cast<TTBCustomItem *>(Sender);
  assert(Item);
  const TCustomCommandType * Command = WinConfiguration->CustomCommandList->Commands[Item->Tag & 0x00FF];
  if (FLAGCLEAR(Item->Tag, 0x0200))
  {
    ScpExplorer->ExecuteFileOperationCommand(foCustomCommand, osRemote,
      FLAGSET(Item->Tag, 0x0100), false, const_cast<TCustomCommandType *>(Command));
  }
  else
  {
    ScpExplorer->BothCustomCommand(*Command);
  }
}
//---------------------------------------------------------------------------
void __fastcall TNonVisualDataModule::CreateSessionColorMenu(TAction * Action)
{
  if (ALWAYS_TRUE(Action->ActionComponent != NULL))
  {
    ::CreateSessionColorMenu(Action->ActionComponent, ScpExplorer->SessionColor,
      SessionColorChange);
  }
}
//---------------------------------------------------------------------------
void __fastcall TNonVisualDataModule::SessionColorChange(TColor Color)
{
  ScpExplorer->SessionColor = Color;
}
//---------------------------------------------------------------------------
void __fastcall TNonVisualDataModule::CreateSessionListMenu(TAction * Action)
{
  StoredSessions->Load();
  CreateSessionListMenuLevel(
    dynamic_cast<TTBCustomItem *>(Action->ActionComponent), 0, 0);
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TNonVisualDataModule::GetSessionFolderRoot(
  TSessionData * Data, int Level)
{
  UnicodeString Path = Data->Name;
  UnicodeString Root;
  for (int ALevel = 0; ALevel < Level; ALevel++)
  {
    Root.Insert(::CutToChar(Path, L'/', false) + L'/', Root.Length() + 1);
  }
  return Root;
}
//---------------------------------------------------------------------------
void __fastcall TNonVisualDataModule::CreateSessionListMenuLevel(
  TTBCustomItem * Menu, int Index, int Level)
{
  Menu->Clear();

  TTBCustomItem * Item = new TTBXItem(Menu);

  UnicodeString Root;
  if (Level == 0)
  {
    Item->Action = SiteManagerAction;
    Root = L"";
  }
  else
  {
    assert(Index < StoredSessions->Count);
    TSessionData * Data = StoredSessions->Sessions[Index];

    Root = GetSessionFolderRoot(Data, Level);

    Item->Caption = LoadStr(SAVEDSESSIONFOLDER_THIS_OPEN);
    Item->Tag = MAKELONG(Index, Level);
    UnicodeString FolderName = Root;
    if (ALWAYS_TRUE(!FolderName.IsEmpty() && FolderName[FolderName.Length()] == L'/'))
    {
      FolderName.Delete(FolderName.Length(), 1);
    }
    Item->Hint = FMTLOAD(SAVEDSESSIONFOLDER_THIS_HINT, (FolderName));
    Item->OnClick = SessionFolderThisItemClick;
  }

  Menu->Insert(Menu->Count, Item);

  AddMenuSeparator(Menu);

  int FirstSession = Menu->Count;
  UnicodeString PrevName;
  while (Index < StoredSessions->Count)
  {
    TSessionData * Data = StoredSessions->Sessions[Index];
    if (!AnsiSameText(Data->Name.SubString(1, Root.Length()), Root))
    {
      // Sessions are sorted, so no chance further sessions may match
      break;
    }
    else if (!Data->IsWorkspace)
    {
      UnicodeString Name = Data->Name.SubString(Root.Length() + 1, Data->Name.Length() - Root.Length());
      int P = Name.Pos(L'/');
      if (P > 0)
      {
        Name.SetLength(P - 1);
      }

      if (Name != PrevName)
      {
        if (P > 0)
        {
          TTBCustomItem * Item = new TTBXSubmenuItem(Menu);
          Item->Caption = Name;
          Item->Tag = ((Level + 1) << 16) | Index; // MAKELONG
          Item->Hint = FMTLOAD(SAVEDSESSIONFOLDER_HINT, (Root + Name));
          Item->ImageIndex = SavedSessionsAction2->ImageIndex;
          Item->OnClick = SessionFolderItemClick;

          Menu->Insert(FirstSession, Item);
          FirstSession++;
        }
        else
        {
          TTBCustomItem * Item = new TTBXItem(Menu);
          Item->Caption = Name;
          Item->Tag = Index;
          Item->Hint = FMTLOAD(SAVEDSESSION_HINT, (Data->Name));
          Item->OnClick = SessionItemClick;
          Menu->Insert(Menu->Count, Item);
        }

        PrevName = Name;
      }
    }
    Index++;
  }
}
//---------------------------------------------------------------------------
void __fastcall TNonVisualDataModule::SessionFolderItemClick(TObject * Sender)
{
  TTBCustomItem * Item = dynamic_cast<TTBCustomItem *>(Sender);
  assert(Item != NULL);
  CreateSessionListMenuLevel(Item, LOWORD(Item->Tag), HIWORD(Item->Tag));
}
//---------------------------------------------------------------------------
void __fastcall TNonVisualDataModule::SessionFolderThisItemClick(TObject * Sender)
{
  TTBCustomItem * Item = NOT_NULL(dynamic_cast<TTBCustomItem *>(Sender));
  int Index = LOWORD(Item->Tag);
  int Level = HIWORD(Item->Tag);
  UnicodeString Folder = GetSessionFolderRoot(StoredSessions->Sessions[Index], Level);
  ScpExplorer->OpenFolderOrWorkspace(Folder);
}
//---------------------------------------------------------------------------
void __fastcall TNonVisualDataModule::SessionItemClick(TObject * Sender)
{
  assert(StoredSessions && (((TTBCustomItem *)Sender)->Tag < StoredSessions->Count));
  ScpExplorer->OpenStoredSession(StoredSessions->Sessions[((TTBCustomItem *)Sender)->Tag]);
}
//---------------------------------------------------------------------------
void __fastcall TNonVisualDataModule::CreateWorkspacesMenu(TAction * Action)
{
  StoredSessions->Load();
  if (!StoredSessions->HasAnyWorkspace())
  {
    Abort();
  }
  else
  {
    TTBCustomItem * Menu =
      NOT_NULL(dynamic_cast<TTBCustomItem *>(Action->ActionComponent));

    Menu->Clear();

    std::unique_ptr<TStrings> Workspaces(StoredSessions->GetWorkspaces());
    for (int Index = 0; Index < Workspaces->Count; Index++)
    {
      TTBCustomItem * Item = new TTBXItem(Menu);
      Item->Caption = Workspaces->Strings[Index];
      Item->Tag = Index;
      Item->Hint = FMTLOAD(WORKSPACE_HINT, (Workspaces->Strings[Index]));
      Item->OnClick = WorkspaceItemClick;
      Menu->Insert(Menu->Count, Item);
    }
  }
}
//---------------------------------------------------------------------------
void __fastcall TNonVisualDataModule::WorkspaceItemClick(TObject * Sender)
{
  std::unique_ptr<TStrings> Workspaces(StoredSessions->GetWorkspaces());
  ScpExplorer->OpenFolderOrWorkspace(
    Workspaces->Strings[NOT_NULL(dynamic_cast<TTBCustomItem *>(Sender))->Tag]);
}
//---------------------------------------------------------------------------
TShortCut __fastcall TNonVisualDataModule::OpenSessionShortCut(int Index)
{
  if (Index >= 0 && Index < 10)
  {
    return ShortCut((Word)(Index < 9 ? L'0' + 1 + Index : L'0'),
      TShiftState() << ssAlt);
  }
  else
  {
    return scNone;
  }
}
//---------------------------------------------------------------------------
void __fastcall TNonVisualDataModule::CreateOpenedSessionListMenu(TAction * Action)
{
  TTBCustomItem * OpenedSessionsMenu = dynamic_cast<TTBCustomItem *>(Action->ActionComponent);
  assert(OpenedSessionsMenu != NULL);
  TTerminalManager * Manager = TTerminalManager::Instance();
  TStrings * TerminalList = Manager->TerminalList;
  int PrevCount = OpenedSessionsMenu->Count;
  for (int Index = 0; Index < TerminalList->Count; Index++)
  {
    TTerminal * Terminal = dynamic_cast<TTerminal *>(TerminalList->Objects[Index]);
    assert(Terminal);
    TTBCustomItem * Item = new TTBXItem(OpenedSessionsMenu);
    Item->Caption = TerminalList->Strings[Index];
    Item->Tag = int(Terminal);
    Item->Hint = FMTLOAD(OPENEDSESSION_HINT, (Item->Caption));
    Item->Checked = (Manager->ActiveTerminal == Terminal);
    Item->ShortCut = OpenSessionShortCut(Index);
    Item->OnClick = OpenedSessionItemClick;
    Item->RadioItem = true;
    OpenedSessionsMenu->Add(Item);
  }
  for (int Index = 0; Index < PrevCount; Index++)
  {
    OpenedSessionsMenu->Delete(0);
  }
}
//---------------------------------------------------------------------------
void __fastcall TNonVisualDataModule::OpenedSessionItemClick(TObject * Sender)
{
  TTerminalManager::Instance()->ActiveTerminal = (TTerminal*)(((TMenuItem *)Sender)->Tag);
}
//---------------------------------------------------------------------------
void __fastcall TNonVisualDataModule::CreateEditorListMenu(TAction * Action)
{
  assert(Action != NULL);
  TTBCustomItem * Menu = dynamic_cast<TTBCustomItem *>(Action->ActionComponent);
  if (Menu != NULL)
  {
    int PrevCount = Menu->Count;

    TTBCustomItem * Item = new TTBXItem(Menu);
    Item->Action = CurrentEditInternalAction;
    Menu->Add(Item);

    AddMenuSeparator(Menu);

    TStringList * UsedEditors = new TStringList();
    try
    {
      UsedEditors->CaseSensitive = false;
      UsedEditors->Sorted = true;

      const TEditorList * EditorList = WinConfiguration->EditorList;
      for (int Index = 0; Index < EditorList->Count; Index++)
      {
        const TEditorPreferences * Editor = EditorList->Editors[Index];

        if ((Editor->Data->Editor == edExternal) &&
            (UsedEditors->IndexOf(Editor->Data->ExternalEditor) < 0))
        {
          UsedEditors->Add(Editor->Data->ExternalEditor);

          TTBCustomItem * Item = new TTBXItem(Menu);
          Item->Caption = Editor->Name;
          Item->Tag = Index;
          Item->Hint = FMTLOAD(EXTERNAL_EDITOR_HINT, (Editor->Name));
          Item->OnClick = EditorItemClick;
          Menu->Add(Item);
        }
      }

      Item = new TTBXItem(Menu);
      Item->Action = CurrentEditWithAction;
      Menu->Add(Item);

      AddMenuSeparator(Menu);

      Item = new TTBXItem(Menu);
      Item->Action = EditorListCustomizeAction;
      Menu->Add(Item);

      for (int Index = 0; Index < PrevCount; Index++)
      {
        Menu->Delete(0);
      }
    }
    __finally
    {
      delete UsedEditors;
    }
  }
}
//---------------------------------------------------------------------------
void __fastcall TNonVisualDataModule::EditorItemClick(TObject * Sender)
{
  int Tag = dynamic_cast<TTBXItem*>(Sender)->Tag;
  const TEditorList * EditorList = WinConfiguration->EditorList;
  // sanity check
  if (Tag < EditorList->Count)
  {
    ScpExplorer->ExecuteFile(osCurrent, efExternalEditor, EditorList->Editors[Tag]->Data,
      true, false);
  }
}
//---------------------------------------------------------------------------
void __fastcall TNonVisualDataModule::QueuePopupPopup(TObject * /*Sender*/)
{
  TAction * Action = NULL;

  switch (ScpExplorer->DefaultQueueOperation())
  {
    case qoItemQuery:
      Action = QueueItemQueryAction;
      break;

    case qoItemError:
      Action = QueueItemErrorAction;
      break;

    case qoItemPrompt:
      Action = QueueItemPromptAction;
      break;

    case qoItemExecute:
      Action = QueueItemExecuteAction;
      break;

    case qoItemPause:
      Action = QueueItemPauseAction;
      break;

    case qoItemResume:
      Action = QueueItemResumeAction;
      break;
  }

  TTBCustomItem * Item;
  for (int Index = 0; Index < QueuePopup->Items->Count; Index++)
  {
    Item = QueuePopup->Items->Items[Index];
    TTBItemOptions O = Item->Options;
    if ((Action != NULL) && (Item->Action == Action))
    {
      O << tboDefault;
    }
    else
    {
      O >> tboDefault;
    }
    Item->Options = O;
  }

  QueueSpeedComboBoxItemUpdate(QueuePopupSpeedComboBoxItem);
}
//---------------------------------------------------------------------------
void __fastcall TNonVisualDataModule::ShowUpdatesUpdate()
{
  TUpdatesConfiguration Updates = WinConfiguration->Updates;
  unsigned short H, M, S, MS;
  DecodeTime(Now(), H, M, S, MS);
  int CurrentCompoundVer = Configuration->CompoundVersion;
  ShowUpdatesAction->ImageIndex =
    ((Updates.HaveResults && (Updates.Results.ForVersion == CurrentCompoundVer) &&
      !Updates.Results.Disabled &&
      ((Updates.Results.Critical && !Updates.ShownResults && (MS >= 500)) ||
       ((!Updates.Results.Critical || Updates.ShownResults) &&
        ((Updates.Results.Version > CurrentCompoundVer) ||
         !Updates.Results.Message.IsEmpty())))) ? 80 :
     ((int(Updates.Period) <= 0) ? 81 : 63));
}
//---------------------------------------------------------------------------
void __fastcall TNonVisualDataModule::PreferencesDialog(TPreferencesMode APreferencesMode)
{
  if (ScpExplorer != NULL)
  {
    ScpExplorer->PreferencesDialog(APreferencesMode);
  }
  else
  {
    DoPreferencesDialog(APreferencesMode);
  }
}
//---------------------------------------------------------------------------
void __fastcall TNonVisualDataModule::CustomCommandsLastUpdate(TAction * Action)
{
  TCustomCommandType Command;
  int State;
  bool Defined = ScpExplorer->GetLastCustomCommand(
    (Action == CustomCommandsLastFocusedAction), Command, State);
  Action->Visible = Defined;
  if (Defined)
  {
    UnicodeString TitleCommand = Command.Command;
    int MaxTitleCommandLen = 20;
    if (TitleCommand.Length() > MaxTitleCommandLen)
    {
      TitleCommand = TitleCommand.SubString(1, MaxTitleCommandLen - 3) + "...";
    }
    Action->Caption = FMTLOAD(CUSTOM_COMMAND_LAST, (TitleCommand));
    Action->Hint = FMTLOAD(CUSTOM_COMMAND_HINT, (Command.Command));
    Action->Enabled = (State > 0);
  }
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TNonVisualDataModule::QueueItemSpeed(const UnicodeString & Text,
  TTBXComboBoxItem * Item)
{
  unsigned long Speed = GetSpeedLimit(Text);
  ScpExplorer->ExecuteQueueOperation(qoItemSpeed, reinterpret_cast<void*>(Speed));

  UnicodeString Result = SetSpeedLimit(Speed);
  SaveToHistory(Item->Strings, Result);
  CustomWinConfiguration->History[L"SpeedLimit"] = Item->Strings;

  return Result;
}
//---------------------------------------------------------------------------
void __fastcall TNonVisualDataModule::QueuePopupSpeedComboBoxItemItemClick(
  TObject * Sender)
{
  TTBXComboBoxItem * Item = dynamic_cast<TTBXComboBoxItem *>(Sender);
  QueueItemSpeedAction->Text = QueueItemSpeed(Item->Text, Item);
}
//---------------------------------------------------------------------------
void __fastcall TNonVisualDataModule::QueueSpeedComboBoxItemAcceptText(
  TObject * Sender, UnicodeString & NewText, bool & /*Accept*/)
{
  TTBXComboBoxItem * Item = dynamic_cast<TTBXComboBoxItem *>(Sender);
  NewText = QueueItemSpeed(NewText, Item);
  QueueItemSpeedAction->Text = NewText;
}
//---------------------------------------------------------------------------
void __fastcall TNonVisualDataModule::QueueSpeedComboBoxItem(TTBXComboBoxItem * Item)
{
  // IDE often looses this link
  Item->OnAcceptText = QueueSpeedComboBoxItemAcceptText;
  Item->OnItemClick = QueuePopupSpeedComboBoxItemItemClick;
}
//---------------------------------------------------------------------------
void __fastcall TNonVisualDataModule::QueueSpeedComboBoxItemUpdate(TTBXComboBoxItem * Item)
{
  Item->Strings = CustomWinConfiguration->History[L"SpeedLimit"];
}
//---------------------------------------------------------------------------
void __fastcall TNonVisualDataModule::SetQueueOnceEmptyAction(TAction * Action)
{
  TAction * Current = CurrentQueueOnceEmptyAction();
  if (Current != Action)
  {
    Current->Checked = false;
    Action->Checked = true;
  }
}
//---------------------------------------------------------------------------
void __fastcall TNonVisualDataModule::CycleQueueOnceEmptyAction()
{
  TAction * Current = CurrentQueueOnceEmptyAction();
  Current->Checked = false;
  if (Current == QueueIdleOnceEmptyAction)
  {
    QueueDisconnectOnceEmptyAction->Checked = true;
  }
  else if (Current == QueueDisconnectOnceEmptyAction)
  {
    QueueShutDownOnceEmptyAction->Checked = true;
  }
  else if (Current == QueueShutDownOnceEmptyAction)
  {
    QueueIdleOnceEmptyAction->Checked = true;
  }
  else
  {
    assert(false);
  }
}
//---------------------------------------------------------------------------
TAction * __fastcall TNonVisualDataModule::CurrentQueueOnceEmptyAction()
{
  TAction * Result;
  if (QueueIdleOnceEmptyAction->Checked)
  {
    Result = QueueIdleOnceEmptyAction;
  }
  else if (QueueDisconnectOnceEmptyAction->Checked)
  {
    Result = QueueDisconnectOnceEmptyAction;
  }
  else if (QueueShutDownOnceEmptyAction->Checked)
  {
    Result = QueueShutDownOnceEmptyAction;
  }
  else
  {
    assert(false);
  }
  return Result;
}
//---------------------------------------------------------------------------
TOnceDoneOperation __fastcall TNonVisualDataModule::CurrentQueueOnceEmptyOperation()
{
  TOnceDoneOperation Result;
  TBasicAction * Current = CurrentQueueOnceEmptyAction();
  if (Current == QueueIdleOnceEmptyAction)
  {
    Result = odoIdle;
  }
  else if (Current == QueueDisconnectOnceEmptyAction)
  {
    Result = odoDisconnect;
  }
  else if (Current == QueueShutDownOnceEmptyAction)
  {
    Result = odoShutDown;
  }
  else
  {
    assert(false);
  }
  return Result;
}
//---------------------------------------------------------------------------
void __fastcall TNonVisualDataModule::ResetQueueOnceEmptyOperation()
{
  SetQueueOnceEmptyAction(QueueIdleOnceEmptyAction);
}
//---------------------------------------------------------------------------
void __fastcall TNonVisualDataModule::StartBusy()
{
  FBusy++;
}
//---------------------------------------------------------------------------
void __fastcall TNonVisualDataModule::EndBusy()
{
  FBusy--;
}
