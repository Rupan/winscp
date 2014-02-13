//---------------------------------------------------------------------------
#ifndef ScpExplorerH
#define ScpExplorerH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <CustomDirView.hpp>
#include <CustomScpExplorer.h>
#include <CustomUnixDirView.hpp>
#include <IEListView.hpp>
#include <NortonLikeListView.hpp>
#include <UnixDirView.h>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include <Menus.hpp>
#include <ToolWin.hpp>
#include <Buttons.hpp>
#include "CustomDriveView.hpp"
#include "UnixDriveView.h"
#include "TB2Dock.hpp"
#include "TBX.hpp"
#include "TB2Item.hpp"
#include "TB2Toolbar.hpp"
#include "TBXStatusBars.hpp"
#include "TBXExtItems.hpp"
#include "TB2ExtItems.hpp"
#include "TBXToolPals.hpp"
#include "TBXLists.hpp"
#include "ThemePageControl.h"
#include "PathLabel.hpp"
//---------------------------------------------------------------------------
class TScpExplorerForm : public TCustomScpExplorerForm
{
__published:
  TTBXToolbar *MenuToolbar;
  TTBXSubmenuItem *TBXSubmenuItem18;
  TTBXItem *TBXItem107;
  TTBXItem *TBXItem108;
  TTBXItem *TBXItem109;
  TTBXItem *TBXItem110;
  TTBXItem *TBXItem111;
  TTBXItem *TBXItem112;
  TTBXSubmenuItem *TBXSubmenuItem5;
  TTBXItem *TBXItem25;
  TTBXItem *TBXItem26;
  TTBXSeparatorItem *TBXSeparatorItem7;
  TTBXItem *TBXItem30;
  TTBXItem *TBXItem31;
  TTBXItem *TBXItem32;
  TTBXItem *TBXItem33;
  TTBXItem *TBXItem34;
  TTBXItem *TBXItem35;
  TTBXItem *TBXItem36;
  TTBXSeparatorItem *TBXSeparatorItem8;
  TTBXSubmenuItem *CustomCommandsMenu;
  TTBXSubmenuItem *TBXSubmenuItem6;
  TTBXItem *TBXItem38;
  TTBXItem *TBXItem39;
  TTBXItem *TBXItem40;
  TTBXSeparatorItem *TBXSeparatorItem9;
  TTBXItem *TBXItem41;
  TTBXSubmenuItem *TBXSubmenuItem7;
  TTBXItem *TBXItem43;
  TTBXItem *TBXItem44;
  TTBXSubmenuItem *QueueSubmenuItem;
  TTBXItem *QueueEnableItem2;
  TTBXItem *TBXItem46;
  TTBXSeparatorItem *TBXSeparatorItem10;
  TTBXItem *TBXItem47;
  TTBXItem *TBXItem48;
  TTBXItem *TBXItem49;
  TTBXSeparatorItem *TBXSeparatorItem11;
  TTBXItem *TBXItem50;
  TTBXItem *TBXItem51;
  TTBXSeparatorItem *TBXSeparatorItem12;
  TTBXItem *TBXItem52;
  TTBXItem *TBXItem53;
  TTBXSeparatorItem *TBXSeparatorItem13;
  TTBXItem *TBXItem54;
  TTBXItem *TBXItem55;
  TTBXSeparatorItem *TBXSeparatorItem14;
  TTBXItem *TBXItem56;
  TTBXItem *TBXItem57;
  TTBXSubmenuItem *TBXSubmenuItem19;
  TTBXItem *TBXItem113;
  TTBXSubmenuItem *TBXSubmenuItem20;
  TTBXSeparatorItem *TBXSeparatorItem29;
  TTBXSubmenuItem *TBXSubmenuItem21;
  TTBXItem *TBXItem114;
  TTBXItem *TBXItem115;
  TTBXSubmenuItem *TBXSubmenuItem14;
  TTBXItem *TBXItem77;
  TTBXItem *TBXItem78;
  TTBXItem *TBXItem79;
  TTBXSeparatorItem *TBXSeparatorItem21;
  TTBXItem *TBXItem80;
  TTBXSeparatorItem *TBXSeparatorItem22;
  TTBXItem *TBXItem81;
  TTBXSubmenuItem *TBXSubmenuItem15;
  TTBXItem *TBXItem84;
  TTBXSeparatorItem *TBXSeparatorItem25;
  TTBXItem *TBXItem85;
  TTBXItem *TBXItem86;
  TTBXItem *TBXItem87;
  TTBXSeparatorItem *TBXSeparatorItem26;
  TTBXItem *TBXItem88;
  TTBXItem *TBXItem89;
  TTBXSubmenuItem *TBXSubmenuItem16;
  TTBXItem *TBXItem93;
  TTBXSeparatorItem *TBXSeparatorItem28;
  TTBXItem *TBXItem94;
  TTBXItem *TBXItem95;
  TTBXItem *TBXItem96;
  TTBXItem *TBXItem97;
  TTBXItem *TBXItem98;
  TTBXItem *TBXItem99;
  TTBXItem *TBXItem100;
  TTBXSubmenuItem *TBXSubmenuItem17;
  TTBXItem *TBXItem101;
  TTBXItem *TBXItem102;
  TTBXItem *TBXItem103;
  TTBXItem *TBXItem104;
  TTBXItem *TBXItem105;
  TTBXItem *TBXItem106;
  TTBXSubmenuItem *TBXSubmenuItem22;
  TTBXItem *TBXItem116;
  TTBXSeparatorItem *TBXSeparatorItem30;
  TTBXItem *TBXItem117;
  TTBXItem *TBXItem118;
  TTBXItem *TBXItem119;
  TTBXSeparatorItem *TBXSeparatorItem31;
  TTBXItem *TBXItem120;
  TTBXSeparatorItem *TBXSeparatorItem32;
  TTBXItem *TBXItem121;
  TTBXSeparatorItem *TBXSeparatorItem33;
  TTBXItem *TBXItem122;
  TTBXSeparatorItem *TBXSeparatorItem1;
  TTBXItem *TBXItem1;
  TTBXItem *TBXItem2;
  TTBXItem *TBXItem5;
  TTBXItem *TBXItem6;
  TTBXSeparatorItem *TBXSeparatorItem2;
  TTBXSubmenuItem *TBXSubmenuItem1;
  TTBXSubmenuItem *TBXSubmenuItem2;
  TTBXItem *TBXItem7;
  TTBXItem *TBXItem8;
  TTBXItem *TBXItem9;
  TTBXItem *TBXItem10;
  TTBXItem *TBXItem11;
  TTBXItem *TBXItem12;
  TTBXItem *TBXItem13;
  TTBXItem *TBXItem14;
  TTBXItem *TBXItem15;
  TTBXSeparatorItem *TBXSeparatorItem3;
  TTBXItem *TBXItem16;
  TTBXItem *TBXItem17;
  TTBXItem *TBXItem18;
  TTBXItem *TBXItem19;
  TTBXSeparatorItem *TBXSeparatorItem4;
  TTBXItem *TBXItem20;
  TTBXSeparatorItem *TBXSeparatorItem5;
  TTBXItem *TBXItem21;
  TTBXToolbar *AddressToolbar;
  TTBXLabelItem *TBXLabelItem1;
  TTBXItem *TBXItem22;
  TTBXToolbar *ButtonsToolbar;
  TTBXSubmenuItem *BackButton;
  TTBXSubmenuItem *ForwardButton;
  TTBXSeparatorItem *TBXSeparatorItem6;
  TTBXItem *TBXItem23;
  TTBXItem *TBXItem24;
  TTBXItem *TBXItem29;
  TTBXItem *TBXItem37;
  TTBXSeparatorItem *TBXSeparatorItem15;
  TTBXItem *TBXItem42;
  TTBXItem *TBXItem45;
  TTBXItem *TBXItem58;
  TTBXItem *TBXItem59;
  TTBXItem *TBXItem60;
  TTBXItem *TBXItem61;
  TTBXSeparatorItem *TBXSeparatorItem16;
  TTBXItem *TBXItem62;
  TTBXItem *TBXItem63;
  TTBXSeparatorItem *TBXSeparatorItem17;
  TTBXItem *TBXItem64;
  TTBXItem *TBXItem65;
  TTBXToolbar *SelectionToolbar;
  TTBXItem *TBXItem66;
  TTBXItem *TBXItem67;
  TTBXSeparatorItem *TBXSeparatorItem18;
  TTBXItem *TBXItem68;
  TTBXItem *TBXItem69;
  TTBXItem *TBXItem70;
  TTBXToolbar *SessionToolbar;
  TTBXItem *TBXItem123;
  TTBXSeparatorItem *TBXSeparatorItem34;
  TTBXItem *TBXItem124;
  TTBXSubmenuItem *TBXSubmenuItem23;
  TTBXItem *TBXItem125;
  TTBXToolbar *PreferencesToolbar;
  TTBXItem *TBXItem126;
  TTBXSeparatorItem *TBXSeparatorItem36;
  TTBXItem *TBXItem127;
  TTBXSubmenuItem *TBXSubmenuItem24;
  TTBXItem *TBXItem128;
  TTBXItem *TBXItem129;
  TTBXItem *TBXItem130;
  TTBXItem *TBXItem71;
  TTBXSubmenuItem *TBXSubmenuItem3;
  TTBXItem *TBXItem72;
  TTBXItem *TBXItem73;
  TTBXItem *TBXItem74;
  TTBXItem *TBXItem75;
  TTBXToolbar *SortToolbar;
  TTBXItem *TBXItem145;
  TTBXSeparatorItem *TBXSeparatorItem40;
  TTBXItem *TBXItem146;
  TTBXItem *TBXItem147;
  TTBXItem *TBXItem149;
  TTBXItem *TBXItem150;
  TTBXItem *TBXItem151;
  TTBXItem *TBXItem152;
  TTBXItem *TBXItem153;
  TTBXDock *LeftDock;
  TTBXDock *RightDock;
  TTBXDock *BottomDock;
  TTBXItem *TBXItem76;
  TTBXToolbar *UpdatesToolbar;
  TTBXSubmenuItem *TBXSubmenuItem4;
  TTBXItem *TBXItem184;
  TTBXSeparatorItem *TBXSeparatorItem46;
  TTBXItem *TBXItem180;
  TTBXItem *TBXItem187;
  TTBXItem *TBXItem181;
  TTBXItem *TBXItem182;
  TTBXItem *TBXItem185;
  TTBXSeparatorItem *TBXSeparatorItem45;
  TTBXItem *TBXItem183;
  TTBXItem *TBXItem82;
  TTBXToolbar *TransferToolbar;
  TTBXItem *TBXItem83;
  TTBXItem *TBXItem91;
  TTBXItem *TBXItem92;
  TTBXSeparatorItem *TBXSeparatorItem19;
  TTBXItem *TBXItem131;
  TTBXItem *TBXItem132;
  TTBXItem *TBXItem133;
  TTBXItem *TBXItem196;
  TTBXItem *TBXItem197;
  TTBXSeparatorItem *TBXSeparatorItem48;
  TTBXSubmenuItem *TBXSubmenuItem13;
  TTBXItem *TBXItem198;
  TTBXItem *TBXItem199;
  TTBXSubmenuItem *TBXSubmenuItem9;
  TTBXItem *TBXItem27;
  TTBXItem *TBXItem134;
  TTBXSubmenuItem *TBXSubmenuItem26;
  TTBXItem *TBXItem135;
  TTBXItem *TBXItem136;
  TTBXItem *TBXItem209;
  TTBXSeparatorItem *TBXSeparatorItem20;
  TTBXItem *TBXItem4;
  TTBXToolbar *CustomCommandsToolbar;
  TTBXItem *TBXItem28;
  TTBXColorItem *ColorMenuItem;
  TTBXSeparatorItem *TBXSeparatorItem23;
  TTBXDropDownItem *TransferDropDown;
  TTBXStringList *TransferList;
  TTBXLabelItem *TransferLabel;
  TTBXSeparatorItem *TBXSeparatorItem52;
  TTBXItem *TBXItem189;
  TTBXItem *TBXItem90;
  TTBXItem *TBXItem137;
  TTBXComboBoxItem *UnixPathComboBox;
  TTBXComboBoxItem *QueueSpeedComboBoxItem;
  TTBXItem *TBXItem138;
  TTBXSubmenuItem *TBXSubmenuItem8;
  TTBXItem *TBXItem222;
  TTBXItem *TBXItem223;
  TTBXItem *TBXItem224;
  TTBXItem *TBXItem3;
  TTBXItem *TBXItem139;
  TTBXSeparatorItem *TBXSeparatorItem53;
  TTBXItem *TBXItem230;
  TTBXSubmenuItem *TBXSubmenuItem231;
  TTBXSeparatorItem *TBXSeparatorItem24;
  TTBXLabelItem *TransferSettingsLabelItem;
  TTBXItem *TBXItem140;
  TTBXItem *TBXItem141;
  TTBXItem *TBXItem142;
  TTBXSeparatorItem *TBXSeparatorItem27;
  TTBXItem *TBXItem143;
  TTBXSeparatorItem *TBXSeparatorItem35;
  void __fastcall RemoteDirViewUpdateStatusBar(TObject *Sender,
          const TStatusFileInfo &FileInfo);
  void __fastcall UnixPathComboBoxBeginEdit(TTBEditItem *Sender,
          TTBEditItemViewer *Viewer, TEdit *EditControl);
  void __fastcall UnixPathComboBoxAcceptText(TObject *Sender,
          UnicodeString &NewText, bool &Accept);
  void __fastcall RemoteDirViewPathChange(TCustomDirView *Sender);
  void __fastcall QueueSubmenuItemPopup(TTBCustomItem *Sender,
          bool FromLink);
private:
  UnicodeString FStatusBarFileText;

