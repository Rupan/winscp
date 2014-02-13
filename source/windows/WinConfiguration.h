//---------------------------------------------------------------------------
#ifndef WinConfigurationH
#define WinConfigurationH
//---------------------------------------------------------------------------
#include "CustomWinConfiguration.h"
#include "CustomDirView.hpp"
//---------------------------------------------------------------------------
enum TEditor { edInternal, edExternal, edOpen };
//---------------------------------------------------------------------------
#define C(Property) (Property != rhc.Property) ||
struct TScpExplorerConfiguration {
  UnicodeString WindowParams;
  UnicodeString DirViewParams;
  UnicodeString ToolbarsLayout;
  bool SessionsTabs;
  bool StatusBar;
  UnicodeString LastLocalTargetDirectory;
  int ViewStyle;
  bool ShowFullAddress;
  bool DriveView;
  int DriveViewWidth;
  int DriveViewWidthPixelsPerInch;
  bool __fastcall operator !=(TScpExplorerConfiguration & rhc)
    { return C(WindowParams) C(DirViewParams) C(ToolbarsLayout)
        C(SessionsTabs) C(StatusBar)
        C(LastLocalTargetDirectory) C(ViewStyle) C(ShowFullAddress)
        C(DriveView) C(DriveViewWidth) C(DriveViewWidthPixelsPerInch) 0; };
};
//---------------------------------------------------------------------------
struct TScpCommanderPanelConfiguration {
  UnicodeString DirViewParams;
  bool StatusBar;
  bool DriveView;
  int DriveViewHeight;
  int DriveViewHeightPixelsPerInch;
  int DriveViewWidth;
  int DriveViewWidthPixelsPerInch;
  bool __fastcall operator !=(TScpCommanderPanelConfiguration & rhc)
    { return C(DirViewParams) C(StatusBar)
        C(DriveView) C(DriveViewHeight) C(DriveViewHeightPixelsPerInch)
        C(DriveViewWidth) C(DriveViewWidthPixelsPerInch) 0; };
};
//---------------------------------------------------------------------------
struct TScpCommanderConfiguration {
  UnicodeString WindowParams;
  double LocalPanelWidth;
  UnicodeString ToolbarsLayout;
  bool SessionsTabs;
  bool StatusBar;
  TOperationSide CurrentPanel;
  TNortonLikeMode NortonLikeMode;
  bool PreserveLocalDirectory;
  TScpCommanderPanelConfiguration LocalPanel;
  TScpCommanderPanelConfiguration RemotePanel;
  bool CompareByTime;
  bool CompareBySize;
  bool SwappedPanels;
  bool TreeOnLeft;
  bool ExplorerKeyboardShortcuts;
  bool SystemContextMenu;
  bool __fastcall operator !=(TScpCommanderConfiguration & rhc)
    { return C(WindowParams) C(LocalPanelWidth) C(ToolbarsLayout)
      C(SessionsTabs) C(StatusBar)
      C(LocalPanel) C(RemotePanel) C(CurrentPanel)
      C(NortonLikeMode) C(PreserveLocalDirectory)
      C(CompareBySize) C(CompareByTime) C(SwappedPanels)
      C(TreeOnLeft) C(ExplorerKeyboardShortcuts) C(SystemContextMenu) 0; };

