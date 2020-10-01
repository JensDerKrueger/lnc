public class Convert {
	
	static final String digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	
	static String reverse(String str) {
		String revStr = "";		
		for (char c : str.toCharArray()) {
			revStr = c + revStr;
		}		
		return revStr;
	}
	
	static int find(char c, String str) {
		for (int index = 0;index<str.length();++index) {
			if (c == str.charAt(index)) return index;
		}
		return 255;
	}
	
	static long strToInt(String inStr, int inBase) {
		long number = 0;
		long multi = 1;
		final String revInStr = reverse(inStr);		
		for (int index = 0;index<inStr.length();++index) {
			int digit = find(revInStr.charAt(index), digits);
			number += digit * multi;
			multi *= inBase;
		}				
		return number;
	}

	static String intToStr(long number, int outBase) {
		String revOutStr = "";		
		if (number == 0) return "0";
				
		while (number > 0) {
			int digit = (int)(number % outBase);
			revOutStr += digits.charAt(digit);
			number = number / outBase;
		}
		return reverse(revOutStr);
	}
	
	static String convert(String inStr, int inBase, int outBase) {
		return intToStr(strToInt(inStr, inBase), outBase);
	}
	
	public static void main(String[] args) {
		System.out.println(args[0] + " -> " + convert(args[0], (int)strToInt(args[1],10), (int)strToInt(args[2], 10)));
	}
}
