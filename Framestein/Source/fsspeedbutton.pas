unit fsspeedbutton;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs,
  Buttons;

type
  TFSSpeedButton = class(TSpeedButton)
  private
    { Private declarations }
    procedure MyOnClick(Sender: TObject);
  protected
    { Protected declarations }
  public
    { Public declarations }
    Receivename: String;
    constructor Create(AOwner: TComponent); override;
  published
    { Published declarations }
  end;

procedure Register;

implementation

uses
  mainunit;

procedure Register;
begin
end;

{ TFSSpeedButton }

constructor TFSSpeedButton.Create(AOwner: TComponent);
begin
  inherited;
  OnClick := MyOnClick;
end;

procedure TFSSpeedButton.MyOnClick(Sender: TObject);
begin
  main.SendReturnValues(Receivename+'=1');
end;

end.