  TCompareCriterias __fastcall CompareCriterias()
  {
    TCompareCriterias Criterias;
    if (CompareByTime)
    {
      Criterias << ccTime;
    }
    if (CompareBySize)
    {
      Criterias << ccSize;
    }
    return Criterias;
  }
};
//---------------------------------------------------------------------------
struct TEditorConfiguration {
  UnicodeString FontName;
  int FontSize;
  int FontCharset;
  int FontStyle;
  bool WordWrap;
  UnicodeString FindText;
  UnicodeString ReplaceText;
  bool FindMatchCase;
  bool FindWholeWord;
  bool FindDown;
  unsigned int TabSize;
  unsigned int MaxEditors;
  unsigned int EarlyClose;
  bool SDIShellEditor;
  UnicodeString WindowParams;
  int Encoding;
  bool WarnOnEncodingFallback;
  bool __fastcall operator !=(TEditorConfiguration & rhc)
    { return C(FontName) C(FontSize)
      C(FontCharset) C(FontStyle) C(WordWrap) C(FindText) C(ReplaceText)
      C(FindMatchCase) C(FindWholeWord) C(FindDown) C(TabSize)
      C(MaxEditors) C(EarlyClose) C(SDIShellEditor) C(WindowParams)
      C(Encoding) C(WarnOnEncodingFallback) 0; };
};
//---------------------------------------------------------------------------
enum TQueueViewShow { qvShow, qvHideWhenEmpty, qvHide };
struct TQueueViewConfiguration {
  int Height;
  int HeightPixelsPerInch;
  UnicodeString Layout;
  TQueueViewShow Show;
  TQueueViewShow LastHideShow;
  bool ToolBar;
  bool Label;
  bool __fastcall operator !=(TQueueViewConfiguration & rhc)
    { return C(Height) C(HeightPixelsPerInch) C(Layout) C(Show) C(LastHideShow) C(ToolBar) C(Label) 0; };
};
//---------------------------------------------------------------------------
struct TUpdatesData
{
  int ForVersion;
  int Version;
  UnicodeString Message;
  bool Critical;
  UnicodeString Release;
  bool Disabled;
  UnicodeString Url;
  UnicodeString UrlButton;
  bool __fastcall operator !=(TUpdatesData & rhc)
    { return C(ForVersion) C(Version) C(Message) C(Critical) C(Release)
             C(Disabled) C(Url) C(UrlButton) 0; };
  void Reset()
  {
    ForVersion = 0;
    Version = 0;
    Message = L"";
    Critical = false;
    Release = L"";
    Disabled = false;
    Url = L"";
    UrlButton = L"";
  }
};
//---------------------------------------------------------------------------
enum TConnectionType { ctDirect, ctAuto, ctProxy };
//---------------------------------------------------------------------------
struct TUpdatesConfiguration
{
  TDateTime Period;
  TDateTime LastCheck;
  TConnectionType ConnectionType;
  UnicodeString ProxyHost;
  int ProxyPort;
  TAutoSwitch BetaVersions;
  bool HaveResults;
  bool ShownResults;
  UnicodeString DotNetVersion;
  UnicodeString ConsoleVersion;
  TUpdatesData Results;
  bool __fastcall operator !=(TUpdatesConfiguration & rhc)
    { return C(Period) C(LastCheck) C(ConnectionType) C(ProxyHost) C(ProxyPort)
        C(BetaVersions) C(HaveResults) C(ShownResults) C(DotNetVersion)
        C(ConsoleVersion) C(Results)  0; };
};
//---------------------------------------------------------------------------
struct TEditorData
{
  __fastcall TEditorData();
  __fastcall TEditorData(const TEditorData & Source);

  TFileMasks FileMask;
  TEditor Editor;
  UnicodeString ExternalEditor;
  bool ExternalEditorText;
  bool SDIExternalEditor;
  bool DetectMDIExternalEditor;

  bool __fastcall operator ==(const TEditorData & rhd) const;
};
//---------------------------------------------------------------------------
#undef C
//---------------------------------------------------------------------------
class TEditorPreferences
{
public:
  __fastcall TEditorPreferences();
  __fastcall TEditorPreferences(const TEditorData & Data);
  bool __fastcall Matches(const UnicodeString FileName, bool Local,
    const TFileMasks::TParams & Params) const;
  void __fastcall Load(THierarchicalStorage * Storage, bool Legacy);
  void __fastcall Save(THierarchicalStorage * Storage) const;
  void __fastcall LegacyDefaults();

  static UnicodeString __fastcall GetDefaultExternalEditor();

  bool __fastcall operator ==(const TEditorPreferences & rhp) const;

  __property const TEditorData * Data = { read = GetConstData };
  __property UnicodeString Name = { read = GetName };

  TEditorData * __fastcall GetData();

private:
  TEditorData FData;
  mutable UnicodeString FName;

  UnicodeString __fastcall GetName() const;
  const TEditorData * __fastcall GetConstData() const { return &FData; };
};
//---------------------------------------------------------------------------
class TEditorList
{
public:
  __fastcall TEditorList();
  virtual __fastcall ~TEditorList();

  const TEditorPreferences * __fastcall Find(const UnicodeString FileName,
    bool Local, const TFileMasks::TParams & Params) const;

