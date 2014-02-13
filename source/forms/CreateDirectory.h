//----------------------------------------------------------------------------
#ifndef CreateDirectoryH
#define CreateDirectoryH
//----------------------------------------------------------------------------
#include "Rights.h"
#include "RightsExt.h"
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.StdCtrls.hpp>
//----------------------------------------------------------------------------
#include <Bookmarks.h>
#include "RightsExt.h"
#include "Rights.h"
//----------------------------------------------------------------------------
class TCreateDirectoryDialog : public TForm
{
__published:
  TButton *OKBtn;
  TButton *CancelBtn;
  TEdit *DirectoryEdit;
  TLabel *EditLabel;
  TButton *HelpButton;
  TPanel *MorePanel;
  TGroupBox *AttributesGroup;
  TRightsExtFrame *RightsFrame;
  TCheckBox *SetRightsCheck;
  TCheckBox *SaveSettingsCheck;
  void __fastcall ControlChange(TObject *Sender);
  void __fastcall DirectoryEditChange(TObject *Sender);
  void __fastcall FormShow(TObject *Sender);
  void __fastcall HelpButtonClick(TObject *Sender);
  void __fastcall FormCloseQuery(TObject *Sender, bool &CanClose);

public:
  __fastcall TCreateDirectoryDialog(TComponent* AOwner);
  virtual __fastcall ~TCreateDirectoryDialog();

  bool __fastcall Execute(UnicodeString & Directory, TRemoteProperties * Properties,
    bool & SaveSettings);

protected:
  void __fastcall UpdateControls();
};
//----------------------------------------------------------------------------
#endif