  void __fastcall RemotePanelSplitterDblClick(TObject * Sender);

protected:
  virtual bool __fastcall CopyParamDialog(TTransferDirection Direction,
    TTransferType Type, bool Temp, TStrings * FileList,
    UnicodeString & TargetDirectory, TGUICopyParamType & CopyParam, bool Confirm,
    bool DragDrop);
  virtual void __fastcall RestoreFormParams();
  virtual void __fastcall RestoreParams();
  virtual void __fastcall ConfigurationChanged();
  virtual TControl * __fastcall GetComponent(Byte Component);
  virtual void __fastcall FixControlsPlacement();
  DYNAMIC void __fastcall DoShow();
  virtual void __fastcall UpdateStatusPanelText(TTBXStatusPanel * Panel);
  virtual void __fastcall UpdateRemotePathComboBox(
    TTBXComboBoxItem * RemotePathComboBox, bool TextOnly);
  UnicodeString __fastcall RemotePathComboBoxText();
  virtual void __fastcall ToolbarItemResize(TTBXCustomDropDownItem * Item, int Width);
  virtual bool __fastcall UpdateToolbarDisplayMode();

public:
  __fastcall TScpExplorerForm(TComponent* Owner);
  virtual Boolean __fastcall AllowedAction(TAction * Action, TActionAllowed Allowed);
  virtual void __fastcall StoreParams();
  virtual void __fastcall FullSynchronizeDirectories();
  virtual void __fastcall SynchronizeDirectories();
  virtual void __fastcall ChangePath(TOperationSide Side);
};
//---------------------------------------------------------------------------
#endif
