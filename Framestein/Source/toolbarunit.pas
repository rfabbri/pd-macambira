unit toolbarunit;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs,
  ComCtrls, ToolWin, Buttons, ExtCtrls, StdCtrls, ImgList;

type
  Ttoolbar = class(TForm)
    Panel1: TPanel;
    LVFilters: TListView;
    Panel2: TPanel;
    LVTools: TListView;
    Splitter1: TSplitter;
    bar: TStatusBar;
    m1: TMemo;
    procedure FormCreate(Sender: TObject);
    procedure LVFiltersCustomDrawItem(Sender: TCustomListView;
      Item: TListItem; State: TCustomDrawState; var DefaultDraw: Boolean);
    procedure LVFiltersChange(Sender: TObject; Item: TListItem;
      Change: TItemChange);
    procedure LVFiltersSelectItem(Sender: TObject; Item: TListItem;
      Selected: Boolean);
    procedure LVToolsSelectItem(Sender: TObject; Item: TListItem;
      Selected: Boolean);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  toolbar: Ttoolbar;

implementation

uses
  mainunit, strz;

{$R *.DFM}

procedure Ttoolbar.FormCreate(Sender: TObject);
var
  i: Integer;
begin
  // Load tools
  if FileExists(main.FSFolder+'\toolbar.txt') then begin
    m1.Lines.LoadFromFile(main.FSFolder+'\toolbar.txt');
    if m1.Lines.Count>0 then
      for i:=0 to m1.Lines.Count-1 do begin
        with LVTools.Items.Add do begin
          Caption := ExtractWord(1, m1.Lines[i], [' ']);
          Data := Pointer(i);
        end;
      end;
  end;

  // Load filters
  if main.Plugins.Names.Count>0 then
    for i:=0 to main.Plugins.Names.Count-1 do begin
      with LVFilters.Items.Add do begin
        Caption := main.Plugins.Names[i];
        SubItems.Add(main.Plugins.Info(i));
        Data := Pointer(i);
      end;
    end;
  Show;
end;

const
  it: TListItem = nil;

procedure Ttoolbar.LVFiltersChange(Sender: TObject; Item: TListItem;
  Change: TItemChange);
begin
  if Item.Selected then
    it := item;
  if Item.ListView.Selected=nil then it:=nil;
end;

procedure Ttoolbar.LVFiltersCustomDrawItem(Sender: TCustomListView;
  Item: TListItem; State: TCustomDrawState; var DefaultDraw: Boolean);
var
  Title: array[0..255] of Char;
  s, cmd: String;
  h: THandle;
begin
  if (it=nil) or (Item.Caption='') then Exit;
  if it.Caption=item.Caption then begin
    Sleep(50); // wait for pd window to get focus
    h := GetForegroundWindow;
    if GetWindowText(h, Title, SizeOf(Title))>0 then begin
      s := StrPas(@Title);
      if Pos(' - ', S)>0 then begin
        Delete(S, Pos(' - ', S), 255);
        if Pos('*', S)>0 then Delete(S, Pos('*', S), 255);
//        main.Post(Item.Caption+' -> '+S);
        if Item.ListView=LVFilters then
          cmd := 'msg'
        else
          cmd := 'obj';
        main.SendReturnValues('obj pd-'+S+'='+cmd+' 10 10 '+Item.Caption+';');
        Item.ListView.Selected := nil;
      end;
    end;
  end;
end;

procedure Ttoolbar.LVFiltersSelectItem(Sender: TObject; Item: TListItem;
  Selected: Boolean);
var
  S: String;
begin
  S := main.Plugins.Info(Integer(Item.Data));
  if S='' then
    bar.SimpleText := '<no info available>'
  else
    bar.SimpleText := Item.Caption+': '+S;
end;

procedure Ttoolbar.LVToolsSelectItem(Sender: TObject; Item: TListItem;
  Selected: Boolean);
var
  S: String;
  i: Integer;
begin
  i := Integer(Item.Data);
  if (i>=0) and (i<m1.Lines.Count) then begin
    S := m1.Lines[i];
    if Pos(' ', S)>0 then Delete(S, 1, Pos(' ', S));
    bar.SimpleText := S;
  end;
end;

end.
