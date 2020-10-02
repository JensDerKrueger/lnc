// typescript with deno from https://deno.land/
// to execute: deno run .\convert.ts 1234 10 20

const digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

interface IteratedChars {
  index: number;
  char: string;
}

// this will iterate over the given string, yielding a char and its index in each step
function* chars(inStr: string): Iterable<IteratedChars> {
  for (let i = 0; i < inStr.length; i++) {
    yield { index: i, char: inStr.charAt(i) };
  }
}

function reverse(inStr: string): string {
  let out = "";
  for (let { char } of chars(inStr)) {
    out = char + out;
  }
  return out;
}

function find(c: string, str: string): number {
  for (let { index, char } of chars(str)) {
    if (c === char) return index;
  }
  return 255;
}

function strToInt(inStr: string, inBase: number): number {
  let number = 0;
  let multi = 1;
  const revInStr = reverse(inStr);
  for (let { char } of chars(revInStr)) {
    const digit = find(char, digits);
    number += digit * multi;
    multi *= inBase;
  }
  return number;
}
function intToStr(number: number, outBase: number): string {
  let revOutStr = "";
  do {
    const digit = number % outBase;
    revOutStr += digits.charAt(digit);
    number = Math.floor(number / outBase);
  } while (number > 0);
  return reverse(revOutStr);
}

function convert(inStr: string, inBase: number, outBase: number) {
  return intToStr(strToInt(inStr, inBase), outBase);
}

console.log(Deno.args[0] + " -> " + convert(Deno.args[0], strToInt(Deno.args[1], 10), strToInt(Deno.args[2], 10)));
