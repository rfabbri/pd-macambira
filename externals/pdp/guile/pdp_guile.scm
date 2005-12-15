; pdp_guile.scm - a simple event dispatcher to be used with pdp_guile

; some global variables
(define input-hash (make-hash-table 31))
(define input-loop-flag #t)
(define input-loop-interval-ms 10)

; add an input handler
(define (add-input! tag handler)
  (hashq-create-handle! input-hash tag handler))

; the main input dispatcher loop
(define (input-loop)
  (while input-loop-flag
	 (usleep (* input-loop-interval-ms 1000))
	 (let nextmsg ((msg (in)))
	   (if msg
	       (begin
		 (let ((fn (hashq-ref input-hash (car msg))))
		   (if fn (fn (cadr msg))))
		 (nextmsg (in)))))))

(define (start)
  (set! input-loop-flag #t)
  (out 'start 'bang)
  (input-loop))


; the control message handler
(add-input! 'control
	    (lambda (thing)
	      (case thing
		('stop (set! input-loop-flag #f))    ; stop the input loop and return to interpreter
		('gc (gc)))))                        ; call the garbage collector



