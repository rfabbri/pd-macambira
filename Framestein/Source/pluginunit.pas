{ Copyright (C) 2001 Juha Vehviläinen
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

  TPointerList = TList;
  TLibraryList = TList;

  TPlugins = class(TComponent)
  private
    Names: TStringList;
    EffectProcs: TPointerList;
    CopyProcs: TPointerList;
    Libs: TLibraryList;
    procedure LoadHandleFile(const SearchRec: TSearchRec;
     const FullPath: String);
  public
    constructor Create(AOwner: TComponent); override;
    destructor Destroy; override;
    procedure Load;
    procedure Reload;
    procedure Clear;
    function IsPlugin(const s: String): Boolean;
    function CallEffect(const d: TDirectDrawSurface;
     const procname: String; const args: String): Boolean;
    function CallCopy(const d1: TDirectDrawSurface;
     const d2: TDirectDrawSurface;
     const procname: String; const args: String): Boolean;
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

{ TPlugins }

constructor TPlugins.Create(AOwner: TComponent);
begin
  inherited;
  Names := TStringList.Create;
  EffectProcs := TPointerList.Create;
  CopyProcs := TPointerList.Create;
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
  s: String;
  i: Integer;
begin
  p:=@b;
  if UpperCase(ExtractFileExt(FullPath))='.DLL' then begin
    h := LoadLibrary(StrPCopy(p, FullPath));
    if h<>0 then begin
      @EffectProc := GetProcAddress(h, EffectProcName);
      @CopyProc := GetProcAddress(h, CopyProcName);
      s := ExtractFileName(FullPath);
      i := Pos('.DLL', UpperCase(s));
      if i>0 then Delete(s, i, 255);
      Names.Add(s);
      EffectProcs.Add(@EffectProc);
      CopyProcs.Add(@CopyProc);
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
  sd.Scan(ExtractFilePath(Application.ExeName)+'\Plugins');
  sd.Free;
  if Names.Count>0 then begin
    s := '';
    for i:=0 to Names.Count-1 do begin
      if s<>'' then s:=s+' ';
      s:=s+Names[i];
    end;
    main.Post('Plugins: '+s);
  end;
end;

procedure TPlugins.Reload;
begin
  Clear;
  Load;
end;

function TPlugins.IsPlugin(const s: String): Boolean;
begin
  Result := Names.IndexOf(s)<>-1;
end;

var
  argsBuf: array[0..255] of Char;

function TPlugins.CallEffect(const d: TDirectDrawSurface;
  const procname: String; const args: String): Boolean;
var
  i: Integer;
  Proc: TEffectProc;
  sd: TDDSurfaceDesc;
  P: PChar;
begin
  Result := False;
  if (Names.Count=0) or (d.Width=0) or (d.Height=0) then Exit;
  i := Names.IndexOf(procname);
  if (i=-1) or (i>=EffectProcs.Count) then Exit;
  @Proc := EffectProcs[i];
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
  const procname: String; const args: String): Boolean;
var
  i: Integer;
  Proc: TCopyProc;
  sd1: TDDSurfaceDesc;
  sd2: TDDSurfaceDesc;
  P: PChar;
begin
  Result := False;
  if (Names.Count=0) or
   (d1.Width=0) or (d1.Height=0) or
   (d2.Width=0) or (d2.Height=0) then Exit;
  i := Names.IndexOf(procname);
  if (i=-1) or (i>=CopyProcs.Count) then Exit;
  @Proc := CopyProcs[i];
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

end.

