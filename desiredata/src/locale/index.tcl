say english    "English"
say francais   "Français"
say deutsch    "Deutsch"
say catala     "Català"
say espanol    "Español"
say portugues  "Português"
say brasileiro "Português do Brasil"
say italiano   "Italiano"
say euskara    "Euskara"
say bokmal     "Norsk Bokmål"
say dansk      "Dansk"
say nederlands "Nederlands"
say polski     "Polski"
say turkce     "Türkçe"

# those were made using:
# iconv -f utf-8 -t ucs-2 | od -tx2 -An | sed 's/ /\\u/g'

say russkij "\u0420\u0443\u0441\u0441\u043a\u0438\u0439"
say chinese "\u4e2d\u6587"
say nihongo "\u65e5\u672c\u8a9e"

set ::langoptions {
	english francais deutsch catala espanol portugues brasileiro
	italiano euskara bokmal dansk nederlands turkce polski russkij chinese nihongo
	
}

proc figure_out_language {language} {
 switch -regexp -- $language {
  ^(en|english)$      {list iso8859-1 english}
  ^(fr|francais)$     {list iso8859-1 francais}
  ^(de|deutsch)$      {list iso8859-1 deutsch}
  ^(ca|catala)$       {list iso8859-1 catala}
  ^(es|espanol)$      {list iso8859-1 espanol}
  ^(pt|portugues)$    {list iso8859-1 portugues}
  ^(it|italiano)$     {list iso8859-1 italiano}
  ^(nb|bokmal)$       {list iso8859-1 bokmal}
  ^(ch|chinese)$      {list utf-8     chinese}
  ^(eu|euskara)$      {list iso8859-1 euskara}
  ^(eo|esperanto)$    {list utf-8     esperanto}
  ^(pl|polski)$       {list utf-8     polski}
  ^(dk|dansk)$        {list iso8859-1 dansk}
  ^(ja|nihongo)$      {list iso8859-1 nihongo}
  ^(br|brasileiro)$   {list iso8859-1 brasileiro}
  ^(tr|turkce)$       {list utf-8     turkce}
  ^(nl|nederlands)$   {list iso8859-1 nederlands}
  ^(ru|russkij)$      {list utf-8     russkij}
  default {error "unknown language: $language"}
 }
}
