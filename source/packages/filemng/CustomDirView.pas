unit CustomDirView;

interface

{$R DirImg.res}

{$WARN UNIT_PLATFORM OFF}
{$WARN SYMBOL_PLATFORM OFF}

uses
  Windows, Messages, Classes, Graphics, Controls,
  Forms, ComCtrls, ShellAPI, ComObj, ShlObj, Dialogs,
  ActiveX, CommCtrl, Extctrls, ImgList, Menus, FileCtrl,
  PIDL, BaseUtils, DragDrop, DragDropFilesEx, IEDriveInfo,
  IEListView, PathLabel, SysUtils, PasTools;

const
  clDefaultItemColor = -(COLOR_ENDCOLORS + 1);
  WM_USER_RENAME = WM_USER + 57;
  oiNoOverlay = $00;
  oiDirUp = $01;
  oiLink = $02;
  oiBrokenLink = $04;
  oiPartial = $08;
  oiShared = $10; // not used
  DefaultHistoryCount = 200;

const
  DDMaxSlowCount = 3;
  DDVScrollDelay = 2000000;
  DDHScrollDelay = 2000000;
  DDDragStartDelay = 500000;
  DirAttrMask = SysUtils.faDirectory or SysUtils.faSysFile or SysUtils.faHidden;

{ Win2000 and newer only }
const
  _WM_XBUTTONUP = $020C;
  _WM_APPCOMMAND = $0319;
  _XBUTTON1 =     $0001;
  _XBUTTON2 =     $0002;
  _APPCOMMAND_BROWSER_BACKWARD   = 1;
  _APPCOMMAND_BROWSER_FORWARD    = 2;
  _APPCOMMAND_BROWSER_REFRESH    = 3;
  _APPCOMMAND_BROWSER_STOP       = 4;
  _APPCOMMAND_BROWSER_SEARCH     = 5;
  _APPCOMMAND_BROWSER_FAVORITES  = 6;
  _APPCOMMAND_BROWSER_HOME       = 7;
  _VK_BROWSER_BACK =      $A6;
  _VK_BROWSER_FORWARD =   $A7;
  _VK_BROWSER_REFRESH =   $A8;
  _VK_BROWSER_STOP =      $A9;
  _VK_BROWSER_SEARCH =    $AA;
  _VK_BROWSER_FAVORITES = $AB;
  _VK_BROWSER_HOME =      $AC;
  _FAPPCOMMAND_MOUSE =    $8000;
  _FAPPCOMMAND_KEY =      0;
  _FAPPCOMMAND_OEM =      $1000;
  _FAPPCOMMAND_MASK =     $F000;

type
  TStatusFileInfo = record
    FilesCount: Integer;
    SelectedCount: Integer;
    FilesSize: Int64;
    SelectedSize: Int64;
    HiddenCount: Integer;
    FilteredCount: Integer;
  end;

type
  {Drag&Drop events:}
  TDDError = (DDCreateShortCutError, DDPathNotFoundError);
  TDDOnDragEnter = procedure(Sender: TObject; DataObj: IDataObject; grfKeyState: Longint; Point: TPoint; var dwEffect: Longint; var Accept: Boolean) of object;
  TDDOnDragLeave = procedure(Sender: TObject) of object;
  TDDOnDragOver = procedure(Sender: TObject; grfKeyState: Longint; Point: TPoint; var dwEffect: Longint) of object;
  TDDOnDrop = procedure(Sender: TObject; DataObj: IDataObject; grfKeyState: Longint; Point: TPoint; var dwEffect: Longint) of object;
  TDDOnQueryContinueDrag = procedure(Sender: TObject; FEscapePressed: BOOL; grfKeyState: Longint; var Result: HResult) of object;
  TDDOnGiveFeedback = procedure(Sender: TObject; dwEffect: Longint; var Result: HResult) of object;
  TDDOnChooseEffect = procedure(Sender: TObject; grfKeyState: Longint; var dwEffect: Longint) of object;
  TDDOnDragDetect = procedure(Sender: TObject; grfKeyState: Longint; DetectStart, Point: TPoint; DragStatus: TDragDetectStatus) of object;
  TDDOnCreateDragFileList = procedure(Sender: TObject; FileList: TFileList; var Created: Boolean) of object;
  TDDOnCreateDataObject = procedure(Sender: TObject; var DataObject: TDataObject) of object;
  TDDOnTargetHasDropHandler = procedure(Sender: TObject; Item: TListItem; var Effect: Integer; var DropHandler: Boolean) of object;
  TOnProcessDropped = procedure(Sender: TObject; grfKeyState: Longint; Point: TPoint; var dwEffect: Longint) of object;

  TDDErrorEvent = procedure(Sender: TObject; ErrorNo: TDDError) of object;
  TDDExecutedEvent = procedure(Sender: TObject; dwEffect: Longint) of object;
  TDDFileOperationEvent = procedure(Sender: TObject; dwEffect: LongInt; SourcePath, TargetPath: string;
    var DoOperation: Boolean) of object;
  TDDFileOperationExecutedEvent = procedure(Sender: TObject; dwEffect: LongInt; SourcePath, TargetPath: string) of object;

  TDirViewExecFileEvent = procedure(Sender: TObject; Item: TListItem; var AllowExec: Boolean) of object;
  TRenameEvent = procedure(Sender: TObject; Item: TListItem; NewName: string) of object;
  TMatchMaskEvent = procedure(Sender: TObject; FileName: string; Directory: Boolean; Size: Int64; Modification: TDateTime; Masks: string; var Matches: Boolean; AllowImplicitMatches: Boolean) of object;
  TDirViewGetOverlayEvent = procedure(Sender: TObject; Item: TListItem; var Indexes: Word) of object;
  TDirViewUpdateStatusBarEvent = procedure(Sender: TObject; const FileInfo: TStatusFileInfo) of object;

