inherited fsbrowser: Tfsbrowser
  Left = 266
  Top = 377
  Width = 237
  Height = 257
  Caption = 'fsbrowser'
  FormStyle = fsNormal
  OldCreateOrder = True
  PixelsPerInch = 96
  TextHeight = 13
  inherited d1: TDXDraw [1]
    Width = 229
    Height = 230
    TabOrder = 2
  end
  inherited PanelMute: TPanel [2]
    Width = 229
    Height = 230
  end
  object br: TWebBrowser [3]
    Left = 0
    Top = 0
    Width = 229
    Height = 230
    Align = alClient
    TabOrder = 0
    OnBeforeNavigate2 = brBeforeNavigate2
    ControlData = {
      4C000000AB170000C51700000000000000000000000000000000000000000000
      000000004C000000000000000000000001000000E0D057007335CF11AE690800
      2B2E126208000000000000004C0000000114020000000000C000000000000046
      8000000000000000000000000000000000000000000000000000000000000000
      00000000000000000100000000000000000000000000000000000000}
  end
end
