{ Copyright (C) 2001-2002 Juha Vehviläinen
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.}

unit mainunit;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs,
  StdCtrls, ScktComp,
  pluginunit, fsformunit, fsframeunit,
  FastDIB,
  ExtCtrls, Menus, C2PhotoShopHost, Filez, ComCtrls, Buttons;

type
  Tmain = class(TFsForm)
    ss1: TServerSocket;
    ImageLogo: TImage;
    PopupMenu1: TPopupMenu;
    MiConfig: TMenuItem;
    MiReset: TMenuItem;
    MiReloadPlugins: TMenuItem;
    ssfs: TServerSocket;
    csfs: TClientSocket;
    MiLog: TMenuItem;
    csToPd: TClientSocket;
    MiExit: TMenuItem;
    REConsole: TRichEdit;
    MiToolbar: TMenuItem;
    procedure ss1ClientRead(Sender: TObject; Socket: TCustomWinSocket);
    procedure FormCreate(Sender: TObject);
    procedure ss1ClientError(Sender: TObject; Socket: TCustomWinSocket;
      ErrorEvent: TErrorEvent; var ErrorCode: Integer);
    procedure ss1ClientDisconnect(Sender: TObject;
      Socket: TCustomWinSocket);
    procedure MiConfigClick(Sender: TObject);
    procedure ss1ClientConnect(Sender: TObject; Socket: TCustomWinSocket);
    procedure MiResetClick(Sender: TObject);
    procedure FormDestroy(Sender: TObject);
    procedure MiReloadPluginsClick(Sender: TObject);
    procedure ssfsClientRead(Sender: TObject; Socket: TCustomWinSocket);
    procedure csfsError(Sender: TObject; Socket: TCustomWinSocket;
      ErrorEvent: TErrorEvent; var ErrorCode: Integer);
    procedure csfsDisconnect(Sender: TObject; Socket: TCustomWinSocket);
    procedure ssfsClientError(Sender: TObject; Socket: TCustomWinSocket;
      ErrorEvent: TErrorEvent; var ErrorCode: Integer);
    procedure csfsConnect(Sender: TObject; Socket: TCustomWinSocket);
    procedure ssfsClientConnect(Sender: TObject; Socket: TCustomWinSocket);
    procedure ssfsClientDisconnect(Sender: TObject;
      Socket: TCustomWinSocket);
    procedure MiLogClick(Sender: TObject);
    procedure csToPdError(Sender: TObject; Socket: TCustomWinSocket;
      ErrorEvent: TErrorEvent; var ErrorCode: Integer);
    procedure csToPdConnect(Sender: TObject; Socket: TCustomWinSocket);
    procedure csToPdDisconnect(Sender: TObject; Socket: TCustomWinSocket);
    procedure MiExitClick(Sender: TObject);
    procedure ImageLogoDblClick(Sender: TObject);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
    procedure MiToolbarClick(Sender: TObject);
  private
    { Private declarations }
    SocketMem: Pointer;
    fbmp: TFastDIB;

    procedure LoadRegSettings;
    procedure ParsePrim(const S: String);
    procedure FreeIfCompExists(const S: String);
    function GetFrameByTag(const tag: String): TFsFrame;
    function ItemCount( const ClassName: String ): Integer;
    procedure Reset;
    procedure minimizeall;
    procedure ReDock;
    procedure DropFileHandler(const h: HWND; const DroppedFileName: String);
    procedure AppMessage(var Msg: Tmsg; var Handled: Boolean);
    procedure ExceptionHandler(Sender: TObject; E: Exception);
  public
    { Public declarations }
    RunConfig: Boolean;
    PdHost: String;          // Host running PD
    PDReceivePort: Integer;  // data from PD
    PDSendPort: Integer;     // data to PD
    FSPort: Integer;         // Framestein connections
    EnableFSConns: Boolean;
    DockMain: Boolean;
    logstate: Boolean;
    Plugins: TPlugins;
    SearchPath: TStringList;
    FSFolder: String;

    function CompName(const S: String): String;
    procedure Parse(const S: String); override;
    procedure Post(const S: String);
    procedure SendFrame(const f: TFsFrame;
     const NameTag: String; const bmp: TFastDIB;
     const quality: Integer; const sendjpg: Boolean );
    procedure SendReturnValues(const S: String);
    procedure SendReturnValuesString(
     const PdName: String; const S: String);
    function FileExistsInSearchPath(var S: String): Boolean; // modifying S allowed!
  end;

