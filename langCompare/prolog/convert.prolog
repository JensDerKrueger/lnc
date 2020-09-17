digits(X) :- string_chars("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz", X).

reverseList([], Acc, Acc).
reverseList([H|T], RL, Acc) :- reverseList(T, RL, [H|Acc]).
reverseList(L, RL) :- reverseList(L, RL, []).

find(_, 255, [], _).
find(C, I, [C|_], I).
find(C, N, [_|T], I) :- I1 is I+1, find(C, N, T, I1).
find(C, N) :- digits(L), find(C, N, L, 0).

strToInt([], _, 0, _).
strToInt([H|T], IB, I, M) :- find(H,D), MB is M*IB, strToInt(T, IB, TI, MB), I is D*M + TI.
strToInt(S, IB, I) :- string_chars(S, L), reverseList(L,RL), strToInt(RL, IB, I, 1).

intToList(0, _, []).
intToList(I, OB, [C|RL]) :- D is mod(I, OB), find(C, D), RI is I // OB, intToList(RI, OB, RL).
intToStr(I, OB, S) :- intToList(I, OB, L), reverseList(L, RL), string_chars(S, RL).

convert(IS, IB, OB, OS) :- strToInt(IS, IB, N), intToStr(N, OB, OS).