  void __fastcall Load(THierarchicalStorage * Storage);
  void __fastcall Save(THierarchicalStorage * Storage) const;

  TEditorList & __fastcall operator=(const TEditorList & rhl);
  bool __fastcall operator==(const TEditorList & rhl) const;

  void __fastcall Clear();
  void __fastcall Add(TEditorPreferences * Editor);
  void __fastcall Insert(int Index, TEditorPreferences * Editor);
  void __fastcall Change(int Index, TEditorPreferences * Editor);
  void __fastcall Move(int CurIndex, int NewIndex);
  void __fastcall Delete(int Index);
  void __fastcall Saved();

  bool __fastcall IsDefaultList() const;

  __property int Count = { read = GetCount };
  __property const TEditorPreferences * Editors[int Index] = { read = GetEditor };
  __property bool Modified = { read = FModified };

private:
  TList * FEditors;
  bool FModified;

  int __fastcall GetCount() const;

  void __fastcall Init();
  void __fastcall Modify();
  const TEditorPreferences * __fastcall GetEditor(int Index) const;
};
//---------------------------------------------------------------------------
class TBookmarks;
class TBookmarkList;
class TCustomCommandList;
enum TPathInCaption { picShort, picFull, picNone };
// constants must be compatible with legacy CopyOnDoubleClick
enum TDoubleClickAction { dcaOpen = 0, dcaCopy = 1, dcaEdit = 2 };
//---------------------------------------------------------------------------
typedef void __fastcall (__closure *TMasterPasswordPromptEvent)();
//---------------------------------------------------------------------------
class TWinConfiguration : public TCustomWinConfiguration
{
private:
  UnicodeString FAutoStartSession;
  TDoubleClickAction FDoubleClickAction;
  bool FCopyOnDoubleClickConfirmation;
  bool FDDAllowMove;
  bool FDDAllowMoveInit;
  TAutoSwitch FDDTransferConfirmation;
  bool FDeleteToRecycleBin;
  bool FDimmHiddenFiles;
  bool FRenameWholeName;
  bool FLogWindowOnStartup;
  UnicodeString FLogWindowParams;
  TScpCommanderConfiguration FScpCommander;
  TScpExplorerConfiguration FScpExplorer;
  bool FSelectDirectories;
  UnicodeString FSelectMask;
  bool FShowHiddenFiles;
  bool FFormatSizeBytes;
  bool FShowInaccesibleDirectories;
  bool FConfirmTransferring;
  bool FConfirmDeleting;
  bool FConfirmRecycling;
  bool FUseLocationProfiles;
  bool FUseSharedBookmarks;
  UnicodeString FDDTemporaryDirectory;
  bool FDDWarnLackOfTempSpace;
  bool FDDExtEnabled;
  int FDDExtInstalled;
  int FDDExtTimeout;
  bool FConfirmClosingSession;
  double FDDWarnLackOfTempSpaceRatio;
  UnicodeString FTemporarySessionFile;
  UnicodeString FTemporaryKeyFile;
  TBookmarks * FBookmarks;
  TCustomCommandList * FCustomCommandList;
  bool FCustomCommandsDefaults;
  TEditorConfiguration FEditor;
  TQueueViewConfiguration FQueueView;
  bool FEnableQueueByDefault;
  bool FEmbeddedSessions;
  bool FExpertMode;
  bool FDisableOpenEdit;
  bool FDefaultDirIsHome;
  int FDDDeleteDelay;
  bool FTemporaryDirectoryAppendSession;
  bool FTemporaryDirectoryAppendPath;
  bool FTemporaryDirectoryCleanup;
  bool FConfirmTemporaryDirectoryCleanup;
  UnicodeString FDefaultTranslationFile;
  UnicodeString FInvalidDefaultTranslationMessage;
  bool FPreservePanelState;
  UnicodeString FTheme;
  UnicodeString FLastStoredSession;
  UnicodeString FLastWorkspace;
  bool FAutoSaveWorkspace;
  bool FAutoSaveWorkspacePasswords;
  UnicodeString FAutoWorkspace;
  TPathInCaption FPathInCaption;
  bool FMinimizeToTray;
  bool FBalloonNotifications;
  unsigned int FNotificationsTimeout;
  unsigned int FNotificationsStickTime;
  TUpdatesConfiguration FUpdates;
  UnicodeString FVersionHistory;
  bool FCopyParamAutoSelectNotice;
  bool FLockToolbars;
  bool FSelectiveToolbarText;
  TEditorList * FEditorList;
  TEditorPreferences * FLegacyEditor;
  UnicodeString FDefaultKeyFile;
  bool FAutoOpenInPutty;
  TDateTime FDefaultUpdatesPeriod;
  bool FRefreshRemotePanel;
  TDateTime FRefreshRemotePanelInterval;
  bool FFullRowSelect;
  bool FOfferedEditorAutoConfig;
  bool FUseMasterPassword;
  UnicodeString FPlainMasterPasswordEncrypt;
  UnicodeString FPlainMasterPasswordDecrypt;
  UnicodeString FMasterPasswordVerifier;
  TMasterPasswordPromptEvent FOnMasterPasswordPrompt;
  UnicodeString FOpenedStoredSessionFolders;
  bool FAutoImportedFromPuttyOrFilezilla;
  int FDontDecryptPasswords;
  int FMasterPasswordSession;
  bool FMasterPasswordSessionAsked;

