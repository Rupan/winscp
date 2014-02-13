//----------------------------------------------------------------------------
#ifndef SiteAdvancedH
#define SiteAdvancedH
//----------------------------------------------------------------------------
#include "ComboEdit.hpp"
#include "PasswordEdit.hpp"
#include "UpDownEdit.hpp"
#include <System.Classes.hpp>
#include <Vcl.ActnList.hpp>
#include <Vcl.ComCtrls.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ImgList.hpp>
#include <Vcl.Mask.hpp>
#include <Vcl.Menus.hpp>
#include <Vcl.StdCtrls.hpp>
//----------------------------------------------------------------------------
#include <Configuration.h>
#include <SessionData.h>
#include <PasTools.hpp>

#include "PngImageList.hpp"
//----------------------------------------------------------------------------
class TSiteAdvancedDialog : public TForm
{
__published:
  TPanel *MainPanel;
  TPageControl *PageControl;
  TTabSheet *AdvancedSheet;
  TGroupBox *ProtocolGroup;
  TLabel *Label7;
  TRadioButton *SshProt1Button;
  TRadioButton *SshProt2Button;
  TCheckBox *CompressionCheck;
  TTabSheet *EnvironmentSheet;
  TTabSheet *ScpSheet;
  TGroupBox *OtherShellOptionsGroup;
  TCheckBox *LookupUserGroupsCheck;
  TCheckBox *ClearAliasesCheck;
  TCheckBox *UnsetNationalVarsCheck;
  TCheckBox *Scp1CompatibilityCheck;
  TGroupBox *ShellGroup;
  TComboBox *ShellEdit;
  TPanel *LeftPanel;
  TTreeView *NavigationTree;
  TTabSheet *ConnSheet;
  TGroupBox *TimeoutGroup;
  TLabel *Label11;
  TLabel *Label12;
  TUpDownEdit *TimeoutEdit;
  TTabSheet *ProxySheet;
  TGroupBox *ProxyTypeGroup;
  TLabel *ProxyHostLabel;
  TLabel *ProxyPortLabel;
  TUpDownEdit *ProxyPortEdit;
  TEdit *ProxyHostEdit;
  TEdit *ProxyUsernameEdit;
  TLabel *ProxyUsernameLabel;
  TLabel *ProxyPasswordLabel;
  TPasswordEdit *ProxyPasswordEdit;
  TGroupBox *ProxySettingsGroup;
  TLabel *ProxyTelnetCommandLabel;
  TEdit *ProxyTelnetCommandEdit;
  TTabSheet *BugsSheet;
  TGroupBox *BugsGroupBox;
  TLabel *BugIgnore1Label;
  TComboBox *BugIgnore1Combo;
  TLabel *BugPlainPW1Label;
  TComboBox *BugPlainPW1Combo;
  TLabel *BugRSA1Label;
  TComboBox *BugRSA1Combo;
  TLabel *BugHMAC2Label;
  TComboBox *BugHMAC2Combo;
  TLabel *BugDeriveKey2Label;
  TComboBox *BugDeriveKey2Combo;
  TLabel *BugRSAPad2Label;
  TComboBox *BugRSAPad2Combo;
  TRadioButton *SshProt1onlyButton;
  TRadioButton *SshProt2onlyButton;
  TTabSheet *AuthSheet;
  TGroupBox *AuthenticationGroup;
  TCheckBox *AuthTISCheck;
  TCheckBox *AuthKICheck;
  TGroupBox *EncryptionGroup;
  TListBox *CipherListBox;
  TLabel *Label8;
  TCheckBox *Ssh2LegacyDESCheck;
  TButton *CipherUpButton;
  TButton *CipherDownButton;
  TGroupBox *EnvironmentGroup;
  TLabel *BugPKSessID2Label;
  TComboBox *BugPKSessID2Combo;
  TCheckBox *ProxyLocalhostCheck;
  TLabel *Label17;
  TGroupBox *PingGroup;
  TLabel *PingIntervalLabel;
  TUpDownEdit *PingIntervalSecEdit;
  TRadioButton *PingOffButton;
  TRadioButton *PingNullPacketButton;
  TRadioButton *PingDummyCommandButton;
  TCheckBox *AuthKIPasswordCheck;
  TTabSheet *DirectoriesSheet;
  TGroupBox *DirectoriesGroup;
  TLabel *LocalDirectoryLabel;
  TLabel *RemoteDirectoryLabel;
  TLabel *LocalDirectoryDescLabel;
  TDirectoryEdit *LocalDirectoryEdit;
  TEdit *RemoteDirectoryEdit;
  TCheckBox *UpdateDirectoriesCheck;
  TCheckBox *CacheDirectoriesCheck;
  TCheckBox *ResolveSymlinksCheck;
  TCheckBox *CacheDirectoryChangesCheck;
  TCheckBox *PreserveDirectoryChangesCheck;
  TGroupBox *DSTModeGroup;
  TRadioButton *DSTModeUnixCheck;
  TRadioButton *DSTModeWinCheck;
  TGroupBox *ScpLsOptionsGroup;
  TCheckBox *IgnoreLsWarningsCheck;
  TCheckBox *SCPLsFullTimeAutoCheck;
  TTabSheet *SftpSheet;
  TGroupBox *SFTPBugsGroupBox;
  TLabel *Label10;
  TLabel *Label36;
  TComboBox *SFTPBugSymlinkCombo;
  TTabSheet *KexSheet;
  TGroupBox *KexOptionsGroup;
  TLabel *Label28;
  TListBox *KexListBox;
  TButton *KexUpButton;
  TButton *KexDownButton;
  TGroupBox *KexReexchangeGroup;
  TLabel *Label31;
  TUpDownEdit *RekeyTimeEdit;
  TLabel *Label32;
  TEdit *RekeyDataEdit;
  TGroupBox *IPvGroup;
  TRadioButton *IPAutoButton;
  TRadioButton *IPv4Button;
  TRadioButton *IPv6Button;
  TLabel *BugRekey2Label;
  TComboBox *BugRekey2Combo;
  TGroupBox *SFTPProtocolGroup;
  TLabel *Label34;
  TComboBox *SFTPMaxVersionCombo;
  TComboBox *SFTPBugSignedTSCombo;
  TGroupBox *DirectoryOptionsGroup;
  TTabSheet *TunnelSheet;
  TGroupBox *TunnelSessionGroup;
  TLabel *Label6;
  TLabel *Label14;
  TLabel *Label15;
  TLabel *Label16;
  TLabel *Label18;
  TEdit *TunnelHostNameEdit;
  TEdit *TunnelUserNameEdit;
  TPasswordEdit *TunnelPasswordEdit;
  TUpDownEdit *TunnelPortNumberEdit;
  TFilenameEdit *TunnelPrivateKeyEdit;
  TLabel *Label19;
  TComboBox *ReturnVarEdit;
  TLabel *Label20;
  TGroupBox *AuthenticationParamsGroup;
  TCheckBox *AgentFwdCheck;
  TCheckBox *TunnelCheck;
  TGroupBox *TunnelOptionsGroup;
  TLabel *Label21;
  TComboBox *TunnelLocalPortNumberEdit;
  TRadioButton *DSTModeKeepCheck;
  TGroupBox *ConnectionGroup;
  TCheckBox *FtpPasvModeCheck;
  TTabSheet *RecycleBinSheet;
  TGroupBox *RecycleBinGroup;
  TLabel *RecycleBinPathLabel;
  TCheckBox *DeleteToRecycleBinCheck;
  TCheckBox *OverwrittenToRecycleBinCheck;
  TEdit *RecycleBinPathEdit;
  TLabel *EOLTypeLabel;
  TComboBox *EOLTypeCombo;
  TLabel *UtfLabel;
  TComboBox *UtfCombo;
  TLabel *TimeDifferenceLabel;
  TUpDownEdit *TimeDifferenceEdit;
  TLabel *TimeDifferenceHoursLabel;
  TUpDownEdit *TimeDifferenceMinutesEdit;
  TLabel *TimeDifferenceMinutesLabel;
  TLabel *Label9;
  TComboBox *ListingCommandEdit;
  TCheckBox *SshNoUserAuthCheck;
  TCheckBox *TryAgentCheck;
  TLabel *ProxyMethodLabel;
  TComboBox *SshProxyMethodCombo;
  TComboBox *ProxyDNSCombo;
  TComboBox *FtpProxyMethodCombo;
  TLabel *ProxyLocalCommandLabel;
  TEdit *ProxyLocalCommandEdit;
  TButton *ProxyLocalCommandBrowseButton;
  TStaticText *ProxyTelnetCommandHintText;
  TStaticText *ProxyLocalCommandHintText;
  TGroupBox *FtpPingGroup;
  TLabel *FtpPingIntervalLabel;
  TUpDownEdit *FtpPingIntervalSecEdit;
  TRadioButton *FtpPingOffButton;
  TRadioButton *FtpPingNullPacketButton;
  TRadioButton *FtpPingDummyCommandButton;
  TLabel *Label23;
  TComboBox *SftpServerEdit;
  TTabSheet *FtpSheet;
  TGroupBox *FtpGroup;
  TLabel *Label25;
  TMemo *PostLoginCommandsMemo;
  TLabel *BugMaxPkt2Label;
  TComboBox *BugMaxPkt2Combo;
  TLabel *FtpListAllLabel;
  TComboBox *FtpListAllCombo;
  TComboBox *FtpForcePasvIpCombo;
  TCheckBox *AllowScpFallbackCheck;
  TCheckBox *BufferSizeCheck;
  TCheckBox *SynchronizeBrowsingCheck;
  TGroupBox *GSSAPIGroup;
  TCheckBox *AuthGSSAPICheck3;
  TCheckBox *GSSAPIFwdTGTCheck;
  TLabel *BugIgnore2Label;
  TComboBox *BugIgnore2Combo;
  TComboBox *WebDavProxyMethodCombo;
  TLabel *Label24;
  TComboBox *FtpUseMlsdCombo;
  TLabel *FtpForcePasvIpLabel;
  TButton *OKBtn;
  TButton *CancelBtn;
  TButton *HelpButton;
  TLabel *FtpAccountLabel;
  TEdit *FtpAccountEdit;
  TLabel *PrivateKeyLabel;
  TFilenameEdit *PrivateKeyEdit;
  TGroupBox *TunnelAuthenticationParamsGroup;
  TImageList *ColorImageList;
  TButton *ColorButton;
  TTabSheet *SslSheet;
  TGroupBox *SslGroup;
  TLabel *Label1;
  TComboBox *MinTlsVersionCombo;
  TLabel *Label2;
  TComboBox *MaxTlsVersionCombo;
  TCheckBox *SslSessionReuseCheck;
  void __fastcall DataChange(TObject *Sender);
  void __fastcall FormShow(TObject *Sender);
  void __fastcall PageControlChange(TObject *Sender);
  void __fastcall AlgListBoxStartDrag(TObject *Sender,
    TDragObject *&DragObject);
  void __fastcall AlgListBoxDragOver(TObject *Sender, TObject *Source,
    int X, int Y, TDragState State, bool &Accept);
  void __fastcall AlgListBoxDragDrop(TObject *Sender, TObject *Source,
    int X, int Y);
  void __fastcall CipherButtonClick(TObject *Sender);
  void __fastcall AuthGSSAPICheck3Click(TObject *Sender);
  void __fastcall KexButtonClick(TObject *Sender);
  void __fastcall HelpButtonClick(TObject *Sender);
  void __fastcall PrivateKeyEditAfterDialog(TObject *Sender,
    UnicodeString &Name, bool &Action);
  void __fastcall FormCloseQuery(TObject *Sender, bool &CanClose);
  void __fastcall PathEditBeforeDialog(TObject *Sender, UnicodeString &Name,
    bool &Action);
  void __fastcall NavigationTreeCollapsing(TObject *Sender,
    TTreeNode *Node, bool &AllowCollapse);
  void __fastcall ProxyLocalCommandBrowseButtonClick(TObject *Sender);
  void __fastcall NavigationTreeChange(TObject * Sender,
    TTreeNode * Node);
  void __fastcall ColorButtonClick(TObject *Sender);
  void __fastcall MinTlsVersionComboChange(TObject *Sender);
  void __fastcall MaxTlsVersionComboChange(TObject *Sender);

public:
  virtual __fastcall TSiteAdvancedDialog(TComponent * AOwner, int Options);
  bool __fastcall Execute(TSessionData * SessionData);

protected:
  void __fastcall ChangePage(TTabSheet * Tab);
  virtual void __fastcall Dispatch(void * Message);
  bool __fastcall AllowAlgDrag(TListBox * AlgListBox, int X, int Y);
  void __fastcall AlgMove(TListBox * AlgListBox, int Source, int Dest);
  void __fastcall InitControls();

private:
  int NoUpdate;
  int FAlgDragSource;
  int FAlgDragDest;
  int FOptions;
  UnicodeString FBeforeDialogPath;
  TFSProtocol FFSProtocol;
  TSessionData * FSessionData;
  TColor FColor;
  std::unique_ptr<TPopupMenu> FColorPopupMenu;

  void __fastcall LoadSession();
  void __fastcall UpdateControls();
  void __fastcall SaveSession();
  void __fastcall CMDialogKey(TWMKeyDown & Message);
  void __fastcall WMHelp(TWMHelp & Message);
  int __fastcall LastSupportedFtpProxyMethod();
  bool __fastcall SupportedFtpProxyMethod(int Method);
  int __fastcall GetSupportedFtpProxyMethod(int Method);
  int __fastcall GetSupportedWebDavProxyMethod(int Method);
  TProxyMethod __fastcall GetProxyMethod();
  int __fastcall GetFtpProxyLogonType();
  void __fastcall UpdateNavigationTree();
  TSshProt __fastcall GetSshProt();
  void __fastcall SetSessionColor(TColor Color);
  void __fastcall SessionColorChange(TColor Color);
  TTlsVersion __fastcall IndexToTlsVersion(int Index);
  int __fastcall TlsVersionToIndex(TTlsVersion TlsVersion);
};
//----------------------------------------------------------------------------
#endif
