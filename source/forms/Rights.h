//---------------------------------------------------------------------------
#ifndef RightsH
#define RightsH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Buttons.hpp>

#include <RemoteFiles.h>
#include <ActnList.hpp>
#include <ImgList.hpp>
#include <Menus.hpp>
#include "GrayedCheckBox.hpp"
#include "PngImageList.hpp"
//---------------------------------------------------------------------------
class TRightsFrame : public TFrame
{
__published:
  TLabel *GroupLabel;
  TLabel *OthersLabel;
  TLabel *OwnerLabel;
  TGrayedCheckBox *OwnerReadCheck;
  TGrayedCheckBox *OwnerWriteCheck;
  TGrayedCheckBox *OwnerExecuteCheck;
  TGrayedCheckBox *GroupReadCheck;
  TGrayedCheckBox *GroupWriteCheck;
  TGrayedCheckBox *GroupExecuteCheck;
  TGrayedCheckBox *OthersReadCheck;
  TGrayedCheckBox *OthersWriteCheck;
  TGrayedCheckBox *OthersExecuteCheck;
  TCheckBox *DirectoriesXCheck;
  TSpeedButton *OwnerButton;
  TSpeedButton *GroupButton;
  TSpeedButton *OthersButton;
  TPopupMenu *RightsPopup;
  TMenuItem *Norights1;
  TMenuItem *Defaultrights1;
  TMenuItem *Allrights1;
  TMenuItem *Leaveasis1;
  TActionList *RightsActions;
  TAction *NoRightsAction;
  TAction *DefaultRightsAction;
  TAction *AllRightsAction;
  TAction *LeaveRightsAsIsAction;
  TPngImageList *RightsImages;
  TMenuItem *N1;
  TAction *CopyTextAction;
  TAction *CopyOctalAction;
  TAction *PasteAction;
  TMenuItem *CopyAsText1;
  TMenuItem *CopyAsOctal1;
  TMenuItem *Paste1;
  void __fastcall ControlChange(TObject *Sender);
  void __fastcall RightsButtonsClick(TObject *Sender);
  void __fastcall RightsActionsExecute(TBasicAction *Action,
          bool &Handled);
  void __fastcall RightsActionsUpdate(TBasicAction *Action, bool &Handled);
  void __fastcall RightsPopupPopup(TObject *Sender);
  void __fastcall FrameContextPopup(TObject *Sender, TPoint &MousePos,
          bool &Handled);
private:
  bool FAllowAddXToDirectories;
  TNotifyEvent FOnChange;
  bool FPopup;
  TWinControl * FPopupParent;
  TButton * FDefaultButton;
  TButton * FCancelButton;
  bool FPopingContextMenu;
  UnicodeString FAddXToDirectoriesSuffix;
  bool FInitialized;

  void __fastcall CycleRights(int Group);
  bool __fastcall GetAddXToDirectories();
  bool __fastcall GetAllowUndef();
  TCheckBox * __fastcall GetChecks(TRights::TRight Right);
  TRights __fastcall GetRights();
  TRights::TState __fastcall GetStates(TRights::TRight Right);
  void __fastcall SetAddXToDirectories(bool value);
  void __fastcall SetAllowAddXToDirectories(bool value);
  void __fastcall SetAllowUndef(bool value);
  void __fastcall SetRights(const TRights & value);
  void __fastcall SetStates(TRights::TRight Right, TRights::TState value);
  UnicodeString __fastcall GetText();
  void __fastcall SetText(UnicodeString value);

public:
  virtual __fastcall ~TRightsFrame();
  __fastcall TRightsFrame(TComponent* Owner);
  void __fastcall DropDown();
  void __fastcall CloseUp();

  __property bool AddXToDirectories = { read = GetAddXToDirectories, write = SetAddXToDirectories };
  __property bool AllowAddXToDirectories = { read = FAllowAddXToDirectories, write = SetAllowAddXToDirectories };
  __property bool AllowUndef = { read = GetAllowUndef, write = SetAllowUndef };
  __property TCheckBox * Checks[TRights::TRight Right] = { read = GetChecks };
  __property TNotifyEvent OnChange = { read = FOnChange, write = FOnChange };
  __property TRights Rights = { read = GetRights, write = SetRights };
  __property UnicodeString Text = { read = GetText, write = SetText };
  __property bool Popup = { read = FPopup, write = SetPopup };
  __property TWinControl * PopupParent = { read = FPopupParent, write = FPopupParent };

protected:
  void __fastcall DoChange();
  virtual void __fastcall UpdateControls();
  virtual void __fastcall SetEnabled(bool Value);
  virtual void __fastcall ForceUpdate();
  virtual void __fastcall CreateParams(TCreateParams & Params);
  virtual void __fastcall CreateWnd();
  virtual void __fastcall Dispatch(void * Message);
  void __fastcall CMCancelMode(TCMCancelMode & Message);
  void __fastcall CMDialogKey(TCMDialogKey & Message);
  void __fastcall WMContextMenu(TWMContextMenu & Message);
  bool __fastcall IsAncestor(TControl * Control, TControl * Ancestor);
  DYNAMIC void __fastcall DoExit();
  virtual void __fastcall SetPopup(bool value);
  void __fastcall DoCloseUp();
  bool __fastcall HasFocus();
  bool __fastcall DirectoriesXEffective();

  __property TRights::TState States[TRights::TRight Right] = { read = GetStates, write = SetStates };
};
//---------------------------------------------------------------------------
#endif
