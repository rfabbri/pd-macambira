{ Copyright (C) 2001 Juha Vehviläinen
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.}

unit fscopyunit;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs,
  mainunit, fsformunit, fsframeunit, StdCtrls, C2PhotoShopHost;

type
  TDrawStyles = (dsCopy, dsROP, dsAlpha, dsAdd, dsSub,
   dsBlend, dsPlugin, dsFilter);
  TRectTypes = (rtAll, rtRandom, rtSpecific);

  Tfscopy = class(TFsForm)
    copypsh: TC2PhotoShopHost;
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
    procedure FormCreate(Sender: TObject);
  private
    { Private declarations }
    f1, f2: TFsFrame;
    DrawStyle: TDrawStyles;
    dsROPMode: Integer;
    SourceType: TRectTypes;
    DestType: TRectTypes;
    SourceRect, DestRect: TRect;
    iAlpha, iAdd, iSub: Integer;
    iBlend: Extended;
    Transparent, MirrorLeftRight, MirrorUpdown: Boolean;
    TransColor: Cardinal;
    iPlugin: Integer;
    iPluginArgs: String;
    iFilter, iFilterArgs: String;

    procedure GetFrames(const S: String);
  public
    { Public declarations }
    procedure Parse(const S: String); override;
  end;

var
  fscopy: Tfscopy;

implementation

{$R *.DFM}

uses
  DxDraws, DirectX,
  effectsunit, fsbrowserunit, pshostunit,
  Strz;

procedure Tfscopy.FormClose(Sender: TObject; var Action: TCloseAction);
begin
  Action := caFree;
end;

procedure Tfscopy.GetFrames(const S: String);
const
  f1name: String = '-1';
  f2name: String = '-1';
var
  s1, s2: String;
begin
  if (Pos(' ', S)=0) then Exit;

  s1 := UpperCase(ExtractWord(1, S, [' ']));
  s2 := UpperCase(ExtractWord(2, S, [' ']));

  if (s1<>f1name) or (f1=nil) then
    f1 := FindFrame(s1);

  if (s2<>f2name) or (f2=nil) then
    f2 := FindFrame(s2);

  f1name := s1;  f2name := s2;
end;

procedure Tfscopy.Parse(const S: String);

  procedure SwapInt(var i1, i2: Integer);
  var
    tmpi: Integer;
  begin
    tmpi := i1;
    i1 := i2;
    i2 := tmpi;
  end;

var
  s1: String;
  bltFlags: Cardinal;
  df: TDDBltFX;
  ddck: TDDColorKey;
  sx1, sy1, sx2, sy2, dx1, dy1, dx2, dy2: Integer;
  r, g, b: Byte;
