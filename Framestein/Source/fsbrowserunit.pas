unit fsbrowserunit;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs,
  fsframeunit, Filez, DIB, Menus, DXDraws, ExtCtrls, OleCtrls, SHDocVw,
  C2PhotoShopHost, ExtDlgs, StdCtrls;

type
  Tfsbrowser = class(Tfsframe)
    br: TWebBrowser;
    procedure FormCreate(Sender: TObject);
    procedure brBeforeNavigate2(Sender: TObject; const pDisp: IDispatch;
      var URL, Flags, TargetFrameName, PostData, Headers: OleVariant;
      var Cancel: WordBool);
  private
    { Private declarations }
  public
    { Public declarations }
    PdName: String;
    procedure Parse(const S: String); override;
    procedure CopyToSurface;
  end;

var
  fsbrowser: Tfsbrowser;

implementation

uses
  ActiveX,
  Strz, mainunit;

{$R *.DFM}

{ Tfsbrowser }

procedure Tfsbrowser.Parse(const S: String);
var
  s1, s2: String;
begin
  s1 := Trim(UpperCase(ExtractWord(1, S, [' '])));
  if s1='' then Exit;
  if (s1[1] in ['0'..'9']) and
   (Pos('X', s1)>0) then begin
    Width := MyStrToInt(ExtractWord(1, s1, ['X']));
    Height := MyStrToInt(ExtractWord(2, s1, ['X']));
  end else
  if s1='OPEN' then begin
    s2 := Copy(S, Length(s1)+2, 255);
    br.Navigate(s2);
  end else
  if s1='STOP' then begin
    br.Stop;
  end else
  if s1='REFRESH' then begin
    br.Refresh;
  end else begin
    if
     (UpperCase(Copy(S, 1, 4))='HTTP') or
     (UpperCase(Copy(S, 1, 4))='FILE')
    then
      br.Navigate(S)
    else
      inherited Parse(S);
  end;
end;

procedure Tfsbrowser.FormCreate(Sender: TObject);
begin
  inherited;
  ClientWidth := 480;
  ClientHeight := 360;
end;

procedure Tfsbrowser.CopyToSurface;
var
  DrawRect: TRect;
begin
  if (br.ClientWidth<>d1.Surface.Width) or
   (br.ClientHeight<>d1.Surface.Height) then
    d1.SetSize(br.ClientWidth, br.ClientHeight);

  DrawRect := Rect(0, 0, d1.Surface.Width, d1.Surface.Height);
  (br.Document as IViewObject).Draw(
   DVASPECT_CONTENT, 1, nil, nil, 0,
   d1.Surface.Canvas.Handle, @DrawRect, nil, nil, 0);

  d1.Surface.Canvas.Release;
end;

procedure Tfsbrowser.brBeforeNavigate2(Sender: TObject;
  const pDisp: IDispatch; var URL, Flags, TargetFrameName, PostData,
  Headers: OleVariant; var Cancel: WordBool);
var
  St: String;
begin
  if UpperCase(Copy(URL, 1, 3))='FS:' then begin
    St := Copy(URL, 4, 255);

    St := SearchAndReplace(St, '%exepath',
     ExtractFilePath(Application.ExeName));
    St := SearchAndReplace(St, '//', '/');
    St := SearchAndReplace(St, '\/', '/');

    main.SendReturnValuesString(PdName+'link', St);
    Cancel:=True;
  end;
end;

end.

