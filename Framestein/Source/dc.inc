{$O+,A+,W-,R-,Q-,B-,X+,T-,P+,H+,J+,C-}

//--------------- Dream Company section ----------------------------

{$IFDEF VER93}
{$DEFINE BCB}
{$DEFINE CPB1}
{$ENDIF}

{$IFNDEF VER80}
 {$IFNDEF VER90}
  {$IFNDEF VER93}
    {$IFDEF VER110}
      {$DEFINE CPB3} { C++ Builder 3.0 or higher}
      {$DEFINE BCB}
    {$ENDIF}
    {$DEFINE D3} { Delphi 3.0 or higher }
  {$ENDIF}
 {$ENDIF}
{$ENDIF}

{$IFDEF VER120}
  {$DEFINE D4}
  {$DEFINE D3}
  {$DEFINE CPB3}
{$ENDIF}

{$IFDEF VER125}
{$DEFINE CPB4}
{$DEFINE D4}
{$DEFINE D3}
{$DEFINE CPB3}
{$DEFINE BCB}
{$ENDIF}

{$IFDEF VER130}
{$DEFINE CPB4}
{$DEFINE D4}
{$DEFINE D3}
{$DEFINE CPB3}
{$DEFINE D5}
{$ENDIF}
