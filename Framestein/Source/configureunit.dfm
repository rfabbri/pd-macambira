object configure: Tconfigure
  Left = 317
  Top = 185
  Width = 443
  Height = 351
  BorderWidth = 4
  Caption = 'Configuration'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poScreenCenter
  OnCreate = FormCreate
  PixelsPerInch = 96
  TextHeight = 13
  object PageControl1: TPageControl
    Left = 0
    Top = 0
    Width = 427
    Height = 275
    ActivePage = TSConnections
    Align = alClient
    TabOrder = 0
    object TSFolders: TTabSheet
      Caption = 'Folders'
      ImageIndex = 2
      TabVisible = False
      object Label6: TLabel
        Left = 16
        Top = 36
        Width = 83
        Height = 13
        Caption = 'Framestein folder:'
      end
      object EditFSFolder: TEdit
        Left = 152
        Top = 32
        Width = 225
        Height = 21
        Ctl3D = True
        ParentCtl3D = False
        TabOrder = 0
        Text = 'E:\'
      end
      object Memo1: TMemo
        Left = 16
        Top = 200
        Width = 389
        Height = 33
        BorderStyle = bsNone
        Enabled = False
        Lines.Strings = (
          
            'Choose the folder where Framestein will find its Plugins and Fil' +
            'ters.'
          '')
        ReadOnly = True
        TabOrder = 1
      end
      object DirectoryListBox1: TDirectoryListBox
        Left = 152
        Top = 88
        Width = 225
        Height = 97
        ItemHeight = 16
        TabOrder = 2
        OnChange = DirectoryListBox1Change
      end
      object DriveComboBox1: TDriveComboBox
        Left = 152
        Top = 64
        Width = 225
        Height = 19
        TabOrder = 3
        OnChange = DriveComboBox1Change
      end
    end
    object TSConnections: TTabSheet
      Caption = 'Connections'
      object Label1: TLabel
        Left = 16
        Top = 36
        Width = 114
        Height = 13
        Caption = 'Host running Pure Data:'
      end
      object Label2: TLabel
        Left = 16
        Top = 60
        Width = 95
        Height = 13
        Caption = 'Listen to Pd on port:'
      end
      object Label3: TLabel
        Left = 16
        Top = 84
        Width = 103
        Height = 13
        Caption = 'Pd is listening on port:'
      end
      object Label4: TLabel
        Left = 16
        Top = 112
        Width = 385
        Height = 13
        Caption = 
          'NOTE: if you change the default ports, you need to modify fs.mai' +
          'n.pd accordingly.'
      end
      object Label5: TLabel
        Left = 16
        Top = 164
        Width = 194
        Height = 13
        Caption = 'Listen to Framestein connections on port:'
      end
      object EditPdHost: TEdit
        Left = 152
        Top = 32
        Width = 225
        Height = 21
        Ctl3D = True
        ParentCtl3D = False
        TabOrder = 0
        Text = 'localhost'
      end
      object EditPdReceivePort: TEdit
        Left = 152
        Top = 56
        Width = 225
        Height = 21
        Ctl3D = True
        ParentCtl3D = False
        TabOrder = 1
        Text = '6001'
      end
      object EditPdSendPort: TEdit
        Left = 152
        Top = 80
        Width = 225
        Height = 21
        Ctl3D = True
        ParentCtl3D = False
        TabOrder = 2
        Text = '6002'
      end
      object EditFsPort: TEdit
        Left = 224
        Top = 160
        Width = 153
        Height = 21
        Ctl3D = True
        ParentCtl3D = False
        TabOrder = 3
        Text = '6010'
      end
      object CBEnableFSConns: TCheckBox
        Left = 224
        Top = 184
        Width = 97
        Height = 17
        Caption = 'Enable'
        TabOrder = 4
      end
    end
    object TSGeneral: TTabSheet
      Caption = 'General'
      ImageIndex = 1
      object CBDockMain: TCheckBox
        Left = 26
        Top = 36
        Width = 377
        Height = 17
        Caption = 'Dock main window to Pd on connect'
        TabOrder = 0
      end
    end
  end
  object Panel1: TPanel
    Left = 0
    Top = 275
    Width = 427
    Height = 41
    Align = alBottom
    BevelOuter = bvNone
    TabOrder = 1
    object ButtonOk: TButton
      Left = 344
      Top = 8
      Width = 75
      Height = 25
      Caption = 'Ok'
      Default = True
      TabOrder = 0
      OnClick = ButtonOkClick
    end
    object ButtonCancel: TButton
      Left = 256
      Top = 8
      Width = 75
      Height = 25
      Cancel = True
      Caption = 'Cancel'
      TabOrder = 1
      OnClick = ButtonCancelClick
    end
  end
end