{$IFDEF FSDLL}
  TMainThread = class(TThread)
  public
    procedure Execute; override;
  end;
{$ENDIF}

const
{$IFDEF FSDLL}
  STARTMSG = 'FramesteinLib 0.33 DLL running...';
{$ELSE}
  STARTMSG = 'Framestein 0.33 running...';
{$ENDIF}
  MCAPTION = 'Framestein 0.33';
  SocketBufferSize = 100000;

var
  main: Tmain;
  DockTitle: String;
  DockHandle: HWND;

{$IFDEF FSDLL}
  MainT: TMainThread;
{$ENDIF}

function WinEnumerator(h: HWND; i: LongInt): BOOL; stdcall;
function WinEnumerator_Exact(h: HWND; i: LongInt): BOOL; stdcall;
function WinEnumerator_SubStr(h: HWND; i: LongInt): BOOL; stdcall;

implementation

{$R *.DFM}

uses
  Registry, DirectX, DxDraws, ShellApi,
  fscopyunit, fstextunit, fsdrawunit, fsbrowserunit,
  fsinfounit, fsaviunit,
  fastfiles,
  Strz, logunit, configureunit, progressunit, toolbarunit;

{$IFDEF FSDLL}
procedure TMainThread.Execute;
begin
  Application.Initialize;
  Application.Title := '';
  Application.CreateForm(Tmain, main);
  Application.CreateForm(Tlog, log);
  Application.CreateForm(Tconfigure, configure);
  Application.CreateForm(TProgress, Progress);
  Application.Run;
// ???
  Application.OnException := nil;
  Application.OnMessage := nil;
end;
{$ENDIF}

function WinEnumerator(h: HWND; i: LongInt): BOOL; stdcall;
var
  Title: array[0..255] of Char;
begin
  Result := True;
  if GetWindowText(h, Title, SizeOf(Title))>0 then begin
    if UpperCase(DockTitle)=
     Copy(UpperCase(StrPas(@Title)), 1, Length(DockTitle)) then begin
      DockHandle := h;
      Result := False; // stop enumerating
    end;
  end;
end;

function WinEnumerator_SubStr(h: HWND; i: LongInt): BOOL; stdcall;
var
  Title: array[0..255] of Char;
begin
  Result := True;
  if GetWindowText(h, Title, SizeOf(Title))>0 then begin
    if Pos(UpperCase(DockTitle), UpperCase(StrPas(@Title)))>0 then begin
      DockHandle := h;
      Result := False; // stop enumerating
    end;
  end;
end;

function WinEnumerator_Exact(h: HWND; i: LongInt): BOOL; stdcall;
var
  Title: array[0..255] of Char;
begin
  Result := True;
  if GetWindowText(h, Title, SizeOf(Title))>0 then begin
    if UpperCase(DockTitle)=
     UpperCase(StrPas(@Title)) then begin
      DockHandle := h;
      Result := False; // stop enumerating
    end;
  end;
end;

{ TForm1 }

function Tmain.ItemCount( const ClassName: String ): Integer;
var
  i, count: Integer;
begin
  count:=0;
  if ComponentCount>0 then
    for i:=1 to ComponentCount-1 do
      if UpperCase(Components[i].ClassName)=UpperCase(ClassName) then
        Inc(count);
  Result := count;
end;

function Tmain.CompName(const S: String): String;
var
  St: String;
  a: Integer;
