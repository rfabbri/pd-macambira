unit Filez;
{$I-}

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs;

type
  THandleFileEvent = procedure( const SearchRec:TSearchRec;
   const FullPath: String ) of object;
  // fullpath contains filepath+filename

  TScanDir = class(TComponent)
  private
    { Private declarations }
    FOnHandleFile : THandleFileEvent;
  protected
    { Protected declarations }
  public
    { Public declarations }
    procedure Scan( const Path : String );
    constructor Create( AOwner:TComponent ); override;
  published
    { Published declarations }
    property OnHandleFile : THandleFileEvent read FOnHandleFile write FOnHandleFile;
  end;

  function FileSizeByName( const FileName:String ):Longint;

procedure Register;

implementation

uses
  Strz;

function FileSizeByName( const FileName:String ):Longint;
var
  F:file of Byte;
begin
  Result := 0;
  AssignFile(F, FileName);
  Reset(F);
  if IoResult<>0 then Exit;
  Result := FileSize(F);
  Close(F);
end;

constructor TScanDir.Create( AOwner:TComponent );
begin
  inherited Create( AOwner );

  FOnHandleFile := nil;
end;

procedure TScanDir.Scan( const Path : String );
var
  SearchRec : TSearchRec;
  Result : Integer;
  S : String;
begin
  if not Assigned(FOnHandleFile) then
    Exit;

  S := VerifyBackSlash(Path);
  Result := FindFirst( S+'*.*', faAnyFile, SearchRec);
  if Result=0 then
    repeat
      if (SearchRec.Name='.') or (SearchRec.Name='..') then
        Continue;

      FOnHandleFile( SearchRec,
       S+SearchRec.Name );

      if SearchRec.Attr and faDirectory>0 then
        Scan( S+SearchRec.Name );
    until FindNext(SearchRec)<>0;
end;

procedure Register;
begin
  RegisterComponents('Labrz', [TScanDir]);
end;

end.

