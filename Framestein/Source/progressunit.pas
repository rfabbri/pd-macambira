unit progressunit;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs,
  ComCtrls;

type
  TProgress = class(TForm)
    pb1: TProgressBar;
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  Progress: TProgress;

implementation

{$R *.DFM}

end.