begin
  St := S;
  if Length(St)>0 then
    for a:=Length(St) downto 1 do
      if St[a] in ['-','.',',',#10,#13] then
        Delete(St, a, 1);

  if Length(St)>0 then
    for a:=1 to Length(St) do
      if St[a] in ['0'..'9'] then
        St[a] := Char(Ord('A')+(Ord(St[a])-Ord('0')));

  Result := St;
end;

procedure Tmain.ParsePrim(const S: String);
var
  s1, s2, s_args: String;
  i: Integer;
  fsframe: TFsFrame;
  fscopy: TFsCopy;
  fstext: TFsText;
  fsdraw: TFsDraw;
  fsbrowser: TFsBrowser;
  fsinfo: TFsInfo;
  fsavi: TFsAvi;
  f: TFsForm;
begin
  if logstate then
    log.add(s);

  s1 := UpperCase(ExtractWord(1, S, [' ']));
  s_args := Copy(S, Length(s1)+2, 255);

  f := TFsForm(FindComponent(CompName(s1)));
  if f<>nil then begin
    f.Parse(s_args);
    Exit;
  end;

  s2 := ExtractWord(2, S, [' ']);

  if S1='RESET' then begin
    Reset;
  end else
  if S1='FRAME' then begin
    FreeIfCompExists(CompName(s2));
    fsframe := TFsFrame.Create(Self);
    fsframe.PdName := s2;
    fsframe.Name := CompName(s2);
    fsframe.Caption := s2;
    i := ItemCount('TFsFrame')-1;
    fsframe.Left := fsframe.Width*(i div 4);
    fsframe.Top := fsframe.Height*(i mod 4);
    fsframe.Show;
  end else
  if S1='COPY' then begin
    FreeIfCompExists(CompName(s2));
    fscopy := TFsCopy.Create(Self);
    fscopy.Name := CompName(s2);
    fscopy.Caption := s2;
  end else
  if S1='TEXT' then begin
    FreeIfCompExists(CompName(s2));
    fstext := TFsText.Create(Self);
    fstext.Name := CompName(s2);
    fstext.Caption := s2;
  end else
  if S1='DRAW' then begin
    FreeIfCompExists(CompName(s2));
    fsdraw := TFsDraw.Create(Self);
    fsdraw.Name := CompName(s2);
    fsdraw.Caption := s2;
  end else
  if S1='BROWSER' then begin
    FreeIfCompExists(CompName(s2));
    fsbrowser := TFsBrowser.Create(Self);
    fsbrowser.PdName := s2;
    fsbrowser.Name := CompName(s2);
    fsbrowser.Caption := s2;
    i := ItemCount('TFsBrowser')-1;
    fsbrowser.Left := fsbrowser.Width*(i div 4);
    fsbrowser.Top := fsbrowser.Height*(i mod 4);
    fsbrowser.Show;
  end else
  if S1='INFO' then begin
    FreeIfCompExists(CompName(s2));
    fsinfo := TFsInfo.Create(Self);
    fsinfo.Name := CompName(s2);
    fsinfo.Caption := s2;
  end else
  if S1='AVI' then begin
    FreeIfCompExists(CompName(s2));
    fsavi := TFsAvi.Create(Self);
    fsavi.PdName := s2;
    fsavi.Name := CompName(s2);
    fsavi.Caption := s2;
  end else
  if S1='MINIMIZEALL' then begin
    minimizeall;
  end else
  if S1='REDOCK' then begin
    ReDock;
  end else
  if s1='PATH' then begin
    SearchPath.Add(Copy(S, Length(s1)+2, 255));
  end else
end;

procedure Tmain.Parse(const S: String);

  function ComponentCreator(const Str: String): Boolean;
  var
    s1: String;
  begin
    s1 := UpperCase(ExtractWord(1, Str, [' ']));
    Result :=
     (s1='RESET') or
     (s1='FRAME') or
     (s1='COPY') or
     (s1='TEXT') or
     (s1='DRAW') or
     (s1='BROWSER') or
     (s1='INFO') or
     (s1='AVI') or
     (s1='MEMO');
  end;

var
  i, c: Integer;
  Str: String;
begin
  if Pos(';', S)=0 then
    ParsePrim(S)
  else begin
    c := WordCount(S, [';']);
    // first commands that create fs.* objects
    for i:=1 to c do begin
      Str := ExtractWord(i, S, [';']);
      if ComponentCreator(Str) then
        ParsePrim(Str);
    end;
    // all the rest
    for i:=1 to c do begin
      Str := ExtractWord(i, S, [';']);
      if not ComponentCreator(Str) then
        ParsePrim(Str);
    end;
  end;
end;

procedure Tmain.ss1ClientRead(Sender: TObject; Socket: TCustomWinSocket);
var
  S: String;
begin
  S := socket.ReceiveText;
  while Pos(#10, S)>0 do
    Delete(S, Pos(#10, S), 1);
  Parse(S);
end;

procedure Tmain.FreeIfCompExists(const S: String);
var
  c: TComponent;
begin
  c :=  FindComponent(S);
  if c<>nil then
    c.Free;
end;

procedure Tmain.FormCreate(Sender: TObject);
begin
  FSFolder := ExtractFilePath(Application.Exename);
  Randomize;
  RunConfig:=False;
  PdHost := 'localhost';
  PDReceivePort := 6001;
  PDSendPort := 6002;
  FSPort := 6010;
  EnableFSConns := False;
  DockMain := True;
  LoadRegSettings;
  if not RunConfig then begin
    ss1.Port := PDReceivePort;
    ss1.Active := True;
    csToPd.Host := PdHost;
    csToPd.Port := PdSendPort;
    csToPd.Active := True;
    ssfs.Port := FSPort;
    ssfs.Active := EnableFSConns;
  end;

  main.Post(STARTMSG);
  Caption := MCAPTION;

  Plugins := TPlugins.Create(Self);
  Plugins.Load;

  SocketMem := AllocMem(SocketBufferSize);
  fBmp := TFastDIB.Create;
  logstate := False;

  SearchPath := TStringList.Create;
  SearchPath.Add(FSFolder);

  Application.OnException := ExceptionHandler;
  Application.OnMessage := AppMessage;
end;

procedure Tmain.FormDestroy(Sender: TObject);
begin
  Plugins.Free;
  SearchPath.Free;
  FreeMem(SocketMem);
  fBmp.Free;
end;

procedure Tmain.ss1ClientConnect(Sender: TObject;
  Socket: TCustomWinSocket);
begin
  Post('Connected on port '+IntToStr(PDReceivePort));
  if not csToPd.Active then
    csToPd.Active := True;

// just a crazy idea
  if DockMain then begin
    DockTitle := 'pd';
    DockHandle:=0;
    EnumWindows(@WinEnumerator_Exact, 0);
    if DockHandle>0 then begin
      ParentWindow := DockHandle;
      BorderStyle := bsNone;
      Left := 4;
      Top := 120;
      Width := 40;
      Height := 40;
      BringToFront;
    end;
  end;
end;

procedure Tmain.ss1ClientDisconnect(Sender: TObject;
  Socket: TCustomWinSocket);
begin
  Post('Disconnected');
  csToPd.Active := False;
  Reset;
end;

procedure Tmain.csToPdConnect(Sender: TObject; Socket: TCustomWinSocket);
begin
  Post('Connected on port '+IntToStr(PDSendPort));
end;

procedure Tmain.csToPdDisconnect(Sender: TObject;
  Socket: TCustomWinSocket);
begin
  Post('Disconnected');
end;

procedure Tmain.ss1ClientError(Sender: TObject; Socket: TCustomWinSocket;
  ErrorEvent: TErrorEvent; var ErrorCode: Integer);
begin
  Post('Disconnected');
  ErrorCode := 0;
end;

procedure Tmain.LoadRegSettings;
var
  Reg: TRegistry;
begin
  Reg := TRegistry.Create;
  try
    Reg.RootKey := HKEY_CURRENT_USER;
    if Reg.OpenKey('\Software\Framestein', True) then begin
      PdHost := Reg.ReadString('PdHost');
      PDReceivePort := Reg.ReadInteger('PDReceivePort');
      PDSendPort := Reg.ReadInteger('PDSendPort');
      FSPort := Reg.ReadInteger('FSPort');
      EnableFSConns := Reg.ReadBool('EnableFSConns');
      DockMain := Reg.ReadBool('DockMain');
{$IFDEF FSDLL}
      FSFolder := Reg.ReadString('FSFolder');
      if FSFolder='' then
        FSFolder := ExtractFilePath(Application.Exename);
{$ENDIF}
    end;
  except
    RunConfig := True;
  end;
  Reg.CloseKey;
  Reg.Free;
end;

procedure Tmain.MiConfigClick(Sender: TObject);
begin
  configure.Execute;
end;

procedure Tmain.Post(const S: String);
begin
  Writeln(TimeToStr(Time), ' ', S);
//  REConsole.Lines.Add(TimeToStr(Time)+' '+S);
end;

procedure Tmain.Reset;
var
  i: Integer;
begin
  if ComponentCount=0 then Exit;
  for i:=ComponentCount-1 downto 0 do
    if Components[i] is TfsForm then
      Components[i].Free;
end;

procedure Tmain.MiResetClick(Sender: TObject);
begin
  Reset;
end;

procedure Tmain.MiReloadPluginsClick(Sender: TObject);
begin
  Plugins.Reload;
  Toolbar.UpdateLists;
end;

function Tmain.GetFrameByTag(const tag: String): TFsFrame;
var
  i: Integer;
  fsframe: TFsFrame;

  function makenew: TfsFrame;
  begin
    FreeIfCompExists(CompName(tag));
    fsframe := TFsFrame.Create(Self);
    fsframe.Name := CompName(tag);
    fsframe.Caption := tag;
    fsframe.NameTag := tag;
    fsframe.Show;
    Result := fsframe;
  end;

begin
  if ComponentCount=0 then begin
    Result := makenew;
    Exit;
  end;
  for i:=ComponentCount-1 downto 0 do
    if Components[i] is TfsFrame then begin
      fsframe := Components[i] as TfsFrame;
      if fsframe.NameTag = tag then begin
        Result := fsframe;
        Exit;
      end;
    end;
  Result := makenew;
end;

const
  fsframeHeader = '!FS';
  fsframeHeaderLen = 27;

procedure Tmain.SendFrame(const F: TFsFrame;
  const NameTag: String; const bmp: TFastDIB;
  const quality: Integer; const sendjpg: Boolean );

  function Header(const _size: Integer): String;
  var
    s, s2: String;
  begin
    s2 := Copy(IntToStr(f.d1.Surface.Width), 1, 4);
    while Length(s2)<4 do s2:='0'+s2;
    s := fsframeHeader+s2;
    s2 := Copy(IntToStr(f.d1.Surface.Height), 1, 4);
    while Length(s2)<4 do s2:='0'+s2;
    s := s+s2;
    s2 := Copy(IntToStr(_size), 1, 7);
    while Length(s2)<7 do s2:='0'+s2;
    s := s+s2;
    s2 := Copy(NameTag, 1, 8);
    while Length(s2)<8 do s2:=s2+' ';
    s := s+s2;
    if sendjpg then s:=s+'1' else s:=s+'0';
    Result := s;
  end;

var
  sd: TDDSurfaceDesc;
  jSize, dSize: Integer;
  JP: Pointer;
begin
  if not csfs.Active then begin
    Post('send: not connected');
    Exit;
  end;
  if sendjpg then begin
    JP := AllocMem(bmp.Size);
    jSize := SaveJPGMem(bmp, JP, bmp.Size, quality);
    while csfs.Socket.SendText(Header(jSize))=-1 do
      Application.HandleMessage;
    while csfs.Socket.SendBuf(JP^, jSize)=-1 do
      Application.HandleMessage;
    FreeMem(JP);
  end else begin
    f.d1.Surface.Lock(sd);
    dSize := sd.lPitch*f.d1.Surface.Height+f.d1.Surface.Width;
    while csfs.Socket.SendText(Header(dSize))=-1 do
      Application.HandleMessage;
    while csfs.Socket.SendBuf(sd.lpSurface^, dSize)=-1 do
      Application.HandleMessage;
    f.d1.Surface.UnLock;
  end;
end;

// TODO: this procedure is more confusing than it needs to be
procedure Tmain.ssfsClientRead(Sender: TObject; Socket: TCustomWinSocket);
const
  Receiving: Boolean = False;
  Recd: Integer = 0;
  fWidth: Integer = 0;
  fHeight: Integer = 0;
  fSize: Integer = 0;
  fJpg: Boolean = True;
  nametag: String = '';
  Buf: Pointer = nil;
  BufPos: Integer = 0;
var
  read: Integer;
  tp: Pointer;
  s: String;
  f: TFsFrame;
  sd: TDDSurfaceDesc;
  temp: array[0..255] of Char;

  // header was received, extract data about incoming frame
  procedure GetHeader;
  var
    i: Integer;
  begin
    S := StrPas(StrLCopy(tp, PChar(SocketMem), fsframeHeaderLen));
    fWidth := MyStrToInt(Copy(S, 4, 4));
    fHeight := MyStrToInt(Copy(S, 8, 4));
    fSize := MyStrToInt(Copy(S, 12, 7));
    nametag := Trim(Copy(S, 19, 8));
    fJpg := Boolean(S[27]='1');
{    Post('Width: '+IntToStr(fWidth)+' Height: '+
     IntToStr(fHeight)+' Size: '+IntToStr(fSize)+
     ' nametag: '+nametag+' jpg: '+IntToStr(Integer(fJpg)));
}
    Receiving := True;
    Recd := 0;
    Buf := AllocMem(fSize);
    BufPos := 0;
    if read>fsframeHeaderLen then begin
      i := read-fsframeHeaderLen;
      if fSize<i then
        i := fSize;
      Move(Pointer(Integer(SocketMem)+fsframeHeaderLen)^,
       Pointer(Integer(Buf)+BufPos)^, i);
      Inc(BufPos, read-fsframeHeaderLen);
      Inc(Recd, read-fsframeHeaderLen);
    end;
  end;

  // frame is complete, display it
  function DoFrame: Boolean;
  var
    offset: Integer;
  begin
    Result := False;
    f := GetFrameByTag(nametag);
    if (f.d1.Surface.Width<>fWidth) or (f.d1.Surface.Height<>fHeight) then
      f.d1.SetSize(fWidth, fHeight);

    if fJpg then begin
      LoadJPGMem(fBmp, Buf, fSize, False);
      fBmp.Draw(f.d1.Surface.Canvas.Handle, 0, 0);
      f.d1.Surface.Canvas.Release;
    end else begin
      f.d1.Surface.Lock(sd);
      Move(Buf^, sd.lpSurface^, fSize);
      f.d1.Surface.UnLock;
    end;
    f.FlipRequest;
    FreeMem(Buf);
    Receiving := False;

    if BufPos>fSize then begin
      // read: bytes read from socket
      // bufpos: amount of total data from this and previous reads
      // fsize: bytes needed for this frame
      // bufpos - fsize: bytes read belonging to next frame
      // read - (bufpos - fsize): offset for the next frame
      offset := read - (BufPos - fSize);
      read := (BufPos - fSize);
      Move(Pointer(Integer(SocketMem)+offset)^, SocketMem^, read);
      Result := True;
    end;
  end;

begin
  tp := @temp;
  read := Socket.ReceiveBuf(SocketMem^, 100000);

  while True do begin
//    Post('read '+IntToStr(read));
    if Receiving then begin
      Inc(Recd, read);

      if BufPos+read > fSize then
        Move(SocketMem^, Pointer(Integer(Buf)+BufPos)^, fSize-BufPos)
      else
        Move(SocketMem^, Pointer(Integer(Buf)+BufPos)^, read);

      Inc(BufPos, read);

      if Recd>=fSize then
        if DoFrame then
          Continue;
      Exit;
    end;
    if (StrLComp(PChar(SocketMem), fsframeHeader, Length(fsframeHeader))=0) then begin
      GetHeader;
      if Recd>=fSize then
        if DoFrame then
          Continue;
    end;
    Break;
  end;
end;

procedure Tmain.csfsError(Sender: TObject; Socket: TCustomWinSocket;
  ErrorEvent: TErrorEvent; var ErrorCode: Integer);
begin
  ErrorCode := 0;
end;

procedure Tmain.ssfsClientError(Sender: TObject; Socket: TCustomWinSocket;
  ErrorEvent: TErrorEvent; var ErrorCode: Integer);
begin
  ErrorCode := 0;
end;

procedure Tmain.csToPdError(Sender: TObject; Socket: TCustomWinSocket;
  ErrorEvent: TErrorEvent; var ErrorCode: Integer);
begin
  ErrorCode := 0;
end;

procedure Tmain.csfsConnect(Sender: TObject; Socket: TCustomWinSocket);
begin
  Post('connected '+csfs.Host+' '+IntToStr(csfs.Port));
end;

procedure Tmain.csfsDisconnect(Sender: TObject; Socket: TCustomWinSocket);
begin
  Post('disconnected '+csfs.Host+' '+IntToStr(csfs.Port));
end;

procedure Tmain.ssfsClientConnect(Sender: TObject;
  Socket: TCustomWinSocket);
begin
  Post('Connection from '+Socket.RemoteAddress);
end;

procedure Tmain.ssfsClientDisconnect(Sender: TObject;
  Socket: TCustomWinSocket);
begin
  Post('Disconnected '+Socket.RemoteAddress);
end;

procedure Tmain.MiLogClick(Sender: TObject);
begin
  MiLog.Checked := not MiLog.Checked;
  log.Visible := MiLog.Checked;
  logstate := log.Visible;
end;

procedure Tmain.SendReturnValues(const S: String);
var
  i, c: Integer;
  s2: String;
begin
  if not csToPd.Active then Exit;
  c := WordCount(S, [';']);
  for i:=1 to c do begin
    s2 := ExtractWord(i, S, [';']);
    if Pos('=', s2)>0 then begin
      csToPd.Socket.SendText(
       ExtractWord(1, s2, ['='])+' '+
       ExtractWord(2, s2, ['='])+';'
      );
    end;
  end;
end;

procedure Tmain.SendReturnValuesString(
 const PdName: String; const S: String);
var
  St: String;
  i: Integer;
begin
  St := S;
  for i:=1 to Length(St) do begin
    if St[i]='\' then St[i]:='/';
    SendReturnValues(PdName+'='+Long2Str(Ord(St[i])));
  end;
  SendReturnValues(PdName+'=0');
end;

procedure Tmain.ExceptionHandler(Sender: TObject; E: Exception);
var
  i: Integer;
  f: TFsFrame;
begin
  if main=nil then Exit;
  with main do
  if Pos('1400', E.Message)>0 then begin // invalid window handle
    // check any fs.frames with invalid window handles
    // (due to closing a patch with docked fs.frames)

(* // Do nothing - fs.frames can be redocked

    if ComponentCount=0 then Exit;
    for i:=ComponentCount-1 downto 0 do
      if Components[i] is TFsFrame then begin
        f := Components[i] as TFsFrame;
        if (f.ParentWindow>0) and
         not IsWindow(f.ParentWindow) then begin
// Q: close or undock fs.frame?
// A: this will not happed immediately when a patch is closed,
// so closing the frame is more intuitive than popping it up
// after a while.
// A2: Nonetheless, let's see how popping it up feels.
//          f.Free;
          f.ParentWindow := 0;
          f.Borders(True);
        end;
      end else
*)
  end else
    ShowMessage(E.Message);
end;

procedure Tmain.DropFileHandler(const h: HWND; const DroppedFileName: String);
var
  i: Integer;
  f: TFsFrame;
begin
  if ComponentCount=0 then Exit;
  for i:=ComponentCount-1 downto 0 do
    if Components[i] is TFsFrame then begin
      f := Components[i] as TFsFrame;
      if f.Handle=h then begin
        f.HandleDroppedFile(DroppedFileName);
        Break;
      end;
    end;
end;

procedure Tmain.AppMessage(var Msg: Tmsg; var Handled: Boolean);
const
  BufferLength : word = 255;
var
  DroppedFilename : string;
  FileIndex : Longword;
  QtyDroppedFiles : word;
  pDroppedFilename : array [0..255] of Char;
//  DroppedFileLength : word;
begin
  if Msg.Message = WM_DROPFILES then begin
    FileIndex := $FFFFFFFF;
    QtyDroppedFiles := DragQueryFile(Msg.WParam, FileIndex,
     pDroppedFilename, BufferLength);
    for FileIndex := 0 to (QtyDroppedFiles - 1) do begin
//      DroppedFileLength :=
      DragQueryFile(Msg.WParam, FileIndex,
       pDroppedFilename, BufferLength);
      DroppedFilename := StrPas(pDroppedFilename);
      DropFileHandler(msg.HWND, DroppedFilename);
    end;
    DragFinish(Msg.WParam);
    Handled := True;
  end;
end;

procedure Tmain.minimizeall;
var
  i: Integer;
begin
  if ComponentCount=0 then Exit;
  for i:=ComponentCount-1 downto 0 do
    if Components[i] is TfsForm then
      TForm(Components[i]).WindowState := wsMinimized;
end;

procedure Tmain.ReDock;
var
  i: Integer;
  f: TFsFrame;
begin
  if ComponentCount=0 then Exit;
  for i:=ComponentCount-1 downto 0 do
    if Components[i] is TFsFrame then begin
      f := Components[i] as TFsFrame;
      if f.doRedock or ((f.ParentWindow>0) and
       not IsWindow(f.ParentWindow)) then begin
        f.Parse('REDOCK');
      end;
    end;
end;

procedure Tmain.MiExitClick(Sender: TObject);
begin
  Application.Terminate;
end;

function Tmain.FileExistsInSearchPath(var S: String): Boolean;
var
  i: Integer;
  filestr: String;
begin
  Result := FileExists(S);
  if not Result and (SearchPath.Count>0) then begin
    filestr := ExtractFileName(S);
    for i:=0 to SearchPath.Count-1 do begin
      if FileExists(SearchPath.Strings[i]+'\'+filestr) then begin
        S := SearchPath.Strings[i]+'\'+filestr;
        Result := True;
        Exit;
      end;
    end;
  end;
end;

procedure Tmain.ImageLogoDblClick(Sender: TObject);
begin
  Reset;
end;

procedure Tmain.FormClose(Sender: TObject; var Action: TCloseAction);
begin
  if main.ParentWindow<>0 then begin
    main.ParentWindow:=0;
  end;
  Action := caFree;
{$IFDEF FSDLL}
  MainT.Terminate;
{$ENDIF}
end;

procedure Tmain.MiToolbarClick(Sender: TObject);
begin
  MiToolbar.Checked := not MiToolbar.Checked;
  Toolbar.Visible := MiToolbar.Checked;
end;

end.

