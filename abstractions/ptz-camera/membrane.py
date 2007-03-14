import pyext
import sys
from numarray import *

class tixel_coords(pyext._class):

  # number of inlets and outlets
  _inlets=1
  _outlets=1

  # Vars
  numWide = float(4)
  numHigh = float(3)
  sectionWidth = float()
  sectionHeight = float()
  myarray = array()

  print "tixel_coords init"

  # Definitions

  # Constructor

  def __init__(self,*args):
    if len(args) == 2:
      self.numWide = args[0]
      self.numHigh = args[1]
      
      self.sectionWidth = 1.0/self.numWide
      self.sectionHeight = 1.0/self.numHigh

      self.myarray = arange(self.numWide*self.numHigh, shape=(self.numHigh,self.numWide), type=Int)
      
    else:
      print "External requires 4 arguments: <numWide> <numHeight>"

  # methods for first inlet
  def bang_(self,n):
    for i in xrange(int(self.numWide)):
      for j in xrange(int(self.numHigh)):
        left = float(i)*self.sectionWidth
        right = left + self.sectionWidth
        bottom = float(j)*self.sectionHeight
        top = bottom+self.sectionHeight
        x = self.myarray[j,i]

        # constructing list for each tixel
        self._outlet(1,x,"coords",left,right,top,bottom)
        
class int2bytes(pyext._class):

  # number of inlets and outlets
  _inlets=1
  _outlets=4

  print "int2bytes init"

  # Constructor

  # methods 
  def float_1(self,number):
    number = int(number)
    for count in xrange(4):
      byte = (number & 0xF000) >> 12
      number <<=4
      self._outlet(count+1,byte)
      
class seekArray3d(pyext._class):

  # number of inlets and outlets
  _inlets=1
  _outlets=1

  print "seekArray init"

  # Vars
  width = int()
  height = int()
  depth = int()
  index = array()

  # Constructor
  def __init__(self,*args):
    if len(args) == 3:
      self.width = args[0]
      self.height = args[1]
      self.depth = args[2]
      
      self.index = arange(self.width*self.height*self.depth,type=Int32,shape=(self.depth,self.height,self.width))
    else:
    	print "you need to specify at least three aguments: <width> <height> <depth>"

  # methods 
  def seek_1(self,*l):
    if len(l) == 3:
      zoom = l[0]
      tilt = l[1]
      pan = l[2]
      position = self.index[zoom+1,tilt+1,pan]
      self._outlet(1,position)
    else:
    	print "you need to specify at least three aguments: seek <zoom> <tilt> <pan>"
      
			
  def float_1(self,number):
    number = int(number)
    for count in xrange(4):
      byte = (number & 0xF000) >> 12
      number <<=4
      self._outlet(count+1,byte)
      