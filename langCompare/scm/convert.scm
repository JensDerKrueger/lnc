(define digits "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz")

(define (reverse-ls-int ls acc) 
	(if (null? ls) 
		acc 
		(reverse-ls-int  (cdr ls) (cons (car ls) acc))))

(define (find-int c l p)
		(if (char-ci=? c (car l))
			p
			(find-int c (cdr l) (+ p 1))))
			
(define (reverse-ls ls)
		(reverse-ls-int ls '()))
		
(define (find c)
		(find-int c (string->list digits) 0))

(define (str-to-int-int ls in-base multi)	
	(if (null? ls)
		0
		(+ (* (find (car ls)) multi) (str-to-int-int (cdr ls) in-base (* multi in-base)))))

(define (str-to-int in-str in-base)
	(str-to-int-int (reverse-ls (string->list in-str)) in-base 1))
			
(define (int-to-str-int number out-base)
	(if (= 0 number)
		'()		
		(cons (string-ref digits (remainder number out-base)) (int-to-str-int(quotient number out-base) out-base))))
		
(define (int-to-str number out-base)		
	(list->string (reverse-ls (int-to-str-int number out-base))))

(define (convert in-str in-base out-base)
	(int-to-str (str-to-int in-str in-base) out-base))