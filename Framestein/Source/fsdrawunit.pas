{ Copyright (C) 2001 Juha Vehviläinen
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.}

unit fsdrawunit;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs,
  fsformunit, fsframeunit, fstextunit;

type
  Tfsdraw = class(TFsForm)
    procedure FormCreate(Sender: TObject);
  private
    dest: TFsFrame;
    { Private declarations }
  public
    { Public declarations }
    procedure Parse(const S: String); override;
  end;

var
  fsdraw: Tfsdraw;

implementation

uses
  mainunit, Strz;

{$R *.DFM}

{ Tfsdraw }

procedure Tfsdraw.Parse(const S: String);
var
  r: TRect;
  x, y: Integer;
  s1, sa: String;
  b: Boolean;
begin
  if (S='') then Exit;
  s1 := ExtractWord(WordCount(S, [' ']), S, [' ']);
  dest := FindFrame(s1);
  if (dest=nil) or (dest.d1=nil) then Exit;

  sa := UpperCase(Copy(S, 1, Pos(s1, S)-2));

  b := ParsePenProperties(dest.d1.Surface.Canvas, sa);
  dest.d1.Surface.Canvas.Release;

  s1 := ExtractWord(1, sa, [' ']);

  if b then Exit else
  if s1='MODE_SOLID' then begin
    dest.d1.Surface.Canvas.Brush.Style := bsSolid;
    dest.d1.Surface.Canvas.Release;
  end else
  if s1='MODE_CLEAR' then begin
    dest.d1.Surface.Canvas.Brush.Style := bsClear;
    dest.d1.Surface.Canvas.Release;
  end else
  if s1='RECT' then begin
    dest.d1.Surface.Canvas.Rectangle(StrToRect(Copy(sa, Length(s1)+2, 255)));
    dest.d1.Surface.Canvas.Release;
    dest.FlipRequest;
  end else
  if s1='LINE' then begin
    r := StrToRect(Copy(sa, Length(s1)+2, 255));
    with dest.d1.Surface.Canvas do begin
      MoveTo(r.Left, r.Top);
      LineTo(r.Right, r.Bottom);
    end;
    dest.d1.Surface.Canvas.Release;
    dest.FlipRequest;
  end else
  if s1='PLOT' then begin
    x := MyStrToInt(ExtractWord(2, sa, [' ']));
    y := MyStrToInt(ExtractWord(3, sa, [' ']));
    dest.d1.Surface.Canvas.Pixels[x, y] :=
     dest.d1.Surface.Canvas.Pen.Color;
    dest.d1.Surface.Canvas.Release;
    dest.FlipRequest;
  end else
end;

procedure Tfsdraw.FormCreate(Sender: TObject);
begin
  dest := nil;
end;

end.

