=begin
	$Id: placebo.rb,v 1.1 2005-10-04 02:02:13 matju Exp $

	GridFlow
	Copyright (c) 2001,2002,2003,2004,2005 by Mathieu Bouchard

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	See file ../COPYING for further informations on licensing terms.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
=end

class Object
  def self.dummy(sel)
    self.module_eval "def #{sel}(*args) GridFlow.post \"dummy #{sel}: %s\", args.inspect end"
  end
end

module GridFlow
  class<<self
  #  def add_creator_2(*args) post "dummy add_creator_2: %s", args.inspect end
    dummy :add_creator_2
    def post_string(s) STDERR.puts s end
  end
  class Clock
    def initialize(victim) @victim=victim end
    dummy :delay
  end
  class Pointer
    dummy :initialize
  end
end
