# $Id: server_1_ppm.rb,v 1.1 2005-10-04 02:02:15 matju Exp $

require "socket"

picture = File.open("../images/teapot.ppm") {|x| x.read }

serv = TCPServer.new 4242
loop {
  puts "waiting for connection (port 4242)"
  sock = serv.accept
  begin
    loop {
      sock.write picture
      puts "wrote one picture"
    }
  rescue Errno::EPIPE # Broken Pipe
    puts "connection closed (by client)"
    # it's ok, go back to waiting.
  end
}
