{ Copyright (C) 2001 Juha Vehviläinen
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.}

unit fstextunit;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs,
  mainunit, fsformunit, fsframeunit;

type
  Tfstext = class(TFsForm)
    procedure FormCreate(Sender: TObject);
  private
    { Private declarations }
    dest: TFsFrame;
    x, y: Integer;
  public
    { Public declarations }
    procedure Parse(const S: String); override;
  end;

var
  fstext: Tfstext;

function ParsePenProperties(const Canvas: TCanvas; const S: String): Boolean;

implementation

{$R *.DFM}

uses
  Strz;

function ParsePenProperties(const Canvas: TCanvas; const S: String): Boolean;
var
  s1: String;
  r, g, b: Integer;
begin
  Result := False;
  s1 := UpperCase(ExtractWord(1, S, [' ']));
  if s1='PEN' then begin
    if WordCount(S, [' '])<4 then
      main.Post('Usage: PEN <r> <g> <b>')
    else begin
      r := MyStrToInt(ExtractWord(2, S, [' ']));
      g := MyStrToInt(ExtractWord(3, S, [' ']));
      b := MyStrToInt(ExtractWord(4, S, [' ']));
      Canvas.Pen.Color := RGB(r, g, b);
    end;
    Result := True;
  end else
  if s1='BRUSH' then begin
    if WordCount(S, [' '])<4 then
      main.Post('Usage: BRUSH <r> <g> <b>')
    else begin
      r := MyStrToInt(ExtractWord(2, S, [' ']));
      g := MyStrToInt(ExtractWord(3, S, [' ']));
      b := MyStrToInt(ExtractWord(4, S, [' ']));
      Canvas.Brush.Color := RGB(r, g, b);
    end;
    Result := True;
  end else
  if s1='COLOR' then begin
    if WordCount(S, [' '])<4 then
      main.Post('Usage: COLOR <r> <g> <b>')
    else begin
      r := MyStrToInt(ExtractWord(2, S, [' ']));
      g := MyStrToInt(ExtractWord(3, S, [' ']));
      b := MyStrToInt(ExtractWord(4, S, [' ']));
      Canvas.Font.Color := RGB(r, g, b);
    end;
    Result := True;
  end else
  if s1='SIZE' then begin
    Canvas.Font.Size := MyStrToInt(ExtractWord(2, S, [' ']));
    Result := True;
  end else
  if s1='FONT' then begin
    Canvas.Font.Name := Copy(S, Length(s1)+2, 255);
    Result := True;
  end else
end;

{ Tfstext }

procedure Tfstext.Parse(const S: String);
const
  y: Integer = 0;
var
  s1, sa: String;
  b: Boolean;
begin
  if (S='') then Exit;

  s1 := ExtractWord(WordCount(S, [' ']), S, [' ']);
  dest := FindFrame(s1);
  if (dest=nil) or (dest.d1=nil) then Exit;

  sa := Copy(S, 1, Pos(s1, S)-2);

  b := ParsePenProperties(dest.d1.Surface.Canvas, UpperCase(sa));
  dest.d1.Surface.Canvas.Release;
  if b then Exit;

  s1 := UpperCase(ExtractWord(1, sa, [' ']));
  if s1='POS' then begin
    self.x := MyStrToInt(ExtractWord(2, sa, [' ']));
    self.y := MyStrToInt(ExtractWord(3, sa, [' ']));
    Exit;
  end;

  with dest.d1.Surface.Canvas do begin
    Brush.Style := bsClear;

    if (self.x>-1) and (self.y>-1) then
      TextOut(self.x, self.y, sa)
    else begin
      TextOut(0, y, sa);

      Inc(Y, TextHeight(S));
      if y>dest.d1.Surface.Height then
        y := 0;
    end;
  end;
  dest.d1.Surface.Canvas.Release;

  dest.FlipRequest;
end;

procedure Tfstext.FormCreate(Sender: TObject);
begin
  dest := nil;
  x := -1;
  y := -1;
end;

end.

