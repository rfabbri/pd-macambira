; load event handler
(load "pdp_guile.scm")


; input - output video plug
(add-input! 'in-vid
	    (lambda (thing)
	      (out 'out-vid thing)))


; start the input loop
(start)
