//---------------------------------------------------------------------------
#ifndef AuthenticateH
#define AuthenticateH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include "PasswordEdit.hpp"
#include "WinInterface.h"
//---------------------------------------------------------------------------
class TAuthenticateForm : public TForm
{
__published:
  TListView *LogView;
  TPanel *PasswordPanel;
  TPanel *PromptEditPanel;
  TLabel *PromptLabel1;
  TPasswordEdit *PromptEdit1;
  TPanel *BannerPanel;
  TMemo *BannerMemo;
  TCheckBox *NeverShowAgainCheck;
  TButton *BannerCloseButton;
  TButton *BannerHelpButton;
  TPanel *SavePasswordPanel;
  TCheckBox *SavePasswordCheck;
  TPanel *ButtonsPanel;
  TButton *PasswordOKButton;
  TButton *PasswordCancelButton;
  TButton *PasswordHelpButton;
  TLabel *InstructionsLabel;
  TLabel *PromptLabel2;
  TPasswordEdit *PromptEdit2;
  TPanel *SessionRememberPasswordPanel;
  TCheckBox *SessionRememberPasswordCheck;
  void __fastcall FormShow(TObject *Sender);
  void __fastcall HelpButtonClick(TObject *Sender);
  void __fastcall FormResize(TObject *Sender);

public:
  __fastcall TAuthenticateForm(TComponent * Owner);
  virtual __fastcall ~TAuthenticateForm();

  void __fastcall Init(TTerminal * Terminal);
  void __fastcall ShowAsModal();
  void __fastcall HideAsModal();
  void __fastcall Log(const UnicodeString Message);
  bool __fastcall PromptUser(TPromptKind Kind, UnicodeString Name, UnicodeString Instructions,
    TStrings * Prompts, TStrings * Results, bool ForceLog, bool StoredCredentialsTried);
  void __fastcall Banner(const UnicodeString & Banner, bool & NeverShowAgain,
    int Options);

  __property TTerminal * Terminal = { read = FTerminal };
  __property TNotifyEvent OnCancel = { read = FOnCancel, write = FOnCancel };

protected:
  void __fastcall ClearLog();
  void __fastcall AdjustControls();
  bool __fastcall Execute(UnicodeString Status, TPanel * Panel,
    TWinControl * FocusControl, TButton * DefaultButton, TButton * CancelButton,
    bool FixHeight, bool Zoom, bool ForceLog);
  virtual void __fastcall CreateParams(TCreateParams & Params);
  virtual void __fastcall Dispatch(void * AMessage);
  void __fastcall WMNCCreate(TWMNCCreate & Message);
  TLabel * __fastcall GenerateLabel(int Current, UnicodeString Caption);
  TCustomEdit * __fastcall GenerateEdit(int Current, bool Echo);
  TList * __fastcall GeneratePrompt(UnicodeString Instructions, TStrings * Prompts);
  void __fastcall DoCancel();
  void __fastcall AdjustLogView();

private:
  void * FShowAsModalStorage;
  TWinControl * FFocusControl;
  TSessionData * FSessionData;
  TTerminal * FTerminal;
  UnicodeString FStatus;
  TWinControl * FPromptParent;
  int FPromptLeft;
  int FPromptTop;
  int FPromptRight;
  int FPromptEditGap;
  int FPromptsGap;
  TNotifyEvent FOnCancel;
};
//---------------------------------------------------------------------------
#endif