type
  TCustomDirView = class;
  TSelAttr = (selDontCare, selYes, selNo);
  TFileFilter = record
    Masks: string;
    IncludeAttr: Word; { see TSearchRec.Attr }
    ExcludeAttr: Word;
    Directories: Boolean;
    FileSizeFrom: Int64;
    FileSizeTo: Int64;
    ModificationFrom: TDateTime;
    ModificationTo: TDateTime;
  end;
  TDirViewNotifyEvent = procedure(Sender: TCustomDirView) of object;
  TDVGetFilterEvent = procedure(Sender: TCustomDirView; Select: Boolean;
    var Filter: TFileFilter) of object;
  TDVHistoryGoEvent = procedure(Sender: TCustomDirView; Index: Integer; var Cancel: Boolean) of object;
  TCompareCriteria = (ccTime, ccSize);
  TCompareCriterias = set of TCompareCriteria;

  TWMXMouse = packed record
    Msg: Cardinal;
    Keys: Word;
    Button: Word;
    Pos: TSmallPoint;
    Result: Longint
  end;

  TCustomizableDragDropFilesEx = class(TDragDropFilesEx)
  public
    function Execute(DataObject: TDataObject): TDragResult;
  end;

  TCustomDirView = class(TCustomIEListView)
  private
    FAddParentDir: Boolean;
    FDimmHiddenFiles: Boolean;
    FShowDirectories: Boolean;
    FShowSubDirSize: Boolean;
    FFormatSizeBytes: Boolean;
    FSortByExtension: Boolean;
    FWantUseDragImages: Boolean;
    FDragDropFilesEx: TCustomizableDragDropFilesEx;
    FSingleClickToExec: Boolean;
    FUseSystemContextMenu: Boolean;
    FOnGetSelectFilter: TDVGetFilterEvent;
    FOnStartLoading: TNotifyEvent;
    FOnLoaded: TNotifyEvent;
    FOnDirUpdated: TNotifyEvent;
    FReloadTime: TSystemTime;
    FDragDrive: TDrive;
    FExeDrag: Boolean;
    FDDLinkOnExeDrag: Boolean;
    FOnDDDragEnter: TDDOnDragEnter;
    FOnDDDragLeave: TDDOnDragLeave;
    FOnDDDragOver: TDDOnDragOver;
    FOnDDDrop: TDDOnDrop;
    FOnDDQueryContinueDrag: TDDOnQueryContinueDrag;
    FOnDDGiveFeedback: TDDOnGiveFeedback;
    FOnDDChooseEffect: TDDOnChooseEffect;
    FOnDDDragDetect: TDDOnDragDetect;
    FOnDDCreateDragFileList: TDDOnCreateDragFileList;
    FOnDDProcessDropped: TOnProcessDropped;
    FOnDDError: TDDErrorEvent;
    FOnDDExecuted: TDDExecutedEvent;
    FOnDDFileOperation: TDDFileOperationEvent;
    FOnDDFileOperationExecuted: TDDFileOperationExecutedEvent;
    FOnDDEnd: TNotifyEvent;
    FOnDDCreateDataObject: TDDOnCreateDataObject;
    FOnDDTargetHasDropHandler: TDDOnTargetHasDropHandler;
    FOnDDMenuPopup: TOnMenuPopup;
    FOnExecFile: TDirViewExecFileEvent;
    FForceRename: Boolean;
    FLastDDResult: TDragResult;
    FLastRenameName: string;
    FContextMenu: Boolean;
    FDragEnabled: Boolean;
    FDragPos: TPoint;
    FStartPos: TPoint;
    FDDOwnerIsSource: Boolean;
    FAbortLoading: Boolean;
    FAnimation: TAnimate;
    FBackCount: Integer;
    FDontRecordPath: Boolean;
    FDragOnDriveIsMove: Boolean;
    FNotifyEnabled: Boolean;
    FDragStartTime: TFileTime;
    FHistoryPaths: TStrings;
    FImageList16: TImageList;
    FImageList32: TImageList;
    FLoadAnimation: Boolean;
    FMaxHistoryCount: Integer;
    FPathLabel: TCustomPathLabel;
    FOnUpdateStatusBar: TDirViewUpdateStatusBarEvent;
    FOnBeginRename: TRenameEvent;
    FOnEndRename: TRenameEvent;
    FOnHistoryChange: TDirViewNotifyEvent;
    FOnHistoryGo: TDVHistoryGoEvent;
    FOnPathChange: TDirViewNotifyEvent;
    FShowHiddenFiles: Boolean;
    FSavedSelection: Boolean;
    FSavedSelectionFile: string;
    FSavedSelectionLastFile: string;
    FSavedNames: TStringList;
    FPendingFocusSomething: Boolean;
    FOnMatchMask: TMatchMaskEvent;
    FOnGetOverlay: TDirViewGetOverlayEvent;
    FMask: string;
    FScrollOnDragOver: TListViewScrollOnDragOver;
    FStatusFileInfo: TStatusFileInfo;
    FDoubleBufferedScrollingWorkaround: Boolean;

    procedure CNNotify(var Message: TWMNotify); message CN_NOTIFY;
    procedure WMKeyDown(var Message: TWMKeyDown); message WM_KEYDOWN;
    procedure WMLButtonDblClk(var Message: TWMLButtonDblClk); message WM_LBUTTONDBLCLK;
    procedure WMLButtonUp(var Message: TWMLButtonUp); message WM_LBUTTONUP;
    procedure WMContextMenu(var Message: TWMContextMenu); message WM_CONTEXTMENU;
    procedure WMLButtonDown(var Message: TWMLButtonDown); message WM_LBUTTONDOWN;
    procedure WMRButtonDown(var Message: TWMRButtonDown); message WM_RBUTTONDOWN;
    procedure WMXButtonUp(var Message: TWMXMouse); message _WM_XBUTTONUP;
    procedure WMAppCommand(var Message: TMessage); message _WM_APPCOMMAND;
    procedure CMColorChanged(var Message: TMessage); message CM_COLORCHANGED;
    procedure LVMSetExtendedListViewStyle(var Message: TMessage); message LVM_SETEXTENDEDLISTVIEWSTYLE;

    procedure DumbCustomDrawItem(Sender: TCustomListView; Item: TListItem;
      State: TCustomDrawState; var DefaultDraw: Boolean);
    procedure DumbCustomDrawSubItem(Sender: TCustomListView;
      Item: TListItem; SubItem: Integer; State: TCustomDrawState;
      var DefaultDraw: Boolean);
    function GetFilesMarkedSize: Int64;
    function GetForwardCount: Integer;
    function GetHistoryPath(Index: Integer): string;
    function GetSelectedNamesSaved: Boolean;

    function GetTargetPopupMenu: Boolean;
    function GetUseDragImages: Boolean;
    procedure SetMaxHistoryCount(Value: Integer);
    procedure SetPathLabel(Value: TCustomPathLabel);
    procedure SetTargetPopupMenu(Value: Boolean);
    procedure WMUserRename(var Message: TMessage); message WM_User_Rename;
  protected
    FCaseSensitive: Boolean;
    FDirty: Boolean;
    FFilesSize: Int64;
    FFilesSelSize: Int64;
    FHasParentDir: Boolean;
    FIsRecycleBin: Boolean;
    FLastPath: string;
    FHistoryPath: string;
    FLoadEnabled: Boolean;
    FLoading: Boolean;
    FSelectFile: string;
    FWatchForChanges: Boolean;
    FInvalidNameChars: string;

    procedure AddToDragFileList(FileList: TFileList; Item: TListItem); virtual;
    function CanEdit(Item: TListItem): Boolean; override;
    function CanChangeSelection(Item: TListItem; Select: Boolean): Boolean; override;
    procedure CancelEdit;
    procedure ClearItems; override;
    function GetDirOK: Boolean; virtual; abstract;
    procedure DDDragDetect(grfKeyState: Longint; DetectStart, Point: TPoint; DragStatus: TDragDetectStatus); virtual;
    procedure DDDragEnter(DataObj: IDataObject; grfKeyState: Longint; Point: TPoint; var dwEffect: longint; var Accept: Boolean);
    procedure DDDragLeave;
    procedure DDDragOver(grfKeyState: Longint; Point: TPoint; var dwEffect: Longint);
    procedure DDChooseEffect(grfKeyState: Integer; var dwEffect: Integer); virtual;
    procedure DDDrop(DataObj: IDataObject; grfKeyState: LongInt; Point: TPoint; var dwEffect: Longint);
    procedure DDDropHandlerSucceeded(Sender: TObject; grfKeyState: Longint; Point: TPoint; dwEffect: Longint); virtual;
    procedure DDGiveFeedback(dwEffect: Longint; var Result: HResult); virtual;
    procedure DDMenuPopup(Sender: TObject; AMenu: HMenu; DataObj: IDataObject;
      AMinCustCmd:integer; grfKeyState: Longint; pt: TPoint); virtual;
    procedure DDMenuDone(Sender: TObject; AMenu: HMenu); virtual;
    procedure DDProcessDropped(Sender: TObject; grfKeyState: Longint;
      Point: TPoint; dwEffect: Longint);
    procedure DDQueryContinueDrag(FEscapePressed: LongBool;
      grfKeyState: Longint; var Result: HResult); virtual;
    procedure DDSpecifyDropTarget(Sender: TObject; DragDropHandler: Boolean;
      Point: TPoint; var pidlFQ : PItemIDList; var Filename: string); virtual;
    procedure GetDisplayInfo(ListItem: TListItem; var DispInfo: TLVItem); virtual;
    function GetDragSourceEffects: TDropEffectSet; virtual;
    function GetPathName: string; virtual; abstract;
    function GetFilesCount: Integer; virtual;
    procedure ColClick(Column: TListColumn); override;
    procedure CreateWnd; override;
    procedure DestroyWnd; override;
    function CustomCreateFileList(Focused, OnlyFocused: Boolean;
      FullPath: Boolean; FileList: TStrings = nil; ItemObject: Boolean = False): TStrings;
    function CustomDrawItem(Item: TListItem; State: TCustomDrawState;
      Stage: TCustomDrawStage): Boolean; override;
    function CustomDrawSubItem(Item: TListItem; SubItem: Integer;
      State: TCustomDrawState; Stage: TCustomDrawStage): Boolean; override;
    procedure CustomSortItems(SortProc: Pointer);
    procedure Delete(Item: TListItem); override;
    procedure DoAnimation(Start: Boolean);
    procedure DoHistoryChange; dynamic;
    function DragCompleteFileList: Boolean; virtual;
    procedure Edit(const HItem: TLVItem); override;
    procedure EndSelectionUpdate; override;
    procedure Execute(Item: TListItem); virtual;
    procedure ExecuteFile(Item: TListItem); virtual; abstract;
    procedure FocusSomething; override;
    function GetIsRoot: Boolean; virtual; abstract;
    procedure IconsSetImageList; virtual;
    function ItemCanDrag(Item: TListItem): Boolean; virtual;
    function ItemColor(Item: TListItem): TColor; virtual;
    function ItemImageIndex(Item: TListItem; Cache: Boolean): Integer; virtual; abstract;
    // ItemIsDirectory and ItemFullFileName is in public block
    function ItemIsRecycleBin(Item: TListItem): Boolean; virtual;
    procedure KeyDown(var Key: Word; Shift: TShiftState); override;
    procedure KeyPress(var Key: Char); override;
    procedure KeyUp(var Key: Word; Shift: TShiftState); override;
    procedure LoadFiles; virtual; abstract;
    procedure PerformItemDragDropOperation(Item: TListItem; Effect: Integer); virtual; abstract;
    procedure ProcessChangedFiles(DirView: TCustomDirView;
      FileList: TStrings; FullPath: Boolean; ExistingOnly: Boolean;
      Criterias: TCompareCriterias);
    procedure ReloadForce(CacheIcons : Boolean);
    procedure RetryRename(NewName: string);
    procedure SelectFiles(Filter: TFileFilter; Select: Boolean);
    procedure SetAddParentDir(Value: Boolean); virtual;
    procedure SetDimmHiddenFiles(Value: Boolean); virtual;
    procedure SetShowDirectories(Value: Boolean); virtual;
    procedure SetItemImageIndex(Item: TListItem; Index: Integer); virtual; abstract;
    procedure SetLoadEnabled(Enabled : Boolean); virtual;
    procedure SetMultiSelect(Value: Boolean); override;
    function GetPath: string; virtual; abstract;
    function GetValid: Boolean; override;
    procedure InternalEdit(const HItem: TLVItem); virtual; abstract;
    function ItemIsFile(Item: TListItem): Boolean; virtual; abstract;
    function ItemMatchesFilter(Item: TListItem; const Filter: TFileFilter): Boolean; virtual; abstract;
    function ItemOverlayIndexes(Item: TListItem): Word; virtual;
    procedure LimitHistorySize;
    procedure Notification(AComponent: TComponent; Operation: TOperation); override;
    procedure PathChanged; virtual;
    procedure PathChanging(Relative: Boolean);
    procedure SetPath(Value: string); virtual; abstract;
    procedure SetSortByExtension(Value: Boolean);
    procedure SetShowHiddenFiles(Value: Boolean); virtual;
    procedure SetShowSubDirSize(Value: Boolean); virtual;
    procedure SetFormatSizeBytes(Value: Boolean);
    procedure SetViewStyle(Value: TViewStyle); override;
    procedure SetWatchForChanges(Value: Boolean); virtual;
    function TargetHasDropHandler(Item: TListItem; Effect: Integer): Boolean; virtual;
    procedure UpdatePathLabel; dynamic;
    procedure UpdateStatusBar; dynamic;
    procedure WndProc(var Message: TMessage); override;
    function FileNameMatchesMasks(FileName: string; Directory: Boolean; Size: Int64; Modification: TDateTime; Masks: string; AllowImplicitMatches: Boolean): Boolean;
    function EnableDragOnClick: Boolean; override;
    procedure SetMask(Value: string); virtual;
    procedure ScrollOnDragOverBeforeUpdate(ObjectToValidate: TObject);
    procedure ScrollOnDragOverAfterUpdate;
    procedure DoHistoryGo(Index: Integer);
    procedure WMSetFocus(var Message: TWMSetFocus); message WM_SETFOCUS;
    procedure WMKillFocus(var Message: TWMKillFocus); message WM_KILLFOCUS;
    procedure EnsureSelectionRedrawn;
    function HiddenCount: Integer; virtual; abstract;
    function FilteredCount: Integer; virtual; abstract;
    property ImageList16: TImageList read FImageList16;
    property ImageList32: TImageList read FImageList32;
  public
    function AnyFileSelected(OnlyFocused: Boolean; FilesOnly: Boolean;
      FocusedFileOnlyWhenFocused: Boolean): Boolean;
    constructor Create(AOwner: TComponent); override;
    procedure CreateDirectory(DirName: string); virtual; abstract;
    destructor Destroy; override;
    procedure Load; virtual;
    procedure Reload(CacheIcons: Boolean); virtual;
    function CreateFocusedFileList(FullPath: Boolean; FileList: TStrings = nil): TStrings;
    function CreateFileList(Focused: Boolean; FullPath: Boolean; FileList: TStrings = nil): TStrings;
    function DoSelectByMask(Select: Boolean): Boolean; override;
    procedure ExecuteHomeDirectory; virtual; abstract;
    procedure ExecuteParentDirectory; virtual; abstract;
    procedure ExecuteRootDirectory; virtual; abstract;
    procedure ExecuteCurrentFile();
    function FindFileItem(FileName: string): TListItem;
    procedure HistoryGo(Index: Integer);
    function ItemIsDirectory(Item: TListItem): Boolean; virtual; abstract;
    function ItemIsParentDirectory(Item: TListItem): Boolean; virtual; abstract;
    function ItemFullFileName(Item: TListItem): string; virtual; abstract;
    function ItemFileName(Item: TListItem): string; virtual; abstract;
    function ItemFileSize(Item: TListItem): Int64; virtual; abstract;
    function ItemFileTime(Item: TListItem; var Precision: TDateTimePrecision): TDateTime; virtual; abstract;
    procedure ReloadDirectory; virtual; abstract;
    procedure DisplayPropertiesMenu; virtual; abstract;
    function CreateChangedFileList(DirView: TCustomDirView; FullPath: Boolean;
      ExistingOnly: Boolean; Criterias: TCompareCriterias): TStrings;
    procedure CompareFiles(DirView: TCustomDirView; ExistingOnly: Boolean;
      Criterias: TCompareCriterias); virtual;
    procedure SaveSelection;
    procedure RestoreSelection;
    procedure DiscardSavedSelection;
    procedure SaveSelectedNames;
    procedure RestoreSelectedNames;
    procedure ContinueSession(Continue: Boolean);
    function CanPasteFromClipBoard: Boolean; dynamic;
    function PasteFromClipBoard(TargetPath: string = ''): Boolean; virtual; abstract;
    function SaveState: TObject;
    procedure RestoreState(AState: TObject);
    procedure ClearState;
    procedure DisplayContextMenu(Where: TPoint); virtual; abstract;
    procedure DisplayContextMenuInSitu;

    property AddParentDir: Boolean read FAddParentDir write SetAddParentDir default False;
    property DimmHiddenFiles: Boolean read FDimmHiddenFiles write SetDimmHiddenFiles default True;
    property ShowDirectories: Boolean read FShowDirectories write SetShowDirectories default True;
    property DragDropFilesEx: TCustomizableDragDropFilesEx read FDragDropFilesEx;
    property ShowSubDirSize: Boolean read FShowSubDirSize write SetShowSubDirSize default False;
    property FormatSizeBytes: Boolean read FFormatSizeBytes write SetFormatSizeBytes default False;
    property SortByExtension: Boolean read FSortByExtension write SetSortByExtension default False;
    property WantUseDragImages: Boolean read FWantUseDragImages write FWantUseDragImages default True;
    property UseDragImages: Boolean read GetUseDragImages stored False;
    property FullDrag default True;
    property TargetPopupMenu: Boolean read GetTargetPopupMenu write SetTargetPopupMenu default True;
    property DDOwnerIsSource: Boolean read FDDOwnerIsSource;
    property FilesSize: Int64 read FFilesSize;
    property FilesSelSize: Int64 read FFilesSelSize;
    property FilesCount: Integer read GetFilesCount;
    property FilesMarkedSize: Int64 read GetFilesMarkedSize;
    property HasParentDir: Boolean read FHasParentDir;
    property Path: string read GetPath write SetPath;
    property PathName: string read GetPathName;
    property ReloadTime: TSystemTime read FReloadTime;
    property SingleClickToExec: Boolean read FSingleClickToExec write FSingleClickToExec default False;
    property UseSystemContextMenu: Boolean read FUseSystemContextMenu
      write FUseSystemContextMenu default True;
    property Loading: Boolean read FLoading;
    property AbortLoading: Boolean read FAbortLoading write FAbortLoading stored False;
    property BackCount: Integer read FBackCount;
    {Enable or disable populating the item list:}
    property LoadAnimation: Boolean read FLoadAnimation write FLoadAnimation default True;
    property LoadEnabled: Boolean read FLoadEnabled write SetLoadEnabled default True;
    {Displayed data is not valid => reload required}
    property Dirty: Boolean read FDirty;
    property DirOK: Boolean read GetDirOK;
    property LastPath: string read FLastPath;
    property IsRecycleBin: Boolean read FIsRecycleBin;
    property DDLinkOnExeDrag: Boolean read FDDLinkOnExeDrag
      write FDDLinkOnExeDrag default False;
    property DragDrive: TDrive read FDragDrive;
    property DragOnDriveIsMove: Boolean read FDragOnDriveIsMove write FDragOnDriveIsMove;
    property DragSourceEffects: TDropEffectSet read GetDragSourceEffects{ write FDragSourceEffects};
    property ExeDrag: Boolean read FExeDrag;
    property ForwardCount: Integer read GetForwardCount;
    property HistoryPath[Index: Integer]: string read GetHistoryPath;
    property IsRoot: Boolean read GetIsRoot;
    property LastDDResult: TDragResult read FLastDDResult;
    property SmallImages;
    property LargeImages;
    property MaxHistoryCount: Integer read FMaxHistoryCount write SetMaxHistoryCount default DefaultHistoryCount;
    property SelectedNamesSaved: Boolean read GetSelectedNamesSaved;
    {filemask, multiple filters are possible: '*.pas;*.dfm'}
    property Mask: string read FMask write SetMask;

    property OnContextPopup;
    property OnBeginRename: TRenameEvent read FOnBeginRename write FOnBeginRename;
    property OnEndRename: TRenameEvent read FOnEndRename write FOnEndRename;
    property OnGetSelectFilter: TDVGetFilterEvent read FOnGetSelectFilter write FOnGetSelectFilter;
    property OnStartLoading: TNotifyEvent read FOnStartLoading write FOnStartLoading;
    property OnLoaded: TNotifyEvent read FOnLoaded write FOnLoaded;
    {This event is fired, when any update has made to the listview}
    property OnDirUpdated: TNotifyEvent read FOnDirUpdated write FOnDirUpdated;
    {The mouse has entered the component window as a target of a drag&drop operation:}
    property OnDDDragEnter: TDDOnDragEnter read FOnDDDragEnter write FOnDDDragEnter;
    {The mouse has leaved the component window as a target of a drag&drop operation:}
    property OnDDDragLeave: TDDOnDragLeave read FOnDDDragLeave write FOnDDDragLeave;
    {The mouse is dragging in the component window as a target of a drag&drop operation:}
    property OnDDDragOver: TDDOnDragOver read FOnDDDragOver write FOnDDDragOver;
    {The Drag&drop operation is about to be executed:}
    property OnDDDrop: TDDOnDrop read FOnDDDrop write FOnDDDrop;
    property OnDDQueryContinueDrag: TDDOnQueryContinueDrag
      read FOnDDQueryContinueDrag write FOnDDQueryContinueDrag;
    property OnDDGiveFeedback: TDDOnGiveFeedback
      read FOnDDGiveFeedback write FOnDDGiveFeedback;
    property OnDDChooseEffect: TDDOnChooseEffect
      read FOnDDChooseEffect write FOnDDChooseEffect;
    {A drag&drop operation is about to be initiated whith
     the components window as the source:}
    property OnDDDragDetect: TDDOnDragDetect
      read FOnDDDragDetect write FOnDDDragDetect;
    property OnDDCreateDragFileList: TDDOnCreateDragFileList
      read FOnDDCreateDragFileList write FOnDDCreateDragFileList;
    property OnDDEnd: TNotifyEvent
      read FOnDDEnd write FOnDDEnd;
    property OnDDCreateDataObject: TDDOnCreateDataObject
      read FOnDDCreateDataObject write FOnDDCreateDataObject;
    property OnDDTargetHasDropHandler: TDDOnTargetHasDropHandler
      read FOnDDTargetHasDropHandler write FOnDDTargetHasDropHandler;
    {The component window is the target of a drag&drop operation:}
    property OnDDProcessDropped: TOnProcessDropped
      read FOnDDProcessDropped write FOnDDProcessDropped;
    {An error has occured during a drag&drop operation:}
    property OnDDError: TDDErrorEvent read FOnDDError write FOnDDError;
    {The drag&drop operation has been executed:}
    property OnDDExecuted: TDDExecutedEvent
      read FOnDDExecuted write FOnDDExecuted;
    {Event is fired just before executing the fileoperation. This event is also fired when
     files are pasted from the clipboard:}
    property OnDDFileOperation: TDDFileOperationEvent
      read FOnDDFileOperation write FOnDDFileOperation;
    {Event is fired after executing the fileoperation. This event is also fired when
     files are pasted from the clipboard:}
    property OnDDFileOperationExecuted: TDDFileOperationExecutedEvent
      read FOnDDFileOperationExecuted write FOnDDFileOperationExecuted;
    {Set AllowExec to false, if actual file should not be executed:}
    property OnDDMenuPopup: TOnMenuPopup read FOnDDMenuPopup write FOnDDMenuPopup;
    property OnExecFile: TDirViewExecFileEvent
      read FOnExecFile write FOnExecFile;
    property OnHistoryChange: TDirViewNotifyEvent read FOnHistoryChange write FOnHistoryChange;
    property OnHistoryGo: TDVHistoryGoEvent read FOnHistoryGo write FOnHistoryGo;
    property OnPathChange: TDirViewNotifyEvent read FOnPathChange write FOnPathChange;
    property OnMatchMask: TMatchMaskEvent read FOnMatchMask write FOnMatchMask;
    property OnGetOverlay: TDirViewGetOverlayEvent read FOnGetOverlay write FOnGetOverlay;
    property PathLabel: TCustomPathLabel read FPathLabel write SetPathLabel;
    property ShowHiddenFiles: Boolean read FShowHiddenFiles write SetShowHiddenFiles default True;
    property OnUpdateStatusBar: TDirViewUpdateStatusBarEvent read FOnUpdateStatusBar write FOnUpdateStatusBar;
    {Watch current directory for filename changes (create, rename, delete files)}
    property WatchForChanges: Boolean read FWatchForChanges write SetWatchForChanges default False;
  end;

