unit configureunit;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs,
  StdCtrls, ExtCtrls, ComCtrls, Buttons, FileCtrl;

type
  Tconfigure = class(TForm)
    PageControl1: TPageControl;
    TSConnections: TTabSheet;
    Panel1: TPanel;
    ButtonOk: TButton;
    ButtonCancel: TButton;
    Label1: TLabel;
    EditPdHost: TEdit;
    Label2: TLabel;
    EditPdReceivePort: TEdit;
    Label3: TLabel;
    EditPdSendPort: TEdit;
    Label4: TLabel;
    Label5: TLabel;
    EditFsPort: TEdit;
    CBEnableFSConns: TCheckBox;
    TSGeneral: TTabSheet;
    CBDockMain: TCheckBox;
    TSFolders: TTabSheet;
    Label6: TLabel;
    EditFSFolder: TEdit;
    Memo1: TMemo;
    DirectoryListBox1: TDirectoryListBox;
    DriveComboBox1: TDriveComboBox;
    procedure ButtonCancelClick(Sender: TObject);
    procedure ButtonOkClick(Sender: TObject);
    procedure FormCreate(Sender: TObject);
    procedure DriveComboBox1Change(Sender: TObject);
    procedure DirectoryListBox1Change(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
    procedure Execute;
  end;

var
  configure: Tconfigure;

implementation

uses
  Registry,
  mainunit;

{$R *.DFM}

procedure Tconfigure.ButtonOkClick(Sender: TObject);
var
  Reg: TRegistry;
begin
  Reg := TRegistry.Create;
  Reg.RootKey := HKEY_CURRENT_USER;

  main.PdHost := EditPdHost.Text;
  main.PdReceivePort := StrToInt(EditPdReceivePort.Text);
  main.PdSendPort := StrToInt(EditPdSendPort.Text);
  main.FSPort := StrToInt(EditFsPort.Text);
  main.EnableFSConns := CBEnableFSConns.Checked;
  main.DockMain := CBDockMain.Checked;
{$IFDEF FSDLL}
  if main.FSFolder <> EditFSFolder.Text then begin
    main.FSFolder := EditFSFolder.Text;
    main.Plugins.Clear;
    main.Plugins.ReLoad;
    main.SearchPath.Add(main.FSFolder);
  end;
{$ENDIF}

  try
    if Reg.OpenKey('\Software\Framestein', True) then begin
      Reg.WriteString('PdHost', main.PdHost);
      Reg.WriteInteger('PDReceivePort', main.PDReceivePort);
      Reg.WriteInteger('PDSendPort', main.PDSendPort);
      Reg.WriteInteger('FSPort', main.FSPort);
      Reg.WriteBool('EnableFSConns', main.EnableFSConns);
      Reg.WriteBool('DockMain', main.DockMain);
{$IFDEF FSDLL}
      Reg.WriteString('FSFolder', main.FSFolder);
{$ENDIF}
    end;
  except
  end;

  Reg.CloseKey;
  Reg.Free;

  main.ss1.Active := False;
  main.ss1.Port := main.PDReceivePort;
  main.ss1.Active := True;

  main.csToPd.Active := False;
  main.csToPd.Host := main.PdHost;
  main.csToPd.Port := main.PdSendPort;
  main.csToPd.Active := True;

  main.ssfs.Active := False;
  main.ssfs.Port := main.FSPort;
  main.ssfs.Active := main.EnableFSConns;

  ModalResult := mrOk;
end;

procedure Tconfigure.ButtonCancelClick(Sender: TObject);
begin
  ModalResult := mrCancel;
end;

procedure Tconfigure.Execute;
begin
{$IFDEF FSDLL}
  TSFolders.TabVisible := True;
  TSFolders.Visible := True;
{$ELSE}
  TSFolders.TabVisible := False;
  TSFolders.Visible := False;
{$ENDIF}

  // load values from main
  EditPdHost.Text := main.PdHost;
  if EditPdHost.Text='' then EditPdHost.Text:='localhost';
  EditPdReceivePort.Text := IntToStr(main.PdReceivePort);
  EditPdSendPort.Text := IntToStr(main.PdSendPort);
  EditFsPort.Text := IntToStr(main.FSPort);
  CBEnableFSConns.Checked := main.EnableFSConns;
  CBDockMain.Checked := main.DockMain;
  EditFSFolder.Text := main.FSFolder;
  // show
  ShowModal;
end;

procedure Tconfigure.FormCreate(Sender: TObject);
begin
  if main.RunConfig then
    Execute;
end;

procedure Tconfigure.DriveComboBox1Change(Sender: TObject);
begin
  DirectoryListBox1.Drive :=
   (Sender as TDriveComboBox).Drive;
end;

procedure Tconfigure.DirectoryListBox1Change(Sender: TObject);
begin
  EditFSFolder.Text :=
   (Sender as TDirectoryListBox).Directory;
end;

end.

