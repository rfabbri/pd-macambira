object toolbar: Ttoolbar
  Left = 188
  Top = 106
  Width = 572
  Height = 496
  BorderStyle = bsSizeToolWin
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poScreenCenter
  OnClose = FormClose
  OnCreate = FormCreate
  PixelsPerInch = 96
  TextHeight = 13
  object bar: TStatusBar
    Left = 0
    Top = 450
    Width = 564
    Height = 19
    Panels = <>
    SimplePanel = False
  end
  object Panel3: TPanel
    Left = 0
    Top = 0
    Width = 564
    Height = 450
    Align = alClient
    BevelOuter = bvNone
    Caption = 'Panel3'
    TabOrder = 1
    object PageControl1: TPageControl
      Left = 0
      Top = 0
      Width = 565
      Height = 451
      ActivePage = TabSheet1
      Anchors = [akLeft, akTop, akRight, akBottom]
      TabOrder = 0
      OnChange = PageControl1Change
      object TabSheet1: TTabSheet
        Caption = 'Tools'
        object Panel2: TPanel
          Left = 0
          Top = 0
          Width = 557
          Height = 423
          Align = alClient
          BevelOuter = bvNone
          Caption = 'Panel2'
          TabOrder = 0
          object LVTools: TListView
            Left = 0
            Top = 0
            Width = 557
            Height = 423
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
            Left = 16
            Top = 8
            Width = 129
            Height = 33
            TabOrder = 1
            Visible = False
            WordWrap = False
          end
        end
      end
      object TabSheet2: TTabSheet
        Caption = 'Filters'
        ImageIndex = 1
        object Panel1: TPanel
          Left = 0
          Top = 0
          Width = 557
          Height = 423
          Align = alClient
          BevelOuter = bvNone
          Caption = 'Panel1'
          TabOrder = 0
          object LVFilters: TListView
            Left = 0
            Top = 0
            Width = 557
            Height = 423
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
            SortType = stText
            TabOrder = 0
            ViewStyle = vsList
            OnChange = LVFiltersChange
            OnCustomDrawItem = LVFiltersCustomDrawItem
            OnSelectItem = LVFiltersSelectItem
          end
        end
      end
    end
  end
  object sd: TScanDir
    OnHandleFile = sdHandleFile
    Left = 104
    Top = 7
  end
end
