interfaceData = """ MH  mmm    INV               ,
 PHP ppp    -$$$$$$$$$$$$$$$$$,
 CHA ccc    -$$$$$$$$$$$$$$$$$,
            -$$$$$$$$$$$$$$$$$,
 TIME nn:ss -$$$$$$$$$$$$$$$$$,
 DAY  d     -$$$$$$$$$$$$$$$$$,"""

# mh, mh, mh, php, php, php, cha, cha, cha, ti

#  C == Constant Character Representation
#  0 == Number Data Representation
#  $ == Character Data Representation
#  & == Dynamic Character Representation
#1 00 00 0
#00 - Character Representation
#00 - Binded Variable
#0 - Number

#mmm
#ppp
#ccc

#nn:ss
#d

charsToDecimal = {
    'A': 1,
    'B': 2,
    'C': 3,
    'D': 4,
    'E': 5,
    'F': 6,
    'G': 7,
    'H': 8,
    'I': 9,
    'J': 10,
    'K': 11,
    'L': 12,
    'M': 13,
    'N': 14,
    'O': 15,
    'P': 16,
    'Q': 17,
    'R': 18,
    'S': 19,
    'T': 20,
    'U': 21,
    'V': 22,
    'W': 23,
    'X': 24,
    'Y': 25,
    'Z': 26,
    '-': 27,
    ':': 28,
    ' ': 29,
    '0': 50,
    '$': 51,
    '&': 52,
    'm': 53,
    'p': 54,
    'c': 55,
    'n': 56,
    's': 57,
    'd': 58
}

maxWidth = 0
maxHeight = 0
px = 0
py = 0
ptotal = 0
for n in interfaceData:
    if n == ',':
        if py == 0:
            maxWidth = px
        py += 1
        px = 0
        continue
    if ptotal % 5 == 0:
        print("")

    if n in charsToDecimal:
        print("1{}{}, ".format(int(charsToDecimal[n] / 10), charsToDecimal[n] % 10), end="")

    px += 1
    ptotal += 1
maxHeight = py
print("\nMax width: {}\nMax height: {}".format(maxWidth, maxHeight))