object toolbar: Ttoolbar
  Left = 264
  Top = 454
  Width = 567
  Height = 282
  BorderStyle = bsSizeToolWin
  Caption = 'Toolbar'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnCreate = FormCreate
  PixelsPerInch = 96
  TextHeight = 13
  object Splitter1: TSplitter
    Left = 0
    Top = 97
    Width = 559
    Height = 6
    Cursor = crVSplit
    Align = alTop
    Beveled = True
  end
  object Panel1: TPanel
    Left = 0
    Top = 103
    Width = 559
    Height = 133
    Align = alClient
    BevelOuter = bvNone
    Caption = 'Panel1'
    TabOrder = 0
    object LVFilters: TListView
      Left = 0
      Top = 0
      Width = 559
      Height = 133
      Align = alClient
      BorderStyle = bsNone
      Color = clBtnFace
      Columns = <
        item
          AutoSize = True
          Caption = 'Name'
        end
        item
          AutoSize = True
          Caption = 'Info'
        end>
      ReadOnly = True
      TabOrder = 0
      ViewStyle = vsList
      OnChange = LVFiltersChange
      OnCustomDrawItem = LVFiltersCustomDrawItem
      OnSelectItem = LVFiltersSelectItem
    end
  end
  object Panel2: TPanel
    Left = 0
    Top = 0
    Width = 559
    Height = 97
    Align = alTop
    BevelOuter = bvNone
    Caption = 'Panel2'
    TabOrder = 1
    object LVTools: TListView
      Left = 0
      Top = 0
      Width = 559
      Height = 97
      Align = alClient
      BorderStyle = bsNone
      Color = clBtnFace
      Columns = <>
      ReadOnly = True
      TabOrder = 0
      ViewStyle = vsList
      OnChange = LVFiltersChange
      OnCustomDrawItem = LVFiltersCustomDrawItem
      OnSelectItem = LVToolsSelectItem
    end
    object m1: TMemo
      Left = 144
      Top = 8
      Width = 129
      Height = 33
      TabOrder = 1
      Visible = False
      WordWrap = False
    end
  end
  object bar: TStatusBar
    Left = 0
    Top = 236
    Width = 559
    Height = 19
    Panels = <>
    SimplePanel = False
  end
end