resourcestring
  SErrorOpenFile = 'Can''t open file: ';
  SErrorRenameFile = 'Can''t rename file or directory: ';
  SErrorRenameFileExists = 'File already exists: ';
  SErrorInvalidName= 'Filename contains invalid characters:';
  STextFileExt = 'File %s';
  STextFiles = '%u Files';
  STextDirectories = '%u Directories';
  SParentDir = 'Parent directory';
  SIconUpdateThreadTerminationError = 'Can''t terminate icon update thread.';
  SDragDropError = 'DragDrop Error: %d';
  SDriveNotReady = 'Drive ''%s:'' is not ready.';
  SDirNotExists = 'Directory ''%s'' doesn''t exist.';

{Additional non-component specific functions:}

{Create and resolve a shell link (file shortcut):}
function CreateFileShortCut(SourceFile, Target, DisplayName: string;
  UpdateIfExists: Boolean = False): Boolean;
function ResolveFileShortCut(SourceFile: string; ShowDialog: Boolean = False): string;

{Gets the shell's display icon for registered file extensions:}
function GetIconIndex(const AFile: string; Attrs: DWORD; Flags: UINT): Integer;

{Gets the shell's inforecord for registered fileextensions:}
function GetshFileInfo(const AFile: string; Attrs: DWORD; Flags: UINT): TSHFileInfo;

{Returns the displayname as used by the shell:}
function GetShellDisplayName(const ShellFolder: IShellFolder; IDList: PItemIDList;
  Flags: DWORD; var Name: string): Boolean;

function IsExecutable(FileName: string): Boolean;

function GetNextMask(var Mask: string): string;

procedure DefaultFileFilter(var Filter: TFileFilter);

function OverlayImageList(Size: Integer): TImageList;

var
  StdDirIcon: Integer;
  StdDirSelIcon: Integer;
  DropSourceControl: TObject;
  UnknownFileIcon: Integer;
  HasExtendedCOMCTL32: Boolean;
  StdDirTypeName: string;
  DefaultExeIcon: Integer;
  UserDocumentDirectory: string;

implementation

uses
  Math, DirViewColProperties;

const
  Space = ' ';
  ResDirUp = 'DIRUP%2.2d';
  ResLink = 'LINK%2.2d';
  ResBrokenLink = 'BROKEN%2.2d';
  ResPartial = 'PARTIAL%2.2d';

type
  TDirViewState = class(TObject)
  public
    destructor Destroy; override;

  private
    HistoryPaths: TStrings;
    BackCount: Integer;
    SortStr: string;
    Mask: string;
    FocusedItem: string;
  end;

var
  WinDir: string;
  TempDir: string;
  COMCTL32Version: DWORD;

destructor TDirViewState.Destroy;
begin
  HistoryPaths.Free;

  inherited;
end;

function IsExecutable(FileName: string): Boolean;
var
  FileExt: string;
begin
  FileExt := UpperCase(ExtractFileExt(FileName));
  Result := (FileExt = '.EXE') or (FileExt = '.COM');
end;

function GetNextMask(var Mask: string): string;
var
  NextPos: Integer;
begin
  NextPos := Pos(';', Mask);
  if NextPos = 0 then
  begin
    Result := Mask;
    SetLength(Mask, 0);
  end
    else
  begin
    Result := Copy(Mask, 1, NextPos - 1);
    Delete(Mask, 1, NextPos);
  end;
end;

procedure DefaultFileFilter(var Filter: TFileFilter);
begin
  with Filter do
  begin
    SetLength(Masks, 0);
    IncludeAttr := 0;
    ExcludeAttr := 0;
    Directories := False;
    FileSizeFrom := 0;
    FileSizeTo := 0;
    ModificationFrom := 0;
    ModificationTo := 0;
  end;
end;

{ Shortcut-handling }

function ResolveFileShortCut(SourceFile: string; ShowDialog: Boolean = False): string;
var
  IUnk: IUnknown;
  HRes: HRESULT;        // OLE-Operation Result
  SL: IShellLink;     // Interface for ShellLink
  PF: IPersistFile;   // Interface for PersistentFile
  SRec: TWIN32FINDDATA; // SearchRec of targetfile
  TargetDir: array[1..Max_Path] of Char; // Working directory of targetfile
  Flags: DWORD;
begin
  Result := '';

  IUnk := CreateComObject(CLSID_ShellLink);
  SL := IUnk as IShellLink;
  PF := IUnk as IPersistFile;

  HRes := PF.Load(PChar(SourceFile), STGM_READ);
  if Succeeded(Hres) then
  begin
    if not ShowDialog then Flags := SLR_NOUPDATE or (1500 shl 8) or SLR_NO_UI
      else Flags := SLR_NOUPDATE;
    HRes := SL.Resolve(Application.Handle, Flags);
    if Succeeded(HRes) then
    begin
      HRes := SL.GetPath(@TargetDir, MAX_PATH, SRec, {SLGP_UNCPRIORITY}{SLGP_SHORTPATH} 0);
      if Succeeded(HRes) then
        Result := string(PChar(@TargetDir));
    end;
  end;
end; {ResolveShortCut}

function CreateFileShortCut(SourceFile, Target, DisplayName: string;
  UpdateIfExists: Boolean): Boolean;
var
  IUnk: IUnknown;
  Hres: HRESULT;
  ShellLink: IShellLink;   // Interface to ShellLink
  IPFile: IPersistFile; // Interface to PersistentFile
  TargetFile: string;
begin
  Result := False;
  if Target = '' then TargetFile := SourceFile + '.lnk'
    else TargetFile := Target;

  IUnk := CreateComObject(CLSID_ShellLink);
  ShellLink := IUnk as IShellLink;
  IPFile := IUnk as IPersistFile;

  if FileExists(TargetFile) and UpdateIfExists then
  begin
    HRes := IPFile.Load(PChar(TargetFile), 0);
    if not Succeeded(HRes) then Exit;
  end;

  with ShellLink do
  begin
    HRes := SetPath(PChar(SourceFile));
    if Succeeded(HRes) then
      HRes := SetWorkingDirectory(PChar(ExtractFilePath(SourceFile)));
    if Succeeded(HRes) and (DisplayName <> '') then
      HRes := SetDescription(PChar(DisplayName));
  end;

  if Succeeded(Hres) then
  begin
    HRes := IPFile.Save(PChar(TargetFile),False);
    if Succeeded(HRes) then Result := True;
  end;
end; {CreateShortCut}

function GetIconIndex(const AFile: string; Attrs: DWORD; Flags: UINT): Integer;
var
  FileInfo: TSHFileInfo;
begin
  try
    SHGetFileInfo(PChar(AFile), Attrs, FileInfo, SizeOf(TSHFileInfo),
      Flags or SHGFI_SYSICONINDEX or SHGFI_USEFILEATTRIBUTES);
    Result := FileInfo.iIcon;
  except
    Result := -1;
  end;
end; {GetIconIndex}

function GetshFileInfo(const AFile: string; Attrs: DWORD; Flags: UINT): TSHFileInfo;
begin
  try
    SHGetFileInfo(PChar(AFile), Attrs, Result, SizeOf(TSHFileInfo), Flags);
  except
    FillChar(Result, SizeOf(Result), 0);
  end;
end; {GetshFileInfo}

function GetShellDisplayName(const ShellFolder: IShellFolder; IDList: PItemIDList;
  Flags: DWORD; var Name: string): Boolean;
var
  Str: TStrRet;
begin
  Result := True;
  Name := '';
  if ShellFolder.GetDisplayNameOf(IDList, Flags, Str) = NOERROR then
  begin
    case Str.uType of
      STRRET_WSTR: Name := WideCharToString(Str.pOleStr);
      STRRET_OFFSET: Name := PChar(UINT(IDList) + Str.uOffset);
      STRRET_CSTR: Name := string(Str.cStr);
      else Result := False;
    end;
  end
    else Result := False;
end; {GetShellDisplayName}

function COMCTL32OK: Boolean;
{Returs, whether COMCTL32 supports the extended display properties:
 COMCTL32.DLL version 4.70 or higher is required. Version 4.70 is
 included in Internet Explorer 4 with Active Desktop.
 Updates of COMCTL32.DLL are available at:
 http://msdn.microsoft.com/developer/downloads/files/40Comupd.htm }
var
  VerInfoSize: DWORD;
  Dummy: DWORD;
  VerInfo: Pointer;
  FileInfo: PVSFixedFileInfo;
  FileInfoSize: UINT;
begin
  Result := False;
  VerInfoSize := GetFileVersionInfoSize('COMCTL32.DLL', Dummy);
  if VerInfoSize > 0 then
  begin
    GetMem(VerInfo, VerInfoSize);
    try
      if GetFileVersionInfo(PChar('COMCTL32.DLL'), 0, VerInfoSize, VerInfo) then
      begin
        if VerQueryValue(VerInfo, '\', Pointer(FileInfo), FileInfoSize) then
        begin
          ComCTL32Version := FileInfo.dwFileVersionMS;
          Result := (ComCTL32Version >= $40046); { COMCTL32 Version >= 4.70  required }
        end
          else ComCTL32Version := 0;
      end;
    finally
      FreeMem(VerInfo, VerInfoSize);
    end;
  end;
end; {COMCTL32OK}

function OverlayImageList(Size: Integer): TImageList;

  procedure GetOverlayBitmap(ImageList: TImageList; BitmapName: string);
  var
    Bitmap: TBitmap;
  begin
    Bitmap := TBitmap.Create;
    try
      Bitmap.LoadFromResourceName(hInstance, BitmapName);
      ImageList.AddMasked(Bitmap, Bitmap.Canvas.Pixels[0, 0]);
    finally
      Bitmap.Free;
    end;
  end; {GetOverlayBitmap}

begin
  Result := TImageList.CreateSize(Size, Size);
  Result.DrawingStyle := dsTransparent;
  Result.BkColor := clNone;
  GetOverlayBitmap(Result, Format(ResDirUp, [Size]));
  GetOverlayBitmap(Result, Format(ResLink, [Size]));
  GetOverlayBitmap(Result, Format(ResBrokenLink, [Size]));
  GetOverlayBitmap(Result, Format(ResPartial, [Size]));
end;


  { TLoadAnimationStartThread }

{constructor TLoadAnimationStartThread.Create(AInterval: Integer; AAnimation: TAnimate);
begin
  inherited Create(True);
  FInterval := AInterval;
  FAnimation := AAnimation;
  Resume;
end;

procedure TLoadAnimationStartThread.Execute;
var
  XInterval: Integer;
begin
  XInterval := FInterval;
  while (not Terminated) and (XInterval > 0) do
  begin
    Sleep(10);
    Dec(XInterval, 10);
  end;
  if (not Terminated) and Assigned(FAnimation) then
    Synchronize(StartAnimation);
end;

procedure TLoadAnimationStartThread.StartAnimation;
begin
  FAnimation.Visible := True;
  FAnimation.Active := True;
end; }

  { TCustomizableDragDropFilesEx }

function TCustomizableDragDropFilesEx.Execute(DataObject: TDataObject): TDragResult;
begin
  if not Assigned(DataObject) then
  begin
    DataObject := CreateDataObject;
  end;
  Result := ExecuteOperation(DataObject);
end;

  { TCustomDirView }

constructor TCustomDirView.Create(AOwner: TComponent);
begin
  inherited;

  FWatchForChanges := False;
  FFilesSize := 0;
  FFilesSelSize := 0;
  FDimmHiddenFiles := True;
  FShowHiddenFiles := True;
  FShowDirectories := True;
  FShowSubDirSize := False;
  FFormatSizeBytes := False;
  FWantUseDragImages := True;
  FAddParentDir := False;
  FullDrag := True;
  FSingleClickToExec := False;
  FInvalidNameChars := '\/:*?"<>|';
  FHasParentDir := False;
  FDragOnDriveIsMove := False;
  FCaseSensitive := False;
  FLoadAnimation := True;
  FAnimation := nil;

  FIsRecycleBin := False;
  FLoading := False;
  FLoadEnabled := True;
  FAbortLoading := False;
  FDirty := False;
  FLastPath := '';
  FHistoryPath := '';
  FNotifyEnabled := True;
  FForceRename := False;
  FLastRenameName := '';
  FSavedSelection := False;
  FPendingFocusSomething := False;
  FSavedNames := TStringList.Create;

  FContextMenu := False;
  FUseSystemContextMenu := True;

  FStartPos.X := -1;
  FStartPos.Y := -1;
  FDragPos := FStartPos;
  FDragEnabled := False;
  FDDOwnerIsSource := False;
  FDDLinkOnExeDrag := False;
  FDragDrive := #0;
  FExeDrag := False;
  FMask := '';
  FDoubleBufferedScrollingWorkaround := not IsVistaHard();

  FOnHistoryChange := nil;
  FOnPathChange := nil;
  FHistoryPaths := TStringList.Create;
  FBackCount := 0;
  FDontRecordPath := False;
  FMaxHistoryCount := DefaultHistoryCount;
  FStatusFileInfo.FilesCount := -1;

  OnCustomDrawItem := DumbCustomDrawItem;
  OnCustomDrawSubItem := DumbCustomDrawSubItem;

  FOnMatchMask := nil;
  FOnGetOverlay := nil;

  FDragDropFilesEx := TCustomizableDragDropFilesEx.Create(Self);
  with FDragDropFilesEx do
  begin
    {$IFDEF OLD_DND}
    AutoDetectDnD := False;
    DragDetectDelta  := 4;
    {$ELSE}
    DragDetect.Automatic := False;
    DragDetect.DeltaX := 4;
    DragDetect.DeltaY := 4;
    {$ENDIF}
    AcceptOwnDnD := True;
    BringToFront := True;
    CompleteFileList := True;

    NeedValid := [nvFileName];
    RenderDataOn := rdoEnterAndDropSync;
    TargetPopUpMenu := True;
    SourceEffects := DragSourceEffects;
    TargetEffects := [deCopy, deMove];

    OnDragEnter := DDDragEnter;
    OnDragLeave := DDDragLeave;
    OnDragOver := DDDragOver;
    OnDrop := DDDrop;
    OnQueryContinueDrag := DDQueryContinueDrag;
    OnSpecifyDropTarget := DDSpecifyDropTarget;
    OnMenuPopup := DDMenuPopup;
    OnMenuDestroy := DDMenuDone;
    OnDropHandlerSucceeded := DDDropHandlerSucceeded;
    OnGiveFeedback := DDGiveFeedback;
    OnProcessDropped := DDProcessDropped;
    OnDragDetect := DDDragDetect;
  end;

  FScrollOnDragOver := TListViewScrollOnDragOver.Create(Self, False);
  FScrollOnDragOver.OnBeforeUpdate := ScrollOnDragOverBeforeUpdate;
  FScrollOnDragOver.OnAfterUpdate := ScrollOnDragOverAfterUpdate;
end;

procedure TCustomDirView.ClearItems;
begin
  CancelEdit;
  if Assigned(DropTarget) then DropTarget := nil;
  try
    inherited;
  finally
    FFilesSelSize := 0;
    FFilesSize := 0;
    UpdateStatusBar;
  end;
end;

procedure TCustomDirView.CNNotify(var Message: TWMNotify);

  procedure DrawOverlayImage(DC: HDC; Image: Integer);
  var
    ImageList: TCustomImageList;
    Rect: TRect;
    Point: TPoint;
    Index: Integer;
  begin
    Rect := Items[PNMCustomDraw(Message.NMHdr)^.dwItemSpec].DisplayRect(drIcon);
    Point := Rect.TopLeft;
    if ViewStyle = vsIcon then
    begin
      ImageList := ImageList32;
    end
      else
    begin
      ImageList := ImageList16;
    end;

    // center on the rect
    Inc(Point.X, (Rect.Width - ImageList.Width) div 2);
    Inc(Point.Y, (Rect.Height - ImageList.Height) div 2);

    Index := 0;
    while Image > 1 do
    begin
      Inc(Index);
      Image := Image shr 1;
    end;

    if 8 + ImageList.Width <= Columns[0].Width then
    begin
      ImageList_Draw(ImageList.Handle, Index, DC,
        Point.X, Point.Y, ILD_TRANSPARENT);
    end;
  end;

var
  FileSize: Int64;
  Item: TListItem;
  InfoMask: LongWord;
  OverlayIndex: Word;
  OverlayIndexes: Word;
  UpdateStatusBarPending: Boolean;
begin
  UpdateStatusBarPending := False;
  case Message.NMHdr^.code of
    LVN_ITEMCHANGED:
      with PNMListView(Message.NMHdr)^ do
        if (uChanged = LVIF_STATE) and Valid and (not FClearingItems) then
        begin
          if ((uOldState and (LVIS_SELECTED or LVIS_FOCUSED)) <>
              (uNewState and (LVIS_SELECTED or LVIS_FOCUSED))) then
                UpdateStatusBarPending := True;
          if ((uOldState and LVIS_SELECTED) <> (uNewState and LVIS_SELECTED)) then
          begin
            FileSize := ItemFileSize(Items[iItem]);
            if (uOldState and LVIS_SELECTED) <> 0 then Dec(FFilesSelSize, FileSize)
              else Inc(FFilesSelSize, FileSize);
          end;
        end;
    LVN_ENDLABELEDIT:
      // enable loading now only when editing was canceled.
      // when it was confirmed, it will be enabled only after actual
      // file renaming is completed. see Edit().
      with PLVDispInfo(Message.NMHdr)^ do
        if (item.pszText = nil) or (item.IItem = -1) then
          LoadEnabled := True;
    LVN_BEGINDRAG:
      if FDragEnabled and (not Loading) then
      begin
        DDBeforeDrag;
        DDDragDetect(MK_LBUTTON, FStartPos, Mouse.CursorPos, ddsDrag);
      end;
    LVN_BEGINRDRAG:
      if FDragEnabled and (not Loading) then
      begin
        DDBeforeDrag;
        DDDragDetect(MK_RBUTTON, FStartPos, Mouse.CursorPos, ddsDrag);
      end;
  end;

  inherited;

  if (Message.NMHdr.code = LVN_GETDISPINFO) and
     FNotifyEnabled and Valid and (not Loading) then
    with PLVDispInfo(Pointer(Message.NMHdr))^.Item do
      try
        InfoMask := PLVDispInfo(Pointer(Message.NMHdr))^.item.Mask;

        if (InfoMask and LVIF_PARAM) <> 0 then Item := TListItem(lParam)
          else
        if iItem < Items.Count then Item := Items[iItem]
          else Item := nil;

        if Assigned(Item) and Assigned(Item.Data) then
          GetDisplayInfo(Item, PLVDispInfo(Pointer(Message.NMHdr))^.item);
      except
      end;

  if (Message.NMHdr.code = NM_CUSTOMDRAW) and
     HasExtendedCOMCTL32 and Valid and (not Loading) then
    with PNMLVCustomDraw(Message.NMHdr)^ do
      try
        Message.Result := Message.Result or CDRF_NOTIFYPOSTPAINT;
        if (nmcd.dwDrawStage = CDDS_ITEMPOSTPAINT) and
           ((nmcd.dwDrawStage and CDDS_SUBITEM) = 0) and
           Assigned(Columns[0]) and (Columns[0].Width > 0) then
        begin
          Assert(Assigned(Items[nmcd.dwItemSpec]));
          OverlayIndexes := ItemOverlayIndexes(Items[nmcd.dwItemSpec]);
          OverlayIndex := 1;
          while OverlayIndexes > 0 do
          begin
            if (OverlayIndex and OverlayIndexes) <> 0 then
            begin
              DrawOverlayImage(nmcd.hdc, OverlayIndex);
              Dec(OverlayIndexes, OverlayIndex);
            end;
            OverlayIndex := OverlayIndex shl 1;
          end;
        end;
      except
      end;

  if UpdateStatusBarPending then UpdateStatusBar;
end;

function TCustomDirView.FileNameMatchesMasks(FileName: string;
  Directory: Boolean; Size: Int64; Modification: TDateTime; Masks: string;
  AllowImplicitMatches: Boolean): Boolean;
begin
  Result := False;
  if Assigned(OnMatchMask) then
    OnMatchMask(Self, FileName, Directory, Size, Modification, Masks, Result, AllowImplicitMatches)
end;

procedure TCustomDirView.SetAddParentDir(Value: Boolean);
begin
  if FAddParentDir <> Value then
  begin
    FAddParentDir := Value;
    if DirOK then Reload(True);
  end;
end;

procedure TCustomDirView.SetDimmHiddenFiles(Value: Boolean);
begin
  if Value <> FDimmHiddenFiles then
  begin
    FDimmHiddenFiles := Value;
    Self.Repaint;
  end;
end; {SetDimmHiddenFiles}

procedure TCustomDirView.SetPathLabel(Value: TCustomPathLabel);
begin
  if FPathLabel <> Value then
  begin
    if Assigned(FPathLabel) and (FPathLabel.FocusControl = Self) then
      FPathLabel.FocusControl := nil;
    FPathLabel := Value;
    if Assigned(Value) then
    begin
      Value.FreeNotification(Self);
      if not Assigned(Value.FocusControl) then
        Value.FocusControl := Self;
      UpdatePathLabel;
    end;
  end;
end; { SetPathLabel }

procedure TCustomDirView.SetShowDirectories(Value: Boolean);
begin
  if Value <> FShowDirectories then
  begin
    FShowDirectories := Value;
    if DirOK then Reload(True);
    Self.Repaint;
  end;
end; {SetShowDirectories}

procedure TCustomDirView.SetShowHiddenFiles(Value: Boolean);
begin
  if ShowHiddenFiles <> Value then
  begin
    FShowHiddenFiles := Value;
    if DirOK then Reload(False);
  end;
end;

procedure TCustomDirView.SetShowSubDirSize(Value: Boolean);
begin
  if Value <> FShowSubDirSize then
    FShowSubDirSize := Value;
end; {SetShowSubDirSize}

procedure TCustomDirView.SetFormatSizeBytes(Value: Boolean);
begin
  if Value <> FFormatSizeBytes then
  begin
    FFormatSizeBytes := Value;
    Self.Repaint;
  end;
end; {SetShowSubDirSize}

procedure TCustomDirView.SetSortByExtension(Value: Boolean);
Begin
  if Value <> FSortByExtension then
  begin
    FSortByExtension := Value;
    SortItems;
  end;
end; {SetSortByExtension}

function TCustomDirView.GetDragSourceEffects: TDropEffectSet;
begin
  Result := [deCopy, deMove, deLink];
end;

function TCustomDirView.GetUseDragImages: Boolean;
begin
  Result := FWantUseDragImages;
end;

procedure TCustomDirView.SetTargetPopupMenu(Value: Boolean);
begin
  if Assigned(FDragDropFilesEx) then FDragDropFilesEx.TargetPopupMenu := Value;
end;

procedure TCustomDirView.CreateWnd;
begin
  inherited;

  if Assigned(PopupMenu) then
    PopupMenu.Autopopup := False;
  FDragDropFilesEx.DragDropControl := Self;

  if not Assigned(FImageList16) then
    FImageList16 := OverlayImageList(16);
  if not Assigned(FImageList32) then
    FImageList32 := OverlayImageList(32);
  IconsSetImageList;
end;

procedure TCustomDirView.LVMSetExtendedListViewStyle(var Message: TMessage);
// Only TWinControl.DoubleBuffered actually prevents flicker
// on Win7 when moving mouse over list view, not LVS_EX_DOUBLEBUFFER.
// But LVS_EX_DOUBLEBUFFER brings nice alpha blended marquee selection.
// Double buffering introduces artefacts when scrolling using
// keyboard (Page-up/Down). This gets fixed by LVS_EX_TRANSPARENTBKGND,
// but that works on Vista and newer only. See WMKeyDown
// for workaround on earlier systems.
const
  RequiredStyles = LVS_EX_DOUBLEBUFFER or LVS_EX_TRANSPARENTBKGND;
begin
  // This prevents TCustomListView.ResetExStyles resetting our styles
  if (Message.WParam = 0) and
     ((Message.LParam and RequiredStyles) <> RequiredStyles) then
  begin
    ListView_SetExtendedListViewStyle(Handle, Message.LParam or RequiredStyles);
  end
    else
  begin
    inherited;
  end;
end;

procedure TCustomDirView.DestroyWnd;
begin
  // to force drag&drop re-registration when recreating handle
  // (occurs when changing ViewStyle)
  FDragDropFilesEx.DragDropControl := nil;
  inherited;
end;

function TCustomDirView.CustomDrawItem(Item: TListItem; State: TCustomDrawState;
  Stage: TCustomDrawStage): Boolean;
var
  FItemColor: TColor;
begin
  if (Item <> nil) and (Stage = cdPrePaint) then
  begin
    FItemColor := ItemColor(Item);
    if (FItemColor <> clDefaultItemColor) and
       (Canvas.Font.Color <> FItemColor) then
         Canvas.Font.Color := FItemColor;
  end;
  Result := inherited CustomDrawItem(Item, State, Stage);
end;

function TCustomDirView.CustomDrawSubItem(Item: TListItem; SubItem: Integer;
  State: TCustomDrawState; Stage: TCustomDrawStage): Boolean;
var
  FItemColor: TColor;
begin
  if Stage = cdPrePaint then
  begin
    FItemColor := ItemColor(Item);
    if (FItemColor <> clDefaultItemColor) and
       (Canvas.Font.Color <> FItemColor) then
         Canvas.Font.Color := FItemColor;
  end;
  Result := inherited CustomDrawSubItem(Item, SubItem, State, Stage);
end;

procedure TCustomDirView.Delete(Item: TListItem);
begin
  if Assigned(Item) then
  begin
    // This causes access violation when size is stored in structure
    // pointed by TListItem->Data and this structure is not valid any more
    if Valid then Dec(FFilesSize, ItemFileSize(Item));
    inherited Delete(Item);
  end;
end;

destructor TCustomDirView.Destroy;
begin
  Assert(not FSavedSelection);

  FreeAndNil(FScrollOnDragOver);
  FreeAndNil(FSavedNames);
  FreeAndNil(FHistoryPaths);

  FreeAndNil(FDragDropFilesEx);
  FreeAndNil(FImageList16);
  FreeAndNil(FImageList32);

  if Assigned(SmallImages) then
  begin
    SmallImages.Free;
    SmallImages := nil;
  end;
  if Assigned(LargeImages) then
  begin
    LargeImages.Free;
    LargeImages := nil;
  end;
  FreeAndNil(FAnimation);

  inherited;
end;

procedure TCustomDirView.SelectFiles(Filter: TFileFilter; Select: Boolean);
var
  Item: TListItem;
  Index: Integer;
  OldCursor: TCursor;
begin
  Assert(Valid);
  OldCursor := Screen.Cursor;
  Items.BeginUpdate;
  BeginSelectionUpdate;
  try
    Screen.Cursor := crHourGlass;
    for Index := 0 to Items.Count-1 do
    begin
      Item := Items[Index];
      Assert(Assigned(Item));
      if (Item.Selected <> Select) and
         ItemMatchesFilter(Item, Filter) then
           Item.Selected := Select;
    end;
  finally
    Screen.Cursor := OldCursor;
    Items.EndUpdate;
    EndSelectionUpdate;
  end;
end;

function TCustomDirView.DoSelectByMask(Select: Boolean): Boolean;
var
  Filter: TFileFilter;
begin
  Result := inherited DoSelectByMask(Select);
  if Assigned(FOnGetSelectFilter) then
  begin
    DefaultFileFilter(Filter);
    FOnGetSelectFilter(Self, Select, Filter);
    SelectFiles(Filter, Select);
    Result := True;
  end;
end;

function TCustomDirView.DragCompleteFileList: Boolean;
begin
  Result := (MarkedCount <= 100) and (not IsRecycleBin);
end;

procedure TCustomDirView.DumbCustomDrawItem(Sender: TCustomListView; Item: TListItem; State: TCustomDrawState; var DefaultDraw: Boolean);
begin
end;

procedure TCustomDirView.DumbCustomDrawSubItem(Sender: TCustomListView;
  Item: TListItem; SubItem: Integer; State: TCustomDrawState;
  var DefaultDraw: Boolean);
begin
end;

function TCustomDirView.GetTargetPopupMenu: Boolean;
begin
  if Assigned(FDragDropFilesEx) then Result := FDragDropFilesEx.TargetPopupMenu
    else Result := True;
end;

procedure TCustomDirView.SetMultiSelect(Value: Boolean);
begin
  if Value <> MultiSelect then
  begin
    inherited SetMultiSelect(Value);
    if not (csLoading in ComponentState) and Assigned(ColProperties) then
    begin
      ColProperties.RecreateColumns;
      SetColumnImages;
      if DirOK then Reload(True);
    end;
  end;
end;

function TCustomDirView.GetValid: Boolean;
begin
  Result := (not (csDestroying in ComponentState)) and
    (not Loading) and (not FClearingItems);
end;

function TCustomDirView.ItemCanDrag(Item: TListItem): Boolean;
begin
  Result := (not ItemIsParentDirectory(Item));
end;

function TCustomDirView.ItemColor(Item: TListItem): TColor;
begin
  Result := clDefaultItemColor;
end;

function TCustomDirView.GetFilesMarkedSize: Int64;
begin
  if SelCount > 0 then Result := FilesSelSize
    else
  if Assigned(ItemFocused) then Result := ItemFileSize(ItemFocused)
    else Result := 0;
end;

procedure TCustomDirView.IconsSetImageList;
begin
  if not Assigned(SmallImages) then
    SmallImages := ShellImageList(Self, SHGFI_SMALLICON);
  if not Assigned(LargeImages) then
    LargeImages := ShellImageList(Self, SHGFI_LARGEICON);
end; {IconsSetImageList}

function TCustomDirView.ItemIsRecycleBin(Item: TListItem): Boolean;
begin
  Result := False;
end;

function TCustomDirView.ItemOverlayIndexes(Item: TListItem): Word;
begin
  Result := oiNoOverlay;
  if Assigned(OnGetOverlay) then
    OnGetOverlay(Self, Item, Result);
end;

procedure TCustomDirView.WMKeyDown(var Message: TWMKeyDown);
begin
  if DoubleBuffered and (Message.CharCode in [VK_PRIOR, VK_NEXT]) and
     FDoubleBufferedScrollingWorkaround then
  begin
    // WORKAROUND
    // When scrolling with double-buffering enabled, ugly artefacts
    // are shown temporarily.
    // LVS_EX_TRANSPARENTBKGND fixes it on Vista and newer
    SendMessage(Handle, WM_SETREDRAW, 0, 0);
    try
      inherited;
    finally
      SendMessage(Handle, WM_SETREDRAW, 1, 0);
    end;
    Repaint;
  end
    else
  begin
    inherited;
  end;
end;

procedure TCustomDirView.KeyDown(var Key: Word; Shift: TShiftState);
begin
  if Valid and (not IsEditing) and (not Loading) then
  begin
    if (Key = VK_RETURN) or
       ((Key = VK_NEXT) and (ssCtrl in Shift)) then
    begin
      if Assigned(ItemFocused) then
      begin
         Key := 0;
         if (Key = VK_RETURN) and (Shift = [ssAlt]) then DisplayPropertiesMenu
           else
         if (Key <> VK_RETURN) or (Shift = []) then Execute(ItemFocused);
      end;
    end
      else
    if ((Key = VK_BACK) or ((Key = VK_PRIOR) and (ssCtrl in Shift))) and
       (not IsRoot) then
    begin
      Key := 0;
      ExecuteParentDirectory;
    end
      else
    if ((Key = VK_UP) and (ssAlt in Shift)) and
       (not IsRoot) then
    begin
      Key := 0;
      // U+25D8 is 'INVERSE BULLET', what is glyph representing '\x8' (or '\b')
      // ('up' key is the '8' key on numeric pad)
      // We could obtain the value programatically using
      // MultiByteToWideChar(CP_OEMCP, MB_USEGLYPHCHARS, "\x8", 1, ...)
      FNextCharToIgnore := $25D8;
      ExecuteParentDirectory;
    end
      else
    if (Key = 220 { backslash }) and (ssCtrl in Shift) and (not IsRoot) then
    begin
      Key := 0;
      ExecuteRootDirectory;
    end
      else
    if (Key = VK_LEFT) and (ssAlt in Shift) then
    begin
      if BackCount >= 1 then DoHistoryGo(-1);
    end
      else
    if (Key = VK_RIGHT) and (ssAlt in Shift) then
    begin
      if ForwardCount >= 1 then DoHistoryGo(1);
    end
      else
    begin
      inherited;
    end;
  end
    else
  begin
    inherited;
  end;
end;

procedure TCustomDirView.KeyPress(var Key: Char);
begin
  if IsEditing and (Pos(Key, FInvalidNameChars) <> 0) Then
  begin
    Beep;
    Key := #0;
  end;
  inherited;
end;

procedure TCustomDirView.DisplayContextMenuInSitu;
var
  R: TRect;
  P: TPoint;
begin
  if Assigned(ItemFocused) then
  begin
    R := ItemFocused.DisplayRect(drIcon);
    P.X := (R.Left + R.Right) div 2;
    P.Y := (R.Top + R.Bottom) div 2;
  end
    else
  begin
    P.X := 0;
    P.Y := 0;
  end;
  P := ClientToScreen(P);
  DisplayContextMenu(P);
end;

procedure TCustomDirView.KeyUp(var Key: Word; Shift: TShiftState);
var
  P: TPoint;
begin
  if Key = VK_APPS then
  begin
    if not Loading then
    begin
      if MarkedCount > 0 then
      begin
        DisplayContextMenuInSitu;
      end
        else
      if Assigned(PopupMenu) then
      begin
        P.X := 0;
        P.Y := 0;
        P := ClientToScreen(P);
        PopupMenu.Popup(P.X, P.Y);
      end;
    end;
  end
    else
  inherited KeyUp(Key, Shift);
end;

procedure TCustomDirView.SetWatchForChanges(Value: Boolean);
begin
  if FWatchForChanges <> Value then
    FWatchForChanges := Value;
end;

function TCustomDirView.TargetHasDropHandler(Item: TListItem; Effect: Integer): Boolean;
begin
  Assert(Assigned(DragDropFilesEx) and Assigned(Item));
  Result :=
    DragDropFilesEx.TargetHasDropHandler(nil, ItemFullFileName(Item), Effect);

  if Assigned(OnDDTargetHasDropHandler) then
  begin
    OnDDTargetHasDropHandler(Self, Item, Effect, Result);
  end;
end;

procedure TCustomDirView.UpdatePathLabel;
begin
  if Assigned(PathLabel) then
  begin
    if csDesigning in ComponentState then
      PathLabel.Caption := PathLabel.Name
    else
      PathLabel.Caption := PathName;
    PathLabel.Mask := Mask;
    PathLabel.UpdateStatus;
  end;
end; { UpdatePathLabel }

procedure TCustomDirView.UpdateStatusBar;
var
  StatusFileInfo: TStatusFileInfo;
begin
  if (FUpdatingSelection = 0) and Assigned(OnUpdateStatusBar) then
  begin
    with StatusFileInfo do
    begin
      SelectedSize := FilesSelSize;
      FilesSize := Self.FilesSize;
      SelectedCount := SelCount;
      FilesCount := Self.FilesCount;
      HiddenCount := Self.HiddenCount;
      FilteredCount := Self.FilteredCount;
    end;

    if not CompareMem(@StatusFileInfo, @FStatusFileInfo, SizeOf(StatusFileInfo)) then
    begin
      FStatusFileInfo := StatusFileInfo;
      OnUpdateStatusBar(Self, FStatusFileInfo);
    end;
  end;
end; { UpdateStatusBar }

procedure TCustomDirView.WMContextMenu(var Message: TWMContextMenu);
var
  Point: TPoint;
begin
  FDragEnabled := False;
  if Assigned(PopupMenu) then
    PopupMenu.AutoPopup := False;

  //inherited;

  if FContextMenu and (not Loading) then
  begin
    Point.X := Message.XPos;
    Point.Y := Message.YPos;
    Point := ScreenToClient(Point);
    if Assigned(OnMouseDown) then
    begin
      OnMouseDown(Self, mbRight, [], Point.X, Point.Y);
    end;
    if FUseSystemContextMenu and Assigned(ItemFocused) and
       (GetItemAt(Point.X, Point.Y) = ItemFocused) then
    begin
      Point.X := Message.XPos;
      Point.Y := Message.YPos;
      DisplayContextMenu(Point);
    end
      else
    if Assigned(PopupMenu) and (not PopupMenu.AutoPopup) then
    begin
      PopupMenu.Popup(Message.XPos, Message.YPos);
    end;
  end;
  FContextMenu := False;
  //inherited;
end;

function TCustomDirView.EnableDragOnClick: Boolean;
begin
  Result := (not Loading) and inherited EnableDragOnClick;
end;

procedure TCustomDirView.WMLButtonDown(var Message: TWMLButtonDown);
begin
  GetCursorPos(FStartPos);
  FDragEnabled := EnableDragOnClick;
  inherited;
end;

procedure TCustomDirView.WMRButtonDown(var Message: TWMRButtonDown);
begin
  GetCursorPos(FStartPos);
  if FDragDropFilesEx.DragDetectStatus <> ddsDrag then
    FDragEnabled := EnableDragOnClick;
  FContextMenu := True;
  inherited;
end;

procedure TCustomDirView.WMLButtonDblClk(var Message: TWMLButtonDblClk);
begin
  inherited;
  if (not SingleClickToExec) and Assigned(ItemFocused) and (not Loading) and
     (GetItemAt(Message.XPos, Message.YPos) = ItemFocused) then
  begin
    if GetKeyState(VK_MENU) < 0 then DisplayPropertiesMenu
      else Execute(ItemFocused);
  end;
end;

procedure TCustomDirView.WMLButtonUp(var Message: TWMLButtonUp);
begin
  if SingleClickToExec and FDragEnabled and Assigned(ItemFocused) and (not Loading) and
     (GetItemAt(Message.XPos, Message.YPos) = ItemFocused) and
     (GetKeyState(VK_SHIFT) >= 0) and (GetKeyState(VK_CONTROL) >= 0) then
  begin
    if GetKeyState(VK_MENU) < 0 then DisplayPropertiesMenu
      else Execute(ItemFocused);
  end;
  FDragEnabled := False;
  inherited;
end;

procedure TCustomDirView.WMXButtonUp(var Message: TWMXMouse);
begin
  if Message.Button = _XBUTTON1 then
  begin
    if BackCount >= 1 then DoHistoryGo(-1);
    Message.Result := 1;
  end
    else
  if Message.Button = _XBUTTON2 then
  begin
    if ForwardCount >= 1 then DoHistoryGo(1);
    Message.Result := 1;
  end;
end;

procedure TCustomDirView.CancelEdit;
begin
  // - Do nothing when handle is not allocated (we cannot be editing anyway
  //   without a handle), otherwise this causes handle allocation,
  //   what is wrong particularly when we are called from ClearItems
  //   when we are being destroyed
  // - If editing, it has to be focused item
  if HandleAllocated and IsEditing and Assigned(ItemFocused) then
  begin
    ItemFocused.CancelEdit;
    FLoadEnabled := True;
  end;
end;

procedure TCustomDirView.Reload(CacheIcons: Boolean);
var
  OldSelection: TStringList;
  OldItemFocused: string;
  OldFocusedShown: Boolean;
  OldShownItemOffset: Integer;
  Index: Integer;
  FoundIndex: Integer;
  IconCache: TStringList;
  Item: TListItem;
  ItemToFocus: TListItem;
  FileName: string;
  R: TRect;
  P: TPoint;
begin
  if Path <> '' then
  begin
    CancelEdit;
    OldSelection := nil;
    IconCache := nil;
    Items.BeginUpdate;
    try
      OldSelection := TStringList.Create;
      OldSelection.CaseSensitive := FCaseSensitive;
      if CacheIcons then
        IconCache := TStringList.Create;

      for Index := 0 to Items.Count-1 do
      begin
        Item := Items[Index];
        // cannot use ItemFileName as for TUnixDirView the file object
        // is no longer valid
        FileName := Item.Caption;
        if Item.Selected then
          OldSelection.Add(FileName);

        if CacheIcons and (ItemImageIndex(Item, True) >= 0) then
          IconCache.AddObject(FileName, TObject(ItemImageIndex(Item, True)));
      end;

      if FSelectFile <> '' then
      begin
        OldItemFocused := FSelectFile;
        OldFocusedShown := False;
        OldShownItemOffset := -1;
        FSelectFile := '';
      end
        else
      begin
        if Assigned(ItemFocused) then
        begin
          if ViewStyle = vsReport then
          begin
            if Assigned(TopItem) then
            begin
              R := ItemFocused.DisplayRect(drBounds);
              if (R.Top < TopItem.DisplayRect(drBounds).Top) or (R.Top > ClientHeight) then
              begin
                OldFocusedShown := False;
                OldShownItemOffset := TopItem.Index;
              end
                else
              begin
                OldFocusedShown := True;
                OldShownItemOffset := ItemFocused.Index - TopItem.Index;
              end;
            end
              else
            begin
              // seen with one user only
              OldFocusedShown := False;
              OldShownItemOffset := 0;
            end;
          end
            else
          begin
            // to satisfy compiler, never used
            OldFocusedShown := False;
            OldShownItemOffset := -1;
          end;
          OldItemFocused := ItemFocused.Caption;
        end
          else
        begin
          OldItemFocused := '';
          OldFocusedShown := False;
          if Assigned(TopItem) then OldShownItemOffset := TopItem.Index
            else OldShownItemOffset := -1;
        end;
      end;

      Load;

      OldSelection.Sort;
      if CacheIcons then IconCache.Sort;
      ItemToFocus := nil;

      for Index := 0 to Items.Count - 1 do
      begin
        Item := Items[Index];
        FileName := ItemFileName(Item);

        if FileName = OldItemFocused then
          ItemToFocus := Item;

        if OldSelection.Find(FileName, FoundIndex) then
          Item.Selected := True;

        if CacheIcons and (ItemImageIndex(Item, True) < 0) then
        begin
          FoundIndex := IconCache.IndexOf(FileName);
          if FoundIndex >= 0 then
            SetItemImageIndex(Item, Integer(IconCache.Objects[FoundIndex]));
        end;
      end;

    finally
      Items.EndUpdate;
      OldSelection.Free;
      if CacheIcons then IconCache.Free;
    end;

    // This is below Items.EndUpdate(), to make Scroll() work properly
    if Assigned(ItemToFocus) then
    begin
      // we have found item that was previously focused and visible, scroll to it
      if (ViewStyle = vsReport) and OldFocusedShown and
         (ItemToFocus.Index > OldShownItemOffset) then
      begin
        P := Items[ItemToFocus.Index - OldShownItemOffset].GetPosition;
        // GetPosition is shifted bit low below actual row top.
        // Scroll to the GetPosition would scroll one line lower.
        Scroll(0, P.Y - Items[0].GetPosition.Y);
      end;
      FocusItem(ItemToFocus);
    end;

    // cannot scroll when focus is not visible because
    // of hack-implementation of FocusItem()
    {$IF False}
    // previously focus item was not visible, scroll to the same position
    // as before
    if (ViewStyle = vsReport) and (not OldFocusedShown) and
       (OldShownItemOffset >= 0) and (Items.Count > 0) then
    begin
      if OldShownItemOffset < Items.Count - VisibleRowCount then
        Scroll(0, OldShownItemOffset)
      else
        Items.Item[Items.Count - 1].MakeVisible(false);
    end
      // do not know where to scroll to, so scroll to focus
      // (or we have tried to scroll to previously focused and visible item,
      // now make sute that it is really visible)
      else {$IFEND}
    if Assigned(ItemToFocus) then ItemToFocus.MakeVisible(false);

    FocusSomething;
  end;
end;

procedure TCustomDirView.Load;
var
  SaveCursor: TCursor;
  LastDirName: string;
begin
  if not FLoadEnabled or Loading then
  begin
    FDirty := True;
    FAbortLoading := True;
  end
    else
  begin
    FLoading := True;
    try
      FHasParentDir := False;

      if Assigned(FOnStartLoading) then FOnStartLoading(Self);

      SaveCursor := Screen.Cursor;
      Screen.Cursor := crHourGlass;

      try
        FNotifyEnabled := False;
        ClearItems;

        GetSystemTime(FReloadTime);

        FFilesSize := 0;
        FFilesSelSize := 0;
        SortType := stNone;
        Items.BeginUpdate;
        try
          try
            DoAnimation(True);
            LoadFiles;
          finally
            DoAnimation(False);
          end;
        finally
          Items.EndUpdate;
        end;
      finally
        Screen.Cursor := SaveCursor;
      end;
    finally
      FLoading := False;

      try
        if FAbortLoading then
        begin
          FAbortLoading := False;
          Reload(False);
        end
          else
        begin
          if DirOK then SortItems;
          FAbortLoading := False;
          FDirty := False;
          if (Length(LastPath) > Length(PathName)) and
             (Copy(LastPath, 1, Length(PathName)) = PathName) and
             (Items.Count > 0) then
          begin
            LastDirName := Copy(LastPath, LastDelimiter('\:/', LastPath) + 1, MaxInt);
            ItemFocused := FindFileItem(LastDirName);
          end;
        end;
      finally
        // nested try .. finally block is included
        // because we really want these to be executed

        FNotifyEnabled := True;

        if DirOK and not FAbortLoading and Assigned(FOnDirUpdated) then
          FOnDirUpdated(Self);

        FocusSomething;

        if Assigned(FOnLoaded) then
        begin
          FOnLoaded(Self);
        end;

        UpdatePathLabel;
        UpdateStatusBar;
      end;
    end;
  end;
end;

procedure TCustomDirView.SetLoadEnabled(Enabled: Boolean);
begin
  if Enabled <> LoadEnabled then
  begin
    FLoadEnabled := Enabled;
    if Enabled and Dirty then Reload(True);
  end;
end;

function TCustomDirView.GetFilesCount: Integer;
begin
  Result := Items.Count;
  if (Result > 0) and HasParentDir then Dec(Result);
end;

procedure TCustomDirView.SetViewStyle(Value: TViewStyle);
begin
  if (Value <> ViewStyle) and (not FLoading) then
  begin
    FNotifyEnabled := False;
    inherited;
    FNotifyEnabled := True;
    // this is workaround for bug in TCustomNortonLikeListView
    // that clears Items on recreating wnd (caused by change to ViewStyle)
    Reload(True);
  end;
end;

procedure TCustomDirView.ColClick(Column: TListColumn);
var
  ScrollToFocused: Boolean;
begin
  ScrollToFocused := Assigned(ItemFocused);
  inherited;
  if ScrollToFocused and Assigned(ItemFocused) then
    ItemFocused.MakeVisible(False);
end;

procedure TCustomDirView.CustomSortItems(SortProc: Pointer);
var
  SavedCursor: TCursor;
  SavedNotifyEnabled: Boolean;
begin
  if HandleAllocated then
  begin
    SavedNotifyEnabled := FNotifyEnabled;
    SavedCursor := Screen.Cursor;
    Items.BeginUpdate;
    try
      Screen.Cursor := crHourglass;
      FNotifyEnabled := False;
      CustomSort(TLVCompare(SortProc), Integer(Pointer(Self)));
    finally
      Screen.Cursor := SavedCursor;
      FNotifyEnabled := SavedNotifyEnabled;
      Items.EndUpdate;
      ItemsReordered;
    end;
  end;
end;

procedure TCustomDirView.ReloadForce(CacheIcons: Boolean);
begin
  FLoadEnabled := True;
  FDirty := False;
  Reload(CacheIcons);
end;

procedure TCustomDirView.ScrollOnDragOverBeforeUpdate(ObjectToValidate: TObject);
begin
  GlobalDragImageList.HideDragImage;
end;

procedure TCustomDirView.ScrollOnDragOverAfterUpdate;
begin
  GlobalDragImageList.ShowDragImage;
end;

procedure TCustomDirView.DDDragEnter(DataObj: IDataObject; grfKeyState: Longint;
  Point: TPoint; var dwEffect: longint; var Accept: Boolean);
var
  Index: Integer;
begin
  Accept := Accept and DirOK and (not Loading);
  if Accept and
     (DragDropFilesEx.FileList.Count > 0) and
     (Length(TFDDListItem(DragDropFilesEx.FileList[0]^).Name) > 0) and
     (not IsRecycleBin or not DragDropFilesEx.FileNamesAreMapped) then
  begin
    FDragDrive := Upcase(TFDDListItem(DragDropFilesEx.FileList[0]^).Name[1]);
    FExeDrag := FDDLinkOnExeDrag and
      (deLink in DragDropFilesEx.TargetEffects) and
      ((DragDropFilesEx.AvailableDropEffects and DropEffect_Link) <> 0);

    if FExeDrag then
    begin
      for Index := 0 to DragDropFilesEx.FileList.Count - 1 do
        if not IsExecutable(TFDDListItem(DragDropFilesEx.FileList[Index]^).Name) then
        begin
          FExeDrag := False;
          Break;
        end;
    end;
  end
    else
  begin
    FDragDrive := #0;
  end;

  FScrollOnDragOver.StartDrag;

  if Assigned(FOnDDDragEnter) then
    FOnDDDragEnter(Self, DataObj, grfKeyState, Point, dwEffect, Accept);
end;

procedure TCustomDirView.DDDragLeave;
begin
  if Assigned(DropTarget) then
  begin
    if GlobalDragImageList.Dragging then
      GlobalDragImageList.HideDragImage;
    DropTarget := nil;
    Update; {ie30}
  end
    else DropTarget := nil;

  if Assigned(FOnDDDragLeave) then
    FOnDDDragLeave(Self);
end;

procedure TCustomDirView.DDDragOver(grfKeyState: Integer; Point: TPoint;
  var dwEffect: Integer);
var
  DropItem: TListItem;
  CanDrop: Boolean;
  HasDropHandler: Boolean;
begin
  FDDOwnerIsSource := DragDropFilesEx.OwnerIsSource;

  {Set droptarget if target is directory:}
  if not Loading then DropItem := GetItemAt(Point.X, Point.Y)
    else DropItem := nil;

  HasDropHandler := (Assigned(DropItem) and (not IsRecycleBin) and
    TargetHasDropHandler(DropItem, dwEffect));
  CanDrop := Assigned(DropItem) and (not IsRecycleBin) and
    (ItemIsDirectory(DropItem) or HasDropHandler);

  if (CanDrop and (DropTarget <> DropItem)) or
     (not CanDrop and Assigned(DropTarget)) then
  begin
    if GlobalDragImageList.Dragging then
    begin
      GlobalDragImageList.HideDragImage;
      DropTarget := nil;
      Update;
      if CanDrop then
      begin
        DropTarget := DropItem;
        Update;
      end;
      GlobalDragImageList.ShowDragImage;
    end
      else
    begin
      DropTarget := nil;
      if CanDrop then DropTarget := DropItem;
    end;
  end;

  if not Loading then
    FScrollOnDragOver.DragOver(Point);

  {Set dropeffect:}
  if (not HasDropHandler) and (not Loading) then
  begin
    DDChooseEffect(grfKeyState, dwEffect);

    if Assigned(FOnDDDragOver) then
      FOnDDDragOver(Self, grfKeyState, Point, dwEffect);

    // cannot drop to dragged files
    if DragDropFilesEx.OwnerIsSource and Assigned(DropItem) then
    begin
      if Assigned(ItemFocused) and (not ItemFocused.Selected) then
      begin
        if DropItem = ItemFocused then
        begin
          dwEffect := DropEffect_None;
        end;
      end
        else
      if DropItem.Selected then
      begin
        dwEffect := DropEffect_None;
      end;
    end;

    if DragDropFilesEx.OwnerIsSource and (dwEffect = DropEffect_Move) and
      (not Assigned(DropTarget)) then
    begin
      dwEffect := DropEffect_None;
    end
      else
    if Assigned(DropTarget) and ItemIsRecycleBin(DropTarget) and
       (dwEffect <> DropEffect_None) then
    begin
      dwEffect := DropEffect_Move;
    end;
  end;
end;

function TCustomDirView.CustomCreateFileList(Focused, OnlyFocused: Boolean;
  FullPath: Boolean; FileList: TStrings; ItemObject: Boolean): TStrings;

  procedure AddItem(Item: TListItem);
  var
    AObject: TObject;
  begin
    Assert(Assigned(Item));
    if ItemObject then AObject := Item
      else AObject := Item.Data;

    if FullPath then Result.AddObject(ItemFullFileName(Item), AObject)
      else Result.AddObject(ItemFileName(Item), AObject);
  end;

var
  Item: TListItem;
begin
  if Assigned(FileList) then Result := FileList
    else Result := TStringList.Create;

  try
    if Assigned(ItemFocused) and
       ((Focused and (not ItemFocused.Selected)) or (SelCount = 0) or OnlyFocused) then
    begin
      AddItem(ItemFocused)
    end
      else
    begin
      Item := GetNextItem(nil, sdAll, [isSelected]);
      while Assigned(Item) do
      begin
        AddItem(Item);
        Item := GetNextItem(Item, sdAll, [isSelected]);
      end;
    end;
  except
    if not Assigned(FileList) then FreeAndNil(Result);
    raise;
  end;
end;

function TCustomDirView.CreateFocusedFileList(FullPath: Boolean; FileList: TStrings): TStrings;
begin
  Result := CustomCreateFileList(False, True, FullPath, FileList);
end;

function TCustomDirView.CreateFileList(Focused: Boolean; FullPath: Boolean;
  FileList: TStrings): TStrings;
begin
  Result := CustomCreateFileList(Focused, False, FullPath, FileList);
end;

procedure TCustomDirView.DDDrop(DataObj: IDataObject; grfKeyState: Integer;
  Point: TPoint; var dwEffect: Integer);
begin
  if GlobalDragImageList.Dragging then
    GlobalDragImageList.HideDragImage;

  if dwEffect = DropEffect_None then
    DropTarget := nil;

  if Assigned(OnDDDrop) then
    OnDDDrop(Self, DataObj, grfKeyState, Point, dwEffect);
end;

procedure TCustomDirView.DDQueryContinueDrag(FEscapePressed: LongBool;
  grfKeyState: Integer; var Result: HResult);
var
  MousePos: TPoint;
  KnowTime: TFileTime;
begin
  // this method cannot throw exceptions, if it does d&d will not be possible
  // anymore (see TDragDrop.ExecuteOperation, global GInternalSource)
  if Result = DRAGDROP_S_DROP then
  begin
    GetSystemTimeAsFileTime(KnowTime);
    if ((Int64(KnowTime) - Int64(FDragStartTime)) <= DDDragStartDelay) then
      Result := DRAGDROP_S_CANCEL;
  end;

  if Assigned(OnDDQueryContinueDrag) then
    OnDDQueryContinueDrag(Self, FEscapePressed, grfKeyState, Result);

  try
    if FEscapePressed then
    begin
      if GlobalDragImageList.Dragging then
        GlobalDragImageList.HideDragImage;
    end
      else
    begin
      if GlobalDragImageList.Dragging Then
      begin
        MousePos := ParentForm.ScreenToClient(Mouse.CursorPos);
        {Move the drag image to the new position and show it:}
        if (MousePos.X <> FDragPos.X) or (MousePos.Y <> FDragPos.Y) then
        begin
          FDragPos := MousePos;
          if PtInRect(ParentForm.BoundsRect, Mouse.CursorPos) then
          begin
            GlobalDragImageList.DragMove(MousePos.X, MousePos.Y);
            GlobalDragImageList.ShowDragImage;
          end
            else GlobalDragImageList.HideDragImage;
        end;
      end;
    end;
  except
    // do not care if the above fails
    // (Mouse.CursorPos fails when desktop is locked by user)
  end;
end;

procedure TCustomDirView.DDSpecifyDropTarget(Sender: TObject;
  DragDropHandler: Boolean; Point: TPoint; var pidlFQ: PItemIDList;
  var Filename: string);
var
  Item: TListItem;
begin
  pidlFQ := nil;
  if DirOK and (not Loading) then
  begin
    if DragDropHandler then
    begin
      if Assigned(DropTarget) and ItemIsDirectory(DropTarget) then
        FileName := ItemFullFileName(DropTarget)
      else
        FileName := PathName;
    end
      else
    begin
      Item := GetItemAt(Point.X, Point.Y);
      if Assigned(Item) and (not ItemIsDirectory(Item)) and
        (not IsRecycleBin) then
        FileName := ItemFullFileName(Item)
      else
        FileName := '';
    end;
  end
    else FileName := '';
end;

procedure TCustomDirView.DDMenuPopup(Sender: TObject; AMenu: HMenu;
  DataObj: IDataObject; AMinCustCmd: Integer; grfKeyState: Longint; pt: TPoint);
begin
  if Assigned(OnDDMenuPopup) then
  begin
    OnDDMenuPopup(Self, AMenu, DataObj, AMinCustCmd, grfKeyState, pt);
  end;
end;

procedure TCustomDirView.DDMenuDone(Sender: TObject; AMenu: HMenu);
begin
end;

procedure TCustomDirView.DDDropHandlerSucceeded(Sender: TObject;
  grfKeyState: Integer; Point: TPoint; dwEffect: Integer);
begin
  DropTarget := nil;
end;

procedure TCustomDirView.DDChooseEffect(grfKeyState: Integer; var dwEffect: Integer);
begin
  if Assigned(FOnDDChooseEffect) then
    FOnDDChooseEffect(Self, grfKeyState, dwEffect);
end;

procedure TCustomDirView.DDGiveFeedback(dwEffect: Integer;
  var Result: HResult);
begin
  if Assigned(FOnDDGiveFeedback) then
    FOnDDGiveFeedback(Self, dwEffect, Result);
end;

procedure TCustomDirView.DDProcessDropped(Sender: TObject;
  grfKeyState: Integer; Point: TPoint; dwEffect: Integer);
begin
  if DirOK and (not Loading) then
  try
    try
      if Assigned(FOnDDProcessDropped) then
        FOnDDProcessDropped(Self, grfKeyState, Point, dwEffect);
      if dwEffect <> DropEffect_None then
      begin
        PerformItemDragDropOperation(DropTarget, dwEffect);
        if Assigned(FOnDDExecuted) then
          FOnDDExecuted(Self, dwEffect);
      end;
    finally
      DragDropFilesEx.FileList.Clear;
      DropTarget := nil;
    end;
  except
    Application.HandleException(Self);
  end;
end;

function TCustomDirView.AnyFileSelected(
  OnlyFocused: Boolean; FilesOnly: Boolean; FocusedFileOnlyWhenFocused: Boolean): Boolean;
var
  Item: TListItem;
begin
  if OnlyFocused or
     ((SelCount = 0) and
      ((not FocusedFileOnlyWhenFocused) or
       (Focused and (GetParentForm(Self).Handle = GetForegroundWindow())))) then
  begin
    Result := Assigned(ItemFocused) and ItemIsFile(ItemFocused) and
      ((not FilesOnly) or (not ItemIsDirectory(ItemFocused)));
  end
    else
  begin
    Result := True;
    Item := GetNextItem(nil, sdAll, [isSelected]);
    while Assigned(Item) do
    begin
      if ItemIsFile(Item) and
         ((not FilesOnly) or (not ItemIsDirectory(Item))) then Exit;
      Item := GetNextItem(Item, sdAll, [isSelected]);
    end;
    Result := False;
  end;
end;

function TCustomDirView.CanEdit(Item: TListItem): Boolean;
begin
  Result :=
    (inherited CanEdit(Item) or FForceRename) and (not Loading) and
     Assigned(Item) and  (not ReadOnly) and (not IsRecycleBin) and
     (not ItemIsParentDirectory(Item));
  if Result then FLoadEnabled := False;
  FForceRename := False;
end;

function TCustomDirView.CanChangeSelection(Item: TListItem;
  Select: Boolean): Boolean;
begin
  Result :=
    (not Loading) and
    not (Assigned(Item) and Assigned(Item.Data) and
     ItemIsParentDirectory(Item));
end;

procedure TCustomDirView.Edit(const HItem: TLVItem);
var
  Item: TListItem;
  Info: string;
  Index: Integer;
begin
  if Length(HItem.pszText) = 0 then LoadEnabled := True
    else
  begin
    Item := GetItemFromHItem(HItem);

    {Does the changed filename contains invalid characters?}
    if StrContains(FInvalidNameChars, HItem.pszText) then
    begin
      Info := FInvalidNameChars;
      for Index := Length(Info) downto 1 do
        System.Insert(Space, Info, Index);
      MessageBeep(MB_ICONHAND);
      if MessageDlg(SErrorInvalidName + Space + Info, mtError,
        [mbOK, mbAbort], 0) = mrOK then RetryRename(HItem.pszText);
      LoadEnabled := True;
    end
      else
    begin
      if Assigned(FOnBeginRename) then
        FOnBeginRename(Self, Item, string(HItem.pszText));

      InternalEdit(HItem);

      if Assigned(FOnEndRename) then
        FOnEndRename(Self, Item, string(HItem.pszText));
    end;
  end;
end; {Edit}

procedure TCustomDirView.EndSelectionUpdate;
begin
  inherited;
  if FUpdatingSelection = 0 then
    UpdateStatusBar;
end; { EndUpdatingSelection }

procedure TCustomDirView.ExecuteCurrentFile;
begin
  Assert(Assigned(ItemFocused));
  Execute(ItemFocused);
end;

procedure TCustomDirView.Execute(Item: TListItem);
var
  AllowExec: Boolean;
begin
  Assert(Assigned(Item));
  if Assigned(Item) and Assigned(Item.Data) and (not Loading) then
  begin
    if IsRecycleBin and (not ItemIsParentDirectory(Item)) then DisplayPropertiesMenu
      else
    begin
      AllowExec := True;
      if Assigned(FOnExecFile) then FOnExecFile(Self, Item, AllowExec);

      if AllowExec then
      begin
        if ItemIsParentDirectory(Item) then ExecuteParentDirectory
          else ExecuteFile(Item);
      end;
    end;
  end;
end;

procedure TCustomDirView.GetDisplayInfo(ListItem: TListItem;
  var DispInfo: TLVItem);
begin
  // Nothing
end;

procedure TCustomDirView.WMUserRename(var Message: TMessage);
begin
  if Assigned(ItemFocused) then
  begin
    FForceRename := True;
    ListView_EditLabel(Handle, ItemFocused.Index);
    SetWindowText(ListView_GetEditControl(Self.Handle),
      PChar(FLastRenameName));
  end;
end;

procedure TCustomDirView.RetryRename(NewName: string);
begin
  FLastRenameName := NewName;
  PostMessage(Self.Handle, WM_USER_RENAME, Longint(PChar(NewName)), 0);
end;

procedure TCustomDirView.AddToDragFileList(FileList: TFileList; Item: TListItem);
begin
  FileList.AddItem(nil, ItemFullFileName(Item));
end;

procedure TCustomDirView.DDDragDetect(grfKeyState: Integer; DetectStart,
  Point: TPoint; DragStatus: TDragDetectStatus);
var
  FilesCount: Integer;
  DirsCount: Integer;
  Item: TListItem;
  FirstItem : TListItem;
  Bitmap: TBitmap;
  ImageListHandle: HImageList;
  Spot: TPoint;
  ItemPos: TPoint;
  DragText: string;
  ClientPoint: TPoint;
  FileListCreated: Boolean;
  AvoidDragImage: Boolean;
  DataObject: TDataObject;
begin
  if Assigned(FOnDDDragDetect) then
    FOnDDDragDetect(Self, grfKeyState, DetectStart, Point, DragStatus);

  FLastDDResult := drCancelled;

  if (DragStatus = ddsDrag) and (not Loading) and (MarkedCount > 0) then
  begin
    DragDropFilesEx.CompleteFileList := DragCompleteFileList;
    DragDropFilesEx.FileList.Clear;
    FirstItem := nil;
    FilesCount := 0;
    DirsCount := 0;
    FileListCreated := False;
    AvoidDragImage := False;

    if Assigned(OnDDCreateDragFileList) then
    begin
      OnDDCreateDragFileList(Self, DragDropFilesEx.FileList, FileListCreated);
      if FileListCreated then
      begin
        AvoidDragImage := True;
      end;
    end;

    if not FileListCreated then
    begin
      if Assigned(ItemFocused) and (not ItemFocused.Selected) then
      begin
        if ItemCanDrag(ItemFocused) then
        begin
          FirstItem := ItemFocused;
          AddToDragFileList(DragDropFilesEx.FileList, ItemFocused);
          if ItemIsDirectory(ItemFocused) then Inc(DirsCount)
            else Inc(FilesCount);
        end;
      end
        else
      if SelCount > 0 then
      begin
        Item := GetNextItem(nil, sdAll, [isSelected]);
        while Assigned(Item) do
        begin
          if ItemCanDrag(Item) then
          begin
            if not Assigned(FirstItem) then FirstItem := Item;
            AddToDragFileList(DragDropFilesEx.FileList, Item);
            if ItemIsDirectory(Item) then Inc(DirsCount)
              else Inc(FilesCount);
          end;
          Item := GetNextItem(Item, sdAll, [isSelected]);
        end;
      end;
    end;

    if DragDropFilesEx.FileList.Count > 0 then
    begin
      FDragEnabled := False;
      {Create the dragimage:}
      GlobalDragImageList := DragImageList;
      if UseDragImages and (not AvoidDragImage) then
      begin
        ImageListHandle := ListView_CreateDragImage(Handle, FirstItem.Index, Spot);
        ItemPos := ClientToScreen(FirstItem.DisplayRect(drBounds).TopLeft);
        if ImageListHandle <> Invalid_Handle_Value then
        begin
          GlobalDragImageList.Handle := ImageListHandle;
          if FilesCount + DirsCount = 1 then
          begin
            ItemPos := ClientToScreen(FirstItem.DisplayRect(drBounds).TopLeft);
            GlobalDragImageList.SetDragImage(0,
              DetectStart.X - ItemPos.X, DetectStart.Y - ItemPos.Y);
          end
            else
          begin
            GlobalDragImageList.Clear;
            GlobalDragImageList.Width := 32;
            GlobalDragImageList.Height := 32;
            if GlobalDragImageList.GetResource(rtBitMap, 'DRAGFILES', 0,
              [lrTransparent], $FFFFFF) Then
            begin
              Bitmap := TBitmap.Create;
              try
                try
                  GlobalDragImageList.GetBitmap(0, Bitmap);
                  Bitmap.Canvas.Font.Assign(Self.Font);
                  DragText := '';
                  if FilesCount > 0 then
                    DragText := Format(STextFiles, [FilesCount]);
                  if DirsCount > 0 then
                  begin
                    if FilesCount > 0 then
                      DragText := DragText + ', ';
                    DragText := DragText + Format(STextDirectories, [DirsCount]);
                  end;
                  Bitmap.Width := 33 + Bitmap.Canvas.TextWidth(DragText);
                  Bitmap.TransparentMode := tmAuto;
                  Bitmap.Canvas.TextOut(33,
                    Max(24 - Abs(Canvas.Font.Height), 0), DragText);
                  GlobalDragImageList.Clear;
                  GlobalDragImageList.Width := Bitmap.Width;
                  GlobalDragImageList.AddMasked(Bitmap,
                    Bitmap.Canvas.Pixels[0, 0]);
                  GlobalDragImageList.SetDragImage(0, 25, 20);
                except
                  if GlobalDragImageList.GetResource(rtBitMap, 'DRAGFILES',
                    0, [lrTransparent], $FFFFFF) then
                      GlobalDragImageList.SetDragImage(0, 25, 20);
                end;
              finally
                Bitmap.Free;
              end;
            end;
          end;
          ClientPoint := ParentForm.ScreenToClient(Point);
          GlobalDragImageList.BeginDrag(ParentForm.Handle,
            ClientPoint.X, ClientPoint.Y);
          GlobalDragImageList.HideDragImage;
          ShowCursor(True);
        end;
      end;

      FContextMenu := False;

      if IsRecycleBin then DragDropFilesEx.SourceEffects := [deMove]
        else DragDropFilesEx.SourceEffects := DragSourceEffects;

      DropSourceControl := Self;

      try
        GetSystemTimeAsFileTime(FDragStartTime);

        DataObject := nil;
        if Assigned(OnDDCreateDataObject) then
        begin
          OnDDCreateDataObject(Self, DataObject);
        end;
        {Execute the drag&drop-Operation:}
        FLastDDResult := DragDropFilesEx.Execute(DataObject);

        {the drag&drop operation is finished, so clean up the used drag image:}
        GlobalDragImageList.EndDrag;
        GlobalDragImageList.Clear;

        Application.ProcessMessages;
      finally
        DragDropFilesEx.FileList.Clear;
        FContextMenu := False;

        try
          if Assigned(OnDDEnd) then
            OnDDEnd(Self);
        finally
          DropTarget := nil;
          DropSourceControl := nil;
        end;
      end;
    end;
  end;
end;

procedure TCustomDirView.Notification(AComponent: TComponent; Operation: TOperation);
begin
  inherited;
  if Operation = opRemove then
  begin
    if AComponent = PathLabel then FPathLabel := nil;
  end;
end; { Notification }

procedure TCustomDirView.WMAppCommand(var Message: TMessage);
var
  Command: Integer;
  Shift: TShiftState;
begin
  Command := HiWord(Message.lParam) and (not _FAPPCOMMAND_MASK);
  Shift := KeyDataToShiftState(HiWord(Message.lParam) and _FAPPCOMMAND_MASK);

  if Shift * [ssShift, ssAlt, ssCtrl] = [] then
  begin
    if Command = _APPCOMMAND_BROWSER_BACKWARD then
    begin
      Message.Result := 1;
      if BackCount >= 1 then DoHistoryGo(-1);
    end
      else
    if Command = _APPCOMMAND_BROWSER_FORWARD then
    begin
      Message.Result := 1;
      if ForwardCount >= 1 then DoHistoryGo(1);
    end
      else
    if Command = _APPCOMMAND_BROWSER_REFRESH then
    begin
      Message.Result := 1;
      ReloadDirectory;
    end
      else
    if Command = _APPCOMMAND_BROWSER_HOME then
    begin
      Message.Result := 1;
      ExecuteHomeDirectory;
    end
      else inherited;
  end
    else inherited;
end;

procedure TCustomDirView.CMColorChanged(var Message: TMessage);
begin
  inherited;
  // particularly when changing color back to default (clWindow),
  // non-client area (border line) is not redrawn,
  // keeping previous color. force redraw here
  if HandleAllocated then
    RedrawWindow(Handle, nil, 0, RDW_INVALIDATE or RDW_FRAME);
end;

procedure TCustomDirView.WndProc(var Message: TMessage);
begin
  case Message.Msg of
    WM_SETFOCUS, WM_KILLFOCUS:
      begin
        UpdatePathLabel;
      end;
  end;
  inherited;
end; { WndProc }

function TCustomDirView.FindFileItem(FileName: string): TListItem;
type
  TFileNameCompare = function(const S1, S2: string): Integer;
var
  Index: Integer;
  CompareFunc: TFileNameCompare;
begin
  if FCaseSensitive then CompareFunc := CompareStr
    else CompareFunc := CompareText;

  for Index := 0 to Items.Count - 1 do
  begin
    if CompareFunc(FileName, ItemFileName(Items[Index])) = 0 then
    begin
      Result := Items[Index];
      Exit;
    end;
  end;
  Result := nil;
end;

procedure TCustomDirView.DoAnimation(Start: Boolean);
begin
  if Start and LoadAnimation then
  begin
    if not Assigned(FAnimation) then
    begin
      FAnimation := TAnimate.Create(Self);
      try
        FAnimation.Top  := (Height - FAnimation.Height) div 2;
        FAnimation.Left := (Width - FAnimation.Width) div 2;
        FAnimation.Parent := Self;
        FAnimation.CommonAVI := aviFindFolder;
        FAnimation.Transparent := True;
        FAnimation.Active := True;
      except
        FreeAndNil(FAnimation);
      end;
    end;
  end
    else
  if not Start then
    FreeAndNil(FAnimation);
end; { DoAnimation }

function TCustomDirView.GetForwardCount: Integer;
begin
  Result := FHistoryPaths.Count - BackCount;
end; { GetForwardCount }

procedure TCustomDirView.LimitHistorySize;
begin
  while FHistoryPaths.Count > MaxHistoryCount do
  begin
    if BackCount > 0 then
    begin
      FHistoryPaths.Delete(0);
      Dec(FBackCount);
    end
      else
    FHistoryPaths.Delete(FHistoryPaths.Count-1);
  end;
end; { LimitHistorySize }

function TCustomDirView.GetHistoryPath(Index: Integer): string;
begin
  Assert(Assigned(FHistoryPaths));
  if Index = 0 then Result := PathName
    else
  if Index < 0 then Result := FHistoryPaths[Index + BackCount]
    else
  if Index > 0 then Result := FHistoryPaths[Index + BackCount - 1];
end; { GetHistoryPath }

procedure TCustomDirView.SetMaxHistoryCount(Value: Integer);
begin
  if FMaxHistoryCount <> Value then
  begin
    FMaxHistoryCount := Value;
    DoHistoryChange;
  end;
end; { SetMaxHistoryCount }

procedure TCustomDirView.DoHistoryChange;
begin
  LimitHistorySize;
  if Assigned(OnHistoryChange) then
    OnHistoryChange(Self);
end; { DoHistoryChange }

procedure TCustomDirView.DoHistoryGo(Index: Integer);
var
  Cancel: Boolean;
begin
  Cancel := False;
  if Assigned(OnHistoryGo) then
    OnHistoryGo(Self, Index, Cancel);

  if not Cancel then HistoryGo(Index);
end;

procedure TCustomDirView.HistoryGo(Index: Integer);
var
  PrevPath: string;
begin
  if Index <> 0 then
  begin
    PrevPath := FHistoryPath;
    FDontRecordPath := True;
    try
      Path := HistoryPath[Index];
    finally
      FDontRecordPath := False;
    end;

    FHistoryPaths.Insert(FBackCount, PrevPath);
    FHistoryPaths.Delete(Index + BackCount);
    Inc(FBackCount, Index);
    DoHistoryChange;
  end;
end; { HistoryGo }

procedure TCustomDirView.PathChanging(Relative: Boolean);
begin
  if Relative then FLastPath := PathName
    else FLastPath := '';
  FSavedNames.Clear;
end;

procedure TCustomDirView.PathChanged;
var
  Index: Integer;
begin
  if Assigned(OnPathChange) then
    OnPathChange(Self);

  if (not FDontRecordPath) and (FHistoryPath <> '') and (FHistoryPath <> PathName) then
  begin
    Assert(Assigned(FHistoryPaths));
    for Index := FHistoryPaths.Count - 1 downto BackCount do
      FHistoryPaths.Delete(Index);
    FHistoryPaths.Add(FHistoryPath);
    Inc(FBackCount);
    DoHistoryChange;
  end;
  FHistoryPath := PathName;
end; { PathChanged }

procedure TCustomDirView.ProcessChangedFiles(DirView: TCustomDirView;
  FileList: TStrings; FullPath: Boolean; ExistingOnly: Boolean;
  Criterias: TCompareCriterias);
var
  Item, MirrorItem: TListItem;
  FileTime, MirrorFileTime: TDateTime;
  OldCursor: TCursor;
  Index: Integer;
  Changed: Boolean;
  SameTime: Boolean;
  Precision, MirrorPrecision: TDateTimePrecision;
begin
  Assert(Valid);
  OldCursor := Screen.Cursor;
  if not Assigned(FileList) then
  begin
    Items.BeginUpdate;
    BeginSelectionUpdate;
  end;

  try
    Screen.Cursor := crHourGlass;
    for Index := 0 to Items.Count-1 do
    begin
      Item := Items[Index];
      Changed := False;

      if not ItemIsDirectory(Item) then
      begin
        MirrorItem := DirView.FindFileItem(ItemFileName(Item));
        if MirrorItem = nil then
        begin
          Changed := not ExistingOnly;
        end
          else
        begin
          if ccTime in Criterias then
          begin
            FileTime := ItemFileTime(Item, Precision);
            MirrorFileTime := DirView.ItemFileTime(MirrorItem, MirrorPrecision);
            if MirrorPrecision < Precision then Precision := MirrorPrecision;
            if Precision <> tpMillisecond then
            begin
              ReduceDateTimePrecision(FileTime, Precision);
              ReduceDateTimePrecision(MirrorFileTime, Precision);
            end;
            SameTime := (FileTime = MirrorFileTime);
            if Precision = tpSecond then
            begin
              // 1 ms more solves the rounding issues
              // (see also Common.cpp)
              MirrorFileTime := MirrorFileTime + EncodeTime(0, 0, 1, 1);
            end;
            Changed :=
              (FileTime > MirrorFileTime);
          end
            else
          begin
            SameTime := True;
          end;

          if (not Changed) and SameTime and (ccSize in Criterias) then
          begin
            Changed := ItemFileSize(Item) <> DirView.ItemFileSize(MirrorItem);
          end
        end;
      end;

      if Assigned(FileList) then
      begin
        if Changed then
        begin
          if FullPath then
          begin
            FileList.AddObject(ItemFullFileName(Item), Item.Data)
          end
            else
          begin
            FileList.AddObject(ItemFileName(Item), Item.Data);
          end;
        end;
      end
        else
      begin
        Item.Selected := Changed;
      end;
    end;
  finally
    Screen.Cursor := OldCursor;
    if not Assigned(FileList) then
    begin
      Items.EndUpdate;
      EndSelectionUpdate;
    end;
  end;
end;

function TCustomDirView.CreateChangedFileList(DirView: TCustomDirView;
  FullPath: Boolean; ExistingOnly: Boolean; Criterias: TCompareCriterias): TStrings;
begin
  Result := TStringList.Create;
  try
    ProcessChangedFiles(DirView, Result, FullPath, ExistingOnly, Criterias);
  except
    FreeAndNil(Result);
    raise;
  end;
end;

function TCustomDirView.CanPasteFromClipBoard: Boolean;
begin
  Result := False;
  if DirOK and (Path <> '') and Windows.OpenClipboard(0) then
  begin
    Result := IsClipboardFormatAvailable(CF_HDROP);
    Windows.CloseClipBoard;
  end;
end; {CanPasteFromClipBoard}

procedure TCustomDirView.CompareFiles(DirView: TCustomDirView;
  ExistingOnly: Boolean; Criterias: TCompareCriterias);
begin
  ProcessChangedFiles(DirView, nil, True, ExistingOnly, Criterias);
end;

procedure TCustomDirView.FocusSomething;
begin
  if FSavedSelection then FPendingFocusSomething := True
    else inherited;
end;

procedure TCustomDirView.SaveSelection;
var
  Closest: TListItem;
begin
  Assert(not FSavedSelection);

  FSavedSelectionFile := '';
  FSavedSelectionLastFile := '';
  if Assigned(ItemFocused) then
  begin
    FSavedSelectionLastFile := ItemFocused.Caption;
  end;

  Closest := ClosestUnselected(ItemFocused);
  if Assigned(Closest) then
  begin
    FSavedSelectionFile := Closest.Caption;
  end;

  FSavedSelection := True;
end;

procedure TCustomDirView.RestoreSelection;
var
  ItemToSelect: TListItem;
begin
  Assert(FSavedSelection);
  FSavedSelection := False;

  if (FSavedSelectionLastFile <> '') and
     ((not Assigned(ItemFocused)) or
      (ItemFocused.Caption <> FSavedSelectionLastFile)) then
  begin
    ItemToSelect := FindFileItem(FSavedSelectionFile);
    if Assigned(ItemToSelect) then
    begin
      ItemFocused := ItemToSelect;
    end;
  end;

  if not Assigned(ItemFocused) then FocusSomething
    else ItemFocused.MakeVisible(False);
end;

procedure TCustomDirView.DiscardSavedSelection;
begin
  Assert(FSavedSelection);
  FSavedSelection := False;

  if FPendingFocusSomething then
  begin
    FPendingFocusSomething := False;
    FocusSomething;
  end;
end;

procedure TCustomDirView.SaveSelectedNames;
var
  Index: Integer;
  Item: TListItem;
begin
  FSavedNames.Clear;
  FSavedNames.CaseSensitive := FCaseSensitive;

  if SelCount > 0 then // optimalisation
  begin
    for Index := 0 to Items.Count-1 do
    begin
      Item := Items[Index];
      if Item.Selected then
        FSavedNames.Add(ItemFileName(Item));
    end;
  end;

  // as optimalisation the list is sorted only when the selection is restored
end;

procedure TCustomDirView.RestoreSelectedNames;
var
  Index, FoundIndex: Integer;
  Item: TListItem;
begin
  FSavedNames.Sort;

  for Index := 0 to Items.Count - 1 do
  begin
    Item := Items[Index];
    Item.Selected := FSavedNames.Find(ItemFileName(Item), FoundIndex);
  end;
end;

function TCustomDirView.GetSelectedNamesSaved: Boolean;
begin
  Result := (FSavedNames.Count > 0);
end;

procedure TCustomDirView.ContinueSession(Continue: Boolean);
begin
  if Continue then FLastPath := PathName
    else FLastPath := '';
end;

function TCustomDirView.SaveState: TObject;
var
  State: TDirViewState;
  DirColProperties: TCustomDirViewColProperties;
begin
  State := TDirViewState.Create;
  State.HistoryPaths := TStringList.Create;
  State.HistoryPaths.Assign(FHistoryPaths);
  State.BackCount := FBackCount;
  // TCustomDirViewColProperties should not be here
  DirColProperties := ColProperties as TCustomDirViewColProperties;
  Assert(Assigned(DirColProperties));
  State.SortStr := DirColProperties.SortStr;
  State.Mask := Mask;
  if Assigned(ItemFocused) then State.FocusedItem := ItemFocused.Caption
    else State.FocusedItem := '';
  Result := State;
end;

procedure TCustomDirView.RestoreState(AState: TObject);
var
  State: TDirViewState;
  DirColProperties: TCustomDirViewColProperties;
  ListItem: TListItem;
begin
  Assert(AState is TDirViewState);
  State := AState as TDirViewState;
  Assert(Assigned(State));

  FHistoryPaths.Assign(State.HistoryPaths);
  FBackCount := State.BackCount;
  DoHistoryChange;
  // TCustomDirViewColProperties should not be here
  DirColProperties := ColProperties as TCustomDirViewColProperties;
  Assert(Assigned(DirColProperties));
  DirColProperties.SortStr := State.SortStr;
  Mask := State.Mask;
  if State.FocusedItem <> '' then
  begin
    ListItem := FindFileItem(State.FocusedItem);
    if Assigned(ListItem) then
    begin
      ItemFocused := ListItem;
      ListItem.MakeVisible(False);
    end;
  end;
end;

procedure TCustomDirView.ClearState;
begin
  FHistoryPaths.Clear;
  FBackCount := 0;
  DoHistoryChange;
end;

procedure TCustomDirView.SetMask(Value: string);
begin
  if Mask <> Value then
  begin
    FMask := Value;
    UpdatePathLabel;
    if DirOK then Reload(False);
  end;
end;{SetMask}

// WM_SETFOCUS works even when focus is moved to another window/app,
// while .Enter works only when focus is moved to order control of the same window.
procedure TCustomDirView.WMSetFocus(var Message: TWMSetFocus);
begin
  inherited;
  EnsureSelectionRedrawn;
end;

procedure TCustomDirView.WMKillFocus(var Message: TWMKillFocus);
begin
  inherited;
  EnsureSelectionRedrawn;
end;

procedure TCustomDirView.EnsureSelectionRedrawn;
begin
  // WORKAROUND
  // when receiving/losing focus, selection is not redrawn in report view
  // (except for focus item selection),
  // probably when double buffering is enabled (LVS_EX_DOUBLEBUFFER).
  // But even without LVS_EX_DOUBLEBUFFER, selection behing file icon is not updated.
  if ViewStyle = vsReport then
  begin
    if (SelCount >= 2) or ((SelCount >= 1) and ((not Assigned(ItemFocused)) or (not ItemFocused.Selected))) then
    begin
      Invalidate;
    end
      else
    if Assigned(ItemFocused) and ItemFocused.Selected then
    begin
      // Optimization. When no item is selected, redraw just the focused item.
      ItemFocused.Update;
    end;
  end;
end;

initialization
  HasExtendedCOMCTL32 := COMCTL32OK;

  DropSourceControl := nil;

  SetLength(WinDir, MAX_PATH);
  SetLength(WinDir, GetWindowsDirectory(PChar(WinDir), MAX_PATH));

  SetLength(TempDir, MAX_PATH);
  SetLength(TempDir, GetTempPath(MAX_PATH, PChar(TempDir)));

  SpecialFolderLocation(CSIDL_PERSONAL, UserDocumentDirectory);

  UnknownFileIcon := GetshFileInfo('$#)(.#$)', FILE_ATTRIBUTE_NORMAL,
    SHGFI_SYSICONINDEX or SHGFI_USEFILEATTRIBUTES).iIcon;
  DefaultExeIcon := GetshFileInfo('.COM',
    FILE_ATTRIBUTE_NORMAL, SHGFI_SYSICONINDEX or SHGFI_USEFILEATTRIBUTES).iIcon;

  with GetshFileInfo(WinDir, FILE_ATTRIBUTE_NORMAL or FILE_ATTRIBUTE_DIRECTORY,
    SHGFI_TYPENAME or SHGFI_SYSICONINDEX or SHGFI_USEFILEATTRIBUTES) do
  begin
    StdDirTypeName := szTypeName;
    StdDirIcon := iIcon;
  end;
  StdDirSelIcon := GetIconIndex(WinDir,
    FILE_ATTRIBUTE_NORMAL or FILE_ATTRIBUTE_DIRECTORY, SHGFI_OPENICON);

  WinDir := IncludeTrailingPathDelimiter(WinDir);
  TempDir := IncludeTrailingPathDelimiter(TempDir);


finalization
  SetLength(StdDirTypeName, 0);
  SetLength(WinDir, 0);
  SetLength(TempDir, 0);
end.