  void __fastcall SetDoubleClickAction(TDoubleClickAction value);
  void __fastcall SetCopyOnDoubleClickConfirmation(bool value);
  void __fastcall SetDDAllowMove(bool value);
  void __fastcall SetDDAllowMoveInit(bool value);
  void __fastcall SetDDTransferConfirmation(TAutoSwitch value);
  void __fastcall SetDeleteToRecycleBin(bool value);
  void __fastcall SetDimmHiddenFiles(bool value);
  void __fastcall SetRenameWholeName(bool value);
  void __fastcall SetLogWindowOnStartup(bool value);
  void __fastcall SetLogWindowParams(UnicodeString value);
  void __fastcall SetScpCommander(TScpCommanderConfiguration value);
  void __fastcall SetScpExplorer(TScpExplorerConfiguration value);
  void __fastcall SetSelectDirectories(bool value);
  void __fastcall SetShowHiddenFiles(bool value);
  void __fastcall SetFormatSizeBytes(bool value);
  void __fastcall SetShowInaccesibleDirectories(bool value);
  void __fastcall SetConfirmTransferring(bool value);
  void __fastcall SetConfirmDeleting(bool value);
  void __fastcall SetConfirmRecycling(bool value);
  void __fastcall SetUseLocationProfiles(bool value);
  void __fastcall SetUseSharedBookmarks(bool value);
  void __fastcall SetDDTemporaryDirectory(UnicodeString value);
  void __fastcall SetDDWarnLackOfTempSpace(bool value);
  void __fastcall SetDDExtEnabled(bool value);
  void __fastcall SetDDExtTimeout(int value);
  void __fastcall SetConfirmClosingSession(bool value);
  void __fastcall SetDDWarnLackOfTempSpaceRatio(double value);
  void __fastcall SetBookmarks(UnicodeString Key, TBookmarkList * value);
  TBookmarkList * __fastcall GetBookmarks(UnicodeString Key);
  void __fastcall SetSharedBookmarks(TBookmarkList * value);
  TBookmarkList * __fastcall GetSharedBookmarks();
  void __fastcall SetAutoStartSession(UnicodeString value);
  void __fastcall SetExpertMode(bool value);
  void __fastcall SetDefaultDirIsHome(bool value);
  void __fastcall SetEditor(TEditorConfiguration value);
  void __fastcall SetQueueView(TQueueViewConfiguration value);
  void __fastcall SetEnableQueueByDefault(bool value);
  void __fastcall SetCustomCommandList(TCustomCommandList * value);
  void __fastcall SetTemporaryDirectoryAppendSession(bool value);
  void __fastcall SetTemporaryDirectoryAppendPath(bool value);
  void __fastcall SetTemporaryDirectoryCleanup(bool value);
  void __fastcall SetConfirmTemporaryDirectoryCleanup(bool value);
  void __fastcall SetPreservePanelState(bool value);
  void __fastcall SetTheme(UnicodeString value);
  void __fastcall SetLastStoredSession(UnicodeString value);
  void __fastcall SetAutoSaveWorkspace(bool value);
  void __fastcall SetAutoSaveWorkspacePasswords(bool value);
  void __fastcall SetAutoWorkspace(UnicodeString value);
  void __fastcall SetPathInCaption(TPathInCaption value);
  void __fastcall SetMinimizeToTray(bool value);
  void __fastcall SetBalloonNotifications(bool value);
  void __fastcall SetNotificationsTimeout(unsigned int value);
  void __fastcall SetNotificationsStickTime(unsigned int value);
  void __fastcall SetCopyParamAutoSelectNotice(bool value);
  TUpdatesConfiguration __fastcall GetUpdates();
  void __fastcall SetUpdates(TUpdatesConfiguration value);
  void __fastcall SetVersionHistory(UnicodeString value);
  void __fastcall SetLockToolbars(bool value);
  void __fastcall SetSelectiveToolbarText(bool value);
  const TEditorList * __fastcall GetEditorList();
  void __fastcall SetEditorList(const TEditorList * value);
  void __fastcall SetAutoOpenInPutty(bool value);
  void __fastcall SetRefreshRemotePanel(bool value);
  void __fastcall SetRefreshRemotePanelInterval(TDateTime value);
  void __fastcall SetFullRowSelect(bool value);
  void __fastcall SetOfferedEditorAutoConfig(bool value);
  void __fastcall SetLastMonitor(int value);
  int __fastcall GetLastMonitor();
  void __fastcall SetOpenedStoredSessionFolders(UnicodeString value);
  void __fastcall SetAutoImportedFromPuttyOrFilezilla(bool value);
  bool __fastcall GetIsBeta();

