unit fsformunit;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs;

type
  TFsForm = class(TForm)
  public
    PdName: String;
    procedure Parse(const S: String); virtual; abstract;
  end;

implementation

end.
 