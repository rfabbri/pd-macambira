unit toolbarunit;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs,
  ComCtrls, ToolWin, Buttons, ExtCtrls, StdCtrls, ImgList,
  SendKeys, Filez;

type
  Ttoolbar = class(TForm)
    bar: TStatusBar;
    sd: TScanDir;
    Panel3: TPanel;
    PageControl1: TPageControl;
    TabSheet1: TTabSheet;
    TabSheet2: TTabSheet;
    Panel2: TPanel;
    LVTools: TListView;
    m1: TMemo;
    Panel1: TPanel;
    LVFilters: TListView;
    procedure FormCreate(Sender: TObject);
    procedure LVFiltersCustomDrawItem(Sender: TCustomListView;
      Item: TListItem; State: TCustomDrawState; var DefaultDraw: Boolean);
    procedure LVFiltersChange(Sender: TObject; Item: TListItem;
      Change: TItemChange);
    procedure LVFiltersSelectItem(Sender: TObject; Item: TListItem;
      Selected: Boolean);
    procedure LVToolsSelectItem(Sender: TObject; Item: TListItem;
      Selected: Boolean);
    procedure sdHandleFile(const SearchRec: TSearchRec;
      const FullPath: String);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
    procedure PageControl1Change(Sender: TObject);
  private
    { Private declarations }
    SendKey: TSendKey;
  public
    { Public declarations }
    procedure UpdateLists;
  end;

var
  toolbar: Ttoolbar;

implementation

uses
  mainunit, strz;

{$R *.DFM}

procedure Ttoolbar.sdHandleFile(const SearchRec: TSearchRec;
  const FullPath: String);
begin
  if ExtractFileExt(UpperCase(FullPath))<>'.8BF' then Exit;

  with LVFilters.Items.Add do begin
    Caption := ExtractFileName(FullPath);
    Caption := Copy(Caption, 1, Length(Caption)-4);
    Data := Pointer(-1);
  end;
end;

procedure Ttoolbar.UpdateLists;
var
  i: Integer;
begin
   // Load tools
  if FileExists(main.FSFolder+'\toolbar.txt') then begin
    LVTools.Items.Clear;
    m1.Lines.LoadFromFile(main.FSFolder+'\toolbar.txt');
    if m1.Lines.Count>0 then
      for i:=0 to m1.Lines.Count-1 do begin
        with LVTools.Items.Add do begin
          Caption := ExtractWord(1, m1.Lines[i], [';']);
          Data := Pointer(i);
        end;
      end;
  end;

  // Load plugins
  if main.Plugins.Names.Count>0 then begin
    LVFilters.Items.Clear;
    for i:=0 to main.Plugins.Names.Count-1 do begin
      with LVFilters.Items.Add do begin
        Caption := main.Plugins.Names[i];
        SubItems.Add(main.Plugins.Info(i));
        Data := Pointer(i);
      end;
    end;
  end;

  // Load photoshop-filters
  sd.Scan(main.FSFolder+'\Filters');
end;

procedure Ttoolbar.FormCreate(Sender: TObject);
begin
  UpdateLists;
  SendKey := TSendKey.Create(Self);
//  Show;
end;

const
  it: TListItem = nil;

procedure Ttoolbar.LVFiltersChange(Sender: TObject; Item: TListItem;
  Change: TItemChange);
begin

(* Trying to provide "paste objects to patches" feature,
   real toolbar-style, but:

   disabled together with CustomDrawItem -
   doesn't work well enough, might introduce new bugs etc.

  if Item.Selected then
    it := item;
  if Item.ListView.Selected=nil then it:=nil;
*)

end;

procedure Ttoolbar.LVFiltersCustomDrawItem(Sender: TCustomListView;
  Item: TListItem; State: TCustomDrawState; var DefaultDraw: Boolean);
var
  Title: array[0..255] of Char;
  s: String;
  h: THandle;
begin

(* NOT GOOD ENOUGH

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
          main.SendReturnValues('obj pd-'+S+'=msg 10 10 '+Item.Caption+';')
        else begin
          SendKey.Delay := 100;
          SendKey.TitleText := S;
          SendKey.Keys := '{^1}'+Item.Caption;
          SendKey.execute;
//          Self.Hide;
        end;
        Item.ListView.Selected := nil;
      end;
    end;
  end;
*)

end;

procedure Ttoolbar.LVFiltersSelectItem(Sender: TObject; Item: TListItem;
  Selected: Boolean);
var
  i: Longint;
  S: String;
begin
  i := Longint(Item.Data);
  if i=-1 then bar.SimpleText := Item.Caption+': Photoshop-filter'
  else begin
    S := main.Plugins.Info(i);
    if S='' then
      bar.SimpleText := Item.Caption+': <no info available>'
    else
      bar.SimpleText := Item.Caption+': '+S;
  end;
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
    if Pos(';', S)>0 then Delete(S, 1, Pos(';', S));
    bar.SimpleText := S;
  end;
end;

procedure Ttoolbar.FormClose(Sender: TObject; var Action: TCloseAction);
begin
  main.MiToolbar.Checked := False;
end;

procedure Ttoolbar.PageControl1Change(Sender: TObject);
begin
  bar.SimpleText := '';
end;

end.

