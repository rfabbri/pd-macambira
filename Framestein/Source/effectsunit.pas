{ Copyright (C) 2001 Juha Vehviläinen
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.}

unit effectsunit;
{This unit demonstrates how to work on the pixel level..

 If you're adding an effect, all it takes is a couple of lines in
 fsframeunit.pas to call your routine. Look for 'scramble' there
 to see how.

 If you're adding a new copymode, look for all occurences of
 'dsBlend' in fscopyunit.pas to see how to do that.

 Also see everything in Framestein\Plugins-directory, you can write
 effects and copymodes as plugins and never recompile framestein..
 or touch the pascal code. This is likely the way I'll be doing them
 from now on, to keep the program "core" as simple and small as possible..
}

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, DirectX, DXDraws;

procedure scramble(const d: TDirectDrawSurface);
procedure blend(const d1, d2: TDirectDrawSurface; const amount: Extended);

implementation

function ScanLine16(const sd: TDDSurfaceDesc; const y: Integer): Pointer;
begin
  Result := Pointer(Integer(sd.lpSurface) + y*sd.lPitch);
end;

procedure scramble(const d: TDirectDrawSurface);
var
  x,y: Integer;
  p16: PWordArray;
  sd: TDDSurfaceDesc;
begin
  if (d.Width=0) or (d.Height=0) then Exit;
  d.Lock(sd);
  case d.BitCount of
    16: for y:=0 to d.Height-1 do begin
      p16 := ScanLine16(sd, y);
      for x:=0 to d.Width-1 do
        p16[x]:=random(16777216);
    end;
  end;
  d.UnLock;
end;

function r16(color: cardinal): byte; register;
begin;
  result := (color shr 11) shl 3;
end;

function g16(color: cardinal): byte; register;
begin;
  result := ((color and 2016) shr 5) shl 2;
end;

function b16(color: cardinal): byte; register;
begin;
  result := (color and 31) shl 3;
end;

procedure blend(const d1, d2: TDirectDrawSurface; const amount: Extended);
var
  x, y, w, h: Integer;
  p1, p2: PWordArray;
  sd1, sd2: TDDSurfaceDesc;
  c1,c2: cardinal;
begin
  if (d1.Width=0) or (d2.Width=0) or
   (d1.BitCount<>16) or (d2.BitCount<>16) then Exit;
  w := d1.Width;
  h := d1.Height;
  if d2.Width<w then w:=d2.Width;
  if d2.Height<h then h:=d2.Height;

  d1.Lock(sd1);
  d2.Lock(sd2);
  for y:=0 to h-1 do begin
    p1:=ScanLine16(sd1, y);
    p2:=ScanLine16(sd2, y);
    for x:=0 to w-1 do begin
      c1 := p1[x];
      c2 := p2[x];
      p2[x] :=
       ((round((1-amount)*r16(c1)+amount*r16(c2)) shr 3) shl 11) or // r value shifted
       ((round((1-amount)*g16(c1)+amount*g16(c2)) shr 2) shl 5) or // g value shifted
       (round((1-amount)*b16(c1)+amount*b16(c2)) shr 3); // add blue
    end;
  end;
  d1.UnLock;
  d2.UnLock;
end;

end.

