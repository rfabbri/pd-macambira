This is the readme for "Chaos PD Externals" a set of objects for PD which 
calculate various "Chaotic Attractors"; including, Lorenz, Rossler, Henon 
and Ikeda. Hopefully more will be on their way. 

If you have any questions/comments you can reach me at ben@ekran.org

Please Note:
These programs are Copyright Ben Bogart 2002

These programs are distributed under the terms of the GNU General Public 
License 

Chaos PD Externals are free software; you can redistribute them and/or modify
them under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or 
(at your option) any later version.

Chaos PD Externals are distributed in the hope that they will be useful, 
but WITHOUT ANY WARRANTY; without even the implied warranty of 
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details. 

You should have received a copy of the GNU General Public License
along with the Chaos PD Externals; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

USAGE:

The package only includes 2 and 3 dimentional attractors. There are
outlets for each dimention. The scale of the values vary between the
different attractors. The object methods are as follows:

bang:	Calculate one interation of the attractor.
reset:	Reset to initial conditions.
param:  Modify the paramaters of the equation, the number of args depend
	on the attractor. (Be careful with the parameters, an attractor
	will go from stable to infinity in very few interations.)

See the example patches for clarification.


Have fun with them. I'd be happy to hear about any interesting uses you
find for them. As well as any interesting attractor equations you come
across.
