(library (read-data)
  (export read-tables)

  (import (rnrs (6))
	  (guile)
	  (lib))

  (define read-lines
    (lambda (port)
      (let ((data (string-split (get-string-all port) #\newline)))
        data)))
  
  (define find-car
    (lambda (pred lst)
      (let loop ((p1 '())
  	         (p2 lst))
        (cond
  	((null? p2)      (values #f lst))
  	((pred (car p2)) (values p2 (reverse p1)))
  	(else            (loop (cons (car p2) p1) (cdr p2)))))))
  
  (define split-list
    (lambda (pred lst)
      (let loop ((n lst)
  	         (r '()))
        (if n 
  	(call-with-values 
  	  (lambda () (find-car pred n))
  	  (lambda (p2 p1)
  	    (loop (if p2 (cdr p2) p2) (cons p1 r))))
  	(reverse r)))))
  
  (define split-line
    (lambda (pred str)
      (map list->string (split-list pred (string->list str)))))
  
  (define read-tables
    (lambda (port)
      (let* ((lines    (read-lines port))
  	     (blocks   (split-list ($ equal? "") lines))
  	     (data     (map ($ map (comp ($ filter id) ($ map string->number)
  				         ($ split-line char-whitespace?)))
			    blocks)))
        (filter (comp not null?) data))))
)

