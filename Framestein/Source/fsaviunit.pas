unit fsaviunit;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs,
  fsformunit,
  fsaviwriter, ExtCtrls, StdCtrls, Buttons;

type
  TFsAvi = class(TFsForm)
    sd1: TSaveDialog;
    procedure FormCreate(Sender: TObject);
    procedure FormDestroy(Sender: TObject);
    procedure BitBtn1Click(Sender: TObject);
    procedure Button1Click(Sender: TObject);
    procedure Button2Click(Sender: TObject);
  private
    { Private declarations }
    a: TAviWriter;
  public
    { Public declarations }
    procedure Parse(const S: String); override;
  end;

var
  FsAvi: TFsAvi;

implementation

uses
  fsframeunit, strz, mainunit;

{$R *.DFM}

{ Tavi }

procedure TFsAvi.Parse(const S: String);
var
  s1: String;
  f: TFsFrame;
begin
  if S='' then Exit;

  s1 := UpperCase(ExtractWord(1, S, [' ']));

  if s1='WRITE' then begin
    a.FileName := Copy(S, Length(s1)+2, 255);
    if a.Filename='' then
      if sd1.Execute then begin
        a.Filename := sd1.Filename;
        if UpperCase(ExtractFileExt(a.Filename))<>'.AVI' then
          a.Filename := a.Filename+'.avi';
      end else
        Exit;
    a.Finish;
    Exit;
  end else
  if s1='FPS' then begin
    a.fps := MyStrToInt(Copy(S, Length(s1)+2, 255));
    Exit;
  end;

  if not a.Prepared then
    a.Prepare;

  f := FindFrame(S);
  if f=nil then Exit;

  a.Width := f.d1.SurfaceWidth;
  a.Height := f.d1.SurfaceHeight;

  f.dDib.DIB.Assign(f.d1.Surface);
  a.AddFrame(f.dDib.DIB);
  main.SendReturnValues(PdName+'pos='+IntToStr(a.FramePos));
end;

procedure TFsAvi.FormCreate(Sender: TObject);
begin
  a := TAviWriter.Create(Self);
end;

procedure TFsAvi.FormDestroy(Sender: TObject);
begin
  a.Free;
end;

procedure TFsAvi.BitBtn1Click(Sender: TObject);
begin
  a.Write;
end;

procedure TFsAvi.Button1Click(Sender: TObject);
begin
  a.Prepare;
end;

procedure TFsAvi.Button2Click(Sender: TObject);
begin
  a.Finish;
end;

end.
