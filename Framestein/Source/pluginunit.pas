{ Copyright (C) 2001-2002 Juha Vehviläinen
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.}

unit pluginunit;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs,
  DirectX, DXDraws;

type
  TEffectProc = procedure(const Bits: Pointer;
   const lPitch, Width, Height, BitCount: Integer;
   const args: PChar;
   const ret: PChar); cdecl;

  TCopyProc = procedure(
   const Bits1: Pointer;
   const lPitch1, Width1, Height1, BitCount1: Integer;
   const Bits2: Pointer;
   const lPitch2, Width2, Height2, BitCount2: Integer;
   const args: PChar;
   const ret: PChar
   ); cdecl;

  TInfoProc = procedure(const str: PChar); cdecl;

  TPointerList = TList;
  TLibraryList = TList;

  TPlugins = class(TComponent)
  private
    EffectProcs: TPointerList;
    CopyProcs: TPointerList;
    InfoProcs: TPointerList;
    Libs: TLibraryList;
    procedure LoadHandleFile(const SearchRec: TSearchRec;
     const FullPath: String);
  public
    Names: TStringList;
    constructor Create(AOwner: TComponent); override;
    destructor Destroy; override;
    procedure Load;
    procedure Reload;
    procedure Clear;
    // IsPlugins sets Index if S is a plugin
    function IsPlugin(var Index: Integer; const s: String): Boolean;
    function CallEffect(const d: TDirectDrawSurface;
     const procIndex: Integer; const args: String): Boolean;
    function CallCopy(const d1: TDirectDrawSurface;
     const d2: TDirectDrawSurface;
     const procIndex: Integer; const args: String): Boolean;
    function Info(const procIndex: Integer): String;
  end;

implementation

uses
  mainunit,
  Filez;

type
  THandlePtr = ^THandle;

const
  EffectProcName = 'perform_effect';
  CopyProcName = 'perform_copy';
  InfoProcName = 'info';

{ TPlugins }

constructor TPlugins.Create(AOwner: TComponent);
begin
  inherited;
  Names := TStringList.Create;
  EffectProcs := TPointerList.Create;
  CopyProcs := TPointerList.Create;
  InfoProcs := TPointerList.Create;
  Libs := TLibraryList.Create;
end;

destructor TPlugins.Destroy;
begin
  Clear;
  Names.Free;
  EffectProcs.Free;
  CopyProcs.Free;
  Libs.Free;
  inherited;
end;

procedure TPlugins.Clear;
var
  hp: THandlePtr;
begin
  while Names.Count>0 do Names.Delete(0);
  while EffectProcs.Count>0 do EffectProcs.Delete(0);
  while CopyProcs.Count>0 do CopyProcs.Delete(0);
  while Libs.Count>0 do begin
    hp := Libs[0];
    FreeLibrary(hp^);
    Libs.Delete(0);
  end;
end;

procedure TPlugins.LoadHandleFile(const SearchRec: TSearchRec;
  const FullPath: String);
var
  h: THandle;
  p: PChar;
  b: array[0..255] of Char;
  EffectProc: TEffectProc;
  CopyProc: TCopyProc;
  InfoProc: TInfoProc;
  s: String;
  i: Integer;
begin
  p:=@b;
  if UpperCase(ExtractFileExt(FullPath))='.DLL' then begin
    h := LoadLibrary(StrPCopy(p, FullPath));
    if h<>0 then begin
      @EffectProc := GetProcAddress(h, EffectProcName);
      @CopyProc := GetProcAddress(h, CopyProcName);
      @InfoProc := GetProcAddress(h, InfoProcName);
      s := ExtractFileName(FullPath);
      i := Pos('.DLL', UpperCase(s));
      if i>0 then Delete(s, i, 255);
      Names.Add(s);
      EffectProcs.Add(@EffectProc);
      CopyProcs.Add(@CopyProc);
      InfoProcs.Add(@InfoProc);
      Libs.Add(@h);
    end;
  end;
end;

procedure TPlugins.Load;
var
  sd: TScanDir;
  s: String;
  i: Integer;
begin
  sd := TScanDir.Create(Self);
  sd.OnHandleFile := LoadHandleFile;
  sd.Scan(main.FSFolder+'\Plugins');
  sd.Free;
  if Names.Count>0 then begin
    s := '';
    for i:=0 to Names.Count-1 do begin
      if s<>'' then s:=s+' ';
      s:=s+Names[i];
    end;
//    main.Post('Plugins ('+IntToStr(Names.Count)+' loaded): '+s);
  end;
end;

procedure TPlugins.Reload;
begin
  Clear;
  Load;
end;

function TPlugins.IsPlugin(var Index: Integer; const s: String): Boolean;
begin
  Index := Names.IndexOf(s);
  Result := Index<>-1;
end;

var
  argsBuf: array[0..255] of Char;

function TPlugins.CallEffect(const d: TDirectDrawSurface;
  const procIndex: Integer; const args: String): Boolean;
var
  Proc: TEffectProc;
  sd: TDDSurfaceDesc;
  P: PChar;
begin
  Result := False;
  if (procIndex=-1) or (procIndex>=EffectProcs.Count) then Exit;
  @Proc := EffectProcs[procIndex];
  if @Proc=nil then Exit;
  P:=@argsBuf;
  P[0]:=#0;
  d.Lock(sd);
  Proc(sd.lpSurface, sd.lPitch, d.Width, d.Height, d.BitCount,
  PChar(args), P);
  d.UnLock;
  if P[0]<>#0 then
    main.SendReturnValues(StrPas(P));
  Result := True;
end;

function TPlugins.CallCopy(const d1, d2: TDirectDrawSurface;
  const procIndex: Integer; const args: String): Boolean;
var
  Proc: TCopyProc;
  sd1: TDDSurfaceDesc;
  sd2: TDDSurfaceDesc;
  P: PChar;
begin
  Result := False;
  if (procIndex=-1) or (procIndex>=CopyProcs.Count) then Exit;
  @Proc := CopyProcs[procIndex];
  if @Proc=nil then Exit;
  P:=@argsBuf;
  P[0]:=#0;
  d1.Lock(sd1);
  d2.Lock(sd2);
  Proc(sd1.lpSurface, sd1.lPitch, d1.Width, d1.Height, d1.BitCount,
   sd2.lpSurface, sd2.lPitch, d2.Width, d2.Height, d2.BitCount,
   PChar(args), P);
  d1.UnLock;
  d2.UnLock;
  if P[0]<>#0 then
    main.SendReturnValues(StrPas(P));
  Result := True;
end;

function TPlugins.Info(const procIndex: Integer): String;
var
  Proc: TInfoProc;
  buf: array[0..255] of Char;
  p: PChar;
begin
  if (procIndex=-1) or (procIndex>=InfoProcs.Count) then Exit;
  @Proc := InfoProcs[procIndex];
  if @Proc=nil then begin
    Result := '';
    Exit;
  end;
  buf[0]:=#0;
  p := @buf;
  Proc(p);
  Result := StrPas(p);
end;

end.

