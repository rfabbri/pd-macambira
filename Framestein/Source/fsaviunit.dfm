object FsAvi: TFsAvi
  Left = 243
  Top = 106
  Width = 226
  Height = 103
  Caption = 'FsAvi'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnCreate = FormCreate
  OnDestroy = FormDestroy
  PixelsPerInch = 96
  TextHeight = 13
  object sd1: TSaveDialog
    Filter = 'AVI files|*.avi'
    Left = 8
    Top = 8
  end
end
