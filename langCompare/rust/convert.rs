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

fn main() {
	let s = "ABCD".to_string();
	println!("->{}<-", reverse(&s));
	println!("->{}<-", find('C', &DIGITS));
}
