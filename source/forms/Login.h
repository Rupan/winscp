//----------------------------------------------------------------------------
#ifndef LoginH
#define LoginH
//----------------------------------------------------------------------------
#include "ComboEdit.hpp"
#include "PasswordEdit.hpp"
#include "UpDownEdit.hpp"
#include "PngImageList.hpp"
#include <System.Classes.hpp>
#include <Vcl.ActnList.hpp>
#include <Vcl.ComCtrls.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ImgList.hpp>
#include <Vcl.Mask.hpp>
#include <Vcl.Menus.hpp>
#include <Vcl.StdCtrls.hpp>
//----------------------------------------------------------------------------
#include <Configuration.h>
#include <SessionData.h>
#include <PasTools.hpp>
//----------------------------------------------------------------------------
class TLoginDialog : public TForm
{
__published:
  TButton *LoginButton;
  TButton *CloseButton;
  TActionList *ActionList;
  TAction *EditSessionAction;
  TAction *SaveSessionAction;
  TAction *DeleteSessionAction;
  TAction *ImportSessionsAction;
  TAction *LoginAction;
  TAction *AboutAction;
  TAction *CleanUpAction;
  TAction *ResetNewSessionAction;
  TPanel *MainPanel;
  TTreeView *SessionTree;
  TAction *SetDefaultSessionAction;
  TButton *ToolsMenuButton;
  TPopupMenu *ToolsPopupMenu;
  TMenuItem *Import1;
  TMenuItem *Cleanup1;
  TAction *DesktopIconAction;
  TAction *SendToHookAction;
  TAction *CheckForUpdatesAction;
  TMenuItem *CheckForUpdates1;
  TButton *SaveButton;
  TButton *HelpButton;
  TAction *RenameSessionAction;
  TPngImageList *SessionImageList;
  TAction *NewSessionFolderAction;
  TPopupMenu *SaveDropDownMenu;
  TMenuItem *SaveSessionMenuItem;
  TMenuItem *Setdefaults1;
  TMenuItem *N1;
  TMenuItem *N2;
  TMenuItem *Pageant1;
  TMenuItem *Puttygen1;
  TAction *RunPageantAction;
  TAction *RunPuttygenAction;
  TAction *ImportAction;
  TAction *ExportAction;
  TMenuItem *N3;
  TMenuItem *ImportConfiguration1;
  TMenuItem *ExportConfiguration1;
  TStaticText *SitesIncrementalSearchLabel;
  TAction *PreferencesAction;
  TMenuItem *N4;
  TMenuItem *Preferences1;
  TMenuItem *About1;
  TPanel *SitesPanel;
  TPanel *ContentsPanel;
  TGroupBox *ContentsGroupBox;
  TLabel *ContentsLabel;
  TEdit *ContentsNameEdit;
  TMemo *ContentsMemo;
  TPanel *SitePanel;
  TGroupBox *BasicGroup;
  TLabel *Label1;
  TLabel *Label2;
  TLabel *Label3;
  TLabel *Label4;
  TLabel *Label22;
  TLabel *FtpsLabel;
  TLabel *WebDavsLabel;
  TEdit *HostNameEdit;
  TEdit *UserNameEdit;
  TPasswordEdit *PasswordEdit;
  TUpDownEdit *PortNumberEdit;
  TComboBox *TransferProtocolCombo;
  TComboBox *FtpsCombo;
  TComboBox *WebDavsCombo;
  TPanel *BasicFtpPanel;
  TCheckBox *AnonymousLoginCheck;
  TPanel *BasicSshPanel;
  TButton *AdvancedButton;
  TButton *ManageButton;
  TPopupMenu *ManageSitePopupMenu;
  TMenuItem *Edit1;
  TMenuItem *Delete1;
  TMenuItem *Rename1;
  TMenuItem *Newfolder1;
  TMenuItem *DesktopIcon2;
  TMenuItem *ExplorersSendToShortcut2;
  TMenuItem *Shellicon2;
  TMenuItem *N5;
  TMenuItem *Shellicon1;
  TMenuItem *N6;
  TPopupMenu *ManageFolderPopupMenu;
  TMenuItem *MenuItem1;
  TMenuItem *MenuItem3;
  TMenuItem *MenuItem4;
  TMenuItem *MenuItem5;
  TMenuItem *MenuItem6;
  TMenuItem *MenuItem7;
  TMenuItem *MenuItem8;
  TPopupMenu *ManageNewSitePopupMenu;
  TMenuItem *MenuItem12;
  TMenuItem *MenuItem13;
  TMenuItem *MenuItem16;
  TMenuItem *MenuItem17;
  TMenuItem *MenuItem21;
  TMenuItem *MenuItem22;
  TMenuItem *Setdefaults2;
  TMenuItem *Reset1;
  TPopupMenu *ManageWorkspacePopupMenu;
  TMenuItem *MenuItem2;
  TMenuItem *MenuItem10;
  TMenuItem *MenuItem11;
  TMenuItem *MenuItem18;
  TMenuItem *MenuItem19;
  TButton *EditCancelButton;
  TAction *EditCancelAction;
  TButton *EditButton;
  TAction *SaveAsSessionAction;
  TMenuItem *SaveAsSessionMenuItem;
  TMenuItem *N7;
  TEdit *TransferProtocolView;
  TEdit *EncryptionView;
  TPanel *ButtonPanel;
  TPopupMenu *SessionAdvancedPopupMenu;
  TMenuItem *MenuItem9;
  TMenuItem *MenuItem14;
  TAction *SessionAdvancedAction;
  TAction *PreferencesLoggingAction;
  TMenuItem *PreferencesLoggingAction1;
  TMenuItem *Session1;
  TPngImageList *ActionImageList;
  TAction *CloneToNewSiteAction;
  TMenuItem *SiteClonetoNewSiteMenuItem;
  TAction *PuttyAction;
  TPopupMenu *LoginDropDownMenu;
  TMenuItem *Login1;
  TMenuItem *OpeninPuTTY1;
  TMenuItem *Login2;
  TMenuItem *N8;
  TMenuItem *Login3;
  TMenuItem *N9;
  TMenuItem *SiteLoginMenuItem;
  TMenuItem *N10;
  TMenuItem *Login5;
  TMenuItem *N11;
  TMenuItem *OpeninPuTTY2;
  TMenuItem *OpeninPuTTY3;
  void __fastcall DataChange(TObject *Sender);
  void __fastcall FormShow(TObject *Sender);
  void __fastcall SessionTreeDblClick(TObject *Sender);
  void __fastcall SessionTreeKeyDown(TObject *Sender, WORD &Key,
    TShiftState Shift);
  void __fastcall EditSessionActionExecute(TObject *Sender);
  void __fastcall SaveSessionActionExecute(TObject *Sender);
  void __fastcall DeleteSessionActionExecute(TObject *Sender);
  void __fastcall ImportSessionsActionExecute(TObject *Sender);
  void __fastcall CleanUpActionExecute(TObject *Sender);
  void __fastcall AboutActionExecute(TObject *Sender);
  void __fastcall ActionListUpdate(TBasicAction *Action,
    bool &Handled);
  void __fastcall PreferencesActionExecute(TObject *Sender);
  void __fastcall ResetNewSessionActionExecute(TObject *Sender);
  void __fastcall SetDefaultSessionActionExecute(TObject *Sender);
  void __fastcall ToolsMenuButtonClick(TObject *Sender);
  void __fastcall DesktopIconActionExecute(TObject *Sender);
  void __fastcall SendToHookActionExecute(TObject *Sender);
  void __fastcall CheckForUpdatesActionExecute(TObject *Sender);
  void __fastcall HelpButtonClick(TObject *Sender);
  void __fastcall FormCloseQuery(TObject *Sender, bool &CanClose);
  void __fastcall RenameSessionActionExecute(TObject * Sender);
  void __fastcall TransferProtocolComboChange(TObject *Sender);
  void __fastcall NavigationTreeCollapsing(TObject *Sender,
    TTreeNode *Node, bool &AllowCollapse);
  void __fastcall SessionTreeChange(TObject *Sender, TTreeNode *Node);
  void __fastcall SessionTreeEdited(TObject *Sender, TTreeNode *Node,
    UnicodeString &S);
  void __fastcall SessionTreeEditing(TObject *Sender, TTreeNode *Node,
    bool &AllowEdit);
  void __fastcall SessionTreeCustomDrawItem(TCustomTreeView *Sender,
    TTreeNode *Node, TCustomDrawState State, bool &DefaultDraw);
  void __fastcall SessionTreeExpandedCollapsed(TObject *Sender, TTreeNode *Node);
  void __fastcall SessionTreeCompare(TObject *Sender, TTreeNode *Node1,
          TTreeNode *Node2, int Data, int &Compare);
  void __fastcall NewSessionFolderActionExecute(TObject *Sender);
  void __fastcall SessionTreeStartDrag(TObject *Sender,
          TDragObject *&DragObject);
  void __fastcall SessionTreeDragDrop(TObject *Sender, TObject *Source,
          int X, int Y);
  void __fastcall SessionTreeMouseMove(TObject *Sender, TShiftState Shift,
          int X, int Y);
  void __fastcall SessionTreeEndDrag(TObject *Sender, TObject *Target,
          int X, int Y);
  void __fastcall AnonymousLoginCheckClick(TObject *Sender);
  void __fastcall SaveButtonDropDownClick(TObject *Sender);
  void __fastcall SessionTreeExpanding(TObject *Sender, TTreeNode *Node, bool &AllowExpansion);
  void __fastcall RunPageantActionExecute(TObject *Sender);
  void __fastcall RunPuttygenActionExecute(TObject *Sender);
  void __fastcall PortNumberEditChange(TObject *Sender);
  void __fastcall ExportActionExecute(TObject *Sender);
  void __fastcall ImportActionExecute(TObject *Sender);
  void __fastcall SessionTreeKeyPress(TObject *Sender, System::WideChar &Key);
  void __fastcall SessionTreeExit(TObject *Sender);
  void __fastcall SessionTreeChanging(TObject *Sender, TTreeNode *Node, bool &AllowChange);
  void __fastcall SessionAdvancedActionExecute(TObject *Sender);
  void __fastcall ManageButtonClick(TObject *Sender);
  void __fastcall SessionTreeMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y);
  void __fastcall SessionTreeContextPopup(TObject *Sender, TPoint &MousePos, bool &Handled);
  void __fastcall EditCancelActionExecute(TObject *Sender);
  void __fastcall SaveAsSessionActionExecute(TObject *Sender);
  void __fastcall AdvancedButtonDropDownClick(TObject *Sender);
  void __fastcall PreferencesLoggingActionExecute(TObject *Sender);
  void __fastcall CloneToNewSiteActionExecute(TObject *Sender);
  void __fastcall LoginActionExecute(TObject *Sender);
  void __fastcall PuttyActionExecute(TObject *Sender);
  void __fastcall LoginButtonDropDownClick(TObject *Sender);

