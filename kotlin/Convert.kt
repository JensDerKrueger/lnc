// lnc exercise in Kotlin with some comments
// For pedagogical reasons, this code is kept close to the Java implementation to make
// a comparison possible.
// val declarations are immutable values, var declarations are mutable, non-final variables
// const is used for compile-time constants
const val digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"

fun reverse(str: String): String {
    var revStr = "" // String type is inferred from the initializer
    for (c in str.toCharArray()) {
        revStr = c + revStr
    }
    return revStr
}

fun find(c: Char, str: String): Int {
    for (index in str.indices) {
        if (c == str[index]) return index
    }
    return 255
}

fun strToInt(inStr: String, inBase: Int): Long {
    var number: Long = 0 // We need the type spec since 0 would be Int otherwise
    var multi = 1L // But this is also possible. Multi is a Long now
    for (element in reverse(inStr)) { // Compare this to the loop in find(..)
        val digit = find(element, digits)
        number += digit * multi
        multi *= inBase
    }
    return number
}

fun intToStr(number: Long, outBase: Int): String {
    var n = number
    var revOutStr = ""
    if (n == 0L) return "0"
    while (n > 0) {
        val digit = (n % outBase).toInt()
        revOutStr += digits[digit]
        n /= outBase
    }
    return reverse(revOutStr)
}

fun convert(inStr: String, inBase: Int, outBase: Int): String {
    return intToStr(strToInt(inStr, inBase), outBase)
}

fun main(args: Array<String>) {
    println(args[0] + " -> " + convert(args[0], strToInt(args[1], 10).toInt(), strToInt(args[2], 10).toInt()))
}
