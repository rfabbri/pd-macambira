To compile:


- Install DelphiX by Hiroyuki Hori, http://www.yks.ne.jp/~hori/ <hori@ingjapan.ne.jp>

The above link doesn't always work, try these:

http://turbo.gamedev.net/DelphiX2000_0717a.zip
http://turbo.gamedev.net/delphix.asp


- Install FastLib by G-Soft, http://gfody.com <gfody@home.com>


- Install TScanDir in Filez.pas (in this directory).


- Correct your Search path and compile Framestein.dpr!


- Framestein needs ijl15.dll (included in Framestein-directory) to load/save jpegs
  and to send jpeg-compressed frames thru network.


--- note to versions >= 0.27 ---

The photoshop filter host functionality is based on commercial code from http://www.case2000.com.
Due to my modifications the original component won't compile with Framestein and
I don't have the permission to release my modified version.

To compile Fs without the PSHost, ignore errors on C2PhotoShopHost, and comment out
the implementation in pshostunit.pas.

I'm planning a .dll of my version of the component to rid of this problem.
Mail me for info.


--- versions > 0.31:

Framestein.dpr is the Delphi-project to use. FramesteinLib.dpr is an experimental
project to compile Framestein as a Pure Data library, loaded with the -lib-parameter.
This seems to give a better response, but still crashes when quitting Pd, so no binary
is yet distributed.
