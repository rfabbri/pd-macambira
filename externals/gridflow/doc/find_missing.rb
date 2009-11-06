#!/usr/bin/env ruby

a=[]
`grep -nriw install ../src/*.[ch]xx ../src/*.m`.each {|line|
  m=/install\(\"([^"]+)/.match(line)
  a<<m[1] if m
}

b=Dir["../abstractions/*.pd"].map{|x|
  x.gsub(/^\.\.\/abstractions\//,"").gsub(/\.pd$/,"")
}

c      = Dir["flow_classes/*-help.pd"   ].map{|x| x.gsub(/^flow_classes\//,"").gsub(/-help\.pd$/,"") }
c.concat Dir["flow_classes/cv/*-help.pd"].map{|x| x.gsub(/^flow_classes\//,"").gsub(/-help\.pd$/,"") }
ab=a+b

d=[]
File.open("index.pd") {|f|
  f.each {|line|
    m=/obj \d+ \d+ ([^ ;]+)/.match(line)
    d<<m[1] if m
  }
}

puts "missing from help files: "
puts (ab-c).sort.join" "
puts (ab-c).size
puts ""

puts "orphan help files:"
puts (c-ab).sort.join" "
puts ""

puts "missing from index:"
puts (ab-d).sort.join" "
puts (ab-d).size
puts ""

puts "orphan index entries: "
puts (d-ab).sort.join" "
