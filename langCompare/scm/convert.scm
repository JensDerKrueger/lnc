(define (reverse-int ls acc) 
	(if (null? ls) 
		acc 
		(reverse-int (cdr ls) (cons (car ls) acc))))

(define find-int
	(lambda (c l p)
		(if (char-ci=? c (car l))
			p
			(findInt c (cdr l) (+ p 1)))))


(define reverse
	(lambda (s)
		(list->string (reverse-int (string->list s) '()))))
		
(define find
	(lambda (c)
		(let ((digits (string->list "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"))) (find-int c digits 0))))


; (define convert  
;	(lambda (inStr inBase outBase) (intToStr (strToInt inStr inBase) outBase) )
;)