# Copyright 2001 by Mathieu Bouchard
# $Id: smpte.rb,v 1.2 2006-03-15 04:40:47 matju Exp $

# The standard SMPTE color test pattern.
# AS SEEN ON TV !!! (but this is a cheap plastic imitation)

def make_smpte(picture="")
  row_1 = ""
  row_2 = ""
  row_3 = ""
  row_3_c = [[0,63,105],[255,255,255],[64,0,119]]
  (0...320).each {|x|
    n_barre_1 = 7 - x*7/320
    n_barre_2 = if n_barre_1&1==0 then 0 else 8 - n_barre_1 end
    row_1 << yield (*([1,2,0].map{|c| 255 * ((n_barre_1 >> c)&1) }))
    row_2 << yield (*([1,2,0].map{|c| 255 * ((n_barre_2 >> c)&1) }))
    row_3 << yield (*(row_3_c[x/57] || [0,0,0]))
  }
  160.times { picture << row_1 }
  20 .times { picture << row_2 }
  60 .times { picture << row_3 }
  picture
end
