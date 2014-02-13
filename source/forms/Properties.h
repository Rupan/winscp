//----------------------------------------------------------------------------
#ifndef PropertiesH
#define PropertiesH
//----------------------------------------------------------------------------
#include "PathLabel.hpp"
#include "Rights.h"
#include "RightsExt.h"
#include <System.Classes.hpp>
#include <Vcl.ComCtrls.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.Graphics.hpp>
#include <Vcl.Menus.hpp>
#include <Vcl.StdCtrls.hpp>
//----------------------------------------------------------------------------
#include "Rights.h"
#include "RightsExt.h"
#include <Menus.hpp>
//----------------------------------------------------------------------------
struct TCalculateSizeStats;
//----------------------------------------------------------------------------
class TPropertiesDialog : public TForm
{
__published:
  TButton *OkButton;
  TButton *CancelButton;
  TPageControl *PageControl;
  TTabSheet *CommonSheet;
  TImage *FilesIconImage;
  TBevel *Bevel1;
  TLabel *FileLabel;
  TLabel *Label1;
  TPathLabel *LocationLabel;
  TLabel *Label2;
  TLabel *SizeLabel;
  TLabel *LinksToLabelLabel;
  TPathLabel *LinksToLabel;
  TBevel *Bevel2;
  TLabel *Label3;
  TBevel *Bevel3;
  TLabel *Label4;
  TComboBox *GroupComboBox;
  TLabel *Label5;
  TComboBox *OwnerComboBox;
  TImage *FileIconImage;
  TBevel *RecursiveBevel;
  TCheckBox *RecursiveCheck;
  TButton *CalculateSizeButton;
  TRightsExtFrame *RightsFrame;
  TButton *HelpButton;
  TTabSheet *ChecksumSheet;
  TListView *ChecksumView;
  TLabel *Label6;
  TComboBox *ChecksumAlgEdit;
  TButton *ChecksumButton;
  TGroupBox *ChecksumGroup;
  TEdit *ChecksumEdit;
  TPopupMenu *ListViewMenu;
  TMenuItem *Copy;
  void __fastcall ControlChange(TObject *Sender);
  void __fastcall FormCloseQuery(TObject *Sender, bool &CanClose);
  void __fastcall CalculateSizeButtonClick(TObject *Sender);
  void __fastcall HelpButtonClick(TObject *Sender);
  void __fastcall ChecksumButtonClick(TObject *Sender);
  void __fastcall PageControlChange(TObject *Sender);
  void __fastcall ChecksumAlgEditChange(TObject *Sender);
  void __fastcall CopyClick(TObject *Sender);
  void __fastcall ChecksumViewContextPopup(TObject *Sender,
          TPoint &MousePos, bool &Handled);
  void __fastcall GroupComboBoxExit(TObject *Sender);
  void __fastcall OwnerComboBoxExit(TObject *Sender);
  void __fastcall FormShow(TObject *Sender);

private:
  int FAllowedChanges;
  bool FUserGroupByID;
  TStrings * FFileList;
  const TRemoteTokenList * FGroupList;
  const TRemoteTokenList * FUserList;
  TRemoteProperties FOrigProperties;
  TImageList * FShellImageList;
  bool FMultiple;
  bool FAllowCalculateStats;
  bool FStatsNotCalculated;
  TCalculateSizeEvent FOnCalculateSize;
  TCalculateChecksumEvent FOnCalculateChecksum;
  bool FChecksumLoaded;
  bool FMultipleChecksum;

  void __fastcall CalculateChecksum();
  void __fastcall NeedChecksum();
  bool __fastcall ChecksumSupported();
  void __fastcall ResetChecksum();
  void __fastcall CalculatedChecksum(
    const UnicodeString & FileName, const UnicodeString & Alg, const UnicodeString & Hash);
  void __fastcall SetFileProperties(const TRemoteProperties & value);
  TRemoteProperties __fastcall GetFileProperties();

protected:
  void __fastcall LoadInfo();
  void __fastcall LoadRemoteTokens(TComboBox * ComboBox, const TRemoteTokenList * List);
  UnicodeString __fastcall LoadRemoteToken(const TRemoteToken & Token);
  void __fastcall LoadRemoteToken(TComboBox * ComboBox, bool Valid, const TRemoteToken & Token);
  TRemoteToken __fastcall StoreRemoteToken(const TRemoteToken & Orig,
    UnicodeString Text, int Message, const TRemoteTokenList * List);
  void __fastcall StoreRemoteToken(TComboBox * ComboBox,
    int ChangeFlag, TValidProperty PropertyFlag, const TRemoteToken & Orig,
    TRemoteToken & Token, int Message, const TRemoteTokenList * List,
    TRemoteProperties & Properties);
  void __fastcall StoreRemoteToken(unsigned int ID, const UnicodeString & Text,
    const TRemoteTokenList * List, TRemoteToken & Result);
  void __fastcall ResolveRemoteToken(
    const TRemoteToken & Orig, int Message, TComboBox * ComboBox,
    const TRemoteTokenList * List);
  void __fastcall UpdateControls();
  void __fastcall LoadStats(__int64 FilesSize, const TCalculateSizeStats & Stats);

public:
  virtual __fastcall TPropertiesDialog(TComponent * AOwner,
    TStrings * FileList, const UnicodeString Directory,
    const TRemoteTokenList * GroupList, const TRemoteTokenList * UserList,
    int AllowedChanges, bool UserGroupByID, TCalculateSizeEvent OnCalculateSize,
    TCalculateChecksumEvent OnCalculateChecksum);

  virtual __fastcall ~TPropertiesDialog();
  bool __fastcall Execute(TRemoteProperties & Properties);
};
//----------------------------------------------------------------------------
#endif
