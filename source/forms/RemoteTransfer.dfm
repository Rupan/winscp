object RemoteTransferDialog: TRemoteTransferDialog
  Left = 296
  Top = 235
  HelpType = htKeyword
  HelpKeyword = 'ui_duplicate'
  BorderIcons = [biSystemMenu, biMinimize, biMaximize, biHelp]
  BorderStyle = bsDialog
  Caption = 'RemoteTransferDialog'
  ClientHeight = 179
  ClientWidth = 368
  Color = clBtnFace
  ParentFont = True
  OldCreateOrder = False
  Position = poOwnerFormCenter
  OnCloseQuery = FormCloseQuery
  OnShow = FormShow
  DesignSize = (
    368
    179)
  PixelsPerInch = 96
  TextHeight = 13
  object SymlinkGroup: TGroupBox
    Left = 8
    Top = 6
    Width = 352
    Height = 131
    Anchors = [akLeft, akTop, akRight, akBottom]
    TabOrder = 0
    DesignSize = (
      352
      131)
    object SessionLabel: TLabel
      Left = 49
      Top = 12
      Width = 74
      Height = 13
      Caption = 'Target &session:'
      FocusControl = SessionCombo
    end
    object Label2: TLabel
      Left = 49
      Top = 60
      Width = 119
      Height = 13
      Caption = 'Target remote &directory:'
      FocusControl = DirectoryEdit
    end
    object Image: TImage
      Left = 11
      Top = 15
      Width = 32
      Height = 32
      AutoSize = True
      Picture.Data = {
        0954506E67496D61676589504E470D0A1A0A0000000D49484452000000200000
        00200806000000737A7AF4000000097048597300000EC300000EC301C76FA864
        0000046D4944415478DAC5D70B4C53571807F0FF6D8BC68DF8082E668988EE99
        A823DB32B285CD644CE5E1109C060DC9C6CC00BB8A7320733E06584031BA301E
        96968A315B266EE8629468CCD4CD17E202F228886C5DE6C6433B68D79652DBD2
        D7D9EDBD5A297D785B597692F6DED3F47CE777BF7BEE775A0AFF73A35C6F5BE4
        174990E39A2C76244973E28C930628DFF036080706A1BF74ECCA6F68550E355B
        EC24E171111E0087935B222E750D40376AC1F55ED563233C00360737406FBF06
        EA11B31B4181A4960BE3342103F26A7F265F09E360B139B1E3D0C58003F666C5
        413F6AC6F0888941685D885B83DD3C8AF74E280806902BBD402A444B61B23A39
        0D9A2A00D4FA71088319CD3D833D951B972D0E09B059F223A9CA898771CC819D
        753F051C5096BD147F0D8DE00ABD0E0CA631F7E72AB51ED59B12A890009BAACE
        92039B1361303B380F74D24F431F0D19501BE0A417EFF91625249F268506D858
        718648725760C4644741DDB947DF82694F043B8FDFBAC10044E58DA4266F2574
        34804BDB53DF84C9AA1B0C40F8E54922CD4F85F69E1D85F2B37E83BD3E560D01
        2CF875D627D82D5A3B2975830164EF3B41649FAD82C61838032DD5CFE1A598D9
        E8ECB0E0AD750731635E2C27C0C4BA41F896951559895A3720ABEC38917EBE86
        0188E5A7FD064AB6E622397D01FE19B6A2BD5983F96F6E87AC232AE0E413EB86
        0BD13CAE6E30808F763710D9B634A88DB680C1DA25CF33008AC787C944D07AE9
        0EC22353F1E2F252F0F8617EC779D70D1762A0BB52B42C9A01AC2FF98EC8B6AF
        857AD48E6279A3DF40A98E7C37009400762785CE6B77D17BE729743E99031B15
        EE35C65537FE54E971F5E6A0CFBAC1003E141F21B21DE9181E0D9C812ED90B1E
        00FAB241283E940A35FA6F132C4A3988F039BE8BA1BFBAC1000E972C21B3A93E
        4E0B6A22807D09A0EA33A0EDEA203AF8191812BCE2777C71E67218CD56ECAFBF
        0C69DEBB2CA0B1741E494EA44F2836983BB0579F3DF706B07D83CE8296734A44
        2CCC44546C1EFD19CF2FA4807EDC65F9290100D4C3AB9B88A17CE2D8BE95DED0
        6E9C57A267280A8A29197060AAC7C4A5C22416507B06F2ADAB8205B0FD4000D7
        B993F0D0DDF407B4BA39589876DC67060A6B4FA36EDB6A5F80FB8128BEC73D0E
        1670F3DA6D286EF1D114B6C56362B130993916494FE1D0CE347F00EF45E60108
        901DD72D68BBA0846DCA6B58105F0E5E98EF8DABA8E6240E17AC1B0748203E17
        5C300083DE8A567A112A8CB1E8E5AD0081F7EEBC4B98721F70025F17A53F0044
        5EA7A3BCC1F931F4717B54FD067435FF8DA7979461C6B3498F8CB34BF203BE11
        BF4F05F50382C954FA7C1AF0303B047CFCDEAD81A2D3845F480674981B3046E1
        C7EFB18003C7F06D4946A800F6CAD952AC82DEF20CE6C6D740302D82732C7175
        03EAF7AC0F1D603613B45CBE0B8566313A9CA970D299E0D2BE10AD618EC55547
        71746F6668002DBD1DB7D1DBF1F497B762E6A20F8209E16EC595F5F87E5F76F0
        80E89808B4DF30237AB51CD69931214DCE028EA061FF86600191F4BF166AFAA9
        7B39AF1A9DB3429EFC410B1AF05FB47F019CF95B3FF06E46AF0000000049454E
        44AE426082}
    end
    object SessionCombo: TComboBox
      Left = 49
      Top = 28
      Width = 292
      Height = 21
      Style = csDropDownList
      Anchors = [akLeft, akTop, akRight]
      DropDownCount = 16
      MaxLength = 250
      TabOrder = 0
      OnChange = SessionComboChange
    end
    object DirectoryEdit: THistoryComboBox
      Left = 49
      Top = 76
      Width = 292
      Height = 21
      AutoComplete = False
      Anchors = [akLeft, akTop, akRight]
      MaxLength = 250
      TabOrder = 1
      OnChange = ControlChange
    end
    object NotDirectCopyCheck: TCheckBox
      Left = 55
      Top = 105
      Width = 288
      Height = 17
      Anchors = [akLeft, akTop, akRight]
      Caption = 'Duplicate via local &temporary copy'
      TabOrder = 2
      OnClick = NotDirectCopyCheckClick
    end
  end
  object OkButton: TButton
    Left = 116
    Top = 146
    Width = 75
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = 'OK'
    Default = True
    ModalResult = 1
    TabOrder = 1
  end
  object CancelButton: TButton
    Left = 200
    Top = 146
    Width = 75
    Height = 25
    Anchors = [akRight, akBottom]
    Cancel = True
    Caption = 'Cancel'
    ModalResult = 2
    TabOrder = 2
  end
  object HelpButton: TButton
    Left = 284
    Top = 146
    Width = 75
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = '&Help'
    TabOrder = 3
    OnClick = HelpButtonClick
  end
end
