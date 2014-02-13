object NonVisualDataModule: TNonVisualDataModule
  OldCreateOrder = False
  Height = 502
  Width = 624
  object LogActions: TActionList
    Images = GlyphsModule.LogImages
    OnExecute = LogActionsExecute
    OnUpdate = LogActionsUpdate
    Left = 32
    Top = 104
    object LogClearAction: TAction
      Category = 'LogMemo'
      Caption = 'C&lear'
      Hint = 'Clear log'
      ImageIndex = 0
      ShortCut = 16430
    end
    object LogSelectAllAction: TAction
      Category = 'LogMemo'
      Caption = 'Select &All'
      Hint = 'Select all'
      ImageIndex = 2
      ShortCut = 16449
    end
    object LogCopyAction: TAction
      Category = 'LogMemo'
      Caption = '&Copy'
      Hint = 'Copy to clipboard'
      ImageIndex = 1
      ShortCut = 16451
    end
    object LogPreferencesAction: TAction
      Category = 'LogForm'
      Caption = 'LogPreferencesAction'
      Hint = 'Configure logging'
      ImageIndex = 3
    end
  end
  object LogMemoPopup: TTBXPopupMenu
    Images = GlyphsModule.LogImages
    Left = 32
    Top = 152
    object Clear1: TTBXItem
      Action = LogClearAction
    end
    object Close1: TTBXItem
      Action = LogCopyAction
    end
    object Selectall1: TTBXItem
      Action = LogSelectAllAction
    end
  end
  object RemoteFilePopup: TTBXPopupMenu
    Images = GlyphsModule.ExplorerImages
    Left = 424
    Top = 336
    object TBXItem23: TTBXItem
      Action = CurrentAddEditLinkContextAction
    end
    object RemoteOpenMenuItem: TTBXItem
      Action = CurrentOpenAction
    end
    object RemoteEditMenuItem: TTBXItem
      Action = CurrentEditFocusedAction
    end
    object RemoteCopyMenuItem: TTBXItem
      Action = RemoteCopyFocusedAction
    end
    object Moveto1: TTBXItem
      Action = RemoteMoveFocusedAction
    end
    object Duplicate3: TTBXItem
      Action = RemoteCopyToAction
    end
    object Moveto6: TTBXItem
      Action = RemoteMoveToFocusedAction
    end
    object Delete1: TTBXItem
      Action = CurrentDeleteFocusedAction
    end
    object Rename1: TTBXItem
      Action = CurrentRenameAction
    end
    object N45: TTBXSeparatorItem
    end
    object RemoteDirViewCustomCommandsMenu: TTBXSubmenuItem
      Action = CustomCommandsAction
      object TTBXItem
      end
    end
    object FileNames3: TTBXSubmenuItem
      Caption = '&File Names'
      HelpKeyword = 'filenames'
      Hint = 'Operations with name(s) of selected file(s)'
      object InserttoCommandLine2: TTBXItem
        Action = FileListToCommandLineAction
      end
      object CopytoClipboard3: TTBXItem
        Action = FileListToClipboardAction
      end
      object CopytoClipboardIncludePaths3: TTBXItem
        Action = FullFileListToClipboardAction
      end
      object CopyURLtoClipboard3: TTBXItem
        Action = UrlToClipboardAction
      end
    end
    object N1: TTBXSeparatorItem
    end
    object Properties1: TTBXItem
      Action = CurrentPropertiesFocusedAction
    end
  end
  object ExplorerActions: TActionList
    Images = GlyphsModule.ExplorerImages
    OnExecute = ExplorerActionsExecute
    OnUpdate = ExplorerActionsUpdate
    Left = 440
    Top = 24
    object LocalCopyFocusedAction: TAction
      Tag = 8
      Category = 'Local Focused Operation'
      Caption = 'Up&load...'
      HelpKeyword = 'task_upload'
      Hint = 'Upload|Upload selected local file(s) to remote directory'
      ImageIndex = 88
    end
    object RemoteCopyFocusedAction: TAction
      Tag = 12
      Category = 'Remote Focused Operation'
      Caption = 'Down&load...'
      HelpKeyword = 'task_download'
      Hint = 'Download|Download selected remote file(s) to local directory'
      ImageIndex = 89
    end
    object RemoteMoveFocusedAction: TAction
      Tag = 12
      Category = 'Remote Focused Operation'
      Caption = 'Download and Dele&te...'
      HelpKeyword = 'task_download'
      Hint = 
        'Download and Delete|Download selected remote file(s) to local di' +
        'rectory and delete original'
      ImageIndex = 97
    end
    object RemoteCopyAction: TAction
      Tag = 14
      Category = 'Remote Selected Operation'
      Caption = 'Down&load...'
      HelpKeyword = 'task_download'
      Hint = 'Download|Download selected remote file(s) to local directory'
      ImageIndex = 89
    end
    object BestFitColumnAction: TAction
      Tag = 15
      Category = 'Columns'
      Caption = '&Best Fit'
      Hint = 'Best fit|Adjust column width to fit its content'
    end
    object GoToTreeAction: TAction
      Tag = 15
      Category = 'View'
      Caption = 'Go to Tree'
      HelpKeyword = 'ui_file_panel#directory_tree'
      Hint = 'Go to tree'
      ImageIndex = 76
      ShortCut = 49236
    end
    object LocalTreeAction: TAction
      Tag = 8
      Category = 'View'
      Caption = '&Tree'
      HelpKeyword = 'ui_file_panel#directory_tree'
      Hint = 'Hide/show directory tree'
      ImageIndex = 76
      ShortCut = 49236
    end
    object RemoteTreeAction: TAction
      Tag = 12
      Category = 'View'
      Caption = '&Tree'
      HelpKeyword = 'ui_file_panel#directory_tree'
      Hint = 'Hide/show directory tree'
      ImageIndex = 76
      ShortCut = 49236
    end
    object QueueItemQueryAction: TAction
      Tag = 12
      Category = 'Queue'
      Caption = '&Show Query'
      HelpKeyword = 'ui_queue#managing_the_queue'
      Hint = 'Show pending query of selected queue item'
      ImageIndex = 67
    end
    object QueueItemErrorAction: TAction
      Tag = 12
      Category = 'Queue'
      Caption = '&Show Error'
      HelpKeyword = 'ui_queue#managing_the_queue'
      Hint = 'Show pending error message of selected queue item'
      ImageIndex = 68
    end
    object QueueItemPromptAction: TAction
      Tag = 12
      Category = 'Queue'
      Caption = '&Show Prompt'
      HelpKeyword = 'ui_queue#managing_the_queue'
      Hint = 'Show pending prompt of selected queue item'
      ImageIndex = 69
    end
    object GoToCommandLineAction: TAction
      Tag = 11
      Category = 'View'
      Caption = 'Go to Comma&nd Line'
      HelpKeyword = 'ui_commander#command_line'
      Hint = 'Go to command line'
      ShortCut = 24654
    end
    object QueueItemDeleteAction: TAction
      Tag = 12
      Category = 'Queue'
      Caption = '&Cancel'
      HelpKeyword = 'ui_queue#managing_the_queue'
      Hint = 'Remove selected queue item'
      ImageIndex = 71
    end
    object QueueItemExecuteAction: TAction
      Tag = 12
      Category = 'Queue'
      Caption = '&Execute Now'
      HelpKeyword = 'ui_queue#managing_the_queue'
      Hint = 
        'Execute selected queue item immediately by granting it additiona' +
        'l connection'
      ImageIndex = 70
    end
    object SelectOneAction: TAction
      Tag = 12
      Category = 'Selection'
      Caption = '&Select/Unselect'
      HelpKeyword = 'ui_file_panel#selecting_files'
      Hint = 'Select|Select/unselect focused file'
    end
    object CurrentRenameAction: TAction
      Tag = 15
      Category = 'Selected Operation'
      Caption = '&Rename'
      HelpKeyword = 'task_rename'
      Hint = 'Rename|Rename selected file'
      ImageIndex = 3
    end
    object LocalSortAscendingAction: TAction
      Tag = 9
      Category = 'Sort'
      Caption = '&Ascending'
      HelpKeyword = 'ui_file_panel#sorting_files'
      Hint = 
        'Ascending/descending|Toggle ascending/descending sort of local p' +
        'anel'
      ImageIndex = 37
    end
    object CurrentEditAction: TAction
      Tag = 15
      Category = 'Selected Operation'
      Caption = '&Edit'
      HelpKeyword = 'task_edit'
      Hint = 'Edit|Edit selected file(s)'
      ImageIndex = 57
    end
    object HideColumnAction: TAction
      Tag = 15
      Category = 'Columns'
      Caption = '&Hide Column'
      HelpKeyword = 'ui_file_panel#selecting_columns'
      Hint = 'Hide column|Hide selected column'
    end
    object LocalBackAction: TAction
      Tag = 9
      Category = 'Local Directory'
      Caption = '&Back'
      HelpKeyword = 'task_navigate#special_commands'
      ImageIndex = 6
      ShortCut = 32805
    end
    object CurrentCycleStyleAction: TAction
      Tag = 7
      Category = 'Style'
      Caption = 'View'
      HelpKeyword = 'ui_file_panel#view_style'
      Hint = 'View|Cycle thru directory view styles'
      ImageIndex = 8
    end
    object CurrentIconAction: TAction
      Tag = 7
      Category = 'Style'
      Caption = '&Large Icons'
      HelpKeyword = 'ui_file_panel#view_style'
      Hint = 'Large Icons|View large icons'
      ImageIndex = 8
    end
    object CurrentSmallIconAction: TAction
      Tag = 7
      Category = 'Style'
      Caption = '&Small Icons'
      HelpKeyword = 'ui_file_panel#view_style'
      Hint = 'Small Icons|View small icons'
      ImageIndex = 9
    end
    object CurrentListAction: TAction
      Tag = 7
      Category = 'Style'
      Caption = 'Lis&t'
      HelpKeyword = 'ui_file_panel#view_style'
      Hint = 'List|View list'
      ImageIndex = 10
    end
    object CurrentReportAction: TAction
      Tag = 7
      Category = 'Style'
      Caption = '&Details'
      HelpKeyword = 'ui_file_panel#view_style'
      Hint = 'Details|View details'
      ImageIndex = 11
    end
    object RemoteMoveToAction: TAction
      Tag = 14
      Category = 'Remote Selected Operation'
      Caption = 'Mo&ve To...'
      HelpKeyword = 'task_move_duplicate#moving_remote_files'
      Hint = 
        'Move|Move selected remote file(s) to another remote directory or' +
        ' another name'
      ImageIndex = 100
    end
    object CurrentDeleteFocusedAction: TAction
      Tag = 12
      Category = 'Focused Operation'
      Caption = '&Delete'
      HelpKeyword = 'task_delete'
      Hint = 'Delete|Delete selected file(s)'
      ImageIndex = 2
    end
    object CurrentPropertiesFocusedAction: TAction
      Tag = 12
      Category = 'Focused Operation'
      Caption = '&Properties'
      HelpKeyword = 'task_properties'
      Hint = 'Properties|Display/change properties of selected file(s)'
      ImageIndex = 4
    end
    object CurrentCreateDirAction: TAction
      Tag = 15
      Category = 'Selected Operation'
      Caption = '&Create Directory...'
      HelpKeyword = 'task_create_directory'
      Hint = 'Create directory|Create new directory'
      ImageIndex = 5
    end
    object CurrentDeleteAction: TAction
      Tag = 15
      Category = 'Selected Operation'
      Caption = '&Delete'
      HelpKeyword = 'task_delete'
      Hint = 'Delete|Delete selected file(s)'
      ImageIndex = 2
    end
    object CurrentPropertiesAction: TAction
      Tag = 15
      Category = 'Selected Operation'
      Caption = '&Properties'
      HelpKeyword = 'task_properties'
      Hint = 'Properties|Display/change properties of selected file(s)'
      ImageIndex = 4
    end
    object RemoteBackAction: TAction
      Tag = 14
      Category = 'Remote Directory'
      Caption = '&Back'
      HelpKeyword = 'task_navigate#special_commands'
      ImageIndex = 6
      ShortCut = 32805
    end
    object RemoteForwardAction: TAction
      Tag = 14
      Category = 'Remote Directory'
      Caption = '&Forward'
      HelpKeyword = 'task_navigate#special_commands'
      ImageIndex = 7
      ShortCut = 32807
    end
    object CommandLinePanelAction: TAction
      Tag = 8
      Category = 'View'
      Caption = 'Comma&nd Line'
      HelpKeyword = 'ui_commander#command_line'
      Hint = 'Hide/show command line'
      ShortCut = 24654
    end
    object RemoteParentDirAction: TAction
      Tag = 12
      Category = 'Remote Directory'
      Caption = '&Parent Directory'
      HelpKeyword = 'task_navigate#special_commands'
      Hint = 'Parent directory|Go to parent directory'
      ImageIndex = 12
    end
    object RemoteRootDirAction: TAction
      Tag = 12
      Category = 'Remote Directory'
      Caption = '&Root Directory'
      HelpKeyword = 'task_navigate#special_commands'
      Hint = 'Root directory|Go to root directory'
      ImageIndex = 13
      ShortCut = 16604
    end
    object RemoteHomeDirAction: TAction
      Tag = 14
      Category = 'Remote Directory'
      Caption = '&Home Directory'
      HelpKeyword = 'task_navigate#special_commands'
      Hint = 'Home directory|Go to home directory'
      ImageIndex = 15
    end
    object RemoteRefreshAction: TAction
      Tag = 14
      Category = 'Remote Directory'
      Caption = '&Refresh'
      Hint = 'Refresh|Refresh directory content'
      ImageIndex = 16
    end
    object AboutAction: TAction
      Tag = 15
      Category = 'Help'
      Caption = '&About...'
      HelpKeyword = 'ui_about'
      Hint = 'About|Show About box'
      ImageIndex = 65
    end
    object StatusBarAction: TAction
      Tag = 15
      Category = 'View'
      Caption = 'Status &Bar'
      Hint = 'Hide/show status bar'
    end
    object SessionsTabsAction: TAction
      Tag = 15
      Category = 'View'
      Caption = 'Sessions Tabs'
      Hint = 'Hide/show sessions tabs'
    end
    object ExplorerAddressBandAction: TAction
      Tag = 7
      Category = 'View'
      Caption = '&Address'
      HelpKeyword = 'ui_toolbars'
      Hint = 'Hide/show address toolbar'
    end
    object ExplorerMenuBandAction: TAction
      Tag = 7
      Category = 'View'
      Caption = '&Menu'
      HelpKeyword = 'ui_toolbars'
      Hint = 'Hide/show menu'
    end
    object ExplorerToolbarBandAction: TAction
      Tag = 7
      Category = 'View'
      Caption = '&Standard Buttons'
      HelpKeyword = 'ui_toolbars'
      Hint = 'Hide/show standard toolbar'
    end
    object RemoteOpenDirAction: TAction
      Tag = 14
      Category = 'Remote Directory'
      Caption = '&Open Directory/Bookmark...'
      HelpKeyword = 'task_navigate#entering_path_manually'
      Hint = 
        'Open directory/bookmark|Open specified directory or saved bookma' +
        'rk'
      ImageIndex = 18
    end
    object SelectAction: TAction
      Tag = 15
      Category = 'Selection'
      Caption = 'Sele&ct Files...'
      HelpKeyword = 'ui_select'
      Hint = 'Select|Select files by mask'
      ImageIndex = 19
    end
    object UnselectAction: TAction
      Tag = 15
      Category = 'Selection'
      Caption = '&Unselect Files...'
      HelpKeyword = 'ui_select'
      Hint = 'Unselect|Unselect files by mask'
      ImageIndex = 20
    end
    object SelectAllAction: TAction
      Tag = 15
      Category = 'Selection'
      Caption = 'Select &All'
      HelpKeyword = 'ui_file_panel#selecting_files'
      Hint = 'Select all files'
      ImageIndex = 21
    end
    object InvertSelectionAction: TAction
      Tag = 15
      Category = 'Selection'
      Caption = '&Invert Selection'
      HelpKeyword = 'ui_file_panel#selecting_files'
      Hint = 'Invert selection'
      ImageIndex = 22
    end
    object ExplorerSelectionBandAction: TAction
      Tag = 7
      Category = 'View'
      Caption = 'Se&lection Buttons'
      HelpKeyword = 'ui_toolbars'
      Hint = 'Hide/show selection toolbar'
    end
    object ClearSelectionAction: TAction
      Tag = 15
      Category = 'Selection'
      Caption = 'C&lear Selection'
      HelpKeyword = 'ui_file_panel#selecting_files'
      Hint = 'Clear selection'
      ImageIndex = 23
    end
    object ExplorerSessionBandAction: TAction
      Tag = 7
      Category = 'View'
      Caption = 'Sessio&n Buttons'
      HelpKeyword = 'ui_toolbars'
      Hint = 'Hide/show session toolbar'
    end
    object ExplorerPreferencesBandAction: TAction
      Tag = 7
      Category = 'View'
      Caption = '&Preferences Buttons'
      HelpKeyword = 'ui_toolbars'
      Hint = 'Hide/show preferences toolbar'
    end
    object ExplorerSortBandAction: TAction
      Tag = 7
      Category = 'View'
      Caption = 'So&rt Buttons'
      HelpKeyword = 'ui_toolbars'
      Hint = 'Hide/show sort toolbar'
    end
    object ExplorerUpdatesBandAction: TAction
      Tag = 7
      Category = 'View'
      Caption = '&Update Button'
      HelpKeyword = 'ui_toolbars'
      Hint = 'Hide/show updates toolbar'
    end
    object ExplorerTransferBandAction: TAction
      Tag = 7
      Category = 'View'
      Caption = '&Transfer Settings'
      HelpKeyword = 'ui_toolbars'
      Hint = 'Hide/show transfer settings toolbar'
    end
    object ExplorerCustomCommandsBandAction: TAction
      Tag = 7
      Category = 'View'
      Caption = 'Custom Co&mmand Buttons'
      HelpKeyword = 'ui_toolbars'
      Hint = 'Hide/show custom commands toolbar'
    end
    object ViewLogAction: TAction
      Tag = 15
      Category = 'View'
      Caption = 'Lo&g Window'
      HelpKeyword = 'ui_log'
      Hint = 'Show/hide log window'
      ImageIndex = 24
    end
    object NewSessionAction: TAction
      Tag = 15
      Category = 'Session'
      Caption = '&New Session...'
      HelpKeyword = 'task_connections#opening_additional_connection'
      Hint = 
        'New session|Opens new session (hold down Shift to open session i' +
        'n new window)'
      ImageIndex = 25
      ShortCut = 16462
      SecondaryShortCuts.Strings = (
        'Ctrl+Shift+N')
    end
    object SiteManagerAction: TAction
      Tag = 15
      Category = 'Session'
      Caption = 'Site &Manager...'
      HelpKeyword = 'ui_login_stored_sessions'
      Hint = 
        'Site Manager|Opens site manager (hold down Shift to open site ma' +
        'nager in new window)'
    end
    object CloseSessionAction: TAction
      Tag = 15
      Category = 'Session'
      Caption = '&Disconnect'
      Hint = 'Close session|Terminate current session'
      ImageIndex = 26
      ShortCut = 24644
      SecondaryShortCuts.Strings = (
        'Ctrl+W')
    end
    object SavedSessionsAction2: TAction
      Tag = 15
      Category = 'Session'
      Caption = 'Si&tes'
      HelpKeyword = 'task_connections#opening_additional_connection'
      Hint = 'Open site'
      ImageIndex = 27
    end
    object WorkspacesAction: TAction
      Tag = 15
      Category = 'Session'
      Caption = '&Workspaces'
      HelpKeyword = 'workspace'
      Hint = 'Open workspace'
      ImageIndex = 101
    end
    object PreferencesAction: TAction
      Tag = 15
      Category = 'View'
      Caption = '&Preferences...'
      HelpKeyword = 'ui_preferences'
      Hint = 'Preferences|Show/change user preferences'
      ImageIndex = 28
      ShortCut = 49232
    end
    object RemoteChangePathAction: TAction
      Tag = 11
      Category = 'Remote Directory'
      Caption = '&Change Directory'
      HelpKeyword = 'task_navigate'
      Hint = 'Allows selection of different directory for remote panel'
      ImageIndex = 29
      ShortCut = 32881
    end
    object LocalForwardAction: TAction
      Tag = 9
      Category = 'Local Directory'
      Caption = '&Forward'
      HelpKeyword = 'task_navigate#special_commands'
      ImageIndex = 7
      ShortCut = 32807
    end
    object LocalParentDirAction: TAction
      Tag = 8
      Category = 'Local Directory'
      Caption = '&Parent Directory'
      HelpKeyword = 'task_navigate#special_commands'
      Hint = 'Parent directory|Go to parent directory'
      ImageIndex = 12
    end
    object LocalRootDirAction: TAction
      Tag = 8
      Category = 'Local Directory'
      Caption = '&Root Directory'
      HelpKeyword = 'task_navigate#special_commands'
      Hint = 'Root directory|Go to root directory'
      ImageIndex = 14
      ShortCut = 16604
    end
    object LocalHomeDirAction: TAction
      Tag = 9
      Category = 'Local Directory'
      Caption = '&Home Directory'
      HelpKeyword = 'task_navigate#special_commands'
      Hint = 'Home directory|Go to home directory'
      ImageIndex = 15
    end
    object LocalRefreshAction: TAction
      Tag = 9
      Category = 'Local Directory'
      Caption = '&Refresh'
      Hint = 'Refresh|Refresh directory content'
      ImageIndex = 16
    end
    object LocalOpenDirAction: TAction
      Tag = 9
      Category = 'Local Directory'
      Caption = '&Open Directory/Bookmark...'
      HelpKeyword = 'task_navigate#entering_path_manually'
      Hint = 
        'Open directory/bookmark|Open specified directory or saved bookma' +
        'rk'
      ImageIndex = 18
    end
    object LocalChangePathAction: TAction
      Tag = 11
      Category = 'Local Directory'
      Caption = '&Change Drive'
      HelpKeyword = 'task_navigate'
      Hint = 'Allows selection of different drive for local panel'
      ImageIndex = 30
      ShortCut = 32880
    end
    object ToolBar2Action: TAction
      Tag = 11
      Category = 'View'
      Caption = '&Hot Keys Toolbar'
      HelpKeyword = 'ui_toolbars'
      Hint = 'Hide/show bottom hot keys toolbar'
    end
    object CommanderMenuBandAction: TAction
      Tag = 11
      Category = 'View'
      Caption = '&Menu'
      HelpKeyword = 'ui_toolbars'
      Hint = 'Hide/show menu'
    end
    object CommanderSessionBandAction: TAction
      Tag = 11
      Category = 'View'
      Caption = 'Sessio&n Buttons'
      HelpKeyword = 'ui_toolbars'
      Hint = 'Hide/show session toolbar'
    end
    object CommanderPreferencesBandAction: TAction
      Tag = 11
      Category = 'View'
      Caption = '&Preferences Buttons'
      HelpKeyword = 'ui_toolbars'
      Hint = 'Hide/show preferences toolbar'
    end
    object CommanderSortBandAction: TAction
      Tag = 11
      Category = 'View'
      Caption = 'So&rt Buttons'
      HelpKeyword = 'ui_toolbars'
      Hint = 'Hide/show sort toolbar'
    end
    object CommanderUpdatesBandAction: TAction
      Tag = 11
      Category = 'View'
      Caption = '&Update Button'
      HelpKeyword = 'ui_toolbars'
      Hint = 'Hide/show updates toolbar'
    end
    object CommanderTransferBandAction: TAction
      Tag = 11
      Category = 'View'
      Caption = '&Transfer Settings'
      HelpKeyword = 'ui_toolbars'
      Hint = 'Hide/show transfer settings toolbar'
    end
    object CommanderCommandsBandAction: TAction
      Tag = 11
      Category = 'View'
      Caption = '&Commands Buttons'
      HelpKeyword = 'ui_toolbars'
      Hint = 'Hide/show commands toolbar'
    end
    object CommanderCustomCommandsBandAction: TAction
      Tag = 11
      Category = 'View'
      Caption = 'Custom Co&mmand Buttons'
      HelpKeyword = 'ui_toolbars'
      Hint = 'Hide/show custom commands toolbar'
    end
    object CommanderLocalHistoryBandAction: TAction
      Tag = 11
      Category = 'View'
      Caption = '&History Buttons'
      HelpKeyword = 'ui_toolbars'
      Hint = 'Hide/show local history toolbar'
    end
    object CommanderLocalNavigationBandAction: TAction
      Tag = 11
      Category = 'View'
      Caption = '&Navigation Buttons'
      HelpKeyword = 'ui_toolbars'
      Hint = 'Hide/show local navigation toolbar'
    end
    object CommanderLocalFileBandAction: TAction
      Tag = 11
      Category = 'View'
      Caption = '&File Buttons'
      HelpKeyword = 'ui_toolbars'
      Hint = 'Hide/show local file toolbar'
    end
    object CommanderLocalSelectionBandAction: TAction
      Tag = 11
      Category = 'View'
      Caption = 'Se&lection Buttons'
      HelpKeyword = 'ui_toolbars'
      Hint = 'Hide/show local selection toolbar'
    end
    object CommanderRemoteHistoryBandAction: TAction
      Tag = 11
      Category = 'View'
      Caption = '&History Buttons'
      HelpKeyword = 'ui_toolbars'
      Hint = 'Hide/show remote history toolbar'
    end
    object CommanderRemoteNavigationBandAction: TAction
      Tag = 11
      Category = 'View'
      Caption = '&Navigation Buttons'
      HelpKeyword = 'ui_toolbars'
      Hint = 'Hide/show remote navigation toolbar'
    end
    object CommanderRemoteFileBandAction: TAction
      Tag = 11
      Category = 'View'
      Caption = '&File Buttons'
      HelpKeyword = 'ui_toolbars'
      Hint = 'Hide/show remote file toolbar'
    end
    object CommanderRemoteSelectionBandAction: TAction
      Tag = 11
      Category = 'View'
      Caption = 'Se&lection Buttons'
      HelpKeyword = 'ui_toolbars'
      Hint = 'Hide/show remote selection toolbar'
    end
    object LocalStatusBarAction: TAction
      Tag = 11
      Category = 'View'
      Caption = 'Status &Bar'
      Hint = 'Hide/show local panel status bar'
    end
    object RemoteStatusBarAction: TAction
      Tag = 11
      Category = 'View'
      Caption = 'Status &Bar'
      Hint = 'Hide/show remote panel status bar'
    end
    object LocalSortByNameAction: TAction
      Tag = 9
      Category = 'Sort'
      Caption = 'By &Name'
      HelpKeyword = 'ui_file_panel#sorting_files'
      Hint = 'Sort by name|Sort local panel by name'
      ImageIndex = 31
      ShortCut = 16498
    end
    object LocalSortByExtAction: TAction
      Tag = 9
      Category = 'Sort'
      Caption = 'By &Extension'
      HelpKeyword = 'ui_file_panel#sorting_files'
      Hint = 'Sort by extension|Sort local panel by file name extension'
      ImageIndex = 32
      ShortCut = 16499
    end
    object LocalSortBySizeAction: TAction
      Tag = 9
      Category = 'Sort'
      Caption = 'By &Size'
      HelpKeyword = 'ui_file_panel#sorting_files'
      Hint = 'Sort by size|Sort local panel by file size'
      ImageIndex = 35
      ShortCut = 16501
    end
    object LocalSortByAttrAction: TAction
      Tag = 9
      Category = 'Sort'
      Caption = 'By A&ttributes'
      HelpKeyword = 'ui_file_panel#sorting_files'
      Hint = 'Sort by attributes|Sort local panel by attributes'
      ImageIndex = 36
      ShortCut = 16502
    end
    object LocalSortByTypeAction: TAction
      Tag = 9
      Category = 'Sort'
      Caption = 'By &Type'
      HelpKeyword = 'ui_file_panel#sorting_files'
      Hint = 'Sort by type|Sort local panel by file type'
      ImageIndex = 34
    end
    object LocalSortByChangedAction: TAction
      Tag = 9
      Category = 'Sort'
      Caption = 'By &Modification'
      HelpKeyword = 'ui_file_panel#sorting_files'
      Hint = 'Sort by time|Sort local panel by last modification time'
      ImageIndex = 33
      ShortCut = 16500
    end
    object RemoteSortAscendingAction: TAction
      Tag = 14
      Category = 'Sort'
      Caption = '&Ascending'
      HelpKeyword = 'ui_file_panel#sorting_files'
      Hint = 
        'Ascending/descending|Toggle ascending/descending sort of remote ' +
        'panel'
      ImageIndex = 37
    end
    object RemoteSortByNameAction: TAction
      Tag = 14
      Category = 'Sort'
      Caption = 'By &Name'
      HelpKeyword = 'ui_file_panel#sorting_files'
      Hint = 'Sort by name|Sort remote panel by name'
      ImageIndex = 31
      ShortCut = 16498
    end
    object RemoteSortByExtAction: TAction
      Tag = 14
      Category = 'Sort'
      Caption = 'By &Extension'
      HelpKeyword = 'ui_file_panel#sorting_files'
      Hint = 'Sort by extension|Sort remote panel by file name extension'
      ImageIndex = 32
      ShortCut = 16499
    end
    object RemoteSortBySizeAction: TAction
      Tag = 14
      Category = 'Sort'
      Caption = 'By &Size'
      HelpKeyword = 'ui_file_panel#sorting_files'
      Hint = 'Sort by size|Sort remote panel by file size'
      ImageIndex = 35
      ShortCut = 16501
    end
    object RemoteSortByRightsAction: TAction
      Tag = 14
      Category = 'Sort'
      Caption = 'By &Permissions'
      HelpKeyword = 'ui_file_panel#sorting_files'
      Hint = 'Sort by permissions|Sort remote panel by permissions'
      ImageIndex = 36
      ShortCut = 16502
    end
    object RemoteSortByChangedAction: TAction
      Tag = 14
      Category = 'Sort'
      Caption = 'By &Modification'
      HelpKeyword = 'ui_file_panel#sorting_files'
      Hint = 'Sort by time|Sort remote panel by last modification time'
      ImageIndex = 33
      ShortCut = 16500
    end
    object RemoteSortByOwnerAction: TAction
      Tag = 14
      Category = 'Sort'
      Caption = 'By &Owner'
      HelpKeyword = 'ui_file_panel#sorting_files'
      Hint = 'Sort by owner|Sort remote panel by file owner'
      ImageIndex = 38
      ShortCut = 16503
    end
    object RemoteSortByGroupAction: TAction
      Tag = 14
      Category = 'Sort'
      Caption = 'By &Group'
      HelpKeyword = 'ui_file_panel#sorting_files'
      Hint = 'Sort by group|Sort remote panel by file group'
      ImageIndex = 39
      ShortCut = 16504
    end
    object RemoteSortByTypeAction: TAction
      Tag = 14
      Category = 'Sort'
      Caption = 'By &Type'
      HelpKeyword = 'ui_file_panel#sorting_files'
      Hint = 'Sort by type|Sort remote panel by file type'
      ImageIndex = 34
    end
    object CurrentSortAscendingAction: TAction
      Tag = 15
      Category = 'Sort'
      Caption = '&Ascending'
      HelpKeyword = 'ui_file_panel#sorting_files'
      Hint = 
        'Ascending/descending|Toggle ascending/descending sort of current' +
        ' panel'
      ImageIndex = 37
    end
    object CurrentSortByNameAction: TAction
      Tag = 15
      Category = 'Sort'
      Caption = 'By &Name'
      HelpKeyword = 'ui_file_panel#sorting_files'
      Hint = 'Sort by name|Sort current panel by name'
      ImageIndex = 31
      ShortCut = 16498
    end
    object CurrentSortByExtAction: TAction
      Tag = 15
      Category = 'Sort'
      Caption = 'By &Extension'
      HelpKeyword = 'ui_file_panel#sorting_files'
      Hint = 'Sort by extension|Sort current panel by file name extension'
      ImageIndex = 32
      ShortCut = 16499
    end
    object CurrentSortBySizeAction: TAction
      Tag = 15
      Category = 'Sort'
      Caption = 'By &Size'
      HelpKeyword = 'ui_file_panel#sorting_files'
      Hint = 'Sort by size|Sort current panel by file size'
      ImageIndex = 35
      ShortCut = 16501
    end
    object CurrentSortByTypeAction: TAction
      Tag = 15
      Category = 'Sort'
      Caption = 'By &Type'
      HelpKeyword = 'ui_file_panel#sorting_files'
      Hint = 'Sort by type|Sort current panel by file type (local panel only)'
      ImageIndex = 34
    end
    object CurrentSortByRightsAction: TAction
      Tag = 15
      Category = 'Sort'
      Caption = 'By &Attributes'
      HelpKeyword = 'ui_file_panel#sorting_files'
      Hint = 
        'Sort by attributes/permissions|Sort current panel by attributes/' +
        'permissions'
      ImageIndex = 36
      ShortCut = 16502
    end
    object CurrentSortByChangedAction: TAction
      Tag = 15
      Category = 'Sort'
      Caption = 'By &Modification'
      HelpKeyword = 'ui_file_panel#sorting_files'
      Hint = 'Sort by time|Sort current panel by last modification time'
      ImageIndex = 33
      ShortCut = 16500
    end
    object CurrentSortByOwnerAction: TAction
      Tag = 14
      Category = 'Sort'
      Caption = 'By &Owner'
      HelpKeyword = 'ui_file_panel#sorting_files'
      Hint = 
        'Sort by owner|Sort current panel by file owner (remote panel onl' +
        'y)'
      ImageIndex = 38
      ShortCut = 16503
    end
    object CurrentSortByGroupAction: TAction
      Tag = 14
      Category = 'Sort'
      Caption = 'By &Group'
      HelpKeyword = 'ui_file_panel#sorting_files'
      Hint = 
        'Sort by group|Sort current panel by file group (remote panel onl' +
        'y)'
      ImageIndex = 39
      ShortCut = 16504
    end
    object SortColumnAscendingAction: TAction
      Tag = 15
      Category = 'Sort'
      Caption = 'Sort &Ascending'
      HelpKeyword = 'ui_file_panel#sorting_files'
      Hint = 'Sort files ascending by selected column'
      ImageIndex = 41
    end
    object SortColumnDescendingAction: TAction
      Tag = 15
      Category = 'Sort'
      Caption = 'Sort &Descending'
      HelpKeyword = 'ui_file_panel#sorting_files'
      Hint = 'Sort files descending by selected column'
      ImageIndex = 40
    end
    object HomepageAction: TAction
      Tag = 15
      Category = 'Help'
      Caption = 'Product &Homepage'
      Hint = 'Opens web browser and points it to project homepage '
      ImageIndex = 42
    end
    object HistoryPageAction: TAction
      Tag = 15
      Category = 'Help'
      Caption = '&Version History'
      Hint = 'Opens web browser and points it to application history page'
    end
    object SaveCurrentSessionAction2: TAction
      Tag = 15
      Category = 'Session'
      Caption = '&Save Session as Site...'
      HelpKeyword = 'task_connections#saving_opened_session'
      Hint = 'Save session as site|Save current session as site'
      ImageIndex = 43
    end
    object ShowHideRemoteNameColumnAction: TAction
      Tag = 15
      Category = 'Columns'
      Caption = '&Name'
      HelpKeyword = 'ui_file_panel#selecting_columns'
      Hint = 'Show/hide name|Show/hide name column on remote panel'
      ImageIndex = 44
    end
    object ShowHideRemoteExtColumnAction: TAction
      Tag = 15
      Category = 'Columns'
      Caption = '&Extension'
      HelpKeyword = 'ui_file_panel#selecting_columns'
      Hint = 'Show/hide extension|Show/hide extension column on remote panel'
      ImageIndex = 45
    end
    object ShowHideRemoteSizeColumnAction: TAction
      Tag = 15
      Category = 'Columns'
      Caption = '&Size'
      HelpKeyword = 'ui_file_panel#selecting_columns'
      Hint = 'Show/hide size|Show/hide size column on remote panel'
      ImageIndex = 47
    end
    object ShowHideRemoteChangedColumnAction: TAction
      Tag = 15
      Category = 'Columns'
      Caption = '&Modification'
      HelpKeyword = 'ui_file_panel#selecting_columns'
      Hint = 
        'Show/hide modification|Show/hide modification column on remote p' +
        'anel'
      ImageIndex = 48
    end
    object ShowHideRemoteRightsColumnAction: TAction
      Tag = 15
      Category = 'Columns'
      Caption = '&Permissions'
      HelpKeyword = 'ui_file_panel#selecting_columns'
      Hint = 
        'Show/hide permissions|Show/hide permissions column on remote pan' +
        'el'
      ImageIndex = 49
    end
    object ShowHideRemoteOwnerColumnAction: TAction
      Tag = 15
      Category = 'Columns'
      Caption = '&Owner'
      HelpKeyword = 'ui_file_panel#selecting_columns'
      Hint = 'Show/hide owner|Show/hide owner column on remote panel'
      ImageIndex = 50
    end
    object ShowHideRemoteGroupColumnAction: TAction
      Tag = 15
      Category = 'Columns'
      Caption = '&Group'
      HelpKeyword = 'ui_file_panel#selecting_columns'
      Hint = 'Show/hide group|Show/hide group column on remote panel'
      ImageIndex = 51
    end
    object ShowHideRemoteLinkTargetColumnAction: TAction
      Tag = 15
      Category = 'Columns'
      Caption = '&Link Target'
      HelpKeyword = 'ui_file_panel#selecting_columns'
      Hint = 
        'Show/hide link target|Show/hide link target column on remote pan' +
        'el'
      ImageIndex = 82
    end
    object ShowHideRemoteTypeColumnAction: TAction
      Tag = 15
      Category = 'Columns'
      Caption = '&Type'
      HelpKeyword = 'ui_file_panel#selecting_columns'
      Hint = 'Show/hide type|Show/hide type column on remote panel'
      ImageIndex = 46
    end
    object ShowHideLocalNameColumnAction: TAction
      Tag = 15
      Category = 'Columns'
      Caption = '&Name'
      HelpKeyword = 'ui_file_panel#selecting_columns'
      Hint = 'Show/hide name|Show/hide name column on local panel'
      ImageIndex = 44
    end
    object ShowHideLocalExtColumnAction: TAction
      Tag = 15
      Category = 'Columns'
      Caption = '&Extension'
      HelpKeyword = 'ui_file_panel#selecting_columns'
      Hint = 'Show/hide extension|Show/hide extension column on local panel'
      ImageIndex = 45
    end
    object ShowHideLocalTypeColumnAction: TAction
      Tag = 15
      Category = 'Columns'
      Caption = '&Type'
      HelpKeyword = 'ui_file_panel#selecting_columns'
      Hint = 'Show/hide type|Show/hide type column on local panel'
      ImageIndex = 46
    end
    object ShowHideLocalSizeColumnAction: TAction
      Tag = 15
      Category = 'Columns'
      Caption = '&Size'
      HelpKeyword = 'ui_file_panel#selecting_columns'
      Hint = 'Show/hide size|Show/hide size column on local panel'
      ImageIndex = 47
    end
    object ShowHideLocalChangedColumnAction: TAction
      Tag = 15
      Category = 'Columns'
      Caption = '&Modification'
      HelpKeyword = 'ui_file_panel#selecting_columns'
      Hint = 
        'Show/hide modification|Show/hide modification column on local pa' +
        'nel'
      ImageIndex = 48
    end
    object ShowHideLocalAttrColumnAction: TAction
      Tag = 15
      Category = 'Columns'
      Caption = '&Attributes'
      HelpKeyword = 'ui_file_panel#selecting_columns'
      Hint = 'Show/hide attributes|Show/hide attributes column on local panel'
      ImageIndex = 49
    end
    object CompareDirectoriesAction: TAction
      Tag = 11
      Category = 'Command'
      Caption = '&Compare Directories'
      HelpKeyword = 'task_compare_directories'
      Hint = 
        'Compare directories|Mark different files in local and remote dir' +
        'ectory'
      ImageIndex = 52
      ShortCut = 8305
    end
    object SynchronizeAction: TAction
      Tag = 15
      Category = 'Command'
      Caption = '&Keep Remote Directory up to Date...'
      HelpKeyword = 'task_keep_up_to_date'
      Hint = 
        'Keep remote directory up to date|Keep remote directory up to dat' +
        'e'
      ImageIndex = 53
      ShortCut = 16469
    end
    object ForumPageAction: TAction
      Tag = 15
      Category = 'Help'
      Caption = '&Support Forum'
      Hint = 'Opens web browser and points it to support forum page'
    end
    object LocalAddBookmarkAction: TAction
      Tag = 9
      Category = 'Local Directory'
      Caption = '&Add Path to Bookmarks'
      HelpKeyword = 'task_navigate#bookmarks'
      Hint = 'Add to bookmarks|Add current local directory to bookmark list'
      ImageIndex = 54
      ShortCut = 16450
    end
    object RemoteAddBookmarkAction: TAction
      Tag = 14
      Category = 'Remote Directory'
      Caption = '&Add Path to Bookmarks'
      HelpKeyword = 'task_navigate#bookmarks'
      Hint = 'Add to bookmarks|Add current remote directory to bookmark list'
      ImageIndex = 54
      ShortCut = 16450
    end
    object ConsoleAction: TAction
      Tag = 15
      Category = 'Command'
      Caption = 'Open &Terminal'
      HelpKeyword = 'ui_console'
      Hint = 
        'Open terminal|Open terminal window that allow executing arbitrar' +
        'y command (with exception of commands that require user input)'
      ImageIndex = 55
      ShortCut = 16468
    end
    object PuttyAction: TAction
      Tag = 15
      Category = 'Command'
      Caption = 'Open in &PuTTY'
      HelpKeyword = 'integration_putty#open_putty'
      Hint = 
        'Open session in PuTTY|Execute PuTTY SSH terminal and opens curre' +
        'nt session with it'
      ImageIndex = 64
      ShortCut = 16464
    end
    object LocalExploreDirectoryAction: TAction
      Tag = 15
      Category = 'Local Directory'
      Caption = '&Explore Directory'
      Hint = 'Opens Windows Explorer with current local directory'
      ImageIndex = 56
      ShortCut = 49221
    end
    object CurrentOpenAction: TAction
      Tag = 15
      Category = 'Focused Operation'
      Caption = '&Open'
      HelpKeyword = 'task_edit'
      Hint = 
        'Open document|Open selected document using application associate' +
        'd with document type'
      ImageIndex = 58
    end
    object SynchronizeBrowsingAction: TAction
      Tag = 11
      Category = 'Command'
      AutoCheck = True
      Caption = 'Synchronize &Browsing'
      HelpKeyword = 'task_navigate#synchronize_browsing'
      Hint = 
        'Synchronize browsing|Synchronize local and remote directory brow' +
        'sing'
      ImageIndex = 59
      ShortCut = 49218
    end
    object CurrentAddEditLinkAction: TAction
      Tag = 15
      Category = 'Selected Operation'
      Caption = 'Edit &Link...'
      HelpKeyword = 'task_link'
      Hint = 
        'Add/edit link|Add new link/shortcut or edit selected link/shortc' +
        'ut'
      ImageIndex = 60
    end
    object CurrentAddEditLinkContextAction: TAction
      Tag = 15
      Category = 'Selected Operation'
      Caption = 'Edit &Link...'
      HelpKeyword = 'task_link'
      Hint = 'Edit link|Edit selected link/shortcut'
      ImageIndex = 60
    end
    object CloseApplicationAction: TAction
      Tag = 15
      Category = 'Command'
      Caption = '&Quit'
      Hint = 
        'Exit application|Terminate opened session(s) and close applicati' +
        'on'
    end
    object OpenedSessionsAction: TAction
      Tag = 15
      Category = 'Session'
      Caption = '&Opened Sessions'
      HelpKeyword = 'task_connections#switching_connections'
      Hint = 'Select session|Select opened session to activate'
      ImageIndex = 62
    end
    object DuplicateSessionAction: TAction
      Tag = 15
      Category = 'Session'
      Caption = '&Duplicate Session'
      HelpKeyword = 'task_connections'
      Hint = 
        'Duplicate session|Opens the same session again (hold down Shift ' +
        'to open session in new window)'
      ImageIndex = 91
    end
    object NewLinkAction: TAction
      Tag = 12
      Category = 'Command'
      Caption = '&Link...'
      HelpKeyword = 'task_link'
      Hint = 'Create link|Create new link/shortcut'
      ImageIndex = 60
    end
    object CustomCommandsAction: TAction
      Tag = 14
      Category = 'Command'
      Caption = '&Custom Commands'
      HelpKeyword = 'remote_command#custom_commands'
      Hint = 'Execute custom commands with selected file(s)'
    end
    object CustomCommandsCustomizeAction: TAction
      Tag = 15
      Category = 'Command'
      Caption = '&Customize...'
      HelpKeyword = 'ui_pref_commands'
      Hint = 'Customize custom commands'
      ImageIndex = 28
    end
    object CustomCommandsEnterAction: TAction
      Tag = 15
      Category = 'Command'
      Caption = '&Enter...'
      HelpKeyword = 'remote_command#executing_and_configuring_custom_commands'
      Hint = 'Enter ad hoc custom command'
      ImageIndex = 90
    end
    object CustomCommandsEnterFocusedAction: TAction
      Tag = 12
      Category = 'Command'
      Caption = '&Enter...'
      HelpKeyword = 'remote_command#executing_and_configuring_custom_commands'
      Hint = 'Enter ad hoc custom command'
      ImageIndex = 90
    end
    object CheckForUpdatesAction: TAction
      Tag = 15
      Category = 'Help'
      Caption = '&Check for Updates'
      HelpKeyword = 'updates'
      Hint = 'Queries application homepage for updates'
      ImageIndex = 63
    end
    object DonatePageAction: TAction
      Tag = 15
      Category = 'Help'
      Caption = '&Donate'
      Hint = 'Opens web browser and points it to project donation page'
    end
    object CustomCommandsLastAction: TAction
      Tag = 15
      Category = 'Command'
      Caption = 'CustomCommandsLastAction'
      HelpKeyword = 'remote_command#executing_and_configuring_custom_commands'
    end
    object CustomCommandsLastFocusedAction: TAction
      Tag = 12
      Category = 'Command'
      Caption = 'CustomCommandsLastFocusedAction'
      HelpKeyword = 'remote_command#executing_and_configuring_custom_commands'
    end
    object FileSystemInfoAction: TAction
      Tag = 15
      Category = 'Command'
      Caption = '&Server/Protocol Information'
      HelpKeyword = 'ui_fsinfo'
      Hint = 'Display server/protocol information'
      ImageIndex = 17
    end
    object ClearCachesAction: TAction
      Tag = 15
      Category = 'Command'
      Caption = 'Clea&r Caches'
      HelpKeyword = 'directory_cache'
      Hint = 'Clear directory listing and directory changes caches'
    end
    object FullSynchronizeAction: TAction
      Tag = 15
      Category = 'Command'
      Caption = '&Synchronize...'
      HelpKeyword = 'task_synchronize_full'
      Hint = 'Synchronize local directory with remote directory'
      ImageIndex = 66
      ShortCut = 16467
    end
    object RemoteMoveToFocusedAction: TAction
      Tag = 14
      Category = 'Remote Focused Operation'
      Caption = 'Mo&ve To...'
      HelpKeyword = 'task_move_duplicate#moving_remote_files'
      Hint = 
        'Move|Move selected remote file(s) to another remote directory or' +
        ' another name'
      ImageIndex = 100
    end
    object ShowHiddenFilesAction: TAction
      Tag = 15
      Category = 'View'
      Caption = 'Show/Hide &Hidden Files'
      HelpKeyword = 'ui_file_panel#special_files'
      Hint = 'Toggle showing hidden files in panel(s)'
      ShortCut = 49224
    end
    object FormatSizeBytesAction: TAction
      Tag = 15
      Category = 'View'
      Caption = 'Short &Files Sizes'
      HelpKeyword = 'ui_file_panel#short_format'
      Hint = 'Show files sizes in short format (KiB, MiB, etc)'
    end
    object LocalPathToClipboardAction: TAction
      Tag = 15
      Category = 'Local Directory'
      Caption = 'Copy &Path to Clipboard'
      HelpKeyword = 'filenames#current_working_directory'
      Hint = 'Copy current local path to clipboard'
    end
    object RemotePathToClipboardAction: TAction
      Tag = 15
      Category = 'Remote Directory'
      Caption = 'Copy &Path to Clipboard'
      HelpKeyword = 'filenames#current_working_directory'
      Hint = 'Copy current remote path to clipboard'
    end
    object FileListToCommandLineAction: TAction
      Tag = 11
      Category = 'Selected Operation'
      Caption = 'Insert to Command &Line'
      HelpKeyword = 'filenames#command_line'
      Hint = 'Insert name(s) of selected file(s) to command line'
      ShortCut = 16397
    end
    object FileListToClipboardAction: TAction
      Tag = 15
      Category = 'Selected Operation'
      Caption = 'Copy to &Clipboard'
      HelpKeyword = 'filenames#file_name'
      Hint = 'Copy name(s) of selected file(s) to clipboard'
      ShortCut = 24643
    end
    object FullFileListToClipboardAction: TAction
      Tag = 15
      Category = 'Selected Operation'
      Caption = 'Copy to Clipboard (Include &Paths)'
      HelpKeyword = 'filenames#file_name'
      Hint = 'Copy name(s) including path of selected file(s) to clipboard'
      ShortCut = 49219
    end
    object QueueGoToAction: TAction
      Tag = 15
      Category = 'Queue'
      Caption = '&Go To'
      HelpKeyword = 'ui_queue#managing_the_queue'
      Hint = 'Go to transfer queue list'
      ImageIndex = 74
      ShortCut = 16465
    end
    object QueueItemUpAction: TAction
      Tag = 12
      Category = 'Queue'
      Caption = 'Move &Up'
      HelpKeyword = 'ui_queue#managing_the_queue'
      Hint = 'Move selected queue item up to be processed earlier'
      ImageIndex = 72
    end
    object QueueItemDownAction: TAction
      Tag = 12
      Category = 'Queue'
      Caption = 'Move &Down'
      HelpKeyword = 'ui_queue#managing_the_queue'
      Hint = 'Move selected queue item down to be processed later'
      ImageIndex = 73
    end
    object QueueToggleShowAction: TAction
      Tag = 12
      Category = 'Queue'
      Caption = '&Queue'
      Hint = 'Show/hide queue list'
      ImageIndex = 74
    end
    object QueueShowAction: TAction
      Tag = 12
      Category = 'Queue'
      Caption = '&Show'
      HelpKeyword = 'ui_queue'
      Hint = 'Show queue list'
    end
    object QueueHideWhenEmptyAction: TAction
      Tag = 12
      Category = 'Queue'
      Caption = 'Hide when &Empty'
      HelpKeyword = 'ui_queue'
      Hint = 'Hide queue list when it is empty'
    end
    object QueueHideAction: TAction
      Tag = 12
      Category = 'Queue'
      Caption = '&Hide'
      HelpKeyword = 'ui_queue'
      Hint = 'Hide queue list'
    end
    object QueueToolbarAction: TAction
      Tag = 12
      Category = 'Queue'
      Caption = '&Toolbar'
      Hint = 'Hide/show queue list toolbar (on queue list panel)'
    end
    object QueuePreferencesAction: TAction
      Tag = 12
      Category = 'Queue'
      Caption = '&Customize...'
      HelpKeyword = 'ui_pref_background'
      Hint = 'Customize queue list'
      ImageIndex = 28
    end
    object PasteAction: TAction
      Tag = 15
      Category = 'Command'
      Caption = '&Paste'
      HelpKeyword = 'task_upload#using_copy_amp:paste'
      Hint = 'Paste files from clipboard to current directory in active panel'
      ImageIndex = 75
      ShortCut = 16470
    end
    object NewFileAction: TAction
      Tag = 15
      Category = 'Command'
      Caption = '&File...'
      HelpKeyword = 'task_edit'
      Hint = 'Create file|Create new file and open it in editor'
      ImageIndex = 77
    end
    object EditorListCustomizeAction: TAction
      Tag = 15
      Category = 'Command'
      Caption = '&Configure...'
      HelpKeyword = 'ui_pref_editor'
      Hint = 'Customize editors'
      ImageIndex = 28
    end
    object RemoteCopyToFocusedAction: TAction
      Tag = 14
      Category = 'Remote Focused Operation'
      Caption = '&Duplicate...'
      HelpKeyword = 'task_move_duplicate#duplicating_remote_files'
      Hint = 
        'Duplicate|Duplicate selected remote file(s) to another remote di' +
        'rectory or another name'
      ImageIndex = 78
    end
    object RemoteCopyToAction: TAction
      Tag = 14
      Category = 'Remote Selected Operation'
      Caption = 'Du&plicate...'
      HelpKeyword = 'task_move_duplicate#duplicating_remote_files'
      Hint = 
        'Duplicate|Duplicate selected remote file(s) to another remote di' +
        'rectory or another name'
      ImageIndex = 78
    end
    object UrlToClipboardAction: TAction
      Tag = 15
      Category = 'Selected Operation'
      Caption = 'Copy &URL to Clipboard'
      HelpKeyword = 'filenames#file_url'
      Hint = 'Copy URL'#39's of selected file(s) to clipboard'
    end
    object TableOfContentsAction: TAction
      Tag = 12
      Category = 'Help'
      Caption = '&Contents'
      Hint = 
        'Opens web browser and points it to documentation table of conten' +
        'ts'
      ImageIndex = 79
      ShortCut = 112
    end
    object FileListFromClipboardAction: TAction
      Tag = 15
      Category = 'Selected Operation'
      Caption = '&Transfer Files in Clipboard'
      Hint = 'Transfer files whose names are in clipboard'
    end
    object LocalCopyAction: TAction
      Tag = 9
      Category = 'Local Selected Operation'
      Caption = 'Up&load...'
      HelpKeyword = 'task_upload'
      Hint = 'Upload|Upload selected local file(s) to remote directory'
      ImageIndex = 88
    end
    object CurrentDeleteAlternativeAction: TAction
      Tag = 15
      Category = 'Selected Operation'
      Caption = '&Delete'
      HelpKeyword = 'task_delete'
      Hint = 'Delete|Delete selected file(s)'
      ImageIndex = 2
    end
    object CurrentEditAlternativeAction: TAction
      Tag = 15
      Category = 'Selected Operation'
      Caption = 'Ed&it (alternative)'
      HelpKeyword = 'task_edit'
      Hint = 
        'Edit (alternative)|Edit selected file(s) using alternative edito' +
        'r'
    end
    object CurrentEditWithAction: TAction
      Tag = 15
      Category = 'Selected Operation'
      Caption = 'Edit &With...'
      HelpKeyword = 'task_edit'
      Hint = 'Edit With|Edit selected file(s) using editor of your choice'
    end
    object DownloadPageAction: TAction
      Tag = 15
      Category = 'Help'
      Caption = '&Download'
      Hint = 'Opens web browser and points it to application download page'
    end
    object UpdatesPreferencesAction: TAction
      Tag = 15
      Category = 'Help'
      Caption = 'Confi&gure...'
      HelpKeyword = 'ui_pref_updates'
      Hint = 'Configure automatic check for application updates'
      ImageIndex = 28
    end
    object ShowUpdatesAction: TAction
      Tag = 15
      Category = 'Help'
      Caption = '&Show Updates'
      HelpKeyword = 'updates'
      Hint = 
        'Shows information about application updates|Shows information ab' +
        'out application updates (queries application homepage if the inf' +
        'ormation is not available at the moment)'
      ImageIndex = 81
    end
    object PresetsPreferencesAction: TAction
      Tag = 15
      Category = 'View'
      Caption = '&Configure...'
      HelpKeyword = 'ui_pref_presets'
      Hint = 'Configure transfers settings presets'
      ImageIndex = 28
    end
    object LockToolbarsAction: TAction
      Tag = 15
      Category = 'View'
      Caption = '&Lock Toolbars'
      HelpKeyword = 'ui_toolbars'
      Hint = 'Prevents moving and (un)docking of all toolbars'
    end
    object SelectiveToolbarTextAction: TAction
      Tag = 15
      Category = 'View'
      Caption = '&Show Selective Text Labels'
      HelpKeyword = 'ui_toolbars'
      Hint = 'Show text labels for selected important commands on toolbars'
    end
    object CustomCommandsBandAction: TAction
      Tag = 15
      Category = 'View'
      Caption = 'Custom Co&mmand Buttons'
      HelpKeyword = 'ui_toolbars'
      Hint = 'Hide/show custom commands toolbar'
    end
    object ColorMenuAction: TAction
      Tag = 15
      Category = 'View'
      Caption = 'C&olor'
      HelpKeyword = 'task_connections#session_color'
      Hint = 'Change color of current session'
    end
    object AutoReadDirectoryAfterOpAction: TAction
      Tag = 15
      Category = 'View'
      Caption = 'Auto&matically Reload Directory'
      Hint = 'Toggle automatic reload of remote directory after operation'
      ShortCut = 49234
    end
    object QueueItemPauseAction: TAction
      Tag = 12
      Category = 'Queue'
      Caption = '&Suspend'
      HelpKeyword = 'ui_queue#managing_the_queue'
      Hint = 'Suspend selected queue item'
      ImageIndex = 83
    end
    object QueueItemResumeAction: TAction
      Tag = 12
      Category = 'Queue'
      Caption = '&Resume'
      HelpKeyword = 'ui_queue#managing_the_queue'
      Hint = 'Resume selected suspended queue item'
      ImageIndex = 70
    end
    object QueuePauseAllAction: TAction
      Tag = 12
      Category = 'Queue'
      Caption = '&Suspend All'
      HelpKeyword = 'ui_queue#managing_the_queue'
      Hint = 'Suspend all running queue items'
      ImageIndex = 84
    end
    object QueueResumeAllAction: TAction
      Tag = 12
      Category = 'Queue'
      Caption = '&Resume All'
      HelpKeyword = 'ui_queue#managing_the_queue'
      Hint = 'Resume all suspended queue items'
      ImageIndex = 85
    end
    object QueueDeleteAllDoneAction: TAction
      Tag = 12
      Category = 'Queue'
      Caption = 'Delete All &Completed'
      HelpKeyword = 'ui_queue#managing_the_queue'
      Hint = 'Remove all completed queue items'
      ImageIndex = 99
    end
    object QueueEnableAction: TAction
      Tag = 15
      Category = 'Queue'
      Caption = '&Process Queue'
      HelpKeyword = 'ui_queue#managing_the_queue'
      Hint = 
        'Enable queue processing|Enable queue processing (pending queue i' +
        'tems will not start, when queue processing is disabled)'
      ImageIndex = 96
      ShortCut = 24657
    end
    object QueueDisconnectOnceEmptyAction: TAction
      Tag = 12
      Category = 'Queue'
      Caption = '&Disconnect'
      HelpKeyword = 'ui_queue'
      Hint = 'Disconnect the session once the queue is empty'
      ImageIndex = 87
    end
    object RestoreSelectionAction: TAction
      Tag = 15
      Category = 'Selection'
      Caption = '&Restore Selection'
      HelpKeyword = 'ui_file_panel#selecting_files'
      Hint = 'Restore previous selection'
      ImageIndex = 86
    end
    object LocalSelectAction: TAction
      Tag = 12
      Category = 'Selection'
      Caption = 'Sele&ct Files...'
      HelpKeyword = 'ui_select'
      Hint = 'Select|Select local files by mask'
      ImageIndex = 19
    end
    object LocalUnselectAction: TAction
      Tag = 12
      Category = 'Selection'
      Caption = '&Unselect Files...'
      HelpKeyword = 'ui_select'
      Hint = 'Unselect|Unselect local files by mask'
      ImageIndex = 20
    end
    object LocalSelectAllAction: TAction
      Tag = 12
      Category = 'Selection'
      Caption = 'Select &All'
      HelpKeyword = 'ui_file_panel#selecting_files'
      Hint = 'Select all local files'
      ImageIndex = 21
    end
    object CurrentEditFocusedAction: TAction
      Tag = 15
      Category = 'Focused Operation'
      Caption = '&Edit'
      HelpKeyword = 'task_edit'
      Hint = 'Edit|Edit selected file(s)'
      ImageIndex = 57
    end
    object NewDirAction: TAction
      Tag = 12
      Category = 'Command'
      Caption = '&Directory...'
      HelpKeyword = 'task_create_directory'
      Hint = 'Create directory|Create new directory'
      ImageIndex = 5
    end
    object QueueShutDownOnceEmptyAction: TAction
      Tag = 12
      Category = 'Queue'
      Caption = '&Shut Down'
      HelpKeyword = 'ui_queue'
      Hint = 'Shut down the computer once the queue is empty'
      ImageIndex = 93
    end
    object QueueIdleOnceEmptyAction: TAction
      Tag = 12
      Category = 'Queue'
      Caption = '&Stay Idle'
      Checked = True
      HelpKeyword = 'ui_queue'
      Hint = 'Stay idle once the queue is empty'
      ImageIndex = 94
    end
    object QueueCycleOnceEmptyAction: TAction
      Tag = 12
      Category = 'Queue'
      Caption = 'Once &Empty'
      HelpKeyword = 'ui_queue'
      Hint = 'Toggle action to perform once the queue list is empty'
      ImageIndex = 94
    end
    object QueueItemSpeedAction: TTBEditAction
      Tag = 12
      Category = 'Queue'
      HelpKeyword = 'ui_queue#managing_the_queue'
      Hint = 'Change speed limit of selected queue item'
      EditCaption = '&Speed'
    end
    object LocalFilterAction: TAction
      Tag = 9
      Category = 'Local Directory'
      Caption = '&Filter...'
      HelpKeyword = 'ui_file_panel#filter'
      Hint = 'Filter|Filter displayed files'
      ImageIndex = 92
    end
    object RemoteFilterAction: TAction
      Tag = 14
      Category = 'Remote Directory'
      Caption = '&Filter...'
      HelpKeyword = 'ui_file_panel#filter'
      Hint = 'Filter|Filter displayed files'
      ImageIndex = 92
    end
    object RemoteFindFilesAction: TAction
      Tag = 15
      Category = 'Remote Directory'
      Caption = '&Find Files...'
      HelpKeyword = 'task_find'
      Hint = 'Find files|Find remote files and directories'
      ImageIndex = 95
    end
    object CurrentEditInternalAction: TAction
      Tag = 15
      Category = 'Selected Operation'
      Caption = '&Internal editor'
      HelpKeyword = 'task_edit'
      Hint = 'Edit (internal)|Edit selected file(s) using internal editor'
    end
    object SaveWorkspaceAction: TAction
      Tag = 15
      Category = 'Session'
      Caption = 'Save Wor&kspace...'
      HelpKeyword = 'workspace'
      Hint = 'Save workspace|Save workspace'
      ImageIndex = 102
    end
    object LocalRenameAction: TAction
      Tag = 12
      Category = 'Local Selected Operation'
      Caption = '&Rename'
      HelpKeyword = 'task_rename'
      Hint = 'Rename|Rename selected local file'
      ImageIndex = 3
    end
    object LocalEditAction: TAction
      Tag = 12
      Category = 'Local Selected Operation'
      Caption = '&Edit'
      HelpKeyword = 'task_edit'
      Hint = 'Edit|Edit selected local file(s)'
      ImageIndex = 57
    end
    object LocalMoveAction: TAction
      Tag = 9
      Category = 'Local Selected Operation'
      Caption = 'Upload and Dele&te...'
      HelpKeyword = 'task_upload'
      Hint = 
        'Upload and Delete|Upload selected local file(s) to remote direct' +
        'ory and delete original'
      ImageIndex = 98
    end
    object LocalCreateDirAction: TAction
      Tag = 12
      Category = 'Local Selected Operation'
      Caption = '&Create Directory...'
      HelpKeyword = 'task_create_directory'
      Hint = 'Create directory|Create new local directory'
      ImageIndex = 5
    end
    object LocalDeleteAction: TAction
      Tag = 12
      Category = 'Local Selected Operation'
      Caption = '&Delete'
      HelpKeyword = 'task_delete'
      Hint = 'Delete|Delete selected local file(s)'
      ImageIndex = 2
    end
    object LocalPropertiesAction: TAction
      Tag = 12
      Category = 'Local Selected Operation'
      Caption = '&Properties'
      HelpKeyword = 'task_properties'
      Hint = 'Properties|Display/change properties of selected local file(s)'
      ImageIndex = 4
    end
    object LocalAddEditLinkAction: TAction
      Tag = 12
      Category = 'Local Selected Operation'
      Caption = 'Edit &Link...'
      HelpKeyword = 'task_link'
      Hint = 
        'Add/edit shortcut|Add new local shortcut or edit selected local ' +
        'shortcut'
      ImageIndex = 60
    end
    object RemoteRenameAction: TAction
      Tag = 12
      Category = 'Remote Selected Operation'
      Caption = '&Rename'
      HelpKeyword = 'task_rename'
      Hint = 'Rename|Rename selected remote file'
      ImageIndex = 3
    end
    object RemoteEditAction: TAction
      Tag = 12
      Category = 'Remote Selected Operation'
      Caption = '&Edit'
      HelpKeyword = 'task_edit'
      Hint = 'Edit|Edit selected remote file(s)'
      ImageIndex = 57
    end
    object RemoteMoveAction: TAction
      Tag = 14
      Category = 'Remote Selected Operation'
      Caption = 'Download and Dele&te...'
      HelpKeyword = 'task_download'
      Hint = 
        'Download and Delete|Download selected remote file(s) to local di' +
        'rectory and delete original'
      ImageIndex = 97
    end
    object RemoteCreateDirAction: TAction
      Tag = 12
      Category = 'Remote Selected Operation'
      Caption = '&Create Directory...'
      HelpKeyword = 'task_create_directory'
      Hint = 'Create directory|Create new remote directory'
      ImageIndex = 5
    end
    object RemoteDeleteAction: TAction
      Tag = 12
      Category = 'Remote Selected Operation'
      Caption = '&Delete'
      HelpKeyword = 'task_delete'
      Hint = 'Delete|Delete selected remote file(s)'
      ImageIndex = 2
    end
    object RemotePropertiesAction: TAction
      Tag = 12
      Category = 'Remote Selected Operation'
      Caption = '&Properties'
      HelpKeyword = 'task_properties'
      Hint = 
        'Properties|Display/change permissions, ownership or other proper' +
        'ties of selected remote file(s) '
      ImageIndex = 4
    end
    object RemoteAddEditLinkAction: TAction
      Tag = 12
      Category = 'Remote Selected Operation'
      Caption = 'Edit &Link...'
      HelpKeyword = 'task_link'
      Hint = 'Add/edit link|Add new remote link or edit selected remote link'
      ImageIndex = 60
    end
    object RemoteSelectAction: TAction
      Tag = 12
      Category = 'Selection'
      Caption = 'Sele&ct Files...'
      HelpKeyword = 'ui_select'
      Hint = 'Select|Select remote files by mask'
      ImageIndex = 19
    end
    object RemoteUnselectAction: TAction
      Tag = 12
      Category = 'Selection'
      Caption = '&Unselect Files...'
      HelpKeyword = 'ui_select'
      Hint = 'Unselect|Unselect remote files by mask'
      ImageIndex = 20
    end
    object RemoteSelectAllAction: TAction
      Tag = 12
      Category = 'Selection'
      Caption = 'Select &All'
      HelpKeyword = 'ui_file_panel#selecting_files'
      Hint = 'Select all remote files'
      ImageIndex = 21
    end
    object LocalMoveFocusedAction: TAction
      Tag = 8
      Category = 'Local Focused Operation'
      Caption = 'Upload and Dele&te...'
      HelpKeyword = 'task_upload'
      Hint = 
        'Upload and Delete|Upload selected local file(s) to remote direct' +
        'ory and delete original'
      ImageIndex = 98
    end
    object CurrentSystemMenuFocusedAction: TAction
      Tag = 12
      Category = 'Focused Operation'
      Caption = '&System Menu'
      Hint = 
        'Display system file context menu (in Properties you can opt to d' +
        'isplay it by default instead of built-in menu)'
    end
  end
  object ExplorerBarPopup: TTBXPopupMenu
    Images = GlyphsModule.ExplorerImages
    Left = 192
    Top = 336
    object Address2: TTBXItem
      Action = ExplorerAddressBandAction
    end
    object StandardButtons1: TTBXItem
      Action = ExplorerToolbarBandAction
    end
    object SelectionButtons1: TTBXItem
      Action = ExplorerSelectionBandAction
    end
    object SessionButtons2: TTBXItem
      Action = ExplorerSessionBandAction
    end
    object PreferencesButtons1: TTBXItem
      Action = ExplorerPreferencesBandAction
    end
    object SortButtons3: TTBXItem
      Action = ExplorerSortBandAction
    end
    object TBXItem3: TTBXItem
      Action = ExplorerUpdatesBandAction
    end
    object TBXItem4: TTBXItem
      Action = ExplorerTransferBandAction
    end
    object TBXItem16: TTBXItem
      Action = ExplorerCustomCommandsBandAction
    end
    object TBXItem7: TTBXItem
      Action = LockToolbarsAction
    end
    object TBXItem48: TTBXItem
      Action = SelectiveToolbarTextAction
    end
    object N5: TTBXSeparatorItem
    end
    object SessionsTabsAction2: TTBXItem
      Action = SessionsTabsAction
    end
    object StatusBar2: TTBXItem
      Action = StatusBarAction
    end
    object N72: TTBXSeparatorItem
    end
    object Queue7: TTBXSubmenuItem
      Caption = '&Queue'
      HelpKeyword = 'ui_queue'
      Hint = 'Configure queue list'
      object Show6: TTBXItem
        Action = QueueShowAction
        RadioItem = True
      end
      object HidewhenEmpty6: TTBXItem
        Action = QueueHideWhenEmptyAction
        RadioItem = True
      end
      object Hide5: TTBXItem
        Action = QueueHideAction
        RadioItem = True
      end
      object N71: TTBXSeparatorItem
      end
      object Toolbar5: TTBXItem
        Action = QueueToolbarAction
      end
      object N70: TTBXSeparatorItem
      end
      object Customize5: TTBXItem
        Action = QueuePreferencesAction
      end
    end
    object Tree4: TTBXItem
      Action = RemoteTreeAction
    end
  end
  object SessionIdleTimer: TTimer
    Enabled = False
    Interval = 500
    OnTimer = SessionIdleTimerTimer
    Left = 32
    Top = 336
  end
  object CommanderBarPopup: TTBXPopupMenu
    Images = GlyphsModule.ExplorerImages
    Left = 424
    Top = 264
    object CommandsButtons2: TTBXItem
      Action = CommanderCommandsBandAction
    end
    object SessionButtons5: TTBXItem
      Action = CommanderSessionBandAction
    end
    object PreferencesButtons4: TTBXItem
      Action = CommanderPreferencesBandAction
    end
    object SortButtons2: TTBXItem
      Action = CommanderSortBandAction
    end
    object TBXItem2: TTBXItem
      Action = CommanderUpdatesBandAction
    end
    object TBXItem5: TTBXItem
      Action = CommanderTransferBandAction
    end
    object TBXItem15: TTBXItem
      Action = CommanderCustomCommandsBandAction
    end
    object TBXItem6: TTBXItem
      Action = LockToolbarsAction
    end
    object TBXItem46: TTBXItem
      Action = SelectiveToolbarTextAction
    end
    object N26: TTBXSeparatorItem
    end
    object SessionsTabsAction1: TTBXItem
      Action = SessionsTabsAction
    end
    object CommandLine2: TTBXItem
      Action = CommandLinePanelAction
    end
    object CommandsToolbar1: TTBXItem
      Action = ToolBar2Action
    end
    object StatusBar8: TTBXItem
      Action = StatusBarAction
    end
    object N27: TTBXSeparatorItem
    end
    object LocalPanel1: TTBXSubmenuItem
      Caption = '&Local Panel'
      HelpKeyword = 'ui_file_panel'
      Hint = 'Change local panel layout'
      object HistoryButtons3: TTBXItem
        Action = CommanderLocalHistoryBandAction
      end
      object NavigationButtons3: TTBXItem
        Action = CommanderLocalNavigationBandAction
      end
      object TBXItem40: TTBXItem
        Action = CommanderLocalFileBandAction
      end
      object TBXItem43: TTBXItem
        Action = CommanderLocalSelectionBandAction
      end
      object N23: TTBXSeparatorItem
      end
      object Tree7: TTBXItem
        Action = LocalTreeAction
      end
      object N77: TTBXSeparatorItem
      end
      object StatusBar6: TTBXItem
        Action = LocalStatusBarAction
      end
    end
    object RemotePanel2: TTBXSubmenuItem
      Caption = '&Remote Panel'
      HelpKeyword = 'ui_file_panel'
      Hint = 'Change remote panel layout'
      object HistoryButtons4: TTBXItem
        Action = CommanderRemoteHistoryBandAction
      end
      object NavigationButtons4: TTBXItem
        Action = CommanderRemoteNavigationBandAction
      end
      object TBXItem41: TTBXItem
        Action = CommanderRemoteFileBandAction
      end
      object TBXItem42: TTBXItem
        Action = CommanderRemoteSelectionBandAction
      end
      object N25: TTBXSeparatorItem
      end
      object Tree8: TTBXItem
        Action = RemoteTreeAction
      end
      object N78: TTBXSeparatorItem
      end
      object StatusBar7: TTBXItem
        Action = RemoteStatusBarAction
      end
    end
    object Options1: TTBXSubmenuItem
      Caption = '&Queue'
      HelpKeyword = 'ui_queue'
      Hint = 'Configure queue list'
      object Show5: TTBXItem
        Action = QueueShowAction
        RadioItem = True
      end
      object HidewhenEmpty5: TTBXItem
        Action = QueueHideWhenEmptyAction
        RadioItem = True
      end
      object Hide4: TTBXItem
        Action = QueueHideAction
        RadioItem = True
      end
      object N69: TTBXSeparatorItem
      end
      object Toolbar4: TTBXItem
        Action = QueueToolbarAction
      end
      object N68: TTBXSeparatorItem
      end
      object Customize4: TTBXItem
        Action = QueuePreferencesAction
      end
    end
  end
  object RemotePanelPopup: TTBXPopupMenu
    Images = GlyphsModule.ExplorerImages
    Left = 312
    Top = 264
    object TBXItem32: TTBXItem
      Action = RemoteRefreshAction
    end
    object TBXItem30: TTBXItem
      Action = RemoteAddBookmarkAction
    end
    object CopyPathtoClipboard1: TTBXItem
      Action = RemotePathToClipboardAction
    end
    object OpenDirectoryBookmark1: TTBXItem
      Action = RemoteOpenDirAction
    end
    object TBXItem26: TTBXItem
      Action = RemoteFilterAction
    end
    object N51: TTBXSeparatorItem
    end
    object HistoryButtons5: TTBXItem
      Action = CommanderRemoteHistoryBandAction
    end
    object NavigationButtons5: TTBXItem
      Action = CommanderRemoteNavigationBandAction
    end
    object TBXItem14: TTBXItem
      Action = CommanderRemoteFileBandAction
    end
    object TBXItem45: TTBXItem
      Action = CommanderRemoteSelectionBandAction
    end
    object TBXItem37: TTBXItem
      Action = LockToolbarsAction
    end
    object TBXItem49: TTBXItem
      Action = SelectiveToolbarTextAction
    end
    object N28: TTBXSeparatorItem
    end
    object Tree5: TTBXItem
      Action = RemoteTreeAction
    end
    object N75: TTBXSeparatorItem
    end
    object StatusBar9: TTBXItem
      Action = RemoteStatusBarAction
    end
  end
  object LocalPanelPopup: TTBXPopupMenu
    Images = GlyphsModule.ExplorerImages
    Left = 312
    Top = 336
    object TBXItem34: TTBXItem
      Action = LocalRefreshAction
    end
    object TBXItem31: TTBXItem
      Action = LocalAddBookmarkAction
    end
    object CopyPathtoClipboard2: TTBXItem
      Action = LocalPathToClipboardAction
    end
    object OpenDirectoryBookmark2: TTBXItem
      Action = LocalOpenDirAction
    end
    object TBXItem27: TTBXItem
      Action = LocalFilterAction
    end
    object N52: TTBXSeparatorItem
    end
    object HistoryButtons6: TTBXItem
      Action = CommanderLocalHistoryBandAction
    end
    object NavigationButtons6: TTBXItem
      Action = CommanderLocalNavigationBandAction
    end
    object TBXItem39: TTBXItem
      Action = CommanderLocalFileBandAction
    end
    object TBXItem44: TTBXItem
      Action = CommanderLocalSelectionBandAction
    end
    object TBXItem38: TTBXItem
      Action = LockToolbarsAction
    end
    object TBXItem47: TTBXItem
      Action = SelectiveToolbarTextAction
    end
    object N29: TTBXSeparatorItem
    end
    object Tree6: TTBXItem
      Action = LocalTreeAction
    end
    object N76: TTBXSeparatorItem
    end
    object StatusBar10: TTBXItem
      Action = LocalStatusBarAction
    end
  end
  object LocalDirViewColumnPopup: TTBXPopupMenu
    Images = GlyphsModule.ExplorerImages
    Left = 248
    Top = 88
    object SortAscending1: TTBXItem
      Action = SortColumnAscendingAction
    end
    object SortDescending1: TTBXItem
      Action = SortColumnDescendingAction
    end
    object LocalSortByExtColumnPopupItem: TTBXItem
      Action = LocalSortByExtAction
    end
    object LocalFormatSizeBytesPopupItem: TTBXItem
      Action = FormatSizeBytesAction
    end
    object Hidecolumn1: TTBXItem
      Action = HideColumnAction
    end
    object N37: TTBXSeparatorItem
    end
    object Showcolumns3: TTBXSubmenuItem
      Caption = 'Show &Columns'
      HelpKeyword = 'ui_file_panel#selecting_columns'
      Hint = 'Select columns to show in panel'
      object Name3: TTBXItem
        Action = ShowHideLocalNameColumnAction
      end
      object Size3: TTBXItem
        Action = ShowHideLocalSizeColumnAction
      end
      object Type2: TTBXItem
        Action = ShowHideLocalTypeColumnAction
      end
      object Modification3: TTBXItem
        Action = ShowHideLocalChangedColumnAction
      end
      object Attributes3: TTBXItem
        Action = ShowHideLocalAttrColumnAction
      end
    end
  end
  object RemoteDirViewColumnPopup: TTBXPopupMenu
    Images = GlyphsModule.ExplorerImages
    Left = 424
    Top = 88
    object MenuItem1: TTBXItem
      Action = SortColumnAscendingAction
      RadioItem = True
    end
    object MenuItem2: TTBXItem
      Action = SortColumnDescendingAction
      RadioItem = True
    end
    object RemoteSortByExtColumnPopupItem: TTBXItem
      Action = RemoteSortByExtAction
    end
    object RemoteFormatSizeBytesPopupItem: TTBXItem
      Action = FormatSizeBytesAction
    end
    object Hidecolumn2: TTBXItem
      Action = HideColumnAction
    end
    object N38: TTBXSeparatorItem
    end
    object Showcolumns4: TTBXSubmenuItem
      Caption = 'Show &Columns'
      HelpKeyword = 'ui_file_panel#selecting_columns'
      Hint = 'Select columns to show in panel'
      object Name4: TTBXItem
        Action = ShowHideRemoteNameColumnAction
      end
      object Size4: TTBXItem
        Action = ShowHideRemoteSizeColumnAction
      end
      object TBXItem8: TTBXItem
        Action = ShowHideRemoteTypeColumnAction
      end
      object Modification4: TTBXItem
        Action = ShowHideRemoteChangedColumnAction
      end
      object Permissions1: TTBXItem
        Action = ShowHideRemoteRightsColumnAction
      end
      object Owner2: TTBXItem
        Action = ShowHideRemoteOwnerColumnAction
      end
      object Group2: TTBXItem
        Action = ShowHideRemoteGroupColumnAction
      end
      object TBXItem1: TTBXItem
        Action = ShowHideRemoteLinkTargetColumnAction
      end
    end
  end
  object QueuePopup: TTBXPopupMenu
    Images = GlyphsModule.ExplorerImages
    OnPopup = QueuePopupPopup
    Left = 392
    Top = 176
    object ShowQuery1: TTBXItem
      Action = QueueItemQueryAction
    end
    object ShowError1: TTBXItem
      Action = QueueItemErrorAction
    end
    object ShowPrompt1: TTBXItem
      Action = QueueItemPromptAction
    end
    object N53: TTBXSeparatorItem
    end
    object ExecuteNow1: TTBXItem
      Action = QueueItemExecuteAction
    end
    object TBXItem9: TTBXItem
      Action = QueueItemPauseAction
    end
    object TBXItem10: TTBXItem
      Action = QueueItemResumeAction
    end
    object Delete4: TTBXItem
      Action = QueueItemDeleteAction
    end
    object QueuePopupSpeedComboBoxItem: TTBXComboBoxItem
      Action = QueueItemSpeedAction
    end
    object N54: TTBXSeparatorItem
    end
    object MoveUp1: TTBXItem
      Action = QueueItemUpAction
    end
    object MoveDown1: TTBXItem
      Action = QueueItemDownAction
    end
    object N67: TTBXSeparatorItem
    end
    object QueueEnableItem: TTBXItem
      Action = QueueEnableAction
    end
    object TBXSubmenuItem1: TTBXSubmenuItem
      Caption = '&All'
      HelpKeyword = 'ui_queue#managing_the_queue'
      Hint = 'Mass queue management commands'
      object TBXItem11: TTBXItem
        Action = QueuePauseAllAction
      end
      object TBXItem12: TTBXItem
        Action = QueueResumeAllAction
      end
      object TBXSeparatorItem5: TTBXSeparatorItem
      end
      object TBXItem51: TTBXItem
        Action = QueueDeleteAllDoneAction
      end
    end
    object TBXSubmenuItem3: TTBXSubmenuItem
      Action = QueueCycleOnceEmptyAction
      DropdownCombo = True
      object TBXItem28: TTBXItem
        Action = QueueIdleOnceEmptyAction
        RadioItem = True
      end
      object TBXItem13: TTBXItem
        Action = QueueDisconnectOnceEmptyAction
        RadioItem = True
      end
      object TBXItem29: TTBXItem
        Action = QueueShutDownOnceEmptyAction
        RadioItem = True
      end
    end
    object Queue2: TTBXSubmenuItem
      Caption = '&Options'
      HelpKeyword = 'ui_queue'
      Hint = 'Configure queue list'
      object Show4: TTBXItem
        Action = QueueShowAction
        RadioItem = True
      end
      object HidewhenEmpty4: TTBXItem
        Action = QueueHideWhenEmptyAction
        RadioItem = True
      end
      object Hide3: TTBXItem
        Action = QueueHideAction
        RadioItem = True
      end
      object N66: TTBXSeparatorItem
      end
      object Toolbar3: TTBXItem
        Action = QueueToolbarAction
      end
      object N65: TTBXSeparatorItem
      end
      object Customize3: TTBXItem
        Action = QueuePreferencesAction
      end
    end
  end
  object RemoteDirViewPopup: TTBXPopupMenu
    AutoPopup = False
    Images = GlyphsModule.ExplorerImages
    Left = 360
    Top = 400
    object GoTo4: TTBXSubmenuItem
      Caption = '&Go To'
      HelpKeyword = 'task_navigate'
      Hint = 'Go to directory'
      object OpenDirectoryBookmark3: TTBXItem
        Action = RemoteOpenDirAction
      end
      object N81: TTBXSeparatorItem
      end
      object ParentDirectory4: TTBXItem
        Action = RemoteParentDirAction
      end
      object RootDirectory4: TTBXItem
        Action = RemoteRootDirAction
      end
      object HomeDirectory4: TTBXItem
        Action = RemoteHomeDirAction
      end
      object N80: TTBXSeparatorItem
      end
      object Back4: TTBXItem
        Action = RemoteBackAction
      end
      object Forward4: TTBXItem
        Action = RemoteForwardAction
      end
    end
    object Refresh4: TTBXItem
      Action = RemoteRefreshAction
    end
    object AddToBookmarks4: TTBXItem
      Action = RemoteAddBookmarkAction
    end
    object TBXItem35: TTBXItem
      Action = RemoteFilterAction
    end
    object CopyPathtoClipboard6: TTBXItem
      Action = RemotePathToClipboardAction
    end
    object N79: TTBXSeparatorItem
    end
    object TBXSubmenuItem26: TTBXSubmenuItem
      Caption = '&New'
      HelpKeyword = 'task_index'
      Hint = 'Create object|Create new object'
      object TBXItem135: TTBXItem
        Action = NewFileAction
      end
      object TBXItem136: TTBXItem
        Action = NewDirAction
      end
      object TBXItem209: TTBXItem
        Action = NewLinkAction
      end
    end
  end
  object LocalDirViewPopup: TTBXPopupMenu
    AutoPopup = False
    Images = GlyphsModule.ExplorerImages
    Left = 480
    Top = 400
    object GoTo5: TTBXSubmenuItem
      Caption = '&Go To'
      HelpKeyword = 'task_navigate'
      Hint = 'Go to directory'
      object OpenDirectoryBookmark4: TTBXItem
        Action = LocalOpenDirAction
      end
      object ExploreDirectory2: TTBXItem
        Action = LocalExploreDirectoryAction
      end
      object N84: TTBXSeparatorItem
      end
      object ParentDirectory5: TTBXItem
        Action = LocalParentDirAction
      end
      object RootDirectory5: TTBXItem
        Action = LocalRootDirAction
      end
      object HomeDirectory5: TTBXItem
        Action = LocalHomeDirAction
      end
      object N83: TTBXSeparatorItem
      end
      object Back5: TTBXItem
        Action = LocalBackAction
      end
      object Forward5: TTBXItem
        Action = LocalForwardAction
      end
    end
    object Refresh5: TTBXItem
      Action = LocalRefreshAction
    end
    object TBXItem36: TTBXItem
      Action = LocalFilterAction
    end
    object AddToBookmarks5: TTBXItem
      Action = LocalAddBookmarkAction
    end
    object CopyPathtoClipboard7: TTBXItem
      Action = LocalPathToClipboardAction
    end
    object N82: TTBXSeparatorItem
    end
    object CreateDirectory4: TTBXItem
      Action = CurrentCreateDirAction
    end
  end
  object RemoteAddressPopup: TTBXPopupMenu
    Images = GlyphsModule.ExplorerImages
    Left = 248
    Top = 400
    object TBXItem33: TTBXItem
      Action = RemoteRefreshAction
    end
    object TBXItem24: TTBXItem
      Action = RemoteAddBookmarkAction
    end
    object TBXItem25: TTBXItem
      Action = RemotePathToClipboardAction
    end
    object TBXSeparatorItem1: TTBXSeparatorItem
    end
    object TBXItem17: TTBXItem
      Action = RemoteOpenDirAction
    end
    object TBXSubmenuItem2: TTBXSubmenuItem
      Caption = '&Go To'
      HelpKeyword = 'task_navigate'
      Hint = 'Go to directory'
      object TBXItem18: TTBXItem
        Action = RemoteParentDirAction
      end
      object TBXItem19: TTBXItem
        Action = RemoteRootDirAction
      end
      object TBXItem20: TTBXItem
        Action = RemoteHomeDirAction
      end
      object TBXSeparatorItem2: TTBXSeparatorItem
      end
      object TBXItem21: TTBXItem
        Action = RemoteBackAction
      end
      object TBXItem22: TTBXItem
        Action = RemoteForwardAction
      end
    end
  end
  object SessionsPopup: TTBXPopupMenu
    Images = GlyphsModule.ExplorerImages
    Left = 480
    Top = 176
    object TBXItem124: TTBXItem
      Action = CloseSessionAction
    end
    object TBXItem219: TTBXItem
      Action = DuplicateSessionAction
    end
    object TBXItem125: TTBXItem
      Action = SaveCurrentSessionAction2
    end
    object TBXSeparatorItem34: TTBXSeparatorItem
    end
    object TBXItem123: TTBXItem
      Action = NewSessionAction
    end
    object TBXSubmenuItem23: TTBXSubmenuItem
      Action = SavedSessionsAction2
      Options = [tboDropdownArrow]
    end
    object TBXSeparatorItem52: TTBXSeparatorItem
    end
    object ColorMenuItem: TTBXColorItem
      Action = ColorMenuAction
      Color = clNone
    end
    object TBXSeparatorItem35: TTBXSeparatorItem
    end
    object SessionsTabsAction4: TTBXItem
      Action = SessionsTabsAction
    end
  end
  object ShellResources: TShellResources
    Left = 48
    Top = 424
  end
  object LocalFilePopup: TTBXPopupMenu
    Images = GlyphsModule.ExplorerImages
    Left = 536
    Top = 336
    object LocalOpenMenuItem: TTBXItem
      Action = CurrentOpenAction
    end
    object LocalEditMenuItem: TTBXItem
      Action = CurrentEditFocusedAction
    end
    object LocalCopyMenuItem: TTBXItem
      Action = LocalCopyFocusedAction
    end
    object TBXItem54: TTBXItem
      Action = LocalMoveFocusedAction
    end
    object TBXItem57: TTBXItem
      Action = CurrentDeleteFocusedAction
    end
    object TBXItem58: TTBXItem
      Action = CurrentRenameAction
    end
    object TBXSeparatorItem3: TTBXSeparatorItem
    end
    object TBXSubmenuItem4: TTBXSubmenuItem
      Action = CustomCommandsAction
      object TTBXItem
      end
    end
    object TBXSubmenuItem5: TTBXSubmenuItem
      Caption = '&File Names'
      HelpKeyword = 'filenames'
      Hint = 'Operations with name(s) of selected file(s)'
      object TBXItem59: TTBXItem
        Action = FileListToCommandLineAction
      end
      object TBXItem60: TTBXItem
        Action = FileListToClipboardAction
      end
      object TBXItem61: TTBXItem
        Action = FullFileListToClipboardAction
      end
      object TBXItem62: TTBXItem
        Action = UrlToClipboardAction
      end
    end
    object TBXSeparatorItem4: TTBXSeparatorItem
    end
    object TBXItem63: TTBXItem
      Action = CurrentPropertiesFocusedAction
    end
    object TBXItem50: TTBXItem
      Action = CurrentSystemMenuFocusedAction
    end
  end
end
