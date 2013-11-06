(import (rnrs (6))
  	(lib)
	(read-data)
	(ice-9 format))

(define filename 
  ($ format #f "~a.~5,'0d.~a.txt"
     (cadr (command-line))
     (exact (* 10000 (string->number (caddr (command-line)))))))

(define open-file
  ($ open-file-input-port -- 
     (file-options no-create)
     (buffer-mode block) #f))

(define compute-persistence
  (lambda (points bmatrix phat masses)
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
                 (list-ref lst i))) lst)))

    (define partition-3
      (lambda (pred lst)
        (let loop ((result '(()()()))
                   (items lst))
          (if (null? items) 
            result
            (loop (add-to result (pred (car items)) (car items))
                  (cdr items))))))

    (let* ((pairs    (partition-3 (comp cell-dim car) phat))
           (persp    (map ($ map ($ vector-ref masses)) pairs))
           (paired   (apply append phat))
           (unpaired (lset-difference = (range (length bmatrix)) paired))
           (singles  (partition-3 cell-dim unpaired))
    (let loop ((pairs phat)
               (diagram '(()()())))

      (cond
        ; no pairs left
        ((null? pairs) (<- diagram)
        (else
         (loop (cdr pairs)
               (add-to diagram (cell-dim (caar pairs)) (map ($ vector-ref masses) (car pairs))))))))

        ; cell is not part of pair -> death at infinity
        ((not (= (caar pairs) cell-idx))
         (loop (+ 1 cell-idx) (cdr pairs)
               (add-to diagram (cell-dim cell-idx) (list (vector-ref masses cell-idx) 0))))

        ; cell is part of pair
        (else
         (loop ((

(let* ((read-txt-file (comp car read-tables open-file filename))
       (points  (read-txt-file "points"))
       (masses  (read-txt-file "values"))
       (bmatrix (read-txt-file "bmatrix"))
       (phat    (read-txt-file "phat")))

  (for-each (lambda (l) (display l) (newline)) phat))

