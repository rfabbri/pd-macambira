unit fsinfounit;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs,
  fsformunit;

type
  Tfsinfo = class(TFsForm)
  private
    { Private declarations }
  public
    { Public declarations }
    procedure Parse(const S: String); override;
  end;

var
  fsinfo: Tfsinfo;

implementation

uses
  mainunit, fsframeunit,
  Strz;

{$R *.DFM}

{ Tfsinfo }

procedure Tfsinfo.Parse(const S: String);
var
  s1: String;
  f: TFsFrame;
begin
  if (S='') or
   (WordCount(S, [' '])<5) or
   (not main.cstoPd.Active) then Exit;

  s1 := ExtractWord(1, S, [' ']);
  f := FindFrame(s1);
  if f=nil then Exit;

  main.SendReturnValues(
   ExtractWord(5, S, [' '])+'='+IntToStr(f.Avi.FrameRate)+';'+
   ExtractWord(4, S, [' '])+'='+IntToStr(f.Avi.FrameCount)+';'+
   ExtractWord(3, S, [' '])+'='+IntToStr(f.d1.Surface.Height)+';'+
   ExtractWord(2, S, [' '])+'='+IntToStr(f.d1.Surface.Width)
  );
end;

end.

