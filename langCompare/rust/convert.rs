static DIGITS : &'static str = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

fn find(c : char, source : &str) -> u64 {
	for index in 0..source.len() {
		if source.chars().nth(index).unwrap() == c {
			return index as u64;
		}
	}
	255	
}

fn reverse(in_str : &String) -> String {
	let mut out_str : String = "".to_string();	
	for c in in_str.chars() {
		out_str = c.to_string() + &out_str;
	}		
	out_str
}

fn str_to_int(in_str : &String, in_base : u64) -> u64 {
	let mut number = 0;
	let mut mult = 1;
	let rev_str = reverse(in_str);	
	for c in rev_str.chars() {
		let digit = find(c, &DIGITS);
		number += digit * mult;
		mult *= in_base;	
	}		
	number
}

fn int_to_str(mut number : u64, out_base : u64) -> String {
	let mut out_str : String = "".to_string();
	if number == 0 {return "0".to_string();}
	while number > 0 {
		let digit = number % out_base;
		out_str = DIGITS.chars().nth(digit as usize).unwrap().to_string() + &out_str;
		number /= out_base;
	}
	out_str
}

fn convert(in_str : &String, in_base : u64, out_base : u64) -> String {
	int_to_str(str_to_int(in_str,in_base),out_base)
} 

fn main() {
	let arguments : Vec<String> = std::env::args().collect();
	println!("{} -> {}", &arguments[1], convert(&arguments[1], str_to_int(&arguments[2], 10), str_to_int(&arguments[3], 10)))
}
