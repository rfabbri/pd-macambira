object log: Tlog
  Left = 239
  Top = 39
  Width = 603
  Height = 666
  BorderStyle = bsSizeToolWin
  Caption = 'Console / Log'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poDesktopCenter
  OnCreate = FormCreate
  PixelsPerInch = 96
  TextHeight = 13
  object PanelLog: TPanel
    Left = 0
    Top = 221
    Width = 595
    Height = 418
    Align = alClient
    BevelOuter = bvNone
    TabOrder = 0
    object Panel2: TPanel
      Left = 0
      Top = 400
      Width = 595
      Height = 18
      Align = alBottom
      AutoSize = True
      BevelOuter = bvNone
      TabOrder = 0
      object ButtonClear: TButton
        Left = 75
        Top = 0
        Width = 75
        Height = 18
        Caption = 'Clear'
        TabOrder = 1
        OnClick = ButtonClearClick
      end
      object ButtonClose: TButton
        Left = 0
        Top = 0
        Width = 75
        Height = 18
        Caption = 'Close'
        Default = True
        TabOrder = 0
        OnClick = ButtonCloseClick
      end
    end
    object RELog: TRichEdit
      Left = 0
      Top = 0
      Width = 595
      Height = 400
      Align = alClient
      BorderStyle = bsNone
      Ctl3D = True
      ParentCtl3D = False
      PlainText = True
      ScrollBars = ssVertical
      TabOrder = 1
      WordWrap = False
    end
  end
  object Panel1: TPanel
    Left = 0
    Top = 203
    Width = 595
    Height = 18
    Align = alTop
    BevelOuter = bvLowered
    Caption = 'Log'
    TabOrder = 2
  end
  object PanelConsole: TPanel
    Left = 0
    Top = 18
    Width = 595
    Height = 185
    Align = alTop
    BevelOuter = bvNone
    TabOrder = 1
  end
  object Panel3: TPanel
    Left = 0
    Top = 0
    Width = 595
    Height = 18
    Align = alTop
    BevelOuter = bvLowered
    Caption = 'Console'
    TabOrder = 3
  end
end
