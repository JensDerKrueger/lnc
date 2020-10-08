program convert;

const
	digits : string = '0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz';


function find(c : char; str : string) : integer;
var
	i : integer;
begin
	find := 255;
	for i := 1 to length(str) do if c = str[i] then exit(i-1);
end;

function reverse(inStr : string) : string;
var
	c   : char;
begin
	reverse := '';
	for c in inStr do reverse := c + reverse;
end;

function strToInt(str : string; base : integer) : longint;
var
	number : longint = 0;
	multi  : longint = 1;
	revStr : string;
	c      : char;
	digit  : integer;
begin
	revStr := reverse(str);
	for c in revStr do
	begin
		digit := find(c, digits);
		number := number + digit * multi;
		multi := multi * base;
	end;
	strToInt := number;
end;


function intToStr(number : longint; outBase : integer) : string;
begin
	intToStr := '';
	repeat
		intToStr := digits[number mod outBase + 1] + intToStr;
		number := number div outBase;
	until number = 0;
end;


operator in (valstr : string; inbase : integer) outVal : longint;
begin
	outVal := strToInt(valstr,inbase);
end;

operator in (number : longint; outBase : integer) outStr : string;
begin
	outStr := intToStr(number,outBase);
end;



function convert(inStr : string; inBase, outBase : integer) : string;
begin
	convert := inStr in inBase in outBase;
end;


begin
	writeln(convert('1234',10,20));
	writeln('1234' in 10 in 20);
	writeln(1234 in 20);
	writeln('1234' in 20);
	writeln(3 in 2 in 3 in 2 in 3 in 2 in 3 in 2 in 3);
	readln;
end.


