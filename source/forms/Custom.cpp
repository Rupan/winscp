//---------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include <Dialogs.hpp>
//---------------------------------------------------------------------
#include <Common.h>
#include <CustomWinConfiguration.h>
#include <WinInterface.h>
#include <VCLCommon.h>
#include <TextsWin.h>
#include <HelpWin.h>
#include <CoreMain.h>
#include <PasTools.hpp>
#include <ProgParams.h>
#include <Tools.h>
#include <HistoryComboBox.hpp>
#include <Math.hpp>

#include "Custom.h"
//---------------------------------------------------------------------
#pragma link "PasswordEdit"
#ifndef NO_RESOURCES
#pragma resource "*.dfm"
#endif
//---------------------------------------------------------------------
__fastcall TCustomDialog::TCustomDialog(UnicodeString AHelpKeyword)
  : TForm(GetFormOwner())
{
  UseSystemSettings(this);

  FPos = ScaleByTextHeight(this, 8);
  FHorizontalMargin = ScaleByTextHeight(this, 8);
  FIndent = FHorizontalMargin;

  HelpKeyword = AHelpKeyword;

  TBorderIcons BI = BorderIcons;
  if (HelpKeyword.IsEmpty())
  {
    BI >> biHelp;

    OKButton->Left = CancelButton->Left;
    CancelButton->Left = HelpButton->Left;
    HelpButton->Visible = false;
  }
  else
  {
    BI << biHelp;
  }
  BorderIcons = BI;
}
//---------------------------------------------------------------------
bool __fastcall TCustomDialog::Execute()
{
  Changed();
  return (ShowModal() == DefaultResult(this));
}
//---------------------------------------------------------------------
void __fastcall TCustomDialog::DoChange(bool & /*CanSubmit*/)
{
  // noop
}
//---------------------------------------------------------------------
void __fastcall TCustomDialog::Changed()
{
  bool CanSubmit = true;
  DoChange(CanSubmit);
  EnableControl(OKButton, CanSubmit);
}
//---------------------------------------------------------------------
void __fastcall TCustomDialog::Change(TObject * /*Sender*/)
{
  Changed();
}
//---------------------------------------------------------------------------
void __fastcall TCustomDialog::HelpButtonClick(TObject * /*Sender*/)
{
  FormHelp(this);
}
//---------------------------------------------------------------------------
void __fastcall TCustomDialog::DoShow()
{
  OKButton->TabOrder = FCount;
  CancelButton->TabOrder = static_cast<short>(FCount + 1);
  HelpButton->TabOrder = static_cast<short>(FCount + 2);
  Changed();
  TForm::DoShow();
}
//---------------------------------------------------------------------------
void __fastcall TCustomDialog::DoValidate()
{
  // noop
}
//---------------------------------------------------------------------------
bool __fastcall TCustomDialog::CloseQuery()
{
  if (ModalResult == DefaultResult(this))
  {
    DoValidate();
  }
  return TForm::CloseQuery();
}
//---------------------------------------------------------------------------
void __fastcall TCustomDialog::AddImage(const UnicodeString & ImageName)
{
  TImage * Image = new TImage(this);
  Image->Name = L"Image";
  Image->Parent = this;
  LoadResourceImage(Image, ImageName);
  Image->SetBounds(FIndent, FPos + ScaleByTextHeight(this, 3), 32, 32);
  FIndent += Image->Width + ScaleByTextHeight(this, 12);
}
//---------------------------------------------------------------------------
void __fastcall TCustomDialog::AddWinControl(TWinControl * Control)
{
  Control->TabOrder = FCount;
  FCount++;
}
//---------------------------------------------------------------------------
TLabel * __fastcall TCustomDialog::CreateLabel(UnicodeString Label)
{
  TLabel * Result = new TLabel(this);
  Result->Caption = Label;
  return Result;
}
//---------------------------------------------------------------------------
void __fastcall TCustomDialog::AddEditLikeControl(TWinControl * Edit, TLabel * Label)
{
  int PrePos = FPos;
  Label->Parent = this;
  Label->Left = FIndent;
  Label->Top = FPos;
  FPos += Label->Height + ScaleByTextHeight(this, 4);

  Edit->Parent = this;
  Edit->Left = FIndent;
  Edit->Top = FPos;
  Edit->Width = ClientWidth - Edit->Left - FHorizontalMargin;
  // this updates Height property to real value
  Edit->HandleNeeded();
  FPos += Edit->Height + ScaleByTextHeight(this, 8);

  if (Label->FocusControl == NULL)
  {
    Label->FocusControl = Edit;
  }
  else
  {
    assert(Label->FocusControl == Edit);
  }

  ClientHeight = ClientHeight + (FPos - PrePos);

  AddWinControl(Edit);
}
//---------------------------------------------------------------------------
void __fastcall TCustomDialog::AddEdit(TCustomEdit * Edit, TLabel * Label)
{
  AddEditLikeControl(Edit, Label);

  TEdit * PublicEdit = reinterpret_cast<TEdit *>(Edit);
  if (PublicEdit->OnChange == NULL)
  {
    PublicEdit->OnChange = Change;
  }
}
//---------------------------------------------------------------------------
void __fastcall TCustomDialog::AddComboBox(TCustomCombo * Combo, TLabel * Label)
{
  AddEditLikeControl(Combo, Label);

  TComboBox * PublicCombo = reinterpret_cast<TComboBox *>(Combo);
  if (PublicCombo->OnChange == NULL)
  {
    PublicCombo->OnChange = Change;
  }
}
//---------------------------------------------------------------------------
void __fastcall TCustomDialog::AddButtonControl(TButtonControl * Control)
{
  int PrePos = FPos;
  Control->Parent = this;
  Control->Left = FIndent + ScaleByTextHeight(this, 6);
  Control->Top = FPos;
  Control->Width = ClientWidth - Control->Left - FHorizontalMargin;
  // this updates Height property to real value
  Control->HandleNeeded();
  // buttons do not scale with text on their own
  Control->Height = ScaleByTextHeight(Control, Control->Height);
  FPos += Control->Height + ScaleByTextHeight(this, 8);

  ClientHeight = ClientHeight + (FPos - PrePos);

  AddWinControl(Control);

  TCheckBox * PublicControl = reinterpret_cast<TCheckBox *>(Control);
  if (PublicControl->OnClick == NULL)
  {
    PublicControl->OnClick = Change;
  }
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
class TSaveSessionDialog : public TCustomDialog
{
public:
  __fastcall TSaveSessionDialog(TComponent* AOwner);
  void __fastcall Init(bool CanSavePassword, bool NotRecommendedSavingPassword,
    TStrings * AdditionalFolders);

  bool __fastcall Execute(UnicodeString & SessionName, bool & SavePassword,
    bool & CreateShortcut, const UnicodeString & OriginalSessionName);

protected:
  virtual void __fastcall DoValidate();
  virtual void __fastcall DoChange(bool & CanSubmit);

private:
  UnicodeString FOriginalSessionName;
  TEdit * SessionNameEdit;
  TComboBox * FolderCombo;
  TCheckBox * SavePasswordCheck;
  TCheckBox * CreateShortcutCheck;
  UnicodeString FRootFolder;

  UnicodeString __fastcall GetSessionName();
};
//---------------------------------------------------------------------------
// Need to have an Owner argument for SafeFormCreate
__fastcall TSaveSessionDialog::TSaveSessionDialog(TComponent* /*AOwner*/) :
  TCustomDialog(HELP_SESSION_SAVE)
{
}
//---------------------------------------------------------------------------
void __fastcall TSaveSessionDialog::Init(bool CanSavePassword,
  bool NotRecommendedSavingPassword, TStrings * AdditionalFolders)
{
  Caption = LoadStr(SAVE_SESSION_CAPTION);

  SessionNameEdit = new TEdit(this);
  AddEdit(SessionNameEdit, CreateLabel(LoadStr(SAVE_SESSION_PROMPT)));

  FRootFolder = LoadStr(SAVE_SESSION_ROOT_FOLDER);
  std::auto_ptr<TStringList> Folders(new TStringList());

  if (AdditionalFolders != NULL)
  {
    Folders->AddStrings(AdditionalFolders);
  }

  for (int Index = 0; Index < StoredSessions->Count; Index++)
  {
    TSessionData * Data = StoredSessions->Sessions[Index];
    if (!Data->Special && !Data->IsWorkspace)
    {
      UnicodeString Folder = Data->FolderName;
      if (!Folder.IsEmpty() && Folders->IndexOf(Folder) < 0)
      {
        Folders->Add(Folder);
      }
    }
  }

  assert(!Folders->CaseSensitive);
  Folders->Sort();

  FolderCombo = new TComboBox(this);
  AddComboBox(FolderCombo, CreateLabel(LoadStr(SAVE_SESSION_FOLDER)));
  FolderCombo->DropDownCount = Max(FolderCombo->DropDownCount, 16);
  FolderCombo->Items->Add(FRootFolder);
  FolderCombo->Items->AddStrings(Folders.get());

  SavePasswordCheck = new TCheckBox(this);
  SavePasswordCheck->Caption = LoadStr(
    NotRecommendedSavingPassword ? SAVE_SESSION_PASSWORD :
      (CustomWinConfiguration->UseMasterPassword ? SAVE_SESSION_PASSWORD_MASTER : SAVE_SESSION_PASSWORD_RECOMMENDED));
  AddButtonControl(SavePasswordCheck);

  CreateShortcutCheck = new TCheckBox(this);
  CreateShortcutCheck->Caption = LoadStr(SAVE_SITE_WORKSPACE_SHORTCUT);
  AddButtonControl(CreateShortcutCheck);

  EnableControl(SavePasswordCheck, CanSavePassword);
}
//---------------------------------------------------------------------------
bool __fastcall TSaveSessionDialog::Execute(
  UnicodeString & SessionName, bool & SavePassword, bool & CreateShortcut,
  const UnicodeString & OriginalSessionName)
{
  FOriginalSessionName = OriginalSessionName;
  SessionNameEdit->Text = TSessionData::ExtractLocalName(SessionName);
  UnicodeString Folder = TSessionData::ExtractFolderName(SessionName);
  if (Folder.IsEmpty())
  {
    FolderCombo->Text = FRootFolder;
  }
  else
  {
    FolderCombo->Text = Folder;
  }
  SavePasswordCheck->Checked = SavePassword;
  CreateShortcutCheck->Checked = CreateShortcut;
  bool Result = TCustomDialog::Execute();
  if (Result)
  {
    SessionName = GetSessionName();
    SavePassword = SavePasswordCheck->Checked;
    CreateShortcut = CreateShortcutCheck->Checked;
  }
  return Result;
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TSaveSessionDialog::GetSessionName()
{
  UnicodeString Folder;
  if (FolderCombo->Text != FRootFolder)
  {
    Folder = FolderCombo->Text;
  }
  return TSessionData::ComposePath(Folder, SessionNameEdit->Text);
}
//---------------------------------------------------------------------------
void __fastcall TSaveSessionDialog::DoValidate()
{
  TSessionData::ValidateName(SessionNameEdit->Text);
  SessionNameValidate(GetSessionName(), FOriginalSessionName);

  UnicodeString Folder = TSessionData::ExtractFolderName(GetSessionName());
  if (!Folder.IsEmpty() && StoredSessions->IsWorkspace(Folder))
  {
    throw Exception(FMTLOAD(WORKSPACE_NOT_FOLDER, (Folder)));
  }

  if (SavePasswordCheck->Enabled && SavePasswordCheck->Checked &&
      CustomWinConfiguration->UseMasterPassword)
  {
    CustomWinConfiguration->AskForMasterPasswordIfNotSet();
  }

  TCustomDialog::DoValidate();
}
//---------------------------------------------------------------------------
void __fastcall TSaveSessionDialog::DoChange(bool & CanSubmit)
{
  CanSubmit = !SessionNameEdit->Text.IsEmpty();
  TCustomDialog::DoChange(CanSubmit);
}
//---------------------------------------------------------------------------
TSessionData * __fastcall DoSaveSession(TSessionData * SessionData,
  TSessionData * OriginalSession, bool ForceDialog,
  TStrings * AdditionalFolders)
{
  bool SavePassword = false;
  bool * PSavePassword;
  bool NotRecommendedSavingPassword =
    !CustomWinConfiguration->UseMasterPassword &&
    !SameText(SessionData->UserName, AnonymousUserName);

  if (Configuration->DisablePasswordStoring ||
      !SessionData->HasAnyPassword())
  {
    PSavePassword = NULL;
  }
  else
  {
    PSavePassword = &SavePassword;
    SavePassword =
      ((OriginalSession != NULL) &&
       !OriginalSession->Password.IsEmpty()) ||
      !NotRecommendedSavingPassword;
  }

  UnicodeString SessionName = SessionData->SessionName;

  bool Result;
  bool CreateShortcut = false;
  if (!ForceDialog && ((PSavePassword == NULL) || SavePassword))
  {
    CustomWinConfiguration->AskForMasterPasswordIfNotSetAndNeededToPersistSessionData(SessionData);
    Result = true;
  }
  else
  {
    // This can be a standalone dialog when used with save URL (from GetLoginData)
    TSaveSessionDialog * Dialog = SafeFormCreate<TSaveSessionDialog>();
    try
    {
      Dialog->Init((PSavePassword != NULL), NotRecommendedSavingPassword, AdditionalFolders);
      Result = Dialog->Execute(SessionName, SavePassword, CreateShortcut, SessionData->Name);
    }
    __finally
    {
      delete Dialog;
    }
  }

  TSessionData * NewSession = NULL;
  if (Result)
  {
    if ((PSavePassword != NULL) && !SavePassword)
    {
      SessionData->Password = L"";
    }

    NewSession =
      StoredSessions->NewSession(SessionName, SessionData);
    // modified only, explicit
    StoredSessions->Save(false, true);
    if (!SessionData->HostKey.IsEmpty())
    {
      SessionData->CacheHostKeyIfNotCached();
    }

    if (CreateShortcut)
    {
      TOperationVisualizer Visualizer;
      UnicodeString AdditionalParams =
        TProgramParams::FormatSwitch(DESKTOP_SWITCH) + L" " +
        TProgramParams::FormatSwitch(UPLOAD_IF_ANY_SWITCH);
      CreateDesktopSessionShortCut(SessionName, L"", AdditionalParams, -1, SITE_ICON);
    }
  }

  return NewSession;
}
//---------------------------------------------------------------------------
void __fastcall SessionNameValidate(const UnicodeString & Text,
  const UnicodeString & OriginalName)
{
  TSessionData::ValidatePath(Text);

  assert(StoredSessions);
  TSessionData * Data = (TSessionData *)StoredSessions->FindByName(Text);
  if (Data && Data->Special)
  {
    MessageDialog(FMTLOAD(CANNOT_OVERWRITE_SPECIAL_SESSION, (Text)),
      qtError, qaOK, HELP_NONE);
    Abort();
  }
  else if ((Data != NULL) && (Text != OriginalName) &&
    MessageDialog(MainInstructions(FMTLOAD(CONFIRM_OVERWRITE_SESSION, (Text))),
      qtConfirmation, qaYes | qaNo, HELP_SESSION_SAVE_OVERWRITE) != qaYes)
  {
    Abort();
  }
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
class TSaveWorkspaceDialog : public TCustomDialog
{
public:
  __fastcall TSaveWorkspaceDialog(bool CanSavePasswords,
    bool NotRecommendedSavingPasswords);

  bool __fastcall Execute(
    UnicodeString & WorkspaceName, bool & SavePasswords, bool & CreateShortcut,
    bool & EnableAutoSave);

protected:
  virtual void __fastcall DoValidate();
  virtual void __fastcall DoChange(bool & CanSubmit);

private:
  TComboBox * WorkspaceNameCombo;
  TCheckBox * SavePasswordsCheck;
  TCheckBox * CreateShortcutCheck;
  TCheckBox * EnableAutoSaveCheck;
};
//---------------------------------------------------------------------------
__fastcall TSaveWorkspaceDialog::TSaveWorkspaceDialog(
    bool CanSavePasswords, bool NotRecommendedSavingPasswords) :
  TCustomDialog(HELP_WORKSPACE_SAVE)
{
  Caption = LoadStr(SAVE_WORKSPACE_CAPTION);

  WorkspaceNameCombo = new TComboBox(this);
  WorkspaceNameCombo->AutoComplete = false;
  AddComboBox(WorkspaceNameCombo, CreateLabel(LoadStr(SAVE_WORKSPACE_PROMPT)));
  WorkspaceNameCombo->DropDownCount = Max(WorkspaceNameCombo->DropDownCount, 16);

  std::unique_ptr<TStrings> Workspaces(StoredSessions->GetWorkspaces());
  WorkspaceNameCombo->Items->AddStrings(Workspaces.get());

  SavePasswordsCheck = new TCheckBox(this);
  SavePasswordsCheck->Caption = LoadStr(
    NotRecommendedSavingPasswords ? SAVE_WORKSPACE_PASSWORDS :
      (CustomWinConfiguration->UseMasterPassword ?
        SAVE_WORKSPACE_PASSWORDS_MASTER : SAVE_WORKSPACE_PASSWORDS_RECOMMENDED));
  AddButtonControl(SavePasswordsCheck);

  EnableControl(SavePasswordsCheck, CanSavePasswords);

  CreateShortcutCheck = new TCheckBox(this);
  CreateShortcutCheck->Caption = LoadStr(SAVE_SITE_WORKSPACE_SHORTCUT);
  AddButtonControl(CreateShortcutCheck);

  EnableAutoSaveCheck = new TCheckBox(this);
  EnableAutoSaveCheck->Caption = LoadStr(SAVE_WORKSPACE_AUTO);
  AddButtonControl(EnableAutoSaveCheck);
}
//---------------------------------------------------------------------------
bool __fastcall TSaveWorkspaceDialog::Execute(
  UnicodeString & WorkspaceName, bool & SavePasswords, bool & CreateShortcut,
  bool & EnableAutoSave)
{
  WorkspaceNameCombo->Text = WorkspaceName;
  SavePasswordsCheck->Checked = SavePasswords;
  CreateShortcutCheck->Checked = CreateShortcut;
  EnableAutoSaveCheck->Checked = EnableAutoSave;
  bool Result = TCustomDialog::Execute();
  if (Result)
  {
    WorkspaceName = WorkspaceNameCombo->Text;
    SavePasswords = SavePasswordsCheck->Checked;
    CreateShortcut = CreateShortcutCheck->Checked;
    EnableAutoSave = EnableAutoSaveCheck->Checked;
  }
  return Result;
}
//---------------------------------------------------------------------------
void __fastcall TSaveWorkspaceDialog::DoValidate()
{
  TSessionData::ValidateName(WorkspaceNameCombo->Text);

  if (StoredSessions->IsFolder(WorkspaceNameCombo->Text))
  {
    throw Exception(FMTLOAD(FOLDER_NOT_WORKSPACE, (WorkspaceNameCombo->Text)));
  }

  if (SavePasswordsCheck->Enabled && SavePasswordsCheck->Checked &&
      CustomWinConfiguration->UseMasterPassword)
  {
    CustomWinConfiguration->AskForMasterPasswordIfNotSet();
  }

  TCustomDialog::DoValidate();
}
//---------------------------------------------------------------------------
void __fastcall TSaveWorkspaceDialog::DoChange(bool & CanSubmit)
{
  CanSubmit = !WorkspaceNameCombo->Text.IsEmpty();

  TCustomDialog::DoChange(CanSubmit);
}
//---------------------------------------------------------------------------
bool __fastcall DoSaveWorkspaceDialog(UnicodeString & WorkspaceName,
  bool * SavePasswords, bool NotRecommendedSavingPasswords,
  bool & CreateShortcut, bool & EnableAutoSave)
{
  std::unique_ptr<TSaveWorkspaceDialog> Dialog(
    new TSaveWorkspaceDialog((SavePasswords != NULL), NotRecommendedSavingPasswords));

  bool Dummy = false;
  if (SavePasswords == NULL)
  {
    SavePasswords = &Dummy;
  }
  return
    Dialog->Execute(
      WorkspaceName, *SavePasswords, CreateShortcut, EnableAutoSave);
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
class TShortCutDialog : public TCustomDialog
{
public:
  __fastcall TShortCutDialog(const TShortCuts & ShortCuts, UnicodeString HelpKeyword);

  bool __fastcall Execute(TShortCut & ShortCut);

private:
  TComboBox * ShortCutCombo;
};
//---------------------------------------------------------------------------
__fastcall TShortCutDialog::TShortCutDialog(const TShortCuts & ShortCuts, UnicodeString HelpKeyword) :
  TCustomDialog(HelpKeyword)
{
  Caption = LoadStr(SHORTCUT_CAPTION);

  ShortCutCombo = new TComboBox(this);
  AddComboBox(ShortCutCombo, CreateLabel(LoadStr(SHORTCUT_LABEL)));
  InitializeShortCutCombo(ShortCutCombo, ShortCuts);
}
//---------------------------------------------------------------------------
bool __fastcall TShortCutDialog::Execute(TShortCut & ShortCut)
{
  SetShortCutCombo(ShortCutCombo, ShortCut);
  bool Result = TCustomDialog::Execute();
  if (Result)
  {
    ShortCut = GetShortCutCombo(ShortCutCombo);
  }
  return Result;
}
//---------------------------------------------------------------------------
bool __fastcall DoShortCutDialog(TShortCut & ShortCut,
  const TShortCuts & ShortCuts, UnicodeString HelpKeyword)
{
  bool Result;
  TShortCutDialog * Dialog = new TShortCutDialog(ShortCuts, HelpKeyword);
  try
  {
    Result = Dialog->Execute(ShortCut);
  }
  __finally
  {
    delete Dialog;
  }
  return Result;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
class TRemoteMoveDialog : public TCustomDialog
{
public:
  __fastcall TRemoteMoveDialog();

  bool __fastcall Execute(UnicodeString & Target, UnicodeString & FileMask);

protected:
  DYNAMIC void __fastcall DoShow();

private:
  THistoryComboBox * Combo;
};
//---------------------------------------------------------------------------
__fastcall TRemoteMoveDialog::TRemoteMoveDialog() :
  TCustomDialog(HELP_REMOTE_MOVE)
{
  Caption = LoadStr(REMOTE_MOVE_TITLE);

  AddImage(L"REMOTE_MOVE_FILE");

  Combo = new THistoryComboBox(this);
  Combo->AutoComplete = false;
  AddComboBox(Combo, CreateLabel(LoadStr(REMOTE_TRANSFER_PROMPT)));
}
//---------------------------------------------------------------------------
bool __fastcall TRemoteMoveDialog::Execute(UnicodeString & Target, UnicodeString & FileMask)
{
  Combo->Items = CustomWinConfiguration->History[L"RemoteTarget"];
  Combo->Text = UnixIncludeTrailingBackslash(Target) + FileMask;
  bool Result = TCustomDialog::Execute();
  if (Result)
  {
    Target = UnixExtractFilePath(Combo->Text);
    FileMask = UnixExtractFileName(Combo->Text);
    Combo->SaveToHistory();
    CustomWinConfiguration->History[L"RemoteTarget"] = Combo->Items;
  }
  return Result;
}
//---------------------------------------------------------------------------
void __fastcall TRemoteMoveDialog::DoShow()
{
  TCustomDialog::DoShow();
  InstallPathWordBreakProc(Combo);
}
//---------------------------------------------------------------------------
bool __fastcall DoRemoteMoveDialog(UnicodeString & Target, UnicodeString & FileMask)
{
  std::auto_ptr<TRemoteMoveDialog> Dialog(new TRemoteMoveDialog());
  return Dialog->Execute(Target, FileMask);
}
