unit logunit;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs,
  StdCtrls, ComCtrls, ExtCtrls;

type
  Tlog = class(TForm)
    PanelLog: TPanel;
    Panel2: TPanel;
    ButtonClear: TButton;
    RELog: TRichEdit;
    PanelConsole: TPanel;
    Panel1: TPanel;
    Panel3: TPanel;
    ButtonClose: TButton;
    procedure ButtonClearClick(Sender: TObject);
    procedure FormCreate(Sender: TObject);
    procedure ButtonCloseClick(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
    procedure add(const s: String);
  end;

var
  log: Tlog;

implementation

uses mainunit;

{$R *.DFM}

procedure Tlog.add(const s: String);
begin
  RELog.Lines.Add(s);
end;

procedure Tlog.ButtonCloseClick(Sender: TObject);
begin
  main.MiLog.Click;
end;

procedure Tlog.ButtonClearClick(Sender: TObject);
begin
  main.REConsole.Lines.Clear;
  RELog.Lines.Clear;
end;

procedure Tlog.FormCreate(Sender: TObject);
begin
  main.REConsole.Parent := PanelConsole;
  main.REConsole.Align := alClient;
end;

end.