private:
  int NoUpdate;
  TSessionData * FNewSiteData;
  bool FNewSiteKeepName;
  TSessionData * FSessionData;
  TStoredSessionList * FStoredSessions;
  int FOptions;
  bool FInitialized;
  bool FLocaleChanging;
  void * FSystemSettings;
  TWndMethod FOldSessionTreeProc;
  TTreeNode * FHintNode;
  TTreeViewScrollOnDragOver * FScrollOnDragOver;
  int FDefaultPort;
  TList * FDataList;
  bool FUpdatePortWithProtocol;
  UnicodeString FSitesIncrementalSearch;
  int FIncrementalSearching;
  bool FSitesIncrementalSearchHaveNext;
  int FBasicGroupBaseHeight;
  TRect FSavedBounds;
  bool FEditing;
  bool FRenaming;
  TSize FOriginalSize;

  void __fastcall LoadSession(TSessionData * SessionData);
  void __fastcall LoadContents();
  void __fastcall UpdateControls();
  void __fastcall SetSessionData(TSessionData * value);
  TSessionData * __fastcall GetSessionData();
  void __fastcall SaveSession(TSessionData * aStoredSession);
  void __fastcall LoadSessions();
  TSessionData * __fastcall GetSelectedSession();
  void __fastcall CMDialogKey(TWMKeyDown & Message);
  int __fastcall FSProtocolToIndex(TFSProtocol FSProtocol, bool & AllowScpFallback);
  TFSProtocol __fastcall IndexToFSProtocol(int Index, bool AllowScpFallback);
  int __fastcall FtpsToIndex(TFtps Ftps);
  TFtps __fastcall GetFtps();
  TFSProtocol __fastcall GetFSProtocol(bool RequireScpFallbackDistinction);
  void __fastcall UpdateFolderNode(TTreeNode * Node);
  TTreeNode * __fastcall AddSession(TSessionData * Data);
  TTreeNode * __fastcall AddSessionPath(UnicodeString Path,
    bool CanCreate, bool IsWorkspace);
  void __fastcall DestroySession(TSessionData * Data);
  void __fastcall CheckDuplicateFolder(TTreeNode * Parent, UnicodeString Text,
    TTreeNode * Node);
  void __fastcall CheckIsSessionFolder(TTreeNode * Node);
  void __fastcall NewSessionFolderInputDialogInitialize(
    TObject * Sender, TInputDialogData * Data);
  UnicodeString __fastcall SessionNodePath(TTreeNode * Node);
  TTreeNode * __fastcall SessionFolderNode(TTreeNode * Node);
  TTreeNode * __fastcall CurrentSessionFolderNode();
  void __fastcall SessionTreeProc(TMessage & Message);
  TTreeNode * __fastcall NormalizeDropTarget(TTreeNode * DropTarget);
  bool __fastcall SessionAllowDrop(TTreeNode * DropTarget);
  int __fastcall DefaultPort();
  void __fastcall MasterPasswordRecrypt(TObject * Sender);
  void __fastcall SaveOpenedStoredSessionFolders(
    TTreeNode * Node, TStrings * OpenedStoredSessionFolders);
  void __fastcall LoadOpenedStoredSessionFolders(
    TTreeNode * Node, TStrings * OpenedStoredSessionFolders);
  bool __fastcall HasNodeAnySession(TTreeNode * Node, bool NeedCanLogin = false);
  void __fastcall SaveDataList(TList * DataList);
  inline bool __fastcall IsFolderNode(TTreeNode * Node);
  inline bool __fastcall IsWorkspaceNode(TTreeNode * Node);
  inline bool __fastcall IsFolderOrWorkspaceNode(TTreeNode * Node);
  inline bool __fastcall IsSessionNode(TTreeNode * Node);
  inline bool __fastcall IsSiteNode(TTreeNode * Node);
  inline bool __fastcall IsNewSiteNode(TTreeNode * Node);
  inline TSessionData * __fastcall GetNodeSession(TTreeNode * Node);
  void __fastcall ExecuteTool(const UnicodeString & Name);
  UnicodeString __fastcall ImportExportIniFilePath();
  void __fastcall ReloadSessions();
  void __fastcall ResetSitesIncrementalSearch();
  bool __fastcall SitesIncrementalSearch(const UnicodeString & Text,
    bool SkipCurrent, bool Reverse);
  TTreeNode * __fastcall SearchSite(const UnicodeString & Text,
    bool AllowExpanding, bool SkipCurrent, bool Reverse);
  TTreeNode * __fastcall GetNextNode(TTreeNode * Node, bool Reverse);
  UnicodeString __fastcall GetFolderOrWorkspaceContents(
    TTreeNode * Node, const UnicodeString & Indent, const UnicodeString & CommonRoot);
  TPopupMenu * __fastcall GetSelectedNodePopupMenu();
  void __fastcall PersistNewSiteIfNeeded();
  TTreeNode * __fastcall FindSessionNode(TSessionData * SessionData, bool ByName);
  void __fastcall UpdateButtonVisibility(TButton * Button);
  void __fastcall Idle();
  void __fastcall DefaultButton(TButton * Button, bool Default);
  TSessionData * __fastcall GetEditingSessionData();
  void __fastcall SaveAsSession(bool ForceDialog);
  void __fastcall InvalidateSessionData();
  bool __fastcall CanLogin();
  bool __fastcall IsCloneToNewSiteDefault();
  bool __fastcall IsDefaultResult(TModalResult Result);
  int __fastcall GetSessionImageIndex(TSessionData * Data);
  void __fastcall SetNodeImage(TTreeNode * Node, int ImageIndex);
  void __fastcall CancelEditing();
  bool __fastcall EnsureNotEditing();
  bool __fastcall IsEditable();
  TSessionData * __fastcall CloneSelectedSession();
  void __fastcall CloneToNewSite();

protected:
  void __fastcall Default();
  void __fastcall NewSite();
  void __fastcall SaveConfiguration();
  void __fastcall LoadState();
  void __fastcall SaveState();
  void __fastcall ShowPreferencesDialog(TPreferencesMode PreferencesMode);
  virtual void __fastcall Dispatch(void * Message);
  void __fastcall Init();
  void __fastcall InitControls();
  void __fastcall EditSession();
  void __fastcall Login();
  __property TSessionData * SelectedSession  = { read=GetSelectedSession };

public:
  virtual __fastcall TLoginDialog(TComponent* AOwner);
  __fastcall ~TLoginDialog();
  void __fastcall Init(TStoredSessionList *SessionList, int Options);
  bool __fastcall Execute(TList * DataList);
};
//----------------------------------------------------------------------------
#endif
