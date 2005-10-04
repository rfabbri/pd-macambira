require "linux/ioctl"
# Copyright (c) 2001, 2003 by Mathieu Bouchard
# this is published under the Ruby license

=begin
  if using a DB-25 female connector as found on a PC,
  then the pin numbering is like:
  13 _____ 1
  25 \___/ 14

  1 = STROBE = the clock line is a square wave, often at 9600 Hz,
      which determines the data rate in usual circumstances.
  2..9 = D0..D7 = the eight ordinary data bits
  10 = -ACK (status bit 6 ?)
  11 = BUSY (status bit 7)
  12 = PAPER_END (status bit 5)
  13 = SELECT (status bit 4 ?)
  14 = -AUTOFD
  15 = -ERROR (status bit 3 ?)
  16 = -INIT
  17 = -SELECT_IN
  18..25 = GROUND
=end

module Linux; module ParallelPort
	extend IoctlClass

	@port_flags = %w[
	LP_EXIST
	LP_SELEC
	LP_BUSY
	LP_OFFL
	LP_NOPA
	LP_ERR
	LP_ABORT
	LP_CAREFUL
	LP_ABORTOPEN
	LP_TRUST_IRQ
	]

	@port_status = %w[
		nil,
		nil,
		nil,
		LP_PERRORP  # unchanged input, active low
		LP_PSELECD  # unchanged input, active high
		LP_POUTPA   # unchanged input, active high
		LP_PACK     # unchanged input, active low
		LP_PBUSY    # inverted input, active high
	]

	LPCHAR = 0x0601
	LPTIME = 0x0602
	LPABORT = 0x0604
	LPSETIRQ = 0x0605
	LPGETIRQ = 0x0606
	LPWAIT = 0x0608
	LPCAREFUL = 0x0609 # obsoleted??? wtf?
	LPABORTOPEN = 0x060a
	LPGETSTATUS = 0x060b # return LP_S(minor)
	LPRESET = 0x060c # reset printer
	LPGETSTATS = 0x060d # struct lp_stats (most likely turned off)
	LPGETFLAGS = 0x060e # get status flags
	LPTRUSTIRQ = 0x060f # set/unset the LP_TRUST_IRQ flag

	ioctl_reader :port_flags , :LPGETFLAGS
	ioctl_reader :port_status, :LPGETSTATUS
	ioctl_writer :port_careful,:LPCAREFUL
	ioctl_writer :port_char,   :LPCHAR

end end