  bool __fastcall GetDDExtInstalled();
  void __fastcall AddVersionToHistory();
  bool __fastcall GetAnyBetaInVersionHistory();
  void __fastcall PurgePassword(UnicodeString & Password);
  void __fastcall UpdateEntryInJumpList(
    bool Session, const UnicodeString & Name, bool Add);
  TStringList * __fastcall LoadJumpList(THierarchicalStorage * Storage,
    UnicodeString Name);
  void __fastcall SaveJumpList(THierarchicalStorage * Storage,
    UnicodeString Name, TStringList * List);
  void __fastcall TrimJumpList(TStringList * List);

protected:
  virtual TStorage __fastcall GetStorage();
  virtual void __fastcall SaveData(THierarchicalStorage * Storage, bool All);
  virtual void __fastcall LoadData(THierarchicalStorage * Storage);
  virtual void __fastcall LoadFrom(THierarchicalStorage * Storage);
  virtual void __fastcall LoadAdmin(THierarchicalStorage * Storage);
  virtual void __fastcall CopyData(THierarchicalStorage * Source, THierarchicalStorage * Target);
  virtual UnicodeString __fastcall GetDefaultKeyFile();
  virtual void __fastcall Saved();
  void __fastcall RecryptPasswords(TStrings * RecryptPasswordErrors);
  virtual bool __fastcall GetUseMasterPassword();
  bool __fastcall SameStringLists(TStrings * Strings1, TStrings * Strings2);
  bool __fastcall InternalReloadComponentRes(const UnicodeString ResName,
    HINSTANCE HInst, TComponent * Instance);
  bool __fastcall InitComponent(TComponent * Instance,
    TClass RootAncestor, TClass ClassType);
  virtual HINSTANCE __fastcall LoadNewResourceModule(LCID Locale,
    UnicodeString * FileName = NULL);
  virtual void __fastcall SetResourceModule(HINSTANCE Instance);
  virtual LCID __fastcall GetLocale();
  void __fastcall CheckTranslationVersion(const UnicodeString FileName,
    bool InternalLocaleOnError);
  virtual void __fastcall DefaultLocalized();
  bool __fastcall DetectRegistryStorage(HKEY RootKey);
  bool __fastcall CanWriteToStorage();
  bool __fastcall DoIsBeta(const UnicodeString & ReleaseType);
  void __fastcall AskForMasterPassword();

public:
  __fastcall TWinConfiguration();
  virtual __fastcall ~TWinConfiguration();
  virtual void __fastcall Default();
  void __fastcall ClearTemporaryLoginData();
  virtual THierarchicalStorage * CreateScpStorage(bool SessionList);
  UnicodeString __fastcall TemporaryDir(bool Mask = false);
  TStrings * __fastcall FindTemporaryFolders();
  void __fastcall CleanupTemporaryFolders(TStrings * Folders = NULL);
  void __fastcall CheckDefaultTranslation();
  const TEditorPreferences * __fastcall DefaultEditorForFile(
    const UnicodeString FileName, bool Local, const TFileMasks::TParams & MaskParams);
  virtual UnicodeString __fastcall DecryptPassword(RawByteString Password, UnicodeString Key);
  virtual RawByteString __fastcall StronglyRecryptPassword(RawByteString Password, UnicodeString Key);
  void __fastcall SetMasterPassword(UnicodeString value);
  void __fastcall ChangeMasterPassword(UnicodeString value, TStrings * RecryptPasswordErrors);
  bool __fastcall ValidateMasterPassword(UnicodeString value);
  void __fastcall ClearMasterPassword(TStrings * RecryptPasswordErrors);
  void __fastcall BeginMasterPasswordSession();
  void __fastcall EndMasterPasswordSession();
  virtual void __fastcall AskForMasterPasswordIfNotSet();
  void __fastcall AddSessionToJumpList(UnicodeString SessionName);
  void __fastcall DeleteSessionFromJumpList(UnicodeString SessionName);
  void __fastcall AddWorkspaceToJumpList(UnicodeString Workspace);
  void __fastcall DeleteWorkspaceFromJumpList(UnicodeString Workspace);
  void __fastcall UpdateJumpList();
  virtual void __fastcall UpdateStaticUsage();

