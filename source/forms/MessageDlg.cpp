//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include <Consts.hpp>
#include <GUITools.h>

#include <Common.h>
#include <VCLCommon.h>
#include <CoreMain.h>
#include <WinInterface.h>
#include <Tools.h>
#include <TextsWin.h>
#include <TextsCore.h>
#include <Vcl.Imaging.pngimage.hpp>
#include <StrUtils.hpp>
#include <PasTools.hpp>
#include <Math.hpp>
#include <vssym32.h>
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
class TMessageButton : public TButton
{
public:
  __fastcall TMessageButton(TComponent * Owner);
protected:
  virtual void __fastcall Dispatch(void * Message);
private:
  void __fastcall WMGetDlgCode(TWMGetDlgCode & Message);
};
//---------------------------------------------------------------------------
__fastcall TMessageButton::TMessageButton(TComponent * Owner) :
  TButton(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TMessageButton::Dispatch(void * Message)
{
  TMessage * M = reinterpret_cast<TMessage*>(Message);
  if (M->Msg == WM_GETDLGCODE)
  {
    WMGetDlgCode(*((TWMGetDlgCode *)Message));
  }
  else
  {
    TButton::Dispatch(Message);
  }
}
//---------------------------------------------------------------------------
void __fastcall TMessageButton::WMGetDlgCode(TWMGetDlgCode & Message)
{
  TButton::Dispatch(&Message);
  // WORKAROUND
  // Windows default handler returns DLGC_WANTARROWS for split buttons,
  // what prevent left/right keys from being used for focusing next/previous buttons/controls.
  // Overrwide that. Though note that we need to pass the up/down keys back to button
  // to allow drop down, see TMessageForm::CMDialogKey
  Message.Result = Message.Result & ~DLGC_WANTARROWS;
}
//---------------------------------------------------------------------------
class TMessageForm : public TForm
{
public:
  static TForm * __fastcall Create(const UnicodeString & Msg, TStrings * MoreMessages,
    TMsgDlgType DlgType, unsigned int Answers,
    const TQueryButtonAlias * Aliases, unsigned int AliasesCount,
    unsigned int TimeoutAnswer, TButton ** TimeoutButton, const UnicodeString & ImageName,
    const UnicodeString & NeverAskAgainCaption);

protected:
  __fastcall TMessageForm(TComponent * AOwner);
  virtual __fastcall ~TMessageForm();

  DYNAMIC void __fastcall KeyDown(Word & Key, TShiftState Shift);
  DYNAMIC void __fastcall KeyUp(Word & Key, TShiftState Shift);
  UnicodeString __fastcall GetFormText();
  UnicodeString __fastcall GetReportText();
  UnicodeString __fastcall NormalizeNewLines(UnicodeString Text);
  virtual void __fastcall CreateParams(TCreateParams & Params);
  DYNAMIC void __fastcall DoShow();
  virtual void __fastcall Dispatch(void * Message);
  void __fastcall MenuItemClick(TObject * Sender);
  void __fastcall ButtonDropDownClick(TObject * Sender);
  void __fastcall UpdateForShiftStateTimer(TObject * Sender);

private:
  typedef std::map<unsigned int, TButton *> TAnswerButtons;

  UnicodeString MessageText;
  TMemo * MessageMemo;
  TShiftState FShiftState;
  TTimer * FUpdateForShiftStateTimer;
  TForm * FDummyForm;

  void __fastcall HelpButtonClick(TObject * Sender);
  void __fastcall ReportButtonClick(TObject * Sender);
  void __fastcall CMDialogKey(TWMKeyDown & Message);
  void __fastcall UpdateForShiftState();
  TButton * __fastcall CreateButton(
    UnicodeString Name, UnicodeString Caption, unsigned int Answer,
    TNotifyEvent OnClick, bool IsTimeoutButton,
    int GroupWith, TShiftState GrouppedShiftState,
    TAnswerButtons & AnswerButtons, bool HasMoreMessages, int & ButtonWidth);
};
//---------------------------------------------------------------------------
__fastcall TMessageForm::TMessageForm(TComponent * AOwner) : TForm(AOwner, 0)
{
  MessageMemo = NULL;
  FUpdateForShiftStateTimer = NULL;
  Position = poOwnerFormCenter;
  UseSystemSettingsPre(this);
  FDummyForm = new TForm(this);
  UseSystemSettings(FDummyForm);
}
//---------------------------------------------------------------------------
__fastcall TMessageForm::~TMessageForm()
{
  SAFE_DESTROY(FDummyForm);
  SAFE_DESTROY(FUpdateForShiftStateTimer);
}
//---------------------------------------------------------------------------
void __fastcall TMessageForm::HelpButtonClick(TObject * /*Sender*/)
{
  if (HelpKeyword != HELP_NONE)
  {
    FormHelp(this);
  }
  else
  {
    MessageWithNoHelp(GetReportText());
  }
}
//---------------------------------------------------------------------------
void __fastcall TMessageForm::ReportButtonClick(TObject * /*Sender*/)
{
  UnicodeString Url =
    FMTLOAD(ERROR_REPORT_URL,
      (EncodeUrlString(GetReportText()), Configuration->ProductVersion,
       IntToHex(__int64(GUIConfiguration->Locale), 4)));

  OpenBrowser(Url);
}
//---------------------------------------------------------------------------
void __fastcall TMessageForm::UpdateForShiftState()
{

  TShiftState ShiftState =
    KeyboardStateToShiftState() *
    (TShiftState() << ssShift << ssCtrl << ssAlt);

  if (FShiftState != ShiftState)
  {
    FShiftState = ShiftState;

    for (int ComponentIndex = 0; ComponentIndex < ComponentCount - 1; ComponentIndex++)
    {
      TButton * Button = dynamic_cast<TButton*>(Components[ComponentIndex]);
      if ((Button != NULL) && (Button->DropDownMenu != NULL))
      {
        TMenuItem * MenuItems = Button->DropDownMenu->Items;
        for (int ItemIndex = 0; ItemIndex < MenuItems->Count; ItemIndex++)
        {
          TMenuItem * Item = MenuItems->Items[ItemIndex];
          TShiftState GrouppedShiftState(Item->Tag >> 16);
          if (Item->Enabled &&
              ((ShiftState.Empty() && Item->Default) ||
               (!ShiftState.Empty() && (ShiftState == GrouppedShiftState))))
          {
            int From = 1;
            Button->Caption = CopyToChars(Item->Caption, From, L"\t", false);
            Button->ModalResult = Item->Tag & 0xFFFF;
            assert(Button->OnClick == NULL);
            assert(Item->OnClick == MenuItemClick);
            break;
          }
        }
      }
    }
  }
}
//---------------------------------------------------------------------------
void __fastcall TMessageForm::KeyUp(Word & Key, TShiftState Shift)
{

  UpdateForShiftState();

  TForm::KeyUp(Key, Shift);
}
//---------------------------------------------------------------------------
void __fastcall TMessageForm::KeyDown(Word & Key, TShiftState Shift)
{
  if (Shift.Contains(ssCtrl) && (Key == L'C'))
  {
    TInstantOperationVisualizer Visualizer;
    CopyToClipboard(GetFormText());
  }
  else
  {
    if (!Shift.Contains(ssCtrl))
    {
      for (int ComponentIndex = 0; ComponentIndex < ComponentCount - 1; ComponentIndex++)
      {
        TButton * Button = dynamic_cast<TButton*>(Components[ComponentIndex]);
        if ((Button != NULL) && (Button->DropDownMenu != NULL))
        {
          TMenuItem * MenuItems = Button->DropDownMenu->Items;
          for (int ItemIndex = 0; ItemIndex < MenuItems->Count; ItemIndex++)
          {
            TMenuItem * Item = MenuItems->Items[ItemIndex];
            if (IsAccel(Key, MenuItems->Items[ItemIndex]->Caption))
            {
              Item->OnClick(Item);
              Key = 0;
              break;
            }
          }
        }

        if (Key == 0)
        {
          break;
        }
      }
    }

    UpdateForShiftState();

    TForm::KeyDown(Key, Shift);
  }
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TMessageForm::NormalizeNewLines(UnicodeString Text)
{
  Text = ReplaceStr(Text, L"\r", L"");
  Text = ReplaceStr(Text, L"\n", L"\r\n");
  return Text;
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TMessageForm::GetFormText()
{
  UnicodeString DividerLine, ButtonCaptions;

  DividerLine = UnicodeString::StringOfChar(L'-', 27) + sLineBreak;
  for (int i = 0; i < ComponentCount - 1; i++)
  {
    if (dynamic_cast<TButton*>(Components[i]) != NULL)
    {
      ButtonCaptions += dynamic_cast<TButton*>(Components[i])->Caption +
        UnicodeString::StringOfChar(L' ', 3);
    }
  }
  ButtonCaptions = ReplaceStr(ButtonCaptions, L"&", L"");
  UnicodeString MoreMessages;
  if (MessageMemo != NULL)
  {
    MoreMessages = MessageMemo->Text + DividerLine;
  }
  UnicodeString MessageCaption = NormalizeNewLines(MessageText);
  UnicodeString Result = FORMAT(L"%s%s%s%s%s%s%s%s%s%s%s", (DividerLine, Caption, sLineBreak,
    DividerLine, MessageCaption, sLineBreak, DividerLine, MoreMessages,
    ButtonCaptions, sLineBreak, DividerLine));
  return Result;
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TMessageForm::GetReportText()
{
  UnicodeString Text = MessageText;
  if (MessageMemo != NULL)
  {
    Text += L"\n" + MessageMemo->Text;
  }
  Text = NormalizeNewLines(Text);
  UnicodeString ReportErrorText = NormalizeNewLines(FMTLOAD(REPORT_ERROR, (L"")));
  Text = ReplaceStr(Text, ReportErrorText, L"");
  Text = Trim(Text);
  return Text;
}
//---------------------------------------------------------------------------
void __fastcall TMessageForm::CMDialogKey(TWMKeyDown & Message)
{
  // this gets used in WinInterface.cpp SetTimeoutEvents
  if (OnKeyDown != NULL)
  {
    OnKeyDown(this, Message.CharCode, KeyDataToShiftState(Message.KeyData));
  }

  if (Message.CharCode == VK_MENU)
  {
    bool AnyButtonWithGrouppedCommandsWithShiftState = false;
    for (int ComponentIndex = 0; ComponentIndex < ComponentCount - 1; ComponentIndex++)
    {
      TButton * Button = dynamic_cast<TButton*>(Components[ComponentIndex]);
      if ((Button != NULL) && (Button->DropDownMenu != NULL))
      {
        // we should check if there are any commands with shift state,
        // but it's bit overkill
        AnyButtonWithGrouppedCommandsWithShiftState = true;
        break;
      }
    }

    // this is to make Alt only alter button meaning (if there is any
    // alternable button) and not popup system menu
    if (AnyButtonWithGrouppedCommandsWithShiftState)
    {
      Message.Result = 1;
      UpdateForShiftState();
    }
    else
    {
      TForm::Dispatch(&Message);
    }
  }
  else if ((Message.CharCode == VK_UP) || (Message.CharCode == VK_DOWN))
  {
    // WORKAROUND
    // noop to make up/down be passed back to button to allow drop down,
    // see TMessageButton::WMGetDlgCode
  }
  else
  {
    TForm::Dispatch(&Message);
  }
}
//---------------------------------------------------------------------------
void __fastcall TMessageForm::Dispatch(void * Message)
{
  TMessage * M = reinterpret_cast<TMessage*>(Message);
  if (M->Msg == CM_DIALOGKEY)
  {
    CMDialogKey(*((TWMKeyDown *)Message));
  }
  else
  {
    TForm::Dispatch(Message);
  }
}
//---------------------------------------------------------------------------
void __fastcall TMessageForm::CreateParams(TCreateParams & Params)
{
  TForm::CreateParams(Params);
  if ((Screen != NULL) && (Screen->ActiveForm != NULL) &&
      Screen->ActiveForm->HandleAllocated())
  {
    Params.WndParent = Screen->ActiveForm->Handle;
  }
}
//---------------------------------------------------------------------------
void __fastcall TMessageForm::DoShow()
{
  UseSystemSettingsPost(this);

  TForm::DoShow();
}
//---------------------------------------------------------------------------
void __fastcall TMessageForm::MenuItemClick(TObject * Sender)
{
  TMenuItem * Item = NOT_NULL(dynamic_cast<TMenuItem *>(Sender));
  ModalResult = (Item->Tag & 0xFFFF);
}
//---------------------------------------------------------------------------
void __fastcall TMessageForm::UpdateForShiftStateTimer(TObject * /*Sender*/)
{
  // this is needed to reflect shift state, even when we do not have a keyboard
  // focus, what happens when drop down menu is popped up
  UpdateForShiftState();
}
//---------------------------------------------------------------------------
void __fastcall TMessageForm::ButtonDropDownClick(TObject * /*Sender*/)
{
  // as optimization, do not waste time running timer, unless
  // user pops up drop down menu. we do not have a way to stop timer, once
  // it closes, but functionaly is does not matter
  if (FUpdateForShiftStateTimer == NULL)
  {
    FUpdateForShiftStateTimer = new TTimer(this);
    FUpdateForShiftStateTimer->Interval = 50;
    FUpdateForShiftStateTimer->OnTimer = UpdateForShiftStateTimer;
  }
}
//---------------------------------------------------------------------------
const ResourceString * Captions[] = { &_SMsgDlgWarning, &_SMsgDlgError, &_SMsgDlgInformation,
  &_SMsgDlgConfirm, NULL };
const wchar_t * IconIDs[] = { IDI_EXCLAMATION, IDI_HAND, IDI_ASTERISK,
  IDI_QUESTION, NULL };
const int mcHorzMargin = 10;
const int mcVertMargin = 13;
const int mcHorzSpacing = 12;
const int mcButtonVertMargin = 7;
const int mcButtonSpacing = 5;
 // includes mcVertMargin
const int mcMoreMessageHeight = 86;
// approximately what Windows Vista task dialogs use,
// actually they probably has fixed width
const int mcMaxDialogWidth = 340;
const int mcMinDialogWidth = 310;
const int mcMinDialogwithMoreMessagesWidth = 400;
//---------------------------------------------------------------------------
static UnicodeString __fastcall GetKeyNameStr(int Key)
{
  wchar_t Buf[MAX_PATH];
  LONG VirtualKey = MapVirtualKey(Key, MAPVK_VK_TO_VSC);
  VirtualKey <<= 16;
  if (GetKeyNameText(VirtualKey, Buf, LENOF(Buf)) > 0)
  {
    NULL_TERMINATE(Buf);
  }
  else
  {
    Buf[0] = L'\0';
  }
  return Buf;
}
//---------------------------------------------------------------------------
TButton * __fastcall TMessageForm::CreateButton(
  UnicodeString Name, UnicodeString Caption, unsigned int Answer,
  TNotifyEvent OnClick, bool IsTimeoutButton,
  int GroupWith, TShiftState GrouppedShiftState,
  TAnswerButtons & AnswerButtons, bool HasMoreMessages, int & ButtonWidth)
{
  UnicodeString MeasureCaption = Caption;
  if (IsTimeoutButton)
  {
    MeasureCaption = FMTLOAD(TIMEOUT_BUTTON, (MeasureCaption, 99));
  }

  TRect TextRect;
  DrawText(Canvas->Handle,
    UnicodeString(MeasureCaption).c_str(), -1,
    &TextRect, DT_CALCRECT | DT_LEFT | DT_SINGLELINE |
    DrawTextBiDiModeFlagsReadingOnly());
  int CurButtonWidth = TextRect.Right - TextRect.Left + ScaleByTextHeightRunTime(this, 16);

  TButton * Button = NULL;

  if (SupportsSplitButton() &&
      (GroupWith >= 0) &&
      ALWAYS_TRUE(AnswerButtons.find(GroupWith) != AnswerButtons.end()))
  {
    TButton * GroupWithButton = AnswerButtons[GroupWith];

    if (GroupWithButton->DropDownMenu == NULL)
    {
      GroupWithButton->Style = TCustomButton::bsSplitButton;
      GroupWithButton->DropDownMenu = new TPopupMenu(this);
      // cannot handle subitems with shift state,
      // if the button has its own handler
      // (though it may not be the case still here)
      assert(GroupWithButton->OnClick == NULL);

      TMenuItem * Item = new TMenuItem(GroupWithButton->DropDownMenu);
      GroupWithButton->DropDownMenu->Items->Add(Item);
      GroupWithButton->OnDropDownClick = ButtonDropDownClick;

      Item->Caption = GroupWithButton->Caption;
      Item->OnClick = MenuItemClick;
      assert(GroupWithButton->ModalResult <= 0xFFFF);
      Item->Tag = GroupWithButton->ModalResult;
      Item->Default = true;
    }

    TMenuItem * Item = new TMenuItem(GroupWithButton->DropDownMenu);
    GroupWithButton->DropDownMenu->Items->Add(Item);

    // See ShortCutToText in Vcl.Menus.pas
    if (GrouppedShiftState == (TShiftState() << ssAlt))
    {
      Caption = Caption + L"\t" + GetKeyNameStr(VK_MENU);
    }
    else if (GrouppedShiftState == (TShiftState() << ssCtrl))
    {
      Caption = Caption + L"\t" + GetKeyNameStr(VK_CONTROL);
    }
    else if (GrouppedShiftState == (TShiftState() << ssShift))
    {
      Caption = Caption + L"\t" + GetKeyNameStr(VK_SHIFT);
    }
    else
    {
      // do not support combined shift states yet
      assert(GrouppedShiftState == TShiftState());
    }

    Item->Caption = Caption;
    if (OnClick != NULL)
    {
      Item->OnClick = OnClick;
    }
    else
    {
      Item->OnClick = MenuItemClick;
      assert((Answer <= 0xFFFF) && (GrouppedShiftState.ToInt() <= 0xFFFF));
      Item->Tag = Answer + (GrouppedShiftState.ToInt() << 16);
    }

    // Hard-coded drop down button width (do not know how to ask for system width).
    // Also we do not update the max button width for the default groupped
    // button caption. We just blindly hope that captions of advanced commands
    // are always longer than the caption of simple default command
    CurButtonWidth += ScaleByTextHeightRunTime(this, 15);
  }
  else
  {
    Button = new TMessageButton(this);

    Button->Name = Name;
    Button->Parent = this;
    Button->Caption = Caption;
    // Scale buttons using regular font, so that they are as large as buttons
    // on other dialogs (note that they are still higher than Windows Task dialog
    // buttons)
    Button->Height = ScaleByTextHeightRunTime(FDummyForm, Button->Height);
    Button->Width = ScaleByTextHeightRunTime(FDummyForm, Button->Width);

    if (OnClick != NULL)
    {
      Button->OnClick = OnClick;
    }
    else
    {
      Button->ModalResult = Answer;
    }

    if (HasMoreMessages)
    {
      Button->Anchors = TAnchors() << akBottom << akLeft;
    }

    // never shrink buttons below their default width
    if (Button->Width > CurButtonWidth)
    {
      CurButtonWidth = Button->Width;
    }
  }

  if (CurButtonWidth > ButtonWidth)
  {
    ButtonWidth = CurButtonWidth;
  }

  return Button;
}
//---------------------------------------------------------------------------
void __fastcall AnswerNameAndCaption(
  unsigned int Answer, UnicodeString & Name, UnicodeString & Caption)
{
  switch (Answer)
  {
    case qaYes:
      Caption = LoadStr(_SMsgDlgYes.Identifier);
      Name = L"Yes";
      break;

    case qaNo:
      Caption = LoadStr(_SMsgDlgNo.Identifier);
      Name = L"No";
      break;

    case qaOK:
      Caption = LoadStr(_SMsgDlgOK.Identifier);
      Name = L"OK";
      break;

    case qaCancel:
      Caption = LoadStr(_SMsgDlgCancel.Identifier);
      Name = L"Cancel";
      break;

    case qaAbort:
      Caption = LoadStr(_SMsgDlgAbort.Identifier);
      Name = L"Abort";
      break;

    case qaRetry:
      Caption = LoadStr(_SMsgDlgRetry.Identifier);
      Name = L"Retry";
      break;

    case qaIgnore:
      Caption = LoadStr(_SMsgDlgIgnore.Identifier);
      Name = L"Ignore";
      break;

    // Own variant to avoid accelerator conflict with "Abort" button.
    // Note that as of now, ALL_BUTTON is never actually used,
    // because qaAll is always aliased
    case qaAll:
      Caption = LoadStr(ALL_BUTTON);
      Name = L"All";
      break;

    case qaNoToAll:
      Caption = LoadStr(_SMsgDlgNoToAll.Identifier);
      Name = L"NoToAll";
      break;

    // Own variant to avoid accelerator conflict with "Abort" button.
    case qaYesToAll:
      Caption = LoadStr(YES_TO_ALL_BUTTON);
      Name = L"YesToAll";
      break;

    case qaHelp:
      Caption = LoadStr(_SMsgDlgHelp.Identifier);
      Name = L"Help";
      break;

    case qaSkip:
      Caption = LoadStr(SKIP_BUTTON);
      Name = L"Skip";
      break;

    case qaReport:
      Caption = LoadStr(REPORT_BUTTON);
      Name = L"Report";
      break;

    default:
      FAIL;
      throw Exception(L"Undefined answer");
  }
}
//---------------------------------------------------------------------------
static int __fastcall CalculateWidthOnCanvas(UnicodeString Text, void * Arg)
{
  TCanvas * Canvas = static_cast<TCanvas *>(Arg);
  return Canvas->TextWidth(Text);
}
//---------------------------------------------------------------------------
TForm * __fastcall TMessageForm::Create(const UnicodeString & Msg,
  TStrings * MoreMessages, TMsgDlgType DlgType, unsigned int Answers,
  const TQueryButtonAlias * Aliases, unsigned int AliasesCount,
  unsigned int TimeoutAnswer, TButton ** TimeoutButton, const UnicodeString & ImageName,
  const UnicodeString & NeverAskAgainCaption)
{
  unsigned int DefaultAnswer;
  if (FLAGSET(Answers, qaOK))
  {
    DefaultAnswer = qaOK;
  }
  else if (FLAGSET(Answers, qaYes))
  {
    DefaultAnswer = qaYes;
  }
  else
  {
    DefaultAnswer = qaRetry;
  }

  unsigned int CancelAnswer = ::CancelAnswer(Answers);

  if (TimeoutButton != NULL)
  {
    *TimeoutButton = NULL;
  }

  TColor MainInstructionColor = Graphics::clNone;
  HFONT MainInstructionFont = 0;
  HFONT InstructionFont = 0;
  HTHEME Theme = OpenThemeData(0, L"TEXTSTYLE");
  if (Theme != NULL)
  {
    LOGFONT AFont;
    COLORREF AColor;

    memset(&AFont, sizeof(AFont), 0);
    if (GetThemeFont(Theme, NULL, TEXT_MAININSTRUCTION, 0, TMT_FONT, &AFont) == S_OK)
    {
      MainInstructionFont = CreateFontIndirect(&AFont);
    }
    if (GetThemeColor(Theme, TEXT_MAININSTRUCTION, 0, TMT_TEXTCOLOR, &AColor) == S_OK)
    {
      MainInstructionColor = (TColor)AColor;
    }

    memset(&AFont, sizeof(AFont), 0);
    if (GetThemeFont(Theme, NULL, TEXT_INSTRUCTION, 0, TMT_FONT, &AFont) == S_OK)
    {
      InstructionFont = CreateFontIndirect(&AFont);
    }

    CloseThemeData(Theme);
  }

  TMessageForm * Result = SafeFormCreate<TMessageForm>();
  if (InstructionFont != 0)
  {
    Result->Font->Handle = InstructionFont;
  }
  else
  {
    Result->Font->Assign(Screen->MessageFont);
  }

  Configuration->Usage->Set(L"ThemeMessageFontSize", Result->Font->Size);

  // make sure we consider sizes of the monitor,
  // that is set in DoFormWindowProc(CM_SHOWINGCHANGED) later.
  Forms::TMonitor * Monitor = FormMonitor(Result);

  Result->BiDiMode = Application->BiDiMode;
  Result->BorderStyle = bsDialog;
  Result->Canvas->Font = Result->Font;
  Result->KeyPreview = true;
  int HorzMargin = ScaleByTextHeightRunTime(Result, mcHorzMargin);
  int VertMargin = ScaleByTextHeightRunTime(Result, mcVertMargin);
  int HorzSpacing = ScaleByTextHeightRunTime(Result, mcHorzSpacing);
  int ButtonVertMargin = ScaleByTextHeightRunTime(Result, mcButtonVertMargin);
  int ButtonWidth = -1;
  int ButtonHeight = -1;
  std::vector<TButton *> ButtonControls;
  TAnswerButtons AnswerButtons;
  for (unsigned int Answer = qaFirst; Answer <= qaLast; Answer = Answer << 1)
  {
    if (FLAGSET(Answers, Answer))
    {
      assert(Answer != mrCancel);
      UnicodeString Caption;
      UnicodeString Name;

      AnswerNameAndCaption(Answer, Name, Caption);

      TNotifyEvent OnClick = NULL;
      int GroupWith = -1;
      TShiftState GrouppedShiftState;
      if (Aliases != NULL)
      {
        for (unsigned int i = 0; i < AliasesCount; i++)
        {
          if (Answer == Aliases[i].Button)
          {
            if (!Aliases[i].Alias.IsEmpty())
            {
              Caption = Aliases[i].Alias;
            }
            OnClick = Aliases[i].OnClick;
            GroupWith = Aliases[i].GroupWith;
            GrouppedShiftState = Aliases[i].GrouppedShiftState;
            assert((OnClick == NULL) || (GrouppedShiftState == TShiftState()));
            break;
          }
        }
      }

      // we hope that all grouped-with buttons are for answer with greater
      // value that the answer to be grouped with
      if (GroupWith >= 0)
      {
        if (ALWAYS_FALSE(GroupWith >= static_cast<int>(Answer)) ||
            ALWAYS_FALSE(Answer == TimeoutAnswer) &&
            ALWAYS_FALSE(Answer == DefaultAnswer) &&
            ALWAYS_FALSE(Answer == CancelAnswer))
        {
          GroupWith = -1;
        }
      }

      bool IsTimeoutButton = (TimeoutButton != NULL) && (Answer == TimeoutAnswer);

      if (Answer == qaHelp)
      {
        assert(OnClick == NULL);
        OnClick = Result->HelpButtonClick;
      }

      if (Answer == qaReport)
      {
        assert(OnClick == NULL);
        OnClick = Result->ReportButtonClick;
      }

      TButton * Button = Result->CreateButton(
        Name, Caption, Answer,
        OnClick, IsTimeoutButton, GroupWith, GrouppedShiftState,
        AnswerButtons, (MoreMessages != NULL), ButtonWidth);

      if (Button != NULL)
      {
        ButtonControls.push_back(Button);

        Button->Default = (Answer == DefaultAnswer);
        Button->Cancel = (Answer == CancelAnswer);
        if (ButtonHeight < 0)
        {
          ButtonHeight = Button->Height;
        }
        assert(ButtonHeight == Button->Height);

        AnswerButtons.insert(TAnswerButtons::value_type(Answer, Button));

        if (IsTimeoutButton)
        {
          *TimeoutButton = Button;
        }
      }
    }
  }

  int NeverAskAgainWidth = 0;
  if (!NeverAskAgainCaption.IsEmpty())
  {
    NeverAskAgainWidth =
      ScaleByTextHeightRunTime(Result, 16) + // checkbox
      Result->Canvas->TextWidth(NeverAskAgainCaption) +
      ScaleByTextHeightRunTime(Result, 16); // margin
  }

  int ButtonSpacing = ScaleByTextHeightRunTime(Result, mcButtonSpacing);
  int ButtonGroupWidth = NeverAskAgainWidth;
  if (!ButtonControls.empty())
  {
    ButtonGroupWidth += ButtonWidth * ButtonControls.size() +
      ButtonSpacing * (ButtonControls.size() - 1);
  }

  int IconWidth = 0;
  const wchar_t * IconID = IconIDs[DlgType];
  bool HasIcon = (IconID != NULL) || !ImageName.IsEmpty();
  if (HasIcon)
  {
    IconWidth = 32 + HorzSpacing;
  }

  assert((ButtonHeight > 0) && (ButtonWidth > 0));

  int MaxTextWidth = ScaleByTextHeightRunTime(Result, mcMaxDialogWidth);
  // if the dialog would be wide anyway (overwrite confirmation on Windows XP),
  // to fit the buttons, do not restrict the text
  if (MaxTextWidth < ButtonGroupWidth - IconWidth)
  {
    MaxTextWidth = ButtonGroupWidth - IconWidth;
  }

  TPanel * Panel = new TPanel(Result);
  Panel->Name = L"Panel";
  Panel->Parent = Result;
  Panel->Color = clWindow;
  Panel->ParentBackground = false;
  Panel->Anchors = TAnchors() << akLeft << akRight << akTop;
  Panel->BevelOuter = bvNone;
  Panel->BevelKind = bkNone;
  Panel->Caption = L"";

  UnicodeString BodyMsg = Msg;
  UnicodeString MainMsg;
  if (ExtractMainInstructions(BodyMsg, MainMsg))
  {
    Result->MessageText = MainMsg + BodyMsg;
    BodyMsg = BodyMsg.TrimLeft();
  }
  else
  {
    Result->MessageText = BodyMsg;
  }

  ApplyTabs(Result->MessageText, L' ', NULL, NULL);

  // Windows XP (not sure about Vista) does not support Hair space.
  // For Windows XP, we still keep the existing hack by using hard-coded spaces
  // in resource string
  if (CheckWin32Version(6, 1))
  {
    // Have to be padding with spaces (the smallest space defined, hair space = 1px),
    // as tabs actually do not tab, just expand to 8 spaces.
    // Otherwise we would have to do custom drawing
    // (using GetTabbedTextExtent and TabbedTextOut)
    const wchar_t HairSpace = L'\x200A';
    ApplyTabs(BodyMsg, HairSpace, CalculateWidthOnCanvas, Result->Canvas);
  }

  assert(MainMsg.Pos(L"\t") == 0);

  int IconTextWidth = -1;
  int IconTextHeight = 0;
  int ALeft = IconWidth + HorzMargin;

  for (int MessageIndex = 0; MessageIndex <= 1; MessageIndex++)
  {
    UnicodeString LabelMsg;
    UnicodeString LabelName;
    TColor LabelColor = Graphics::clNone;
    HFONT LabelFont = 0;
    switch (MessageIndex)
    {
      case 0:
        LabelMsg = MainMsg;
        LabelName = L"MainMessage";
        LabelColor = MainInstructionColor;
        LabelFont = MainInstructionFont;
        break;

      case 1:
        LabelMsg = BodyMsg;
        LabelName = L"Message";
        break;

      default:
        FAIL;
        break;
    }

    if (!LabelMsg.IsEmpty())
    {
      TLabel * Message = new TLabel(Panel);
      Message->Name = LabelName;
      Message->Parent = Panel;
      Message->WordWrap = true;
      Message->Caption = LabelMsg;
      Message->BiDiMode = Result->BiDiMode;
      // added to show & as & for messages containing !& pattern of custom commands
      // (suppose that we actually never want to use & as accel in message text)
      Message->ShowAccelChar = false;
      if (LabelFont != 0)
      {
        Message->Font->Handle = LabelFont;
        if (ALWAYS_TRUE(LabelFont == MainInstructionFont))
        {
          Configuration->Usage->Set(L"ThemeMainInstructionFontSize", Message->Font->Size);
        }
      }
      if (LabelColor != Graphics::clNone)
      {
        Message->Font->Color = LabelColor;
      }

      TRect TextRect;
      SetRect(&TextRect, 0, 0, MaxTextWidth, 0);
      DrawText(Message->Canvas->Handle, LabelMsg.c_str(), LabelMsg.Length() + 1, &TextRect,
        DT_EXPANDTABS | DT_CALCRECT | DT_WORDBREAK | DT_NOPREFIX |
        Result->DrawTextBiDiModeFlagsReadingOnly());
      int MaxWidth = Monitor->Width - HorzMargin * 2 - IconWidth - 30;
      if (TextRect.right > MaxWidth)
      {
        // this will truncate the text, we should implement something smarter eventually
        TextRect.right = MaxWidth;
      }

      IconTextWidth = Max(IconTextWidth, IconWidth + TextRect.Right);

      if (IconTextHeight > 0)
      {
        IconTextHeight += VertMargin;
      }
      Message->SetBounds(ALeft, VertMargin + IconTextHeight, TextRect.Right, TextRect.Bottom);
      IconTextHeight += TextRect.Bottom;
    }
  }

  assert((IconTextWidth > 0) && (IconTextHeight > 0));

  if (HasIcon && (IconTextHeight < 32))
  {
    IconTextHeight = 32;
  }

  int MoreMessageHeight = (MoreMessages != NULL ?
    ScaleByTextHeightRunTime(Result, mcMoreMessageHeight) : 0);

  Panel->SetBounds(0, 0, Result->ClientWidth, VertMargin + IconTextHeight + VertMargin + MoreMessageHeight);

  if (MoreMessages != NULL)
  {
    TMemo * MessageMemo = new TMemo(Panel);
    MessageMemo->Name = L"MessageMemo";
    MessageMemo->Parent = Panel;
    MessageMemo->ReadOnly = true;
    MessageMemo->WantReturns = False;
    MessageMemo->ScrollBars = ssVertical;
    MessageMemo->Anchors = TAnchors() << akLeft << akRight << akTop;
    MessageMemo->Lines->Text = MoreMessages->Text;

    Result->MessageMemo = MessageMemo;
  }

  int MinClientWidth =
    ScaleByTextHeightRunTime(Result,
      (MoreMessages != NULL) ? mcMinDialogwithMoreMessagesWidth : mcMinDialogWidth);
  int AClientWidth =
    Max(
      (IconTextWidth > ButtonGroupWidth ? IconTextWidth : ButtonGroupWidth) +
        HorzMargin * 2,
      MinClientWidth);

  Result->ClientWidth = AClientWidth;
  Result->ClientHeight =
    Panel->Height + ButtonVertMargin + ButtonHeight + ButtonVertMargin;
  Result->Left = (Monitor->Width / 2) - (Result->Width / 2);
  Result->Top = (Monitor->Height / 2) - (Result->Height / 2);
  if (DlgType != mtCustom)
  {
    Result->Caption = LoadResourceString(Captions[DlgType]);
  }
  else
  {
    Result->Caption = Application->Title;
  }

  if ((IconID != NULL) || !ImageName.IsEmpty())
  {
    TImage * Image = new TImage(Panel);
    Image->Name = L"Image";
    Image->Parent = Panel;
    if (!ImageName.IsEmpty())
    {
      LoadResourceImage(Image, ImageName);
    }
    else
    {
      Image->Picture->Icon->Handle = LoadIcon(0, IconID);
    }
    Image->SetBounds(HorzMargin, VertMargin, 32, 32);
  }

  if (Result->MessageMemo != NULL)
  {
    Result->MessageMemo->SetBounds(
      ALeft,
      Panel->Height - MoreMessageHeight,
      Result->ClientWidth - ALeft - HorzMargin,
      MoreMessageHeight - VertMargin);
  }

  int ButtonTop = Panel->Height + ButtonVertMargin;
  int X = Result->ClientWidth - ButtonGroupWidth + NeverAskAgainWidth - HorzMargin;
  for (unsigned int i = 0; i < ButtonControls.size(); i++)
  {
    ButtonControls[i]->SetBounds(
      X, ButtonTop, ButtonWidth, ButtonControls[i]->Height);
    X += ButtonWidth + ButtonSpacing;
  }

  if (!NeverAskAgainCaption.IsEmpty() &&
      !ButtonControls.empty())
  {
    TCheckBox * NeverAskAgainCheck = new TCheckBox(Result);
    NeverAskAgainCheck->Name = L"NeverAskAgainCheck";
    NeverAskAgainCheck->Parent = Result;
    NeverAskAgainCheck->Caption = NeverAskAgainCaption;
    NeverAskAgainCheck->Anchors = TAnchors() << akBottom << akLeft;

    TButton * FirstButton = ButtonControls[0];
    int NeverAskAgainHeight = ScaleByTextHeightRunTime(Result, NeverAskAgainCheck->Height);
    int NeverAskAgainTop = FirstButton->Top + ((FirstButton->Height - NeverAskAgainHeight) / 2);
    int NeverAskAgainLeft = HorzMargin;

    NeverAskAgainCheck->SetBounds(
      NeverAskAgainLeft, NeverAskAgainTop, NeverAskAgainWidth, NeverAskAgainHeight);
  }

  return Result;
}
//---------------------------------------------------------------------------
TForm * __fastcall CreateMoreMessageDialog(const UnicodeString & Msg,
  TStrings * MoreMessages, TMsgDlgType DlgType, unsigned int Answers,
  const TQueryButtonAlias * Aliases, unsigned int AliasesCount,
  unsigned int TimeoutAnswer, TButton ** TimeoutButton, const UnicodeString & ImageName,
  const UnicodeString & NeverAskAgainCaption)
{
  return TMessageForm::Create(Msg, MoreMessages, DlgType, Answers,
    Aliases, AliasesCount, TimeoutAnswer, TimeoutButton, ImageName,
    NeverAskAgainCaption);
}
