#!/usr/bin/env python


import pyext,os,traceback
from supercollider import *


class supertest(pyext._class):
    _inlets=1
    _outlets=0

    def start_1(self,arg):
        try:
            self.buffer=BufferRead(self.server,arg)
            self.bsynth=Synth(self.server,"fileplayer",["bufnum",self.buffer.id])
        except:
            traceback.print_exc()

    def stop_1(self):
        try:
            del self.bsynth
        except:
            traceback.print_exc()

    def pan_1(self,val):
        try:
            self.bsynth.set("pan",val)
        except:
            traceback.print_exc()

    def rate_1(self,rate):
        try:
            self.bsynth.set("rate",rate)
        except:
            traceback.print_exc()
                
    def __init__(self):
        try:
            self.server=localServer;
            self.server.dumpOSC(3)
            self.server.evalSynth("fileplayer");
        except:
            traceback.print_exc()


