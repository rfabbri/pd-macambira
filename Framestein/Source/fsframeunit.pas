{ Copyright (C) 2001 Juha Vehviläinen
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.}

unit fsframeunit;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs,
  Menus, Jpeg, DXDraws,
  FastDIB, fsDcAvi,
  fsformunit, ExtCtrls, DIB, Filez, ExtDlgs, C2PhotoShopHost, Buttons,
  StdCtrls;

type
  TFastDIBList = TList;

  Tfsframe = class(TFsForm)
    d1: TDXDraw;
    PopupMenu1: TPopupMenu;
    Mi11: TMenuItem;
    Mi12: TMenuItem;
    MiMute: TMenuItem;
    dDib: TDXDIB;
    MiBorders: TMenuItem;
    MiStayOnTop: TMenuItem;
    dirBuffer: TScanDir;
    PanelMute: TPanel;
    MiMouseTrack: TMenuItem;
    MiBufferImages: TMenuItem;
    opd1: TOpenPictureDialog;
    MiSaveFrame: TMenuItem;
    Mi176x144: TMenuItem;
    MiHideCursor: TMenuItem;
    framepsh: TC2PhotoShopHost;
    MiUndock: TMenuItem;
    svd1: TSaveDialog;
    dirUse: TScanDir;
    LabelFoo: TLabel;
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
    procedure Mi11Click(Sender: TObject);
    procedure Mi12Click(Sender: TObject);
    procedure MiMuteClick(Sender: TObject);
    procedure FormCreate(Sender: TObject);
    procedure FormDestroy(Sender: TObject);
    procedure MiBordersClick(Sender: TObject);
    procedure MiStayOnTopClick(Sender: TObject);
    procedure dirBufferHandleFile(const SearchRec: TSearchRec;
      const FullPath: String);
    procedure d1MouseDown(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure d1MouseMove(Sender: TObject; Shift: TShiftState; X,
      Y: Integer);
    procedure d1MouseUp(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure MiMouseTrackClick(Sender: TObject);
    procedure MiBufferImagesClick(Sender: TObject);
    procedure PopupMenu1Popup(Sender: TObject);
    procedure MiSaveFrameClick(Sender: TObject);
    procedure Mi176x144Click(Sender: TObject);
    procedure MiHideCursorClick(Sender: TObject);
    procedure FormMouseWheel(Sender: TObject; Shift: TShiftState;
      WheelDelta: Integer; MousePos: TPoint; var Handled: Boolean);
    procedure MiUndockClick(Sender: TObject);
    procedure dirUseHandleFile(const SearchRec: TSearchRec;
      const FullPath: String);
  protected
    fx1, fx2: TFastDIB;
  private
    { Private declarations }
    AutoFlip, Smooth: Boolean;
    picBuf: TFastDIBList;
    picIndex: Integer;
    AutoSend: Boolean;
    AutoSend_Address: String;
    AutoSend_jpegqualitynet: Integer;
    AutoSend_sendjpg: Boolean;
    MouseTrack: Boolean;      // report mouse x y
    MouseRect: Boolean;       // report dragged rectangle
    FHideCursor: Boolean;      // hide cursor in this frame
    jpgquality: Integer;
    jpegqualitynet: Integer;
    prevtop: Integer;
    prevmute: Boolean;
    filelist: TStringList;
    filelistIndex: Integer;

    procedure ClearPicBuf;
    procedure StayOnTop(const Yes: Boolean);
    function Getnextfilename(const dir, ext: String): String;
    procedure _prefx;
    procedure _postfx(const f: TFastDIB);
    procedure _postavi;
    procedure SendMouseTrack(const X, Y: Integer);
    procedure SendMouseRect(const x1, y1, x2, y2: Integer);
    function GetDocked: Boolean;
    procedure SetHideCursor(const Value: Boolean);
    procedure NewButton(const receivename: String;
     const x, y: Integer; const buttoncaption: String);
  public
    { Public declarations }
    avi: TDcAviPlayer;
    NameTag: String;
    procedure Borders(const Yes: Boolean);
    procedure Parse(const S: String); override;
    procedure FlipRequest;
    procedure AutoSendRequest;
    procedure HandleDroppedFile(const S: String);
    property Docked: Boolean read GetDocked;
    property HideCursor: Boolean read FHideCursor write SetHideCursor;
  end;

var
  fsframe: Tfsframe;

function FindFrame(const S: String): TFsFrame;

implementation

{$R *.DFM}

uses
  ShellApi,
  FastFX, FastFiles,
  mainunit, effectsunit, fsspeedbutton,
  Strz,
  FileCtrl,
  pshostunit;

function FindFrame(const S: String): TFsFrame;
begin
  Result := TFsFrame(main.FindComponent(main.CompName(S)));
end;

{ Tfsframe }

procedure Tfsframe._prefx;
begin
  dDib.DIB.Assign( d1.Surface );
  fx1.LoadFromHandle( dDib.DIB.Handle );
end;

procedure Tfsframe._postfx(const f: TFastDIB);
begin
  f.Draw(d1.Surface.Canvas.Handle, 0, 0);
  d1.Surface.Canvas.Release;
  FlipRequest;
end;

procedure Tfsframe._postavi;
begin
  d1.SetSize(avi.Width, avi.Height);
  avi.DrawFrameToDC(d1.Surface.Canvas.Handle);
  d1.Surface.Canvas.Release;
  FlipRequest;
end;

procedure Tfsframe.Parse(const S: String);
var
  fs, s1, Ext: String;
  i, o, u, x, y: Integer;
  fpic: TFastDIB;
  savejpg, sendjpg: Boolean;
  R: TRect;
begin
  if S='' then Exit;
  if (ParentWindow>0) and not
   IsWindow(ParentWindow) then Exit;

  s1 := UpperCase(ExtractWord(1, S, [' ']));
  Ext := UpperCase(ExtractFileExt(S));
  if s1='SAVE' then begin // SAVE <dir> <"bmp" or number for jpeg quality>
    s1 := ExtractWord(2, S, [' ']);
    if s1='' then Exit;
    savejpg := True;
    if WordCount(S, [' '])>2 then begin
      Ext := UpperCase(ExtractWord(3, S, [' ']));
      if Ext='BMP' then savejpg := False else begin
        i := StrToInt(Ext); if i>0 then jpgquality := i;
      end;
    end;
    if not DirectoryExists(s1) then begin
      main.Post('Save: Directory '+s1+' doesn''t exist.');
      Exit;
    end;
    if savejpg then Ext:='.jpg' else Ext:='.bmp';
    fs := Getnextfilename(s1, Ext);
    _prefx;
    if savejpg then
      SaveJPGFile(fx1, s1+'\'+fs, jpgquality)
    else
      fx1.SaveToFile(s1+'\'+fs);
  end;
  if (s1='BUFFER') or (Ext='.JPG') or (Ext='.BMP') then begin
    fs := S;
    if UpperCase(ExtractWord(1, fs, [' ']))<>'BUFFER' then begin
      if main.FileExistsInSearchPath(fs) then begin
        d1.Surface.LoadFromFile(fs);
        FlipRequest;
      end else
        main.Post('Failed opening '+fs);
    end else begin
      fs := Copy(fs, 8, 255);
      if main.FileExistsInSearchPath(fs) then begin
        main.Post('buffering '+fs);
        fpic := TFastDIB.Create;
        fpic.Bpp := 16;
        if Ext='.JPG' then
          LoadJPGFile(fpic, fs, True)
        else if Ext='.BMP' then
          fpic.LoadFromFile(fs);
        picBuf.Add(fpic);
      end else
      if DirectoryExists(fs) then begin
        main.Post('buffering directory '+fs);
        dirBuffer.Scan(fs);
      end else
        main.Post('Failed opening '+fs);
    end;
  end else
  if s1='FLIP_MANUAL' then begin
    AutoFlip := False;
  end else
  if s1='FLIP_AUTO' then begin
    AutoFlip := True;
  end else
  if s1='FLIP' then begin
    d1.Flip;
    AutoSendRequest;
  end else
  if s1='NEXT' then begin
    if picBuf.Count=0 then begin
      if filelist.Count=0 then begin
        if avi.Open then begin
          if avi.Position=avi.FrameCount-1 then
            avi.Seek(0)
          else
            avi.Seek(avi.Position+1);
          _postavi;
        end;
        Exit;
      end;
      Inc(filelistIndex);
      if filelistIndex>=filelist.Count then
        filelistIndex := 0;
      Parse(filelist.Strings[filelistIndex]);
      Exit;
    end;
    Inc(picIndex);
    if picIndex>=picBuf.Count then
      picIndex := 0;
    fpic := picBuf[picIndex];
    d1.Surface.SetSize(fpic.Width, fpic.Height);
    _postfx(fpic);
  end else
  if s1='PREV' then begin
    if picBuf.Count=0 then begin
      if filelist.Count=0 then begin
        // avi
        if avi.Open then begin
          if avi.Position=0 then
            avi.Seek(avi.FrameCount-1)
          else
            avi.Seek(avi.Position-1);
          _postavi;
        end;
        Exit;
      end;

      // filelist
      Dec(filelistIndex);
      if filelistIndex<0 then
        filelistIndex := filelist.Count-1;
      Parse(filelist.Strings[filelistIndex]);
      Exit;

    end;
    // picbuf
    Dec(picIndex);
    if picIndex<0 then
      picIndex := PicBuf.Count-1;
    fpic := picBuf[picIndex];
    d1.Surface.SetSize(fpic.Width, fpic.Height);
    _postfx(fpic);

  end else
  if s1='SEEK' then begin
    s1 := ExtractWord(2, S, [' ']);
    if s1='' then Exit;
    if picBuf.Count=0 then begin
      if filelist.Count=0 then begin
        if avi.Open then begin
          if s1[Length(s1)]='*' then
            i := Trunc(MyStrToFloat(Copy(s1, 1, Length(s1)-1))*avi.FrameCount-1)
          else
            i := MyStrToInt(s1);
          avi.Seek(i);
          _postavi;
        end;
        Exit;
      end;

      if s1[Length(s1)]='*' then
        i := Trunc(MyStrToFloat(Copy(s1, 1, Length(s1)-1))*filelist.Count-1)
      else
        i := MyStrToInt(s1);
      if (i>=0) and (i<filelist.Count) then
        Parse(filelist.Strings[i]);
      Exit;
    end;
    if s1[Length(s1)]='*' then
      i := Trunc(MyStrToFloat(Copy(s1, 1, Length(s1)-1))*picBuf.Count-1)
    else
      i := MyStrToInt(s1);
    if (i>=0) and (i<picBuf.Count) then begin
      fpic := picBuf[i];
      d1.Surface.SetSize(fpic.Width, fpic.Height);
      _postfx(fpic);
    end;
  end else
  if s1='RANDOM' then begin
    if picBuf.Count=0 then begin
      if filelist.Count=0 then begin
        if avi.Open then begin
          avi.Seek(Random(avi.FrameCount));
          _postavi;
        end;
        Exit;
      end;
      Parse(filelist.Strings[Random(filelist.Count)]);
      Exit;
    end;
    fpic := picBuf[Random(picBuf.Count)];
    d1.Surface.SetSize(fpic.Width, fpic.Height);
    _postfx(fpic);
  end else
  if s1='CLEAR' then begin
    ClearPicBuf;
    filelist.Clear;
  end else
  if (Ext='.AVI') then begin
    fs := S;
    if main.FileExistsInSearchPath(fs) then begin
      avi.FileName := fs;
      if avi.Open then begin
        avi.Seek(0);
        _postavi;
      end;
    end else
      main.Post('Failed opening '+fs);
  end else
  if S[1] in ['0'..'9'] then begin
    s1 := UpperCase(S);
    if Pos('X', s1)>0 then begin
      x := MyStrToInt(Trim(ExtractWord(1, s1, ['X'])));
      y := MyStrToInt(Trim(ExtractWord(2, s1, ['X'])));
      d1.Surface.SetSize(x, y);
      d1.Initialize;
      ClientWidth := x;
      ClientHeight := y;
    end else
    if Pos('+', s1)>0 then begin
      x := MyStrToInt(Trim(ExtractWord(1, s1, ['+'])));
      y := MyStrToInt(Trim(ExtractWord(2, s1, ['+'])));
      Left := x;
      Top := y;
    end else
      Exit;
  end else
  if s1='DISPLAY' then begin
    s1 := UpperCase(Trim(Copy(S, 8, 255)));
    x := MyStrToInt(Trim(ExtractWord(1, s1, ['X'])));
    y := MyStrToInt(Trim(ExtractWord(2, s1, ['X'])));
    if (x>0) and (y>0) then begin
      ClientWidth := x;
      ClientHeight := y;
    end;
  end else
  if s1='BORDERS_0' then begin
    Borders(False);
  end else
  if s1='BORDERS_1' then begin
    Borders(True);
  end else
  if s1='STAYONTOP_1' then begin
    StayOnTop(True);
  end else
  if s1='STAYONTOP_0' then begin
    StayOnTop(False);
  end else
  if s1='MUTE_0' then begin
    MiMute.Checked := True;
    MiMute.Click;
  end else
  if s1='MUTE_1' then begin
    MiMute.Checked := False;
    MiMute.Click;
  end else
  // FastLib effects
  if s1='MOSAIC' then begin
    i := MyStrToInt(ExtractWord(2, S, [' ']));
    if i=0 then i:=6;
    _prefx;
    FastFX.Mosaic(fx1, i, i);
    _postfx(fx1);
  end else
  if s1='SMOOTH_0' then begin
    Smooth := False;
  end else
  if s1='SMOOTH_1' then begin
    Smooth := True;
  end else
  if s1='ROTATE' then begin
    i := MyStrToInt(ExtractWord(2, S, [' ']));
    if i=0 then Exit;
    _prefx;
    fx2.SetSize(fx1.Width, fx1.Height, fx1.Bpp);
    fx2.Clear(IntToColor(0));
    FastFX.Rotate(fx1, fx2, i, Smooth);
    _postfx(fx2);
  end else
  if s1='ROTOZOOM' then begin
    i := MyStrToInt(ExtractWord(2, S, [' ']));
    o := MyStrToInt(ExtractWord(3, S, [' ']));
    if (i=0) and (o=0) then Exit;
    _prefx;
    fx2.SetSize(fx1.Width, fx1.Height, fx1.Bpp);
    fx2.Clear(IntToColor(0));
    FastFX.Rotozoom(fx1, fx2, i, o, Smooth);
    _postfx(fx2);
  end else
  if s1='SATURATION' then begin
    i := MyStrToInt(ExtractWord(2, S, [' ']));
    _prefx;
    FastFX.Saturation(fx1, i);
    _postfx(fx1);
  end else
  if s1='INVERT' then begin
    _prefx;
    FastFX.Invert(fx1);
    _postfx(fx1);
  end else
  if s1='SOFT' then begin
    _prefx;
    FastFX.QuickSoft(fx1);
    _postfx(fx1);
  end else
  if s1='SHARP' then begin
    _prefx;
    FastFX.QuickSharp(fx1);
    _postfx(fx1);
  end else
  if s1='EMBOSS' then begin
    _prefx;
    FastFX.QuickEmboss(fx1);
    _postfx(fx1);
  end else
  if s1='ADDITION' then begin
    i := MyStrToInt(ExtractWord(2, S, [' ']));
    o := MyStrToInt(ExtractWord(3, S, [' ']));
    u := MyStrToInt(ExtractWord(4, S, [' ']));
    if (i=0) and (o=0) and (u=0) then Exit;
    _prefx;
    FastFX.Addition(fx1, i, o, u);
    _postfx(fx1);
  end else
  if s1='GAMMA' then begin
    i := MyStrToInt(ExtractWord(2, S, [' ']));
    o := MyStrToInt(ExtractWord(3, S, [' ']));
    u := MyStrToInt(ExtractWord(4, S, [' ']));
    if (i=0) and (o=0) and (u=0) then Exit;
    _prefx;
    FastFX.Gamma(fx1, i, o, u);
    _postfx(fx1);
  end else
  if s1='CONTRAST' then begin
    i := MyStrToInt(ExtractWord(2, S, [' ']));
    o := MyStrToInt(ExtractWord(3, S, [' ']));
    u := MyStrToInt(ExtractWord(4, S, [' ']));
    if (i=0) and (o=0) and (u=0) then Exit;
    _prefx;
    FastFX.Contrast(fx1, i, o, u);
    _postfx(fx1);
  end else
  if s1='LIGHTNESS' then begin
    i := MyStrToInt(ExtractWord(2, S, [' ']));
    o := MyStrToInt(ExtractWord(3, S, [' ']));
    u := MyStrToInt(ExtractWord(4, S, [' ']));
    if (i=0) and (o=0) and (u=0) then Exit;
    _prefx;
    FastFX.Lightness(fx1, i, o, u);
    _postfx(fx1);
  end else
  if s1='SCRAMBLE' then begin
    scramble(d1.Surface);
    FlipRequest;
  end else
  if s1='CONNECT' then begin
    i:=WordCount(S, [' ']);
    if i<3 then begin
      main.Post('Usage: Connect <host> <port>');
      Exit;
    end;
    s1:=ExtractWord(2, S, [' ']);
    i := MyStrToInt(ExtractWord(3, S, [' ']));
    if i<=0 then Exit;
    if (main.csfs.Host<>s1) or (main.csfs.Port<>i) then begin
      if main.csfs.Active then
        main.csfs.Active:=False;
      main.csfs.Host:=s1;
      main.csfs.Port:=i;
    end;
    main.csfs.Open;
  end else
  if s1='DISCONNECT' then begin
    main.csfs.Close;
  end else
  if (s1='SEND') or (s1='SEND_AUTO') then begin
    i:=WordCount(S, [' ']);
    if i<2 then begin
      main.Post('Usage: Send <nametag> ["pure" for uncompressed or jpeg quality (default=80)]');
      main.Post('Use "receive <nametag>" to set receiving fs.frame on the other end.');
      Exit;
    end;
    sendjpg := True;
    if i>2 then begin
      Ext := UpperCase(ExtractWord(3, S, [' ']));
      if Ext='PURE' then
        sendjpg := False
      else begin
        jpegqualitynet := MyStrToInt(Ext);
        if jpegqualitynet<=0 then jpegqualitynet:=80;
      end;
    end;
    if sendjpg then
      _prefx;
    main.SendFrame(Self, ExtractWord(2, S, [' ']),
     fx1, jpegqualitynet, sendjpg);
    if s1='SEND_AUTO' then begin
      AutoSend := True;
      AutoSend_Address := ExtractWord(2, S, [' ']);
      AutoSend_jpegqualitynet := jpegqualitynet;
      AutoSend_sendjpg := sendjpg;
    end;
  end else
  if s1='SEND_MANUAL' then begin
    AutoSend := False;
  end else
  if s1='RECEIVE' then begin
    Self.NameTag := ExtractWord(2, S, [' ']);
  end else
  if s1='MOUSETRACK_1' then begin
    MouseTrack := True;
    MiMouseTrack.Checked := True;
  end else
  if s1='MOUSETRACK_0' then begin
    MouseTrack := False;
    MiMouseTrack.Checked := False;
  end else
  if s1='MOUSERECT_1' then begin
    MouseRect := True;
  end else
  if s1='MOUSERECT_0' then begin
    MouseRect := False;
  end else
  if s1='HIDECURSOR_1' then begin
    MiHideCursor.Checked := True;
    HideCursor := True;
  end else
  if s1='HIDECURSOR_0' then begin
    MiHideCursor.Checked := False;
    HideCursor := False;
  end else
  if s1='DOCK' then begin
    DockTitle := Copy(S, Length(s1)+2, 255);
    DockHandle:=0;
    EnumWindows(@mainunit.WinEnumerator_SubStr, 0);
    if DockHandle>0 then begin
      if MiBorders.Checked then
        MiBorders.Click;
      if GetWindowRect(DockHandle, R) then begin
        d1.Hide;
        WindowState := wsNormal;
        ParentWindow := 0;
        Parent := nil;
        ParentWindow := DockHandle;
        Left := 0;
        Top := 0;
        d1.Show;
        d1.Initialize;
        BringToFront;
        DragAcceptFiles(Handle, True);
      end;
    end;
  end else
  if s1='SHOW' then begin
    Self.Top := prevtop;
    if not prevmute and MiMute.Checked then
      MiMute.Click;
  end else
  if s1='HIDE' then begin
    // ugly, but.
    prevtop := Self.Top;
    Self.Top := Screen.Height;
    prevmute := MiMute.Checked;
    if not prevmute then MiMute.Click;
  end else
  if s1='BUFFERIZE' then begin
    _prefx;
    fpic := TFastDIB.Create;
    fpic.LoadFromHandle(fx1.Handle);
    picBuf.Add(fpic);
  end else
  if s1='MAXIMIZE' then begin
    WindowState := wsMaximized;
  end else
  if s1='MINIMIZE' then begin
    WindowState := wsMinimized;
  end else
  if s1='BRINGTOFRONT' then begin
    BringToFront;
  end else
  if s1='USE' then begin // use directory
    s1 := Copy(S, Length(s1)+2, 255);
    if DirectoryExists(s1) then begin
      i := filelist.Count;
      dirUse.Scan(s1);
      main.Post('Using '+IntToStr(filelist.Count-i)+' images from '+s1+', total of '+IntToStr(filelist.Count));
    end else
      main.Post('nonexistent directory: '+s1);
  end else
  if s1='BUTTON' then begin
    s1 := Copy(S, Length(s1)+2, 255);
    if WordCount(s1, [' '])<>4 then Exit;
    NewButton(ExtractWord(1, s1, [' ']),      // receive name
     MyStrToInt(ExtractWord(2, s1, [' '])),   // x
     MyStrToInt(ExtractWord(3, s1, [' '])),   // y
     ExtractWord(4, s1, [' '])                // caption
    );
  end else
  if False{your new effect here} then begin
  end else
  if main.Plugins.IsPlugin(s1) then begin
    if main.Plugins.CallEffect(d1.Surface, s1, Copy(S, Length(s1)+2, 255)) then
      FlipRequest;
  end else
  if pshostunit.IsFilter(s1) then begin
    pshostunit.Filter_effect(framepsh, d1.Surface, s1, Copy(S, Length(s1)+2, 255));
    FlipRequest;
  end else
end;

procedure Tfsframe.FormClose(Sender: TObject; var Action: TCloseAction);
begin
  Action := caFree;
end;

procedure Tfsframe.Mi11Click(Sender: TObject);
begin
  ClientWidth := d1.Surface.Width;
  ClientHeight := d1.Surface.Height;
end;

procedure Tfsframe.Mi12Click(Sender: TObject);
begin
  ClientWidth := d1.Surface.Width * 2;
  ClientHeight := d1.Surface.Height * 2;
end;

procedure Tfsframe.MiMuteClick(Sender: TObject);
begin
  MiMute.Checked := not MiMute.Checked;
  d1.Visible := not MiMute.Checked;
end;

procedure Tfsframe.ClearPicBuf;
var
  f: TFastDIB;
  i: Integer;
begin
  if picBuf.Count=0 then Exit;
  for i:=picBuf.Count-1 downto 0 do begin
    f := picBuf[i];
    f.Free;
    picBuf.Delete(i);
  end;
end;

procedure Tfsframe.FlipRequest;
begin
  if AutoFlip then begin
    d1.Flip;
    AutoSendRequest;
  end;
end;

procedure Tfsframe.AutoSendRequest;
begin
  if AutoSend and main.csfs.Active then begin
    if AutoSend_sendjpg then
      _prefx;
    main.SendFrame(Self, AutoSend_address,
     fx1, AutoSend_jpegqualitynet, AutoSend_sendjpg);
  end;
end;

procedure Tfsframe.Borders(const Yes: Boolean);
begin
  _prefx;
  d1.Hide;
  if Yes and (ParentWindow=0) then
    BorderStyle := bsSizeable
  else
    BorderStyle := bsNone;
  d1.Show;
  d1.Initialize;
  _postfx(fx1);
  MiBorders.Checked := Yes;
  DragAcceptFiles(Handle, True);
end;

procedure Tfsframe.StayOnTop(const Yes: Boolean);
begin
  _prefx;
  d1.Hide;
  if Yes and (ParentWindow=0) then
    FormStyle := fsStayOnTop
  else
    FormStyle := fsNormal;
  d1.Show;
  d1.Initialize;
  _postfx(fx1);
  MiStayOnTop.Checked := Yes;
  DragAcceptFiles(Handle, True);
end;

procedure Tfsframe.MiBordersClick(Sender: TObject);
begin
  if Docked then Exit;
  MiBorders.Checked := not MiBorders.Checked;
  Borders(MiBorders.Checked);
end;

procedure Tfsframe.MiStayOnTopClick(Sender: TObject);
begin
  if Docked then Exit;
  MiStayOnTop.Checked := not MiStayOnTop.Checked;
  StayOnTop(MiStayOnTop.Checked);
end;

procedure Tfsframe.dirBufferHandleFile(const SearchRec: TSearchRec;
  const FullPath: String);
var
  S: String;
begin
  S := UpperCase(ExtractFileExt(FullPath));
  if (S='.BMP') or (S='.JPG') then
    Parse('BUFFER '+FullPath);
end;

procedure Tfsframe.dirUseHandleFile(const SearchRec: TSearchRec;
  const FullPath: String);
var
  S: String;
begin
  S := UpperCase(ExtractFileExt(FullPath));
  if (S='.BMP') or (S='.JPG') then
    filelist.Add(FullPath);
end;

function Tfsframe.Getnextfilename(const dir, ext: String): String;
const
  pref = 'fs';
  last: Integer = 0;
  predir: string = '';
var
  S: String;
begin
  if dir<>predir then last := 0;
  repeat
    Inc(last);
    S := pref + IntToStr(last);
    while Length(S)<8 do Insert('0', S, 3);
    S := S+ext;
  until not FileExists(S);
  Result := S;
  predir := dir;
end;

procedure Tfsframe.FormCreate(Sender: TObject);
begin
  ClientWidth := 176;
  ClientHeight := 144;
  d1.SetSize( ClientWidth, ClientHeight );
  d1.Initialize;
  with d1.Surface.Canvas do begin
    Pen.Color := clWhite;
    Brush.Color := clWhite;
    Font.Color := clWhite;
  end;
  d1.Surface.Canvas.Release;
  AutoFlip := True;
  AutoSend := False;
  Smooth := True;
  avi := TDcAviPlayer.Create(Self);
  avi.Parent := Self;
  avi.Visible := False;
  avi.Transparent := False;
  avi.Stretch := False;
  avi.PlaySound := False;
  fx1 := TFastDIB.Create;
  fx2 := TFastDIB.Create;
  picBuf := TFastDIBList.Create;
  picIndex := -1;
  MouseTrack := False;
  MouseRect := False;
  HideCursor := False;
  jpgquality := 85;
  jpegqualitynet := 80;
  prevtop := 0;
  prevmute := False;
  filelist := TStringList.Create;
  filelistIndex := -1;
  DragAcceptFiles(Handle, True);
end;

procedure Tfsframe.FormDestroy(Sender: TObject);
begin
  avi.Free;
  fx1.Free;
  fx2.Free;
  ClearPicBuf;
  picBuf.Free;
  filelist.Free;
end;

var
  LeftDown: Boolean;
  OrgX, OrgY: Integer;

procedure Tfsframe.d1MouseDown(Sender: TObject; Button: TMouseButton;
  Shift: TShiftState; X, Y: Integer);
begin
  case Button of
    mbLeft: begin
      LeftDown := True;
      main.SendReturnValues(PdName+'mousedown=1');        
      if MouseTrack and not MouseRect and not (ssAlt in Shift) then
        SendMouseTrack(X, Y)
      else begin
        OrgX := X;
        OrgY := Y;
      end;
    end;
  end;
end;

procedure Tfsframe.d1MouseMove(Sender: TObject; Shift: TShiftState; X,
  Y: Integer);
begin
  if LeftDown then begin
    if MouseTrack and not (ssAlt in Shift) then begin
      if not MouseRect then
        SendMouseTrack(X, Y)
      else
        SendMouseRect(OrgX, OrgY, X, Y);
    end else begin
      Left := Left + (X-OrgX);
      Top := Top + (Y-OrgY);
    end;
  end;
end;

procedure Tfsframe.d1MouseUp(Sender: TObject; Button: TMouseButton;
  Shift: TShiftState; X, Y: Integer);
begin
  case Button of
    mbLeft: begin
      LeftDown:=False;
(* these values were sent at "mousemove"
      if MouseRect then
        SendMouseRect(OrgX, OrgY, X, Y);*)
      main.SendReturnValues(PdName+'mouseup=1');        
      if not MouseTrack or (ssAlt in Shift) then begin
//        main.Post('moved to '+
//         IntToStr(Left)+'+'+
//         IntToStr(Top)
//        );
        main.SendReturnValues(
         PdName+'winy='+IntToStr(Top)+';'+
         PdName+'winx='+IntToStr(Left)
        );
      end;
    end;
  end;
end;

procedure Tfsframe.SendMouseTrack(const X, Y: Integer);
var
  a, b: Integer;
begin
  a := X;
  b := Y;
  // handle aspect ratios different from 1:1
  if d1.Surface.Width<>d1.Width then
    a := Trunc((d1.Surface.Width / d1.Width) * X);
  if d1.Surface.Height<>d1.Height then
    b := Trunc((d1.Surface.Height / d1.Height) * Y);

  main.SendReturnValues(
   PdName+'y='+IntToStr(b)+';'+
   PdName+'x='+IntToStr(a)
  );
end;

procedure Tfsframe.SendMouseRect(const x1, y1, x2, y2: Integer);
var
  a, b, c, d: Integer;
begin
  a := x1;
  b := y1;
  c := x2;
  d := y2;
  // handle aspect ratios different from 1:1
  if d1.Surface.Width<>d1.Width then
    a := Trunc((d1.Surface.Width / d1.Width) * x1);
  if d1.Surface.Height<>d1.Height then
    b := Trunc((d1.Surface.Height / d1.Height) * y1);
  if d1.Surface.Width<>d1.Width then
    c := Trunc((d1.Surface.Width / d1.Width) * x2);
  if d1.Surface.Height<>d1.Height then
    d := Trunc((d1.Surface.Height / d1.Height) * y2);

  main.SendReturnValues(
   PdName+'y2='+IntToStr(d)+';'+
   PdName+'x2='+IntToStr(c)+';'+
   PdName+'y1='+IntToStr(b)+';'+
   PdName+'x1='+IntToStr(a)
  );
end;

procedure Tfsframe.MiMouseTrackClick(Sender: TObject);
begin
  MouseTrack := not MouseTrack;
  MiMouseTrack.Checked := MouseTrack;
end;

procedure Tfsframe.HandleDroppedFile(const S: String);
var
  i: Integer;
  St: String;
begin
  Parse(S);
  main.SendReturnValues(PdName+'bang=1');
  // send string as a series of ascii..
  main.SendReturnValuesString(PDName+'file', S);
end;

procedure Tfsframe.MiBufferImagesClick(Sender: TObject);
var
  i: Integer;
  s: String;
begin
  if opd1.Execute then begin
    if opd1.Files.Count>0 then
      for i:=0 to opd1.Files.Count-1 do begin
        if '.AVI'<>UpperCase(ExtractFileExt(opd1.files[i])) then
          s := 'BUFFER '
        else
          s := '';
        Parse(s+opd1.files[i]);
      end;
    // tee uusi canvas jossa valitut failit....
  end;
{
#N canvas 0 0 700 300 12;
#X obj 23 17 inlet;
#X obj 15 45 outlet;
#X msg 71 17 buffer file1.bmp \, buffer file2.bmp \, buffer file3.bmp;
#X connect 0 0 2 0;
#X connect 2 0 1 0;
}
end;

function Tfsframe.GetDocked: Boolean;
begin
  Result := ParentWindow<>0;
end;

procedure Tfsframe.PopupMenu1Popup(Sender: TObject);
begin
  MiBorders.Enabled := not Docked;
  MiStayOnTop.Enabled := not Docked;
  MiUndock.Enabled := Docked;
end;

procedure Tfsframe.MiSaveFrameClick(Sender: TObject);
var
  S: String;
begin
  if svd1.Execute then begin
    S := svd1.Filename;
    if Uppercase(ExtractFileExt(S))<>'.BMP' then
      S := S+'.bmp';
    _prefx;
    fx1.SaveToFile(S);
  end;
{
  _prefx;
  if savejpg then
    SaveJPGFile(fx1, s1+'\'+fs, jpgquality)
  else
    fx1.SaveToFile(s1+'\'+fs);
}
end;

procedure Tfsframe.Mi176x144Click(Sender: TObject);
begin
  Parse('display 176x144');
end;

procedure Tfsframe.SetHideCursor(const Value: Boolean);
begin
  FHideCursor := Value;
  if Value then
    d1.Cursor := crNone
  else
    d1.Cursor := crDefault;
end;

procedure Tfsframe.MiHideCursorClick(Sender: TObject);
begin
  MiHideCursor.Checked := not MiHideCursor.Checked;
  HideCursor := MiHideCursor.Checked;
end;

procedure Tfsframe.FormMouseWheel(Sender: TObject; Shift: TShiftState;
  WheelDelta: Integer; MousePos: TPoint; var Handled: Boolean);
var
  asp: Extended;
begin
  if WheelDelta>0 then begin
    ClientWidth := Trunc(ClientWidth * (WheelDelta / 100));
    if not (ssShift in Shift) then
      ClientHeight := Trunc(ClientHeight * (WheelDelta / 100));
  end else begin
    ClientWidth := Trunc(ClientWidth / Abs(WheelDelta / 100));
    if not (ssShift in Shift) then
      ClientHeight := Trunc(ClientHeight / Abs(WheelDelta / 100));
  end;
  // correct aspect ratio...
  if ssShift in Shift then begin
    asp := d1.Surface.Width / d1.Surface.Height;
    if asp>0 then
      ClientHeight := Trunc(ClientWidth / asp);
  end;
end;

procedure Tfsframe.MiUndockClick(Sender: TObject);
begin
  ParentWindow := 0;
  Borders(True);
end;

procedure Tfsframe.NewButton(const receivename: String; const x,
  y: Integer; const buttoncaption: String);
var
  p: TPanel;
  b: TFSSpeedButton;
begin
  p := TPanel.Create(Self);
  b := TFSSpeedButton.Create(Self);

  b.Width := LabelFoo.Canvas.TextWidth(buttoncaption)+12;
  b.Height := 18;
  b.Parent := p;
  b.Left := 0;
  b.Top := 0;
  b.Flat := True;
  b.Caption := buttoncaption;
  b.Receivename := receivename;
  p.AutoSize := True;
  p.Left := x;
  p.Top := y;
  p.BevelOuter := bvNone;
  p.Parent := d1;
end;

end.