begin
  if (S='') then Exit;

  s1 := UpperCase(ExtractWord(1, S, [' ']));

  if Pos('FS.FRAME', s1)=0 then
  if s1='COPY' then begin DrawStyle := dsCopy; Exit; end else
  if s1='BLEND' then begin
    DrawStyle := dsBlend;
    iBlend := MyStrToFloat(ExtractWord(2, S, [' ']));
    Exit;
  end else
  if s1='ALPHA' then begin
    DrawStyle := dsAlpha;
    iAlpha := MyStrToInt(ExtractWord(2, S, [' ']));
    Exit;
  end else
  if s1='ADD' then begin
    DrawStyle := dsAdd;
    iAdd := MyStrToInt(ExtractWord(2, S, [' ']));
    Exit;
  end else
  if s1='SUB' then begin
    DrawStyle := dsSub;
    iSub := MyStrToInt(ExtractWord(2, S, [' ']));
    Exit;
  end else
  if s1='SOURCE' then begin
    if Pos(' ', S)>0 then begin // at least x1 specified
      SourceRect := StrToRect(Copy(S, Length(s1)+2, 255));
      SourceType := rtSpecific;
      Exit;
    end;
  end else
  if s1='DEST' then begin
    if Pos(' ', S)>0 then begin // at least x1 specified
      DestRect := StrToRect(Copy(S, Length(s1)+2, 255));
      DestType := rtSpecific;
      Exit;
    end;
  end else
  if s1='SOURCE_ALL' then begin SourceType := rtAll; Exit; end else
  if s1='SOURCE_RANDOM' then begin SourceType := rtRandom; Exit; end else
  if s1='DEST_ALL' then begin DestType := rtAll; Exit; end else
  if s1='DEST_RANDOM' then begin DestType := rtRandom; Exit; end else

  if s1='BLACKNESS' then begin DrawStyle := dsROP; dsROPMode := cmBlackness; Exit; end else
  if s1='DSTINVERT' then begin DrawStyle := dsROP; dsROPMode := cmDstInvert; Exit; end else
  if s1='MERGECOPY' then begin DrawStyle := dsROP; dsROPMode := cmMergeCopy; Exit; end else
  if s1='MERGEPAINT' then begin DrawStyle := dsROP; dsROPMode := cmMergePaint; Exit; end else
  if s1='NOTSRCCOPY' then begin DrawStyle := dsROP; dsROPMode := cmNotSrcCopy; Exit; end else
  if s1='NOTSRCERASE' then begin DrawStyle := dsROP; dsROPMode := cmNotSrcErase; Exit; end else
  if s1='PATCOPY' then begin DrawStyle := dsROP; dsROPMode := cmPatCopy; Exit; end else
  if s1='PATINVERT' then begin DrawStyle := dsROP; dsROPMode := cmPatInvert; Exit; end else
  if s1='PATPAINT' then begin DrawStyle := dsROP; dsROPMode := cmPatPaint; Exit; end else
  if s1='SRCAND' then begin DrawStyle := dsROP; dsROPMode := cmSrcAnd; Exit; end else
  if s1='SRCCOPY' then begin DrawStyle := dsROP; dsROPMode := cmSrcCopy; Exit; end else
  if s1='SRCERASE' then begin DrawStyle := dsROP; dsROPMode := cmSrcErase; Exit; end else
  if s1='SRCINVERT' then begin DrawStyle := dsROP; dsROPMode := cmSrcInvert; Exit; end else
  if s1='SRCPAINT' then begin DrawStyle := dsROP; dsROPMode := cmSrcPaint; Exit; end else
  if s1='WHITENESS' then begin DrawStyle := dsROP; dsROPMode := cmWhiteness; Exit; end else

  if s1='TRANSPARENT_0' then begin Transparent := False; Exit; end else
  if s1='TRANSPARENT_1' then begin Transparent := True; Exit; end else
  if s1='TRANSCOLOR' then begin
    r := MyStrToInt(ExtractWord(2, S, [' ']));
    g := MyStrToInt(ExtractWord(3, S, [' ']));
    b := MyStrToInt(ExtractWord(4, S, [' ']));
    TransColor := RGB(r, g, b);
    Exit;
  end else
  if s1='MIRRORLEFTRIGHT_0' then begin MirrorLeftRight := False; Exit; end else
  if s1='MIRRORLEFTRIGHT_1' then begin MirrorLeftRight := True; Exit; end else
  if s1='MIRRORUPDOWN_0' then begin MirrorUpDown := False; Exit; end else
  if s1='MIRRORUPDOWN_1' then begin MirrorUpDown := True; Exit; end else
  if main.Plugins.IsPlugin(iPlugin, s1) then begin
    DrawStyle := dsPlugin;
    iPluginArgs := Copy(S, Length(s1)+2, 255);
    Exit;
  end else
  if pshostunit.IsFilter(s1) then begin
    DrawStyle := dsFilter;
    iFilter := s1;
    iFilterArgs := Copy(S, Length(s1)+2, 255);
    Exit;
  end;

  GetFrames(S);
  if (f1=nil) or (f2=nil) then Exit;
  if (f1.d1=nil) or (f2.d1=nil) then Exit;

  if (f1.ParentWindow>0) and not IsWindow(f1.ParentWindow) then Exit;
  if (f2.ParentWindow>0) and not IsWindow(f2.ParentWindow) then Exit;

  if f1 is TFsBrowser then begin
    (f1 as TFsBrowser).CopyToSurface;
  end;

  case SourceType of
    rtAll: begin
      sx1 := 0;
      sy1 := 0;
      sx2 := f1.d1.Surface.Width;
      sy2 := f1.d1.Surface.Height;
    end;
    rtRandom: begin
      sx1 := Random(f1.d1.Surface.Width);
      sx2 := Random(f1.d1.Surface.Width);
      sy1 := Random(f1.d1.Surface.Height);
      sy2 := Random(f1.d1.Surface.Height);
    end;
    rtSpecific: begin
      sx1 := SourceRect.Left;
      sy1 := SourceRect.Top;
      sx2 := SourceRect.Right;
      sy2 := SourceRect.Bottom;
    end;
  end;

  case DestType of
    rtAll: begin
      dx1 := 0;
      dy1 := 0;
      dx2 := f2.d1.Surface.Width;
      dy2 := f2.d1.Surface.Height;
    end;
    rtRandom: begin
      dx1 := Random(f2.d1.Surface.Width);
      dx2 := Random(f2.d1.Surface.Width);
      dy1 := Random(f2.d1.Surface.Height);
      dy2 := Random(f2.d1.Surface.Height);
    end;
    rtSpecific: begin
      dx1 := DestRect.Left;
      dy1 := DestRect.Top;
      dx2 := DestRect.Right;
      dy2 := DestRect.Bottom;
    end;
  end;

  if sx2<sx1 then swapint(sx2, sx1);
  if sy2<sy1 then swapint(sy2, sy1);
  if dx2<dx1 then swapint(dx2, dx1);
  if dy2<dy1 then swapint(dy2, dy1);

  if sx2=sx1 then inc(sx2);
  if sy2=sy1 then inc(sy2);
  if dx2=dx1 then inc(dx2);
  if dy2=dy1 then inc(dy2);

  if sx1<0 then sx1:=0;
  if sy1<0 then sy1:=0;
  if sx2>f1.d1.Surface.Width then sx2:=f1.d1.Surface.Width;
  if sy2>f1.d1.Surface.Height then sy2:=f1.d1.Surface.Height;
  if dx1<0 then dx1:=0;
  if dy1<0 then dy1:=0;
  if dx2>f2.d1.Surface.Width then dx2:=f1.d1.Surface.Width;
  if dy2>f2.d1.Surface.Height then dy2:=f2.d1.Surface.Height;

  case DrawStyle of
    dsCopy: begin
      ddck.dwColorSpaceLowValue := TransColor;
      ddck.dwColorSpaceHighValue := TransColor;
      DF.dwsize := SizeOf(DF);
      DF.dwROP := cmSrcCopy;
      DF.dwDDFX := 0;
      DF.ddckSrcColorkey := ddck;
      DF.ddckDestColorkey := ddck;
      bltFlags := DDBLT_DDFX;
      if Transparent then bltFlags := bltFlags or DDBLT_KEYSRCOVERRIDE;//OVERRIDE;
      if MirrorLeftRight then
        DF.dwDDFX := DF.dwDDFX or DDBLTFX_MIRRORLEFTRIGHT;
      if MirrorUpDown then
        DF.dwDDFX := DF.dwDDFX or DDBLTFX_MIRRORUPDOWN;

      if not f2.d1.Surface.Blt(
       Rect(dx1, dy1, dx2, dy2),
       Rect(sx1, sy1, sx2, sy2),
       bltFlags or {DDBLT_ROP or} DDBLT_WAIT,
       DF, f1.d1.Surface ) then
        main.Post(Self.Name+': blt failed.')
      else
        f2.FlipRequest;
    end;
    dsROP: begin
      StretchBlt(f2.d1.Surface.Canvas.Handle,
       dx1, dy1, dx2-dx1, dy2-dy1,
       f1.d1.Surface.Canvas.Handle,
       sx1, sy1, sx2-sx1, sy2-sy1, dsROPMode);

      f1.d1.Surface.Canvas.Release;
      f2.d1.Surface.Canvas.Release;
      f2.FlipRequest;
    end;
    dsAlpha: begin
      f2.d1.Surface.DrawAlpha(
       Rect(dx1, dy1, dx2, dy2),
       Rect(sx1, sy1, sx2, sy2),
       f1.d1.Surface, False, iAlpha);
      f2.FlipRequest;
    end;
    dsAdd: begin
      f2.d1.Surface.DrawAdd(
       Rect(dx1, dy1, dx2, dy2),
       Rect(sx1, sy1, sx2, sy2),
       f1.d1.Surface, False, iAdd);
      f2.FlipRequest;
    end;
    dsSub: begin
      f2.d1.Surface.DrawSub(
       Rect(dx1, dy1, dx2, dy2),
       Rect(sx1, sy1, sx2, sy2),
       f1.d1.Surface, False, iSub);
      f2.FlipRequest;
    end;
    dsBlend: begin
      blend(f1.d1.Surface, f2.d1.Surface, iBlend);
      f2.FlipRequest;
    end;
    dsPlugin: begin
      if main.Plugins.CallCopy(f1.d1.Surface, f2.d1.Surface,
       iPlugin, iPluginArgs) then begin
        f1.FlipRequest;
        f2.FlipRequest;
      end;
    end;
    dsFilter: begin
      if pshostunit.Filter_copy(Self.copypsh,
       f1.d1.Surface, f2.d1.Surface,
       iFilter, iFilterArgs) then begin
        f1.FlipRequest;
        f2.FlipRequest;
      end;
    end;
  end;
end;

procedure Tfscopy.FormCreate(Sender: TObject);
begin
  f1 := nil;  f2 := nil;
  DrawStyle := dsCopy;
  SourceType := rtAll;
  DestType := rtAll;
  iAlpha := 50;  iAdd := 255;  iSub := 255;
  Transparent := False;
  TransColor := 0;
  MirrorLeftRight := False;
  MirrorUpDown := False;
end;

end.

