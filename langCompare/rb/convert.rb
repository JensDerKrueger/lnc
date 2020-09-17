#!/usr/bin/env ruby

$digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"

def reverse(inStr)
    out = ""	
    for index in (0...inStr.length)
        out = inStr[index] + out
    end
    return out
end
    
def find(c, inStr)
    i = inStr.index(c)
    i ? i : 255
end

def strToInt(inStr, inBase)
	number = 0
	multi = 1
    reverse(inStr).each_char do |c|
		digit = find(c, $digits)
		number += digit * multi
        multi *= inBase
    end
    number
end

def intToStr(number, outBase)
	return "0" if number == 0
	revOutStr = ""
	while number > 0
		digit = number % outBase
		revOutStr += $digits[digit]
        number = number / outBase
    end
    reverse(revOutStr)
end

def convert(inStr, inBase, outBase)
    intToStr(strToInt(inStr, inBase), outBase)
end

# example how to use functional programming to convert the string to a number
def convertCodeGolf(inStr, inBase, outBase)
    number = inStr.chars.reverse.map.with_index{ |c,i|
        inBase**i*$digits.index(c) }.reduce &:+
    intToStr(number, outBase)
end

# example how to use built-in functions to do the conversion
def convertUpToBase36(inStr, inBase, outBase)
    inStr.to_i(inBase.to_i).to_s(outBase.to_i).upcase
end

puts "#{ARGV[0]} -> #{convert(ARGV[0], strToInt(ARGV[1], 10), strToInt(ARGV[2], 10))}"
puts "#{ARGV[0]} -> #{convertCodeGolf(ARGV[0], strToInt(ARGV[1], 10), strToInt(ARGV[2], 10))}"
puts "#{ARGV[0]} -> #{convertUpToBase36(ARGV[0], strToInt(ARGV[1], 10), strToInt(ARGV[2], 10))}"
