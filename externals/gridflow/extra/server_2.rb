# a server program to connect 2 or more clients together.
# by Mathieu Bouchard

require "fcntl"
require "socket"

class IO
  def nonblock=flag
    bit = Fcntl::O_NONBLOCK
    fcntl(Fcntl::F_SETFL, (fcntl(Fcntl::F_GETFL) & ~bit) |
      if flag then bit else 0 end)
  end
  # does not work with any ruby version, due to a bug. see below.
  def read_at_most n
    s=""
    k=1<<(Math.log(n)/Math.log(2)).to_i
    while k>0
      unless k+s.length>n
        puts "trying #{k}"
        (s << read(k)) rescue Errno::EWOULDBLOCK
      end
      k>>=1
    end
    s
  end
  # this one works but is slow.
  def bugfree_read_at_most n
    s=""
    (s << (read 1) while s.length<n) rescue Errno::EWOULDBLOCK
    s 
  end
end

serv = TCPServer.new 4242
socks = [serv]

loop {
  puts "waiting for connection (port 4242)"
  begin
    loop {
      puts "waiting"
      ready,blah,crap = IO.select socks, [], socks, 1
      (ready||[]).each {|s|
	if s==serv then
          sock = serv.accept
          sock.nonblock=true
          socks << sock
          puts "incoming connection (total: #{socks.length-1})"
	else
          other = socks.find_all{|x|not TCPServer===x} - [s]
          stuff = s.bugfree_read_at_most 1024
          p stuff
          (s.close; socks.delete s) if not stuff or stuff.length==0
          other.each {|x|
            p x
            x.write stuff
          }
        end
      }
    }
  rescue Errno::EPIPE # Broken Pipe
    puts "connection closed (by client)"
    # it's ok, go back to waiting.
  end
}