  __property TScpCommanderConfiguration ScpCommander = { read = FScpCommander, write = SetScpCommander };
  __property TScpExplorerConfiguration ScpExplorer = { read = FScpExplorer, write = SetScpExplorer };
  __property bool SelectDirectories = { read = FSelectDirectories, write = SetSelectDirectories };
  __property UnicodeString SelectMask = { read = FSelectMask, write = FSelectMask };
  __property bool ShowHiddenFiles = { read = FShowHiddenFiles, write = SetShowHiddenFiles };
  __property bool FormatSizeBytes = { read = FFormatSizeBytes, write = SetFormatSizeBytes };
  __property bool ShowInaccesibleDirectories = { read = FShowInaccesibleDirectories, write = SetShowInaccesibleDirectories };
  __property TEditorConfiguration Editor = { read = FEditor, write = SetEditor };
  __property TQueueViewConfiguration QueueView = { read = FQueueView, write = SetQueueView };
  __property bool EnableQueueByDefault = { read = FEnableQueueByDefault, write = SetEnableQueueByDefault };
  __property TUpdatesConfiguration Updates = { read = GetUpdates, write = SetUpdates };
  __property UnicodeString VersionHistory = { read = FVersionHistory, write = SetVersionHistory };
  __property bool AnyBetaInVersionHistory = { read = GetAnyBetaInVersionHistory };
  __property bool IsBeta = { read = GetIsBeta };
  __property UnicodeString AutoStartSession = { read = FAutoStartSession, write = SetAutoStartSession };
  __property TDoubleClickAction DoubleClickAction = { read = FDoubleClickAction, write = SetDoubleClickAction };
  __property bool CopyOnDoubleClickConfirmation = { read = FCopyOnDoubleClickConfirmation, write = SetCopyOnDoubleClickConfirmation };
  __property bool DDAllowMove = { read = FDDAllowMove, write = SetDDAllowMove };
  __property bool DDAllowMoveInit = { read = FDDAllowMoveInit, write = SetDDAllowMoveInit };
  __property TAutoSwitch DDTransferConfirmation = { read = FDDTransferConfirmation, write = SetDDTransferConfirmation };
  __property bool LogWindowOnStartup = { read = FLogWindowOnStartup, write = SetLogWindowOnStartup };
  __property bool DeleteToRecycleBin = { read = FDeleteToRecycleBin, write = SetDeleteToRecycleBin };
  __property bool DimmHiddenFiles = { read = FDimmHiddenFiles, write = SetDimmHiddenFiles };
  __property bool RenameWholeName = { read = FRenameWholeName, write = SetRenameWholeName };
  __property UnicodeString LogWindowParams = { read = FLogWindowParams, write = SetLogWindowParams };
  __property bool ConfirmTransferring = { read = FConfirmTransferring, write = SetConfirmTransferring};
  __property bool ConfirmDeleting = { read = FConfirmDeleting, write = SetConfirmDeleting};
  __property bool ConfirmRecycling = { read = FConfirmRecycling, write = SetConfirmRecycling};
  __property bool UseLocationProfiles = { read = FUseLocationProfiles, write = SetUseLocationProfiles};
  __property bool UseSharedBookmarks = { read = FUseSharedBookmarks, write = SetUseSharedBookmarks};
  __property UnicodeString DDTemporaryDirectory  = { read=FDDTemporaryDirectory, write=SetDDTemporaryDirectory };
  __property bool DDWarnLackOfTempSpace  = { read=FDDWarnLackOfTempSpace, write=SetDDWarnLackOfTempSpace };
  __property bool DDExtEnabled = { read=FDDExtEnabled, write=SetDDExtEnabled };
  __property bool DDExtInstalled = { read=GetDDExtInstalled };
  __property int DDExtTimeout = { read=FDDExtTimeout, write=SetDDExtTimeout };
  __property bool ConfirmClosingSession  = { read=FConfirmClosingSession, write=SetConfirmClosingSession };
  __property double DDWarnLackOfTempSpaceRatio  = { read=FDDWarnLackOfTempSpaceRatio, write=SetDDWarnLackOfTempSpaceRatio };
  __property TBookmarkList * Bookmarks[UnicodeString Key] = { read = GetBookmarks, write = SetBookmarks };
  __property TBookmarkList * SharedBookmarks = { read = GetSharedBookmarks, write = SetSharedBookmarks };
  __property bool EmbeddedSessions = { read = FEmbeddedSessions };
  __property bool ExpertMode = { read = FExpertMode, write = SetExpertMode };
  __property bool DefaultDirIsHome = { read = FDefaultDirIsHome, write = SetDefaultDirIsHome };
  __property bool DisableOpenEdit = { read = FDisableOpenEdit };
  __property TCustomCommandList * CustomCommandList = { read = FCustomCommandList, write = SetCustomCommandList };
  __property int DDDeleteDelay = { read = FDDDeleteDelay };
  __property bool TemporaryDirectoryAppendSession = { read = FTemporaryDirectoryAppendSession, write = SetTemporaryDirectoryAppendSession };
  __property bool TemporaryDirectoryAppendPath = { read = FTemporaryDirectoryAppendPath, write = SetTemporaryDirectoryAppendPath };
  __property bool TemporaryDirectoryCleanup = { read = FTemporaryDirectoryCleanup, write = SetTemporaryDirectoryCleanup };
  __property bool ConfirmTemporaryDirectoryCleanup = { read = FConfirmTemporaryDirectoryCleanup, write = SetConfirmTemporaryDirectoryCleanup };
  __property bool PreservePanelState = { read = FPreservePanelState, write = SetPreservePanelState };
  __property UnicodeString Theme = { read = FTheme, write = SetTheme };
  __property UnicodeString LastStoredSession = { read = FLastStoredSession, write = SetLastStoredSession };
  __property UnicodeString LastWorkspace = { read = FLastWorkspace, write = FLastWorkspace };
  __property bool AutoSaveWorkspace = { read = FAutoSaveWorkspace, write = SetAutoSaveWorkspace };
  __property bool AutoSaveWorkspacePasswords = { read = FAutoSaveWorkspacePasswords, write = SetAutoSaveWorkspacePasswords };
  __property UnicodeString AutoWorkspace = { read = FAutoWorkspace, write = SetAutoWorkspace };
  __property TPathInCaption PathInCaption = { read = FPathInCaption, write = SetPathInCaption };
  __property bool MinimizeToTray = { read = FMinimizeToTray, write = SetMinimizeToTray };
  __property bool BalloonNotifications = { read = FBalloonNotifications, write = SetBalloonNotifications };
  __property unsigned int NotificationsTimeout = { read = FNotificationsTimeout, write = SetNotificationsTimeout };
  __property unsigned int NotificationsStickTime = { read = FNotificationsStickTime, write = SetNotificationsStickTime };
  __property UnicodeString DefaultTranslationFile = { read = FDefaultTranslationFile };
  __property bool CopyParamAutoSelectNotice = { read = FCopyParamAutoSelectNotice, write = SetCopyParamAutoSelectNotice };
  __property bool LockToolbars = { read = FLockToolbars, write = SetLockToolbars };
  __property bool SelectiveToolbarText = { read = FSelectiveToolbarText, write = SetSelectiveToolbarText };
  __property bool AutoOpenInPutty = { read = FAutoOpenInPutty, write = SetAutoOpenInPutty };
  __property bool RefreshRemotePanel = { read = FRefreshRemotePanel, write = SetRefreshRemotePanel };
  __property TDateTime RefreshRemotePanelInterval = { read = FRefreshRemotePanelInterval, write = SetRefreshRemotePanelInterval };
  __property bool FullRowSelect = { read = FFullRowSelect, write = SetFullRowSelect };
  __property bool OfferedEditorAutoConfig = { read = FOfferedEditorAutoConfig, write = SetOfferedEditorAutoConfig };
  __property int LastMonitor = { read = GetLastMonitor, write = SetLastMonitor };
  __property const TEditorList * EditorList = { read = GetEditorList, write = SetEditorList };
  __property UnicodeString DefaultKeyFile = { read = GetDefaultKeyFile, write = FDefaultKeyFile };
  __property UnicodeString OpenedStoredSessionFolders = { read = FOpenedStoredSessionFolders, write = SetOpenedStoredSessionFolders };
  __property bool AutoImportedFromPuttyOrFilezilla = { read = FAutoImportedFromPuttyOrFilezilla, write = SetAutoImportedFromPuttyOrFilezilla };
  __property TMasterPasswordPromptEvent OnMasterPasswordPrompt = { read = FOnMasterPasswordPrompt, write = FOnMasterPasswordPrompt };
};
//---------------------------------------------------------------------------
class TCustomCommandType
{
public:
  __fastcall TCustomCommandType();
  __fastcall TCustomCommandType(const TCustomCommandType & Other);

