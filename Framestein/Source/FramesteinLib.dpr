{ Copyright (C) 2001-2002 Juha Vehviläinen
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.}

Library Framestein;

{%ToDo 'Framestein.todo'}
{%ToDo 'FramesteinLib.todo'}

uses
  ActiveX,
  Forms,
  Classes,
  Windows,
  mainunit in 'mainunit.pas' {main},
  fsframeunit in 'fsframeunit.pas' {fsframe},
  fscopyunit in 'fscopyunit.pas' {fscopy},
  fstextunit in 'fstextunit.pas' {fstext},
  fsformunit in 'fsformunit.pas',
  fsdrawunit in 'fsdrawunit.pas' {fsdraw},
  effectsunit in 'effectsunit.pas',
  pluginunit in 'pluginunit.pas',
  logunit in 'logunit.pas' {log},
  fsbrowserunit in 'fsbrowserunit.pas' {fsbrowser},
  fsinfounit in 'fsinfounit.pas' {fsinfo},
  configureunit in 'configureunit.pas' {configure},
  pshostunit in 'pshostunit.pas',
  fsaviunit in 'fsaviunit.pas' {FsAvi},
  progressunit in 'progressunit.pas' {Progress};

{$R *.RES}

function framesteinlib_setup: Longint; stdcall;
begin
  mainunit.MainT := TMainThread.Create(False);
  Result := 0;
end;

exports
  framesteinlib_setup;

var
  SaveExit: Pointer;

procedure FSExit;
begin
  Application.MessageBox('exitproc called!', '', 0);
  ExitProc := SaveExit;
end;

begin
  SaveExit := ExitProc;
  ExitProc := @FSExit;
end.

