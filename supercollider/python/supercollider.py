
#/* --------------------------- supercollider.py  ----------------------------------- */
#/*   ;; Kjetil S. Matheussen, 2004.                                             */
#/*                                                                              */
#/* This program is free software; you can redistribute it and/or                */
#/* modify it under the terms of the GNU General Public License                  */
#/* as published by the Free Software Foundation; either version 2               */
#/* of the License, or (at your option) any later version.                       */
#/*                                                                              */
#/* This program is distributed in the hope that it will be useful,              */
#/* but WITHOUT ANY WARRANTY; without even the implied warranty of               */
#/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                */
#/* GNU General Public License for more details.                                 */
#/*                                                                              */
#/* You should have received a copy of the GNU General Public License            */
#/* along with this program; if not, write to the Free Software                  */
#/* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.  */
#/*                                                                              */
#/* ---------------------------------------------------------------------------- */



# This file is far from complete, but its a start.
# -Kjetil.


import OSC,tempfile,xreadlines,os,time,types

standardport=57110
startnode=1001;
startbuffer=0;
    
sc_head=0
sc_tail=1
sc_before=2
sc_after=3
sc_replace=4


# For some crazy reason, the sclang command tells the server to quits when exiting.
# To fix that strange and unpleasant behaviour, uncommnet the following lines in etc. etc. etc.


class Server:
    def __init__(self,magic,port=standardport):
        if magic!=1234:
            print "Server.__init__: Are you sure you know what you are doing?"
            print "Seems like you probably wanted to use the 'localServer' variable."
        self.port=port
        self.freenode=startnode;
        self.freebuffer=startbuffer;
    def sendMsg(self,command,*args):
        def floatToInt(x):
            if type(x)==types.FloatType:
                return int(x)
            else:
                return x
        OSC.Message(command,map(floatToInt,args)).sendlocal(self.port)
    def sendgetMsg(self,command,*args):
        apply(self.sendMsg,[command]+list(args))
        time.sleep(1)
    def dumpOSC(self,code):
        self.sendMsg("dumpOSC",code);
    def nextNodeID(self):
        self.freenode+=1;
        return self.freenode-1
    def nextBufferID(self):
        self.freebuffer+=1;
        return self.freebuffer-1;
    def loadSynthDef(self,name):
        self.sendgetMsg("/d_load",name)
    def loadSynthDefDir(self,dir):
        self.sendMsg("/d_loadDir",dir)
    def evalSynth(self,synthname):
        tmpname=tempfile.mktemp(".sc")
        outfile=open(tmpname,"w")
        outfile.write('SynthDef("'+synthname+'",{')
        for line in xreadlines.xreadlines(open(synthname+".sc","r")):
            outfile.write(line)
        tmpname2=tempfile.mktemp("")
        outfile.write('}).writeDefFile("'+tmpname2+'");\n')
        outfile.close()
        os.system("sclang "+tmpname)
        tmpname2+=synthname+".scsyndef"
        self.loadSynthDef(tmpname2)
        os.system("rm "+tmpname+" "+tmpname2)
        
localServer=Server(1234)

class Node:
    def __del__(self):
        self.server.sendMsg("/n_free",self.id)
    def set(self,*args):
        apply(self.server.sendMsg,["/n_set",self.id]+list(args))


class Synth(Node):
    def __init__(self,server,name,args=[],position=sc_tail):
        self.server=server
        self.id=server.nextNodeID()
        apply(self.server.sendMsg,["/s_new",name,self.id,position,0]+args)


class BufferSuper:
    def __init__(self,server,numFrames=0,numChannels=1,filename="",startFrame=0):
        self.server=server
        self.id=server.nextBufferID()
        if numChannels==-1:
            server.sendMsg("/b_allocRead",self.id,filename,startFrame,numFrames,0)
        else:
            server.sendMsg("/b_alloc",self.id,numFrames,numChannels,0)
        
    def __del__(self):
        self.server.sendMsg("/b_free",self.id)


class Buffer(BufferSuper):
    def __init__(self,server,numFrames,numChannels=1):
        BufferSuper.__init__(self,server,numFrames,numChannels)

class BufferRead(BufferSuper):
    def __init__(self,server,filename,startFrame=0,numFrames=0):
        BufferSuper.__init__(self,server,numFrames,-1,filename,startFrame)



