//---------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "WinInterface.h"
#include "Properties.h"

#include <VCLCommon.h>
#include <Common.h>
#include <Terminal.h>
#include <TextsWin.h>
#include <GUITools.h>
#include <CoreMain.h>
#include <Tools.h>
#include <BaseUtils.hpp>
//---------------------------------------------------------------------
#pragma link "PathLabel"
#pragma link "Rights"
#pragma link "RightsExt"
#ifndef NO_RESOURCES
#pragma resource "*.dfm"
#endif
//---------------------------------------------------------------------
bool __fastcall DoPropertiesDialog(TStrings * FileList,
  const UnicodeString Directory, const TRemoteTokenList * GroupList,
  const TRemoteTokenList * UserList, TRemoteProperties * Properties,
  int AllowedChanges, bool UserGroupByID, TCalculateSizeEvent OnCalculateSize,
  TCalculateChecksumEvent OnCalculateChecksum)
{
  bool Result;
  TPropertiesDialog * PropertiesDialog = new TPropertiesDialog(Application,
    FileList, Directory, GroupList, UserList, AllowedChanges, UserGroupByID,
    OnCalculateSize, OnCalculateChecksum);
  try
  {
    Result = PropertiesDialog->Execute(*Properties);
  }
  __finally
  {
    delete PropertiesDialog;
  }
  return Result;
}
//---------------------------------------------------------------------
__fastcall TPropertiesDialog::TPropertiesDialog(TComponent* AOwner,
  TStrings * FileList, const UnicodeString Directory,
  const TRemoteTokenList * GroupList, const TRemoteTokenList * UserList,
  int AllowedChanges, bool UserGroupByID, TCalculateSizeEvent OnCalculateSize,
  TCalculateChecksumEvent OnCalculateChecksum)
  : TForm(AOwner)
{
  FOnCalculateSize = OnCalculateSize;
  FOnCalculateChecksum = OnCalculateChecksum;
  RightsFrame->OnChange = ControlChange;

  FShellImageList = SharedSystemImageList(true);
  FShellImageList->DrawingStyle = dsTransparent;

  FFileList = new TStringList();
  FFileList->Assign(FileList);
  FAllowedChanges = AllowedChanges;
  FUserGroupByID = UserGroupByID;

  FAllowCalculateStats = false;
  FStatsNotCalculated = false;
  FChecksumLoaded = false;
  FMultipleChecksum = false;

  LocationLabel->Caption = Directory;

  FGroupList = GroupList;
  FUserList = UserList;

  LoadInfo();

  UseSystemSettings(this);
}
//---------------------------------------------------------------------------
__fastcall TPropertiesDialog::~TPropertiesDialog()
{
  delete FFileList;
  FFileList = NULL;
  delete FShellImageList;
  FShellImageList = NULL;
}
//---------------------------------------------------------------------
bool __fastcall TPropertiesDialog::Execute(TRemoteProperties & Properties)
{
  SetFileProperties(Properties);

  PageControl->ActivePage = CommonSheet;
  if (FAllowedChanges & cpGroup) ActiveControl = GroupComboBox;
    else
  if (FAllowedChanges & cpOwner) ActiveControl = OwnerComboBox;
    else
  if (FAllowedChanges & cpMode) ActiveControl = RightsFrame;
    else ActiveControl = CancelButton;

  ChecksumAlgEdit->Text = GUIConfiguration->ChecksumAlg;
  ResetChecksum();

  UpdateControls();

  bool Result = (ShowModal() == DefaultResult(this));

  if (Result)
  {
    Properties = GetFileProperties();
  }

  GUIConfiguration->ChecksumAlg = ChecksumAlgEdit->Text;

  return Result;
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TPropertiesDialog::LoadRemoteToken(
  const TRemoteToken & Token)
{
  UnicodeString Result;
  if (FUserGroupByID)
  {
    if (Token.IDValid)
    {
      if (Token.NameValid)
      {
        Result = FORMAT(L"%s [%d]", (Token.Name, int(Token.ID)));
      }
      else
      {
        Result = FORMAT(L"[%d]", (int(Token.ID)));
      }
    }
    else
    {
      // be it valid or not
      Result = Token.Name;
    }
  }
  else
  {
    // what if name is not filled in?
    Result = Token.Name;
  }
  return Result;
}
//---------------------------------------------------------------------------
void __fastcall TPropertiesDialog::LoadRemoteToken(
  TComboBox * ComboBox, bool Valid, const TRemoteToken & Token)
{
  if (Valid)
  {
    ComboBox->Text = LoadRemoteToken(Token);
  }
  else
  {
    ComboBox->Text = L"";
  }
}
//---------------------------------------------------------------------------
void __fastcall TPropertiesDialog::LoadRemoteTokens(TComboBox * ComboBox,
  const TRemoteTokenList * List)
{
  TStrings * Items = ComboBox->Items;
  Items->BeginUpdate();
  try
  {
    Items->Clear();
    if (List != NULL)
    {
      int Count = List->Count();
      for (int Index = 0; Index < Count; Index++)
      {
        Items->Add(LoadRemoteToken(*List->Token(Index)));
      }
    }
  }
  __finally
  {
    Items->EndUpdate();
  }
}
//---------------------------------------------------------------------------
void __fastcall TPropertiesDialog::LoadInfo()
{
  assert(FFileList->Count > 0);
  FMultiple = FFileList->Count > 1;
  FMultipleChecksum = FMultiple;

  FAllowCalculateStats = false;
  FStatsNotCalculated = false;

  __int64 FilesSize = 0;
  TCalculateSizeStats Stats;

  for (int Index = 0; Index < FFileList->Count; Index++)
  {
    TRemoteFile * File = (TRemoteFile *)(FFileList->Objects[Index]);
    if (File->IsDirectory)
    {
      Stats.Directories++;
      if (!File->IsSymLink)
      {
        FAllowCalculateStats = true;
        FStatsNotCalculated = true;
        FMultipleChecksum = true;
      }
    }
    else
    {
      Stats.Files++;
    }

    if (File->IsSymLink)
    {
      Stats.SymLinks++;
    }
    FilesSize += File->Size;
  }

  LoadRemoteTokens(GroupComboBox, FGroupList);
  LoadRemoteTokens(OwnerComboBox, FUserList);

  RightsFrame->AllowAddXToDirectories = (Stats.Directories > 0);
  RecursiveCheck->Visible = (Stats.Directories > 0);
  RecursiveBevel->Visible = (Stats.Directories > 0);

  LoadStats(FilesSize, Stats);

  FileIconImage->Picture->Bitmap = NULL;

  RightsFrame->AllowUndef = FMultiple;

  if (!FMultiple)
  {
    TRemoteFile * File = (TRemoteFile *)(FFileList->Objects[0]);
    assert(File && FShellImageList);

    // shell image list does not have fixed large icon size
    // (it is probably 32x32 min, but can be larged, on xp it is 48x48 if
    // large icons are enabled, on vista can be even larger).
    // so we stretch (shrink) the icon to 32x32 here to be sure.
    Graphics::TBitmap * Bitmap = new Graphics::TBitmap;
    try
    {
      FShellImageList->GetBitmap(File->IconIndex, Bitmap);
      FileIconImage->Picture->Bitmap->Width = FileIconImage->Width;
      FileIconImage->Picture->Bitmap->Height = FileIconImage->Height;
      FileIconImage->Picture->Bitmap->Canvas->StretchDraw(
        TRect(0, 0, FileIconImage->Width, FileIconImage->Height),
        Bitmap);
    }
    __finally
    {
      delete Bitmap;
    }

    LinksToLabelLabel->Visible = File->IsSymLink;
    LinksToLabel->Visible = File->IsSymLink;
    if (File->IsSymLink)
    {
      LinksToLabel->Caption = File->LinkTo;
    }

    RightsFrame->AllowAddXToDirectories = File->IsDirectory;
    Caption = FMTLOAD(PROPERTIES_FILE_CAPTION, (File->FileName));
    RecursiveCheck->Visible = File->IsDirectory;
    RecursiveBevel->Visible = File->IsDirectory;
  }
  else
  {
    Caption = FMTLOAD(PROPERTIES_FILES_CAPTION, (FFileList->Strings[0]));
    LinksToLabelLabel->Hide();
    LinksToLabel->Hide();
  }

  FilesIconImage->Visible = FMultiple;
  FileIconImage->Visible = !FMultiple;
  ChecksumGroup->Visible = !FMultipleChecksum;
  ChecksumView->Visible = FMultipleChecksum;
}
//---------------------------------------------------------------------------
void __fastcall TPropertiesDialog::LoadStats(__int64 FilesSize,
  const TCalculateSizeStats & Stats)
{
  UnicodeString SizeStr;
  UnicodeString FilesStr;
  if (FStatsNotCalculated)
  {
    SizeStr = LoadStr(PROPERTIES_UNKNOWN_SIZE);
  }
  else
  {
    SizeStr = FormatBytes(FilesSize);
    UnicodeString SizeUnorderedStr = FormatBytes(FilesSize, false);
    if (SizeStr != SizeUnorderedStr)
    {
      SizeStr = FORMAT(L"%s (%s)", (SizeStr, SizeUnorderedStr));
    }
  }

  if (((Stats.Files + Stats.Directories) == 0) && !FMultiple)
  {
    TRemoteFile * File = (TRemoteFile *)(FFileList->Objects[0]);
    assert(File != NULL);
    FilesStr = File->FileName;
  }
  else
  {
    if (Stats.Files > 0)
    {
      FilesStr = (Stats.Files == 1) ? FMTLOAD(PROPERTIES_FILE, (Stats.Files)) :
        FMTLOAD(PROPERTIES_FILES, (Stats.Files));
      if (Stats.Directories > 0)
      {
        FilesStr = FORMAT(L"%s, ", (FilesStr));
      }
    }
    if (Stats.Directories > 0)
    {
      FilesStr += (Stats.Directories == 1) ? FMTLOAD(PROPERTIES_DIRECTORY, (Stats.Directories)) :
        FMTLOAD(PROPERTIES_DIRECTORIES, (Stats.Directories));
    }
    if (Stats.SymLinks > 0)
    {
      UnicodeString SymLinksStr;
      SymLinksStr = (Stats.SymLinks == 1) ? FMTLOAD(PROPERTIES_SYMLINK, (Stats.SymLinks)) :
        FMTLOAD(PROPERTIES_SYMLINKS, (Stats.SymLinks));
      FilesStr = FORMAT(L"%s (%s)", (FilesStr, SymLinksStr));
    }
  }

  SizeLabel->Caption = SizeStr;
  FileLabel->Caption = FilesStr;
}
//---------------------------------------------------------------------------
void __fastcall TPropertiesDialog::SetFileProperties(const TRemoteProperties & value)
{
  TValidProperties Valid;
  if (value.Valid.Contains(vpRights) && FAllowedChanges & cpMode) Valid << vpRights;
  if (value.Valid.Contains(vpOwner) && FAllowedChanges & cpOwner) Valid << vpOwner;
  if (value.Valid.Contains(vpGroup) && FAllowedChanges & cpGroup) Valid << vpGroup;
  FOrigProperties = value;
  FOrigProperties.Valid = Valid;
  FOrigProperties.Recursive = false;

  if (value.Valid.Contains(vpRights))
  {
    RightsFrame->Rights = value.Rights;
    RightsFrame->AddXToDirectories = value.AddXToDirectories;
  }
  else
  {
    RightsFrame->Rights = TRights();
    RightsFrame->AddXToDirectories = false;
  }
  LoadRemoteToken(GroupComboBox, value.Valid.Contains(vpGroup), value.Group);
  LoadRemoteToken(OwnerComboBox, value.Valid.Contains(vpOwner), value.Owner);
  RecursiveCheck->Checked = value.Recursive;
  UpdateControls();
}
//---------------------------------------------------------------------------
void __fastcall TPropertiesDialog::StoreRemoteToken(unsigned int ID,
  const UnicodeString & Text, const TRemoteTokenList * List, TRemoteToken & Result)
{
  assert(List != NULL);
  const TRemoteToken * Token = List->Find(ID);
  if (Token == NULL)
  {
    Result.ID = ID;
    Result.Name = Text;
  }
  else
  {
    Result = *Token;
  }
}
//---------------------------------------------------------------------------
TRemoteToken __fastcall TPropertiesDialog::StoreRemoteToken(const TRemoteToken & Orig,
  UnicodeString Text, int Message, const TRemoteTokenList * List)
{
  TRemoteToken Result;
  Text = Text.Trim();
  if (!Text.IsEmpty())
  {
    if (FUserGroupByID)
    {
      assert(List != NULL);
      int IDStart = Text.LastDelimiter(L"[");
      if (!Text.IsEmpty() && (IDStart >= 0) && (Text[Text.Length()] == L']'))
      {
        int ID;
        UnicodeString IDStr = Text.SubString(IDStart + 1, Text.Length() - IDStart - 1);
        if (!TryStrToInt(IDStr, ID))
        {
          throw Exception(Message);
        }
        else
        {
          StoreRemoteToken(ID, Text.SubString(1, IDStart - 1).Trim(), List, Result);
        }
      }
      else
      {
        const TRemoteToken * Token = List->Find(Text);
        if (Token == NULL)
        {
          int ID;
          if (TryStrToInt(Text, ID))
          {
            StoreRemoteToken(ID, Text, List, Result);
          }
          else
          {
            throw Exception(MainInstructions(FMTLOAD(PROPERTIES_UNKNOWN_TOKEN, (Text))));
          }
        }
        else
        {
          Result = *Token;
        }
      }
    }
    else
    {
      Result.Name = Text;
    }
  }

  if (LoadRemoteToken(Result) == LoadRemoteToken(Orig))
  {
    Result = Orig;
  }

  return Result;
}
//---------------------------------------------------------------------------
void __fastcall TPropertiesDialog::StoreRemoteToken(TComboBox * ComboBox,
  int ChangeFlag, TValidProperty PropertyFlag, const TRemoteToken & Orig,
  TRemoteToken & Token, int Message, const TRemoteTokenList * List,
  TRemoteProperties & Properties)
{
  UnicodeString Text = ComboBox->Text.Trim();
  if (FLAGSET(FAllowedChanges, ChangeFlag))
  {
    Token = StoreRemoteToken(Orig, Text, Message, List);
    if (Token.IsSet)
    {
      Properties.Valid << PropertyFlag;
    }
  }
}
//---------------------------------------------------------------------------
TRemoteProperties __fastcall TPropertiesDialog::GetFileProperties()
{
  TRemoteProperties Result;

  if (FAllowedChanges & cpMode)
  {
    Result.Valid << vpRights;
    Result.Rights = RightsFrame->Rights;
    Result.AddXToDirectories = RightsFrame->AddXToDirectories;
  }

  StoreRemoteToken(GroupComboBox, cpGroup, vpGroup, FOrigProperties.Group,
    Result.Group, PROPERTIES_INVALID_GROUP, FGroupList, Result);
  StoreRemoteToken(OwnerComboBox, cpOwner, vpOwner, FOrigProperties.Owner,
    Result.Owner, PROPERTIES_INVALID_OWNER, FUserList, Result);

  Result.Recursive = RecursiveCheck->Checked;

  return Result;
}
//---------------------------------------------------------------------------
void __fastcall TPropertiesDialog::ControlChange(TObject * /*Sender*/)
{
  if (Visible)
  {
    UpdateControls();
  }
}
//---------------------------------------------------------------------------
void __fastcall TPropertiesDialog::UpdateControls()
{
  bool Allow;
  try
  {
    Allow =
      !TRemoteProperties::ChangedProperties(FOrigProperties, GetFileProperties()).Valid.Empty() ||
      RecursiveCheck->Checked;
  }
  catch(...)
  {
    // when properties are invalid allow submitting the form,
    // because that reveals the cause to the user, otherwise he/she
    // may not be able to tell what is wrong

    Allow = true;
  }
  EnableControl(OkButton, Allow);

  EnableControl(GroupComboBox, FAllowedChanges & cpGroup);
  EnableControl(OwnerComboBox, FAllowedChanges & cpOwner);
  EnableControl(RightsFrame, FAllowedChanges & cpMode);
  CalculateSizeButton->Visible = FAllowCalculateStats;

  if (!FMultiple)
  {
    // when setting properties for one file only, allow undef state
    // only when the input right explicitly requires it or
    // when "recursive" is on (possible for directory only).
    bool AllowUndef =
      (FOrigProperties.Valid.Contains(vpRights) &&
       FOrigProperties.Rights.AllowUndef) ||
      (RecursiveCheck->Checked);
    if (!AllowUndef)
    {
      // when disallowing undef state, make sure, all undef are turned into unset
      RightsFrame->Rights = TRights(RightsFrame->Rights.NumberSet);
    }
    RightsFrame->AllowUndef = AllowUndef;
  }

  EnableControl(ChecksumSheet, ChecksumSupported());
  EnableControl(ChecksumButton, ChecksumSheet->Enabled &&
    !ChecksumAlgEdit->Text.IsEmpty());
  // hide checksum edit at least if it is disabled to get rid of ugly
  // visage on XP
  ChecksumEdit->Visible = ChecksumEdit->Enabled;
}
//---------------------------------------------------------------------------
void __fastcall TPropertiesDialog::FormCloseQuery(TObject * /*Sender*/,
      bool & /*CanClose*/)
{
  if (ModalResult == DefaultResult(this))
  {
    ExitActiveControl(this);
  }
}
//---------------------------------------------------------------------------
void __fastcall TPropertiesDialog::CalculateSizeButtonClick(
      TObject * /*Sender*/)
{
  assert(FOnCalculateSize != NULL);

  bool DoClose = false;
  try
  {
    __int64 Size;
    TCalculateSizeStats Stats;
    FOnCalculateSize(FFileList, Size, Stats, DoClose);
    FStatsNotCalculated = false;
    LoadStats(Size, Stats);
  }
  __finally
  {
    if (DoClose)
    {
      Close();
    }
  }
}
//---------------------------------------------------------------------------
void __fastcall TPropertiesDialog::HelpButtonClick(TObject * /*Sender*/)
{
  FormHelp(this);
}
//---------------------------------------------------------------------------
void __fastcall TPropertiesDialog::ResetChecksum()
{
  ChecksumView->Items->Clear();
  ChecksumEdit->Text = LoadStr(PROPERTIES_CHECKSUM_UNKNOWN);
}
//---------------------------------------------------------------------------
void __fastcall TPropertiesDialog::CalculateChecksum()
{
  assert(FOnCalculateChecksum != NULL);

  ResetChecksum();
  FChecksumLoaded = true;

  bool DoClose = false;
  try
  {
    FOnCalculateChecksum(ChecksumAlgEdit->Text, FFileList, CalculatedChecksum, DoClose);
  }
  __finally
  {
    if (DoClose)
    {
      Close();
    }
  }
}
//---------------------------------------------------------------------------
void __fastcall TPropertiesDialog::CalculatedChecksum(
  const UnicodeString & FileName, const UnicodeString & /*Alg*/,
  const UnicodeString & Hash)
{
  if (FMultipleChecksum)
  {
    TListItem * Item = ChecksumView->Items->Add();
    Item->Caption = FileName;
    Item->SubItems->Add(Hash);
  }
  else
  {
    ChecksumEdit->Text = Hash;
  }
}
//---------------------------------------------------------------------------
void __fastcall TPropertiesDialog::NeedChecksum()
{
  if (!FChecksumLoaded && ChecksumSupported())
  {
    CalculateChecksum();
  }
}
//---------------------------------------------------------------------------
bool __fastcall TPropertiesDialog::ChecksumSupported()
{
  return (FOnCalculateChecksum != NULL);
}
//---------------------------------------------------------------------------
void __fastcall TPropertiesDialog::ChecksumButtonClick(TObject * /*Sender*/)
{
  CalculateChecksum();
}
//---------------------------------------------------------------------------
void __fastcall TPropertiesDialog::PageControlChange(TObject * /*Sender*/)
{
  if (PageControl->ActivePage == ChecksumSheet)
  {
    NeedChecksum();
  }
}
//---------------------------------------------------------------------------
void __fastcall TPropertiesDialog::ChecksumAlgEditChange(TObject * /*Sender*/)
{
  ResetChecksum();
  UpdateControls();
}
//---------------------------------------------------------------------------
void __fastcall TPropertiesDialog::CopyClick(TObject * /*Sender*/)
{
  TInstantOperationVisualizer Visualizer;

  TListView * ListView = dynamic_cast<TListView *>(ListViewMenu->PopupComponent);
  assert(ListView != NULL);

  int Count = 0;
  UnicodeString SingleText;
  UnicodeString Text;
  TListItem * Item = ListView->GetNextItem(NULL, sdAll, TItemStates() << isSelected);
  while (Item != NULL)
  {
    assert(Item->Selected);

    SingleText = Item->SubItems->Strings[0];
    Text += FORMAT(L"%s = %s\r\n", (Item->Caption, Item->SubItems->Strings[0]));
    Count++;

    Item = ListView->GetNextItem(Item, sdAll, TItemStates() << isSelected);
  }

  CopyToClipboard(Count == 1 ? SingleText : Text);
}
//---------------------------------------------------------------------------
void __fastcall TPropertiesDialog::ChecksumViewContextPopup(
  TObject * Sender, TPoint & MousePos, bool & Handled)
{
  MenuPopup(Sender, MousePos, Handled);
}
//---------------------------------------------------------------------------
void __fastcall TPropertiesDialog::ResolveRemoteToken(
  const TRemoteToken & Orig, int Message, TComboBox * ComboBox,
  const TRemoteTokenList * List)
{
  try
  {
    ComboBox->Text =
      LoadRemoteToken(StoreRemoteToken(Orig, ComboBox->Text, Message, List));
  }
  catch(...)
  {
    ComboBox->SetFocus();
    throw;
  }
}
//---------------------------------------------------------------------------
void __fastcall TPropertiesDialog::GroupComboBoxExit(TObject * Sender)
{
  ResolveRemoteToken(FOrigProperties.Group, PROPERTIES_INVALID_GROUP,
    dynamic_cast<TComboBox *>(Sender), FGroupList);
}
//---------------------------------------------------------------------------
void __fastcall TPropertiesDialog::OwnerComboBoxExit(TObject * Sender)
{
  ResolveRemoteToken(FOrigProperties.Owner, PROPERTIES_INVALID_OWNER,
    dynamic_cast<TComboBox *>(Sender), FUserList);
}
//---------------------------------------------------------------------------
void __fastcall TPropertiesDialog::FormShow(TObject * /*Sender*/)
{
  UpdateControls();
}
//---------------------------------------------------------------------------
