unit Strz;

interface

uses
  Windows;

type
  CharSet = set of Char;

// General
function MyStrToInt(const S: String): Integer; // some special checking, no exceptions
function MyStrToFloat(const S: String): Extended; // some special checking, no exceptions
function StrToRect(const S: String): TRect;
function Long2Str(L : LongInt) : string;
function SearchAndReplace( S, Old, New:String ):String;
function WordCount(S : string; WordDelims : CharSet) : Integer;
  {-Given a set of word delimiters, return number of words in S}
function ExtractWord(N : Integer; S : string; WordDelims : CharSet) : string;
  {-Given a set of word delimiters, return the N'th word in S}

// Filez
function VerifyBackSlash( const FileName:String ):String;

implementation

uses
  Classes, SysUtils;

function StrToRect(const S: String): TRect;
var
  i: Integer;
begin
  Result := Rect(0, 0, 0, 0);
  if S='' then Exit;
  i := WordCount(S, [' ']);
  Result.Left := MyStrToInt(ExtractWord(1, S, [' ']));
  if i>1 then
    Result.Top := MyStrToInt(ExtractWord(2, S, [' ']));
  if i>2 then
    Result.Right := MyStrToInt(ExtractWord(3, S, [' ']));
  if i>3 then
    Result.Bottom := MyStrToInt(ExtractWord(4, S, [' ']));
end;

function MyStrToInt(const S: String): Integer;
var
  St: String;
begin
  Result := 0;
  if (S='') or (S='-') or (S='0') then Exit;
  try
    Result := StrToInt(S);
  except
    St := S;
    while Pos('.', St)>0 do
      St[Pos('.', St)] := ',';
    try Result := Trunc(StrToFloat(St)); except end;
  end;
end;

function MyStrToFloat(const S: String): Extended;
var
  St: String;
  i: Integer;
begin
  Result := 0;
  if (S='') or (S='-') or (S='0') then Exit;
  St := S;
  i := Pos('.', St);
  if i>0 then
    St[i] := ',';
  try
    Result := StrToFloat(St);
  except
  end;
end;

function VerifyBackSlash( const FileName:String ):String;
begin
  if FileName[Length(FileName)]='\' then
    Result := FileName
  else
    Result := FileName + '\';
end;

function Long2Str(L : LongInt) : string;
  {-Convert a long/word/integer/byte/shortint to a string}
var
  S : string;
begin
  Str(L, S);
  Long2Str := S;
end;

{-----}

  function WordCount(S : string; WordDelims : CharSet) : Integer;
    {-Given a set of word delimiters, return number of words in S}
  var
    {I,} Count : Integer;                       {!!.12}
    I, SLen : Word;                                {!!.12}
  begin
    Count := 0;
    I := 1;
    SLen:=Length(S);

    while I <= SLen do begin
      {skip over delimiters}
      while (I <= SLen) and (S[I] in WordDelims) do
        Inc(I);

      {if we're not beyond end of S, we're at the start of a word}
      if I <= SLen then
        Inc(Count);

      {find the end of the current word}
      while (I <= SLen) and not(S[I] in WordDelims) do
        Inc(I);
    end;

    WordCount := Count;
  end;

  function ExtractWord(N : Integer; S : string; WordDelims : CharSet) : string;
    {-Given a set of word delimiters, return the N'th word in S}
  var
    I : Word;                 {!!.12}
    Count, Len : Integer;
    SLen : Longint;
    Str : String;
  begin
    Count := 0;
    I := 1;
    Len := 0;
    Str := '';
    SLen := Length(S);

//    ExtractWord[0] := #0;

    while (I <= SLen) and (Count <> N) do begin
      {skip over delimiters}
      while (I <= SLen) and (S[I] in WordDelims) do
        Inc(I);

      {if we're not beyond end of S, we're at the start of a word}
      if I <= SLen then
        Inc(Count);

      {find the end of the current word}
      while (I <= SLen) and not(S[I] in WordDelims) do begin
        {if this is the N'th word, add the I'th character to Tmp}
        if Count = N then begin
          Inc(Len);
          SetLength(Str, Len);
//          ExtractWord[0] := Char(Len);
          Str[Len] := S[I];
//          ExtractWord[Len] := S[I];
        end;

        Inc(I);
      end;
    end;
    ExtractWord := Str;
  end;

function SearchAndReplace( S, Old, New:String ):String;
var
  Ts : String;
  i : Integer;
begin
  Ts := S;
  i := Pos(Old, Ts);
  while i>0 do begin
    Delete(Ts, i, Length(Old));
    Insert(New, Ts, i);
    i := Pos(Old, Ts);
  end;
  SearchAndReplace := Ts;
end;

end.