  TCustomCommandType & operator=(const TCustomCommandType & Other);
  bool __fastcall Equals(const TCustomCommandType * Other) const;

  __property UnicodeString Name = { read = FName, write = FName };
  __property UnicodeString Command = { read = FCommand, write = FCommand };
  __property int Params = { read = FParams, write = FParams };
  __property TShortCut ShortCut = { read = FShortCut, write = FShortCut };

private:
  UnicodeString FName;
  UnicodeString FCommand;
  int FParams;
  TShortCut FShortCut;
};
//---------------------------------------------------------------------------
class TCustomCommandList
{
public:
  __fastcall TCustomCommandList();
  __fastcall ~TCustomCommandList();

  void __fastcall Load(THierarchicalStorage * Storage);
  void __fastcall Save(THierarchicalStorage * Storage);
  void __fastcall Reset();
  void __fastcall Modify();

  void __fastcall Clear();
  void __fastcall Add(const UnicodeString Name, const UnicodeString Command, int Params);
  void __fastcall Add(TCustomCommandType * Command);
  void __fastcall Insert(int Index, TCustomCommandType * Command);
  void __fastcall Change(int Index, TCustomCommandType * Command);
  void __fastcall Move(int CurIndex, int NewIndex);
  void __fastcall Delete(int Index);

  const TCustomCommandType * Find(const UnicodeString Name) const;
  const TCustomCommandType * Find(TShortCut ShortCut) const;

  bool __fastcall Equals(const TCustomCommandList * Other) const;
  void __fastcall Assign(const TCustomCommandList * Other);

  void __fastcall ShortCuts(TShortCuts & ShortCuts) const;

  __property bool Modified = { read = FModified };
  __property int Count = { read = GetCount };
  __property const TCustomCommandType * Commands[int Index] = { read = GetConstCommand };

private:
  bool FModified;
  TList * FCommands;
  int __fastcall GetCount() const;
  const TCustomCommandType * __fastcall GetConstCommand(int Index) const;
  TCustomCommandType * __fastcall GetCommand(int Index);
};
//---------------------------------------------------------------------------
#define WinConfiguration (dynamic_cast<TWinConfiguration *>(Configuration))
//---------------------------------------------------------------------------
#endif
