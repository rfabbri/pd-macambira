unit pshostunit;

interface

uses
  Forms, SysUtils, DirectX, DxDraws,
  fsframeunit, C2PhotoShopHost;

function IsFilter(const S: String): Boolean;

function Filter_effect(const psh: TC2PhotoShopHost;
 const d: TDirectDrawSurface;
 const procname: String; const args: String): Boolean;

function Filter_copy(const psh: TC2PhotoShopHost;
 const d1, d2: TDirectDrawSurface;
 const procname: String; const args: String): Boolean;

implementation

uses
  Windows, Graphics, DIB,
  mainunit;

function FullFilterPath(const S: String): String;
begin
  Result := main.FSFolder+'\Filters\'+S;
  if Uppercase(ExtractFileExt(S))<>'.8BF' then
    Result := Result+'.8BF';
end;

function IsFilter(const S: String): Boolean;
begin
  Result := FileExists(FullFilterPath(S));
end;

function Filter_effect(const psh: TC2PhotoShopHost;
 const d: TDirectDrawSurface;
 const procname: String; const args: String): Boolean;
const
  Active: Boolean = False;
var
  sd: TDDSurfaceDesc;
  dib: TDxDib;
begin
  Result := False;
  if Active then Exit; // disable recursion
  Active := True;
  if not psh.LoadFilterDLL(FullFilterPath(procname)) then begin
    main.Post('Filter load error: '+procname+'. Requires Adobe''s Plugin.dll?');
    Active := False;
    Exit;
  end;

  d.Lock(sd);
  psh.Surface := @sd;

  if d.BitCount=24 then begin
    psh.SrcPtr := sd.lpSurface;
    psh.DstPtr := sd.lpSurface;
  end else begin
    // DIB will ensure we're @ 24 bits per pixel...
    dib := TDxDib.Create(main);
    dib.DIB.Assign(d);

    psh.DIB := dib;
    psh.SrcPtr := dib.DIB.PBits;
    psh.DstPtr := dib.DIB.PBits;
  end;

  psh.CallDialog := (args='');
  psh.args := args;
  psh.RunFilter;

  d.UnLock;

  if d.BitCount<>24 then begin
    d.Assign(dib.DIB);
    dib.Free;
  end;
  Result := True;
  Active := False;
end;

function Filter_copy(const psh: TC2PhotoShopHost;
 const d1, d2: TDirectDrawSurface;
 const procname: String; const args: String): Boolean;
const
  Active: Boolean = False;
var
  sd1, sd2: TDDSurfaceDesc;
  dib1, dib2: TDxDib;
begin
  Result := False;
  if Active then Exit; // disable recursion
  Active := True;
  if not psh.LoadFilterDLL(FullFilterPath(procname)) then begin
    main.Post('Filter load error: '+procname+'. Requires Adobe''s Plugin.dll?');
    Active := False;
    Exit;
  end;

  d1.Lock(sd1);
  d2.Lock(sd2);

  psh.Surface := @sd1;

  if d1.BitCount=24 then begin
    psh.SrcPtr := sd1.lpSurface;
    psh.DstPtr := sd2.lpSurface;
  end else begin
    // DIB will ensure we're @ 24 bits per pixel...
    dib1 := TDxDib.Create(main);
    dib2 := TDxDib.Create(main);
    dib1.DIB.Assign(d1);
    dib2.DIB.SetSize(dib1.DIB.Width, dib1.DIB.Height, 24);

    psh.DIB := dib1;
    psh.SrcPtr := dib1.DIB.PBits;
    psh.DstPtr := dib2.DIB.PBits;
  end;

  psh.CallDialog := (args='');
  psh.args := args;
  psh.RunFilter;

  d1.UnLock;
  d2.UnLock;

  if d1.BitCount<>24 then begin
    d2.Assign(dib2.DIB);
    dib1.Free;
    dib2.Free;
  end;
  Result := True;
  Active := False;
end;

end.

