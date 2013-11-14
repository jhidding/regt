(import (rnrs (6))
  	(lib)
	(read-data)
	(ice-9 format))

(define filename 
  ($ format #f "~a.~a.~5,'0d.conan"
     (cadr (command-line)) --
     (exact (* 10000 (string->number (caddr (command-line)))))))

(define open-file
  ($ open-file-input-port -- 
     (file-options no-create)
     (buffer-mode block) #f))

(define compute-persistence
  (lambda (bmatrix phat masses)
    (define cell-dim 
      (lambda (idx) 
        (cond
          ((= (car (vector-ref bmatrix idx)) 0) 0)  ; vertex
          ((= (car (vector-ref bmatrix idx)) 2) 1)  ; edge
          (else 2))))                               ; face

    (define add-to
      (lambda (lst idx item)
        (map (lambda (i)
               (if (= i idx) 
                 (cons item (list-ref lst i))
                 (list-ref lst i))) (range (length lst)))))

    (define partition-3
      (lambda (pred lst)
        (let loop ((result '(()()()))
                   (items lst))
          (if (null? items) 
            result
            (loop (add-to result (pred (car items)) (car items))
                  (cdr items))))))

    (define difference
      (lambda (= < A B)
        (let loop ((result '())
                   (a A) (b B))
          (cond
	    ((null? a) (reverse result))
	    ((= (car a) (car b)) (loop result (cdr a) (cdr b)))
	    ((< (car a) (car b)) (loop (cons (car a) result) (cdr a) b))
	    (else                (loop result a (cdr b)))))))

    (let* ((pairs    (partition-3 (comp cell-dim car) phat))
           (pers-p   (map ($ map ($ map ($ vector-ref masses))) pairs))
           (paired   (apply append phat))
           (unpaired (difference = < (range (vector-length bmatrix)) (list-sort < paired)))
           (singles  (partition-3 cell-dim unpaired))
	   (pers-s   (map ($ map (juxt ($ vector-ref masses) (constant 0))) singles)))

      (map append pers-p pers-s))))

(let* ((read-txt-file (comp car read-tables open-file filename))
       ;(points  (read-txt-file "points"))
       (alphas  (apply append (map car (read-txt-file "alpha"))))
       (bmatrix (read-txt-file "bmatrix"))
       (phat    (cdr (read-txt-file "phat")))

       (pers    (compute-persistence 
                  (list->vector bmatrix) phat (list->vector alphas))))

  (for-each (lambda (x)
    (for-each (comp ($ format #t "~a ~a\n") <-) x)
    (newline) (newline)) pers))


