program convert;

const
	digits : string = '0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz';


function find(c : char; str : string) : integer;
var
	i : integer;
begin
	for i := 1 to length(str) do 
	begin
		if c = str[i] then exit(i-1);
	end;
	find := 255;
end;

function reverse(inStr : string) : string;
var
	out : string;
	c   : char;
begin
	out := '';	
	for c in inStr do
	begin
		out := c + out;
	end;	
	reverse := out;
end;

function strToInt(str : string; base : integer) : integer;
var
	number : integer = 0;
	multi  : integer = 1;
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

begin
	writeln(strToInt(paramStr(1), 20));
end.



{
procedure reverseInPlace(var str : string);
var
	i : integer;
	t : char;
begin
	for i := 1 to length(str) div 2 do 
	begin
		t := str[i];
		str[i] := str[length(str)-(i-1)];
		str[length(str)-(i-1)] := t;
	end;
end;	
}