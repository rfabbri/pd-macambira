say english    "English"
say francais   "Français"
say espanol    "Español"
say deutsch    "Deutsch"
say bokmal     "Norsk Bokmål"
say italiano   "Italiano"
say portugues  "Português"
say catala     "Català"
say euskara    "Euskara"
say polski     "Polski"
say dansk      "Dansk"
say nederlands "Nederlands"
say turkce     "Türkçe"
say brasiliano "Brasiliano"

# those were made using:
# iconv -f utf-8 -t ucs-2 | od -tx2 -An | sed 's/ /\\u/g'

say chinese "\u4e2d\u6587"
say nihongo "\u65e5\u672c\u8a9e"
say russkij "\u0420\u0443\u0441\u0441\u043a\u0438\u0439"
