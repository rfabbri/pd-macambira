program Framestein;

uses
  ActiveX,
  Forms,
  mainunit in 'mainunit.pas' {main},
  pshostunit in 'pshostunit.pas',
  effectsunit in 'effectsunit.pas',
  fsaviunit in 'fsaviunit.pas' {FsAvi},
  fsbrowserunit in 'fsbrowserunit.pas',
  fscopyunit in 'fscopyunit.pas' {fscopy},
  fsdrawunit in 'fsdrawunit.pas' {fsdraw},
  fsformunit in 'fsformunit.pas',
  fsframeunit in 'fsframeunit.pas' {fsframe},
  fsinfounit in 'fsinfounit.pas' {fsinfo},
  fstextunit in 'fstextunit.pas' {fstext},
  logunit in 'logunit.pas' {log},
  pluginunit in 'pluginunit.pas',
  progressunit in 'progressunit.pas' {Progress},
  configureunit in 'configureunit.pas' {configure},
  toolbarunit in 'toolbarunit.pas' {toolbar};

{$R *.RES}

begin
  Application.Initialize;
  Application.Title := 'Framestein';
  Application.CreateForm(Tmain, main);
  Application.CreateForm(TProgress, Progress);
  Application.CreateForm(Tconfigure, configure);
  Application.CreateForm(Tlog, log);
  Application.CreateForm(Ttoolbar, toolbar);
  Application.Run;
end.

