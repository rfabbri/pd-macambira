object fsframe: Tfsframe
  Left = 583
  Top = 507
  Width = 277
  Height = 198
  Caption = 'fs.frame'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  PopupMenu = PopupMenu1
  Position = poDefaultSizeOnly
  OnClose = FormClose
  OnCreate = FormCreate
  OnDestroy = FormDestroy
  OnMouseWheel = FormMouseWheel
  PixelsPerInch = 96
  TextHeight = 13
  object LabelFoo: TLabel
    Left = 8
    Top = 136
    Width = 44
    Height = 13
    Caption = 'LabelFoo'
    Visible = False
  end
  object PanelMute: TPanel
    Left = 0
    Top = 0
    Width = 269
    Height = 171
    Align = alClient
    BevelOuter = bvNone
    Caption = 'mute'
    Ctl3D = True
    ParentCtl3D = False
    TabOrder = 1
  end
  object d1: TDXDraw
    Left = 0
    Top = 0
    Width = 269
    Height = 171
    AutoInitialize = True
    AutoSize = False
    Color = clBtnFace
    Display.FixedBitCount = False
    Display.FixedRatio = False
    Display.FixedSize = False
    Options = [doStretch, doDirectX7Mode, doHardware, doSelectDriver]
    SurfaceHeight = 171
    SurfaceWidth = 269
    Align = alClient
    PopupMenu = PopupMenu1
    TabOrder = 0
    OnMouseDown = d1MouseDown
    OnMouseMove = d1MouseMove
    OnMouseUp = d1MouseUp
  end
  object PopupMenu1: TPopupMenu
    OnPopup = PopupMenu1Popup
    Left = 8
    Top = 8
    object MiBufferImages: TMenuItem
      Caption = 'Buffer images'
      OnClick = MiBufferImagesClick
    end
    object MiSaveFrame: TMenuItem
      Caption = 'Save this image as ..'
      OnClick = MiSaveFrameClick
    end
    object Mi11: TMenuItem
      Caption = 'zoom 1:&1'
      OnClick = Mi11Click
    end
    object Mi12: TMenuItem
      Caption = 'zoom 1:&2'
      OnClick = Mi12Click
    end
    object Mi176x144: TMenuItem
      Caption = 'zoom 176x144'
      OnClick = Mi176x144Click
    end
    object MiMute: TMenuItem
      Caption = 'Mute'
      OnClick = MiMuteClick
    end
    object MiBorders: TMenuItem
      Caption = 'Borders'
      Checked = True
      OnClick = MiBordersClick
    end
    object MiStayOnTop: TMenuItem
      Caption = 'StayOnTop'
      OnClick = MiStayOnTopClick
    end
    object MiMouseTrack: TMenuItem
      Caption = 'MouseTrack'
      OnClick = MiMouseTrackClick
    end
    object MiHideCursor: TMenuItem
      Caption = 'HideCursor'
      OnClick = MiHideCursorClick
    end
    object MiUndock: TMenuItem
      Caption = 'Undock'
      OnClick = MiUndockClick
    end
  end
  object dDib: TDXDIB
    Left = 8
    Top = 40
  end
  object dirBuffer: TScanDir
    OnHandleFile = dirBufferHandleFile
    Left = 8
    Top = 72
  end
  object opd1: TOpenPictureDialog
    Filter = 'All (*.jpg;*.bmp;*.avi)|*.jpg;*.bmp;*.avi'
    Options = [ofHideReadOnly, ofAllowMultiSelect, ofFileMustExist, ofEnableSizing]
    Left = 8
    Top = 104
  end
  object framepsh: TC2PhotoShopHost
    SerialNumber = 0
    BackColor = clBlack
    ForeColor = clBlack
    Left = 40
    Top = 8
  end
  object svd1: TSaveDialog
    Filter = '*.bmp|BMP files'
    Options = [ofOverwritePrompt, ofHideReadOnly, ofPathMustExist, ofEnableSizing]
    Left = 40
    Top = 104
  end
  object dirUse: TScanDir
    OnHandleFile = dirUseHandleFile
    Left = 40
    Top = 72
  end
end
