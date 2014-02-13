object PropertiesDialog: TPropertiesDialog
  Left = 416
  Top = 133
  HelpType = htKeyword
  HelpKeyword = 'ui_properties'
  BorderIcons = [biSystemMenu, biMinimize, biMaximize, biHelp]
  BorderStyle = bsDialog
  Caption = 'Properties'
  ClientHeight = 416
  ClientWidth = 357
  Color = clBtnFace
  ParentFont = True
  OldCreateOrder = True
  Position = poOwnerFormCenter
  OnCloseQuery = FormCloseQuery
  OnShow = FormShow
  DesignSize = (
    357
    416)
  PixelsPerInch = 96
  TextHeight = 13
  object OkButton: TButton
    Left = 99
    Top = 386
    Width = 75
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = 'OK'
    Default = True
    ModalResult = 1
    TabOrder = 1
  end
  object CancelButton: TButton
    Left = 187
    Top = 386
    Width = 75
    Height = 25
    Anchors = [akRight, akBottom]
    Cancel = True
    Caption = 'Cancel'
    ModalResult = 2
    TabOrder = 2
  end
  object PageControl: TPageControl
    Left = 5
    Top = 5
    Width = 346
    Height = 374
    ActivePage = CommonSheet
    Anchors = [akLeft, akTop, akRight, akBottom]
    TabOrder = 0
    OnChange = PageControlChange
    object CommonSheet: TTabSheet
      Caption = 'Common'
      DesignSize = (
        338
        346)
      object FilesIconImage: TImage
        Left = 11
        Top = 8
        Width = 32
        Height = 32
        Picture.Data = {
          07544269746D617076020000424D760200000000000076000000280000002000
          000020000000010004000000000000020000C40E0000C40E0000100000000000
          00000402040084828400C4C2C400FC02FC00FCFEFC0000C681000012E7000000
          770000175400004EC50000F61200007700000000000000000000001300000000
          0000330000000000000000000000033333333312222222222222222222220333
          3333331444444444444444444442000333333314444444444444444444420203
          3333331444444444444444444442020003333314444444444444444444420202
          0333331444444444444444444442020203333314444444444444444444420202
          0333331444444444444444444442020203333314444444444444444444420202
          0333331444444444444444444442020203333314444444444444444444420202
          0333331444444444444444444442020203333314444444444444444444420202
          0333331444444444444444444442020203333314444444444444444444420202
          0333331444444444444444444442020203333314444444444444444444420202
          0333331444444444444444444442020203333314444444444444444444420202
          0333331444444444444444444442020203333314444444444444441000000202
          0333331444444444444444144210420203333314444444444444441421000002
          0333331444444444444444121042104203333314444444444444441104210000
          0333331444444444444444101210421033333311111111111111111411042103
          3333333314444444444444441012103333333333111111111111111114110333
          3333333333144444444444444410333333333333331111111111111111133333
          3333}
        Transparent = True
      end
      object Bevel1: TBevel
        Left = 8
        Top = 47
        Width = 320
        Height = 9
        Anchors = [akLeft, akTop, akRight]
        Shape = bsTopLine
      end
      object Label1: TLabel
        Left = 8
        Top = 58
        Width = 44
        Height = 13
        Caption = 'Location:'
      end
      object LocationLabel: TPathLabel
        Left = 85
        Top = 58
        Width = 243
        Height = 13
        UnixPath = True
        IndentHorizontal = 0
        IndentVertical = 0
        Align = alNone
        Anchors = [akLeft, akTop, akRight]
        AutoSize = False
      end
      object FileLabel: TLabel
        Left = 85
        Top = 18
        Width = 241
        Height = 13
        AutoSize = False
        Caption = 'FileLabel'
      end
      object Label2: TLabel
        Left = 8
        Top = 80
        Width = 23
        Height = 13
        Caption = 'Size:'
      end
      object SizeLabel: TLabel
        Left = 85
        Top = 80
        Width = 160
        Height = 13
        Anchors = [akLeft, akTop, akRight]
        AutoSize = False
        Caption = 'SizeLabel'
      end
      object LinksToLabelLabel: TLabel
        Left = 8
        Top = 102
        Width = 40
        Height = 13
        Caption = 'Links to:'
      end
      object LinksToLabel: TPathLabel
        Left = 88
        Top = 102
        Width = 240
        Height = 13
        UnixPath = True
        IndentHorizontal = 0
        IndentVertical = 0
        Align = alNone
        Anchors = [akLeft, akTop, akRight]
        AutoSize = False
      end
      object Bevel2: TBevel
        Left = 8
        Top = 125
        Width = 320
        Height = 9
        Anchors = [akLeft, akTop, akRight]
        Shape = bsTopLine
      end
      object Label3: TLabel
        Left = 8
        Top = 203
        Width = 59
        Height = 13
        Caption = 'Permissions:'
        FocusControl = RightsFrame
      end
      object Bevel3: TBevel
        Left = 8
        Top = 193
        Width = 320
        Height = 9
        Anchors = [akLeft, akTop, akRight]
        Shape = bsTopLine
      end
      object Label4: TLabel
        Left = 8
        Top = 138
        Width = 33
        Height = 13
        Caption = 'Group:'
        FocusControl = GroupComboBox
      end
      object Label5: TLabel
        Left = 8
        Top = 166
        Width = 36
        Height = 13
        Caption = 'Owner:'
        FocusControl = OwnerComboBox
      end
      object FileIconImage: TImage
        Left = 11
        Top = 8
        Width = 32
        Height = 32
      end
      object RecursiveBevel: TBevel
        Left = 8
        Top = 312
        Width = 320
        Height = 9
        Anchors = [akLeft, akTop, akRight]
        Shape = bsTopLine
      end
      inline RightsFrame: TRightsExtFrame
        Left = 84
        Top = 200
        Width = 244
        Height = 109
        TabOrder = 3
      end
      object GroupComboBox: TComboBox
        Left = 85
        Top = 135
        Width = 161
        Height = 21
        DropDownCount = 16
        MaxLength = 50
        TabOrder = 1
        Text = 'GroupComboBox'
        OnChange = ControlChange
        OnExit = GroupComboBoxExit
      end
      object OwnerComboBox: TComboBox
        Left = 85
        Top = 163
        Width = 161
        Height = 21
        DropDownCount = 16
        MaxLength = 50
        TabOrder = 2
        Text = 'OwnerComboBox'
        OnChange = ControlChange
        OnExit = OwnerComboBoxExit
      end
      object RecursiveCheck: TCheckBox
        Left = 12
        Top = 322
        Width = 317
        Height = 17
        Anchors = [akLeft, akTop, akRight]
        Caption = 'Set group, owner and permissions &recursively'
        TabOrder = 4
        OnClick = ControlChange
      end
      object CalculateSizeButton: TButton
        Left = 248
        Top = 72
        Width = 80
        Height = 25
        Anchors = [akTop, akRight]
        Caption = 'C&alculate'
        TabOrder = 0
        OnClick = CalculateSizeButtonClick
      end
    end
    object ChecksumSheet: TTabSheet
      Caption = 'Checksum'
      ImageIndex = 1
      DesignSize = (
        338
        346)
      object Label6: TLabel
        Left = 13
        Top = 13
        Width = 49
        Height = 13
        Caption = '&Algorithm:'
        FocusControl = ChecksumAlgEdit
      end
      object ChecksumView: TListView
        Left = 6
        Top = 40
        Width = 326
        Height = 297
        Anchors = [akLeft, akTop, akRight, akBottom]
        Columns = <
          item
            Caption = 'File'
            Width = -2
            WidthType = (
              -2)
          end
          item
            Caption = 'Checksum'
            Width = -2
            WidthType = (
              -2)
          end>
        ColumnClick = False
        DoubleBuffered = True
        MultiSelect = True
        ReadOnly = True
        RowSelect = True
        ParentDoubleBuffered = False
        PopupMenu = ListViewMenu
        TabOrder = 2
        ViewStyle = vsReport
        OnContextPopup = ChecksumViewContextPopup
      end
      object ChecksumAlgEdit: TComboBox
        Left = 80
        Top = 9
        Width = 121
        Height = 21
        AutoComplete = False
        Anchors = [akLeft, akTop, akRight]
        MaxLength = 250
        TabOrder = 0
        OnChange = ChecksumAlgEditChange
        Items.Strings = (
          'md5'
          'sha1'
          'sha224'
          'sha256'
          'sha384'
          'sha512'
          'crc32')
      end
      object ChecksumButton: TButton
        Left = 208
        Top = 7
        Width = 122
        Height = 25
        Anchors = [akTop, akRight]
        Caption = '&Calculate checksum'
        TabOrder = 1
        OnClick = ChecksumButtonClick
      end
      object ChecksumGroup: TGroupBox
        Left = 6
        Top = 40
        Width = 326
        Height = 41
        Anchors = [akLeft, akRight, akBottom]
        Caption = 'Checksum'
        TabOrder = 3
        DesignSize = (
          326
          41)
        object ChecksumEdit: TEdit
          Left = 10
          Top = 18
          Width = 306
          Height = 19
          TabStop = False
          Anchors = [akLeft, akTop, akRight]
          BorderStyle = bsNone
          Color = clBtnFace
          ReadOnly = True
          TabOrder = 0
          Text = 'ChecksumEdit'
        end
      end
    end
  end
  object HelpButton: TButton
    Left = 275
    Top = 386
    Width = 75
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = '&Help'
    TabOrder = 3
    OnClick = HelpButtonClick
  end
  object ListViewMenu: TPopupMenu
    Left = 16
    Top = 384
    object Copy: TMenuItem
      Caption = '&Copy'
      OnClick = CopyClick
    end
  end
end
