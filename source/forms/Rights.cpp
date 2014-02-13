//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "Rights.h"

#include <Common.h>

#include <VCLCommon.h>
#include <Tools.h>
#include <GUITools.h>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "GrayedCheckBox"
#pragma link "PngImageList"
#ifndef NO_RESOURCES
#pragma resource "*.dfm"
#endif
//---------------------------------------------------------------------------
__fastcall TRightsFrame::TRightsFrame(TComponent* Owner)
        : TFrame(Owner)
{
  FAddXToDirectoriesSuffix = L"+x";
  FOnChange = NULL;
  FAllowAddXToDirectories = true;
  FPopup = false;
  FPopupParent = NULL;
  FDefaultButton = NULL;
  FCancelButton = NULL;
  // to avoid duplication of reference in forms that uses the frame
  PopupMenu = RightsPopup;
  FPopingContextMenu = false;
  FInitialized = false;

  #define COPY_HINT(R) \
    Checks[TRights::rrGroup ## R]->Hint = Checks[TRights::rrUser ## R]->Hint; \
    Checks[TRights::rrOther ## R]->Hint = Checks[TRights::rrUser ## R]->Hint;
  COPY_HINT(Read);
  COPY_HINT(Write);
  COPY_HINT(Exec);
  #undef COPY_HINT

  UpgradeSpeedButton(OwnerButton);
  UpgradeSpeedButton(GroupButton);
  UpgradeSpeedButton(OthersButton);
}
//---------------------------------------------------------------------------
__fastcall TRightsFrame::~TRightsFrame()
{
}
//---------------------------------------------------------------------------
void __fastcall TRightsFrame::SetStates(TRights::TRight Right, TRights::TState value)
{
  assert(((value == TRights::rsNo) || (value == TRights::rsYes) || (value == TRights::rsUndef)));
  TCheckBox * CheckBox = Checks[Right];
  if (CheckBox != NULL)
  {
    switch (value) {
      case TRights::rsNo: CheckBox->State = cbUnchecked; break;
      case TRights::rsYes: CheckBox->State = cbChecked; break;
      case TRights::rsUndef: CheckBox->State = cbGrayed; break;
    }
  }
}
//---------------------------------------------------------------------------
TRights::TState __fastcall TRightsFrame::GetStates(TRights::TRight Right)
{
  TCheckBox * CheckBox = Checks[Right];
  if (CheckBox != NULL)
  {
    switch (CheckBox->State) {
      case cbUnchecked: return TRights::rsNo;
      case cbChecked: return TRights::rsYes;
      case cbGrayed:
      default: return TRights::rsUndef;
    }
  }
  else
  {
    return TRights::rsNo;
  }
}
//---------------------------------------------------------------------------
void __fastcall TRightsFrame::SetRights(const TRights & value)
{
  bool Changed = (AllowUndef != value.AllowUndef);
  AllowUndef = true; // temporarily
  for (int Right = TRights::rrFirst; Right <= TRights::rrLast; Right++)
  {
    TRights::TRight R = static_cast<TRights::TRight>(Right);
    if (States[R] != value.RightUndef[R])
    {
      States[R] = value.RightUndef[R];
      Changed = true;
    }
  }
  AllowUndef = value.AllowUndef;
  if (Changed || !FInitialized)
  {
    UpdateControls();
  }
}
//---------------------------------------------------------------------------
TRights __fastcall TRightsFrame::GetRights()
{
  TRights Result;
  Result.AllowUndef = AllowUndef;
  for (int Right = TRights::rrFirst; Right <= TRights::rrLast; Right++)
  {
    Result.RightUndef[static_cast<TRights::TRight>(Right)] =
      States[static_cast<TRights::TRight>(Right)];
  }
  return Result;
}
//---------------------------------------------------------------------------
void __fastcall TRightsFrame::SetAllowUndef(bool value)
{
  for (int Right = TRights::rrFirst; Right <= TRights::rrLast; Right++)
  {
    TCheckBox * CheckBox = Checks[static_cast<TRights::TRight>(Right)];
    if (CheckBox != NULL)
    {
      CheckBox->AllowGrayed = value;
    }
  }
}
//---------------------------------------------------------------------------
bool __fastcall TRightsFrame::GetAllowUndef()
{
  bool Result = false, First = true;
  for (int Right = TRights::rrFirst; Right <= TRights::rrLast; Right++)
  {
    TCheckBox * Check = Checks[static_cast<TRights::TRight>(Right)];
    if (Check != NULL)
    {
      if (First)
      {
        Result = Check->AllowGrayed;
        First = false;
      }
      else if (Result != Check->AllowGrayed)
      {
        assert(false);
      }
    }
  }
  return Result;
}
//---------------------------------------------------------------------------
TCheckBox * __fastcall TRightsFrame::GetChecks(TRights::TRight Right)
{
  for (int Index = 0; Index < ControlCount; Index++)
  {
    if (Controls[Index]->InheritsFrom(__classid(TCheckBox)) &&
        (Controls[Index]->Tag == TRights::RightToFlag(Right)))
    {
      return ((TCheckBox *)Controls[Index]);
    }
  }
  return NULL;
}
//---------------------------------------------------------------------------
void __fastcall TRightsFrame::SetAddXToDirectories(bool value)
{
  DirectoriesXCheck->Checked = value;
}
//---------------------------------------------------------------------------
bool __fastcall TRightsFrame::GetAddXToDirectories()
{
  return DirectoriesXCheck->Checked;
}
//---------------------------------------------------------------------------
void __fastcall TRightsFrame::ControlChange(TObject * /*Sender*/)
{
  UpdateControls();
}
//---------------------------------------------------------------------------
bool __fastcall TRightsFrame::DirectoriesXEffective()
{
  return !((Rights.NumberSet & TRights::rfExec) == TRights::rfExec);
}
//---------------------------------------------------------------------------
void __fastcall TRightsFrame::UpdateControls()
{
  Color = (FPopup ? clWindow : clBtnFace);
  DirectoriesXCheck->Visible = AllowAddXToDirectories;
  EnableControl(DirectoriesXCheck,
    Enabled && DirectoriesXEffective());
  FInitialized = true;
  DoChange();
}
//---------------------------------------------------------------------------
void __fastcall TRightsFrame::CycleRights(int Group)
{
  TRights::TState State;
  bool Same = true;
  for (int Right = 0; Right < 3; Right++)
  {
    TRights::TState CState = States[static_cast<TRights::TRight>(
      TRights::rrUserRead + Right + ((Group - 1) * 3))];

    if (Right == 0) State = CState;
      else
    if (State != CState) Same = False;
  }

  if (!Same)
  {
    State = TRights::rsYes;
  }
  else
  {
    switch (State) {
      case TRights::rsYes:
        State = TRights::rsNo;
        break;

      case TRights::rsNo:
        State = AllowUndef ? TRights::rsUndef : TRights::rsYes;
        break;

      case TRights::rsUndef:
        State = TRights::rsYes;
        break;
    }
  }

  for (int Right = 0; Right < 3; Right++)
  {
    States[static_cast<TRights::TRight>(
      TRights::rrUserRead + Right + ((Group - 1) * 3))] = State;
  }
  UpdateControls();
}
//---------------------------------------------------------------------------
void __fastcall TRightsFrame::RightsButtonsClick(TObject *Sender)
{
  CycleRights(((TComponent*)Sender)->Tag);
}
//---------------------------------------------------------------------------
void __fastcall TRightsFrame::SetAllowAddXToDirectories(bool value)
{
  if ((FAllowAddXToDirectories != value) || !FInitialized)
  {
    FAllowAddXToDirectories = value;
    UpdateControls();
  }
}
//---------------------------------------------------------------------------
void __fastcall TRightsFrame::DoChange()
{
  if (FOnChange)
  {
    FOnChange(this);
  }
}
//---------------------------------------------------------------------------
void __fastcall TRightsFrame::SetEnabled(bool Value)
{
  TFrame::SetEnabled(Value);
  UpdateControls();
}
//---------------------------------------------------------------------------
void __fastcall TRightsFrame::ForceUpdate()
{
}
//---------------------------------------------------------------------------
bool __fastcall TRightsFrame::HasFocus()
{
  return
    (Focused() ||
    ((Screen->ActiveControl != NULL) && (Screen->ActiveControl->Parent == this))) ||
    ((PopupParent != NULL) && PopupParent->Focused());
}
//---------------------------------------------------------------------------
void __fastcall TRightsFrame::RightsActionsExecute(TBasicAction * Action,
  bool & Handled)
{
  // prevent shortcuts to be avaluated when frame does not have a focus
  if (HasFocus())
  {
    bool Changed = true;
    TRights R = Rights;
    R.Number = TRights::rfNo;

    Handled = true;
    if (Action == NoRightsAction)
    {
      R = TRights::rfNo;
    }
    else if (Action == DefaultRightsAction)
    {
      R = TRights::rfDefault;
    }
    else if (Action == AllRightsAction)
    {
      R = TRights::rfAll;
    }
    else if (Action == LeaveRightsAsIsAction)
    {
      R.AllUndef();
    }
    else if (Action == CopyTextAction)
    {
      TInstantOperationVisualizer Visualizer;
      CopyToClipboard(Text);
      Changed = false;
    }
    else if (Action == CopyOctalAction)
    {
      TRights R = Rights;
      assert(!R.IsUndef);
      if (!R.IsUndef)
      {
        TInstantOperationVisualizer Visualizer;
        CopyToClipboard(R.Octal);
      }
      Changed = false;
    }
    else if (Action == PasteAction)
    {
      UnicodeString S;
      if (TextFromClipboard(S))
      {
        Text = S;
      }
      // trigger on change event, even if no change actually occured to
      // allow parent form to visualize feedback of an action
      DoChange();
      Changed = false;
    }
    else
    {
      Handled = false;
    }

    if (Changed)
    {
      Rights = R;
      ForceUpdate();
      DoChange();
    }
  }
}
//---------------------------------------------------------------------------
void __fastcall TRightsFrame::RightsActionsUpdate(TBasicAction *Action,
      bool &Handled)
{
  TRights R = Rights;

  Handled = true;
  if (Action == NoRightsAction)
  {
    // explicitly enable as action gets disabled, if its shortcut is pressed,
    // while the form does not have a focus and OnExecute handler denies the execution
    NoRightsAction->Enabled = true;
    NoRightsAction->Checked = !R.IsUndef && (R.NumberSet == TRights::rfNo);
  }
  else if (Action == DefaultRightsAction)
  {
    DefaultRightsAction->Enabled = true;
    DefaultRightsAction->Checked = !R.IsUndef && (R.NumberSet == TRights::rfDefault);
  }
  else if (Action == AllRightsAction)
  {
    AllRightsAction->Enabled = true;
    AllRightsAction->Checked = !R.IsUndef && (R.NumberSet == TRights::rfAll);
  }
  else if (Action == LeaveRightsAsIsAction)
  {
    LeaveRightsAsIsAction->Enabled = true;
    LeaveRightsAsIsAction->Visible = R.AllowUndef;
    LeaveRightsAsIsAction->Checked = (R.NumberSet == TRights::rfNo) &&
      (R.NumberUnset == TRights::rfNo);
  }
  else if (Action == CopyTextAction)
  {
    CopyTextAction->Enabled = !R.IsUndef;
  }
  else if (Action == CopyOctalAction)
  {
    CopyOctalAction->Enabled = !R.IsUndef;
  }
  else if (Action == PasteAction)
  {
    PasteAction->Enabled = IsFormatInClipboard(CF_TEXT);
  }
  else
  {
    Handled = false;
  }
}
//---------------------------------------------------------------------------
void __fastcall TRightsFrame::CreateParams(TCreateParams & Params)
{
  TFrame::CreateParams(Params);
  if (FPopup)
  {
    Params.Style |= WS_BORDER;
  }
}
//---------------------------------------------------------------------------
void __fastcall TRightsFrame::CreateWnd()
{
  if (FPopup)
  {
    Width += 2 * GetSystemMetrics(SM_CXBORDER);
    Height += 2 * GetSystemMetrics(SM_CYBORDER);
  }
  TFrame::CreateWnd();
}
//---------------------------------------------------------------------------
void __fastcall TRightsFrame::SetPopup(bool value)
{
  if (Popup != value)
  {
    FPopup = value;
    Visible = !FPopup;
  }
}
//---------------------------------------------------------------------------
void __fastcall TRightsFrame::DoCloseUp()
{
  Hide();

  if (FDefaultButton != NULL)
  {
    FDefaultButton->Default = true;
    FDefaultButton = NULL;
  }

  if (FCancelButton != NULL)
  {
    FCancelButton->Cancel = true;
    FCancelButton = NULL;
  }
}
//---------------------------------------------------------------------------
void __fastcall TRightsFrame::DoExit()
{
  // this is bit hack to make frame hide when ComboEdit button is pressed while
  // from is poped-up already.
  if (FPopup && !IsAncestor(Screen->ActiveControl, FPopupParent))
  {
    DoCloseUp();
  }
  TFrame::DoExit();
}
//---------------------------------------------------------------------------
bool __fastcall TRightsFrame::IsAncestor(TControl * Control, TControl * Ancestor)
{
  while ((Control != NULL) &&
         (Control->Parent != Ancestor))
  {
    Control = Control->Parent;
  }
  return (Control != NULL);
}
//---------------------------------------------------------------------------
void __fastcall TRightsFrame::WMContextMenu(TWMContextMenu & Message)
{
  FPopingContextMenu = true;
  try
  {
    TFrame::Dispatch(&Message);
  }
  __finally
  {
    FPopingContextMenu = false;
  }
}
//---------------------------------------------------------------------------
void __fastcall TRightsFrame::CMDialogKey(TCMDialogKey & Message)
{
  if (FPopup && Visible &&
      ((Message.CharCode == VK_RETURN) ||
       (Message.CharCode == VK_ESCAPE)) &&
      KeyDataToShiftState(Message.KeyData).Empty())
  {
    CloseUp();
    Message.Result = 1;
  }
  else
  {
    TFrame::Dispatch(&Message);
  }
}
//---------------------------------------------------------------------------
void __fastcall TRightsFrame::CMCancelMode(TCMCancelMode & Message)
{
  if (FPopup && Visible && !FPopingContextMenu &&
      ((Message.Sender == NULL) ||
       (!IsAncestor(Message.Sender, this) &&
        !IsAncestor(Message.Sender, FPopupParent) &&
        (Message.Sender != this))))
  {
    CloseUp();
  }
  TFrame::Dispatch(&Message);
}
//---------------------------------------------------------------------------
void __fastcall TRightsFrame::Dispatch(void * Message)
{
  TMessage & AMessage = *static_cast<TMessage *>(Message);

  switch (AMessage.Msg)
  {
    case CM_CANCELMODE:
      CMCancelMode(*(TCMCancelMode *)Message);
      break;

    case CM_DIALOGKEY:
      CMDialogKey(*(TCMDialogKey *)Message);
      break;

    case WM_CONTEXTMENU:
      WMContextMenu(*(TWMContextMenu *)Message);
      break;

    default:
      TFrame::Dispatch(Message);
      break;
  }
}
//---------------------------------------------------------------------------
void __fastcall TRightsFrame::DropDown()
{
  TCustomForm * Form = GetParentForm(this);
  // due to lack of better idea, we clear "default" and "cancel" flags of respective
  // form buttons to prevent them to handle ESC/ENTER keys.
  for (int Index = 0; Index < Form->ControlCount; Index++)
  {
    TButton * Button = dynamic_cast<TButton *>(Form->Controls[Index]);
    if (Button != NULL)
    {
      if (Button->Default)
      {
        assert(FDefaultButton == NULL);
        FDefaultButton = Button;
        Button->Default = false;
      }

      if (Button->Cancel)
      {
        assert(FCancelButton == NULL);
        FCancelButton = Button;
        Button->Cancel = false;
      }
    }
  }

  TPoint Origin(PopupParent->Left, PopupParent->Top + PopupParent->Height);
  Origin = Parent->ScreenToClient(PopupParent->Parent->ClientToScreen(Origin));
  if (Origin.x + Width > Parent->ClientWidth)
  {
    Origin.x += PopupParent->Width - Width;
  }
  Left = Origin.x;
  Top = Origin.y;
  Show();
  SetFocus();
}
//---------------------------------------------------------------------------
void __fastcall TRightsFrame::CloseUp()
{
  assert(FPopup);
  assert(Visible);
  if (!Focused())
  {
    // this can happen only if called from on-click handler of drop down button
    DoCloseUp();
  }
  FPopupParent->SetFocus();
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TRightsFrame::GetText()
{
  UnicodeString Result = Rights.Text;
  if (AllowAddXToDirectories && DirectoriesXEffective() && AddXToDirectories)
  {
    Result = FORMAT(L"%s (%s)", (Result, FAddXToDirectoriesSuffix));
  }
  return Result;
}
//---------------------------------------------------------------------------
void __fastcall TRightsFrame::SetText(UnicodeString value)
{
  if (Text != value)
  {
    UnicodeString RightsStr = value;

    int P = RightsStr.LowerCase().Pos(FAddXToDirectoriesSuffix);
    bool AAddXToDirectories = (P > 0);
    if (AAddXToDirectories)
    {
      RightsStr.Delete(P, FAddXToDirectoriesSuffix.Length());
    }
    RightsStr = DeleteChar(DeleteChar(RightsStr, L'('), L')').Trim();
    TRights R = Rights;
    if (((RightsStr.Length() == 3) || (RightsStr.Length() == 4)) &&
        IsNumber(RightsStr))
    {
      R.Octal = RightsStr;
    }
    else
    {
      R.Text = RightsStr;
    }

    Rights = R;
    AddXToDirectories = AAddXToDirectories;
  }
}
//---------------------------------------------------------------------------
void __fastcall TRightsFrame::RightsPopupPopup(TObject * /*Sender*/)
{
  if (!HasFocus())
  {
    SetFocus();
  }
}
//---------------------------------------------------------------------------
void __fastcall TRightsFrame::FrameContextPopup(TObject * Sender,
  TPoint & MousePos, bool & Handled)
{
  MenuPopup(Sender, MousePos, Handled);
}
//---------------------------------------------------------------------------
