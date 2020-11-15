# cAES

A really bad implementation of the AES block cipher in C.
Written by someone that doesn't know C😕

## 🔷 What is AES?

**A**dvanced **E**ncryption **S**tandard ([**AES**][wikipedia]) is a symmetric block cipher.
Here is a high-level overview of how it works (spcifically in the implementation
I do here, other ways exist):

1. **[Key expansion][key_schedule]** - 11 keys are generated from the original
secret key and the syncornization. I havn't implemented this yet so I can't go
into too much details (becausue I don't know too much details...).
2. **Initial key addition** - the bits of the message are XORed with the bits of the
   first generated round key.
3. **Loop 9 times:**
    1. **[Substitude bytes][s-box]** - All the bytes of the message are substituted
    according to the **s-box** lookup table. The byte `b` would be replaced by
    `s` according to the formula:

    ```c
    s = b ^ (b <<< 1) ^ (b <<< 2) ^ (b <<< 3) ^ (b <<< 4) ^ 0x63
    ```
    
    Where `^` is the bit-wise XOR operation, and `<<<` is the left circular bit-shift.

    The inverse operation to this is given by

    ```c
    b = (s <<< 1) ^ (s <<< 3) ^ (s <<< 6) ^ 0x05
    ```

    2. **Shift rows** - The bytes of msg are shaped into a 4 by 4 grid. At this
    stage, each row is shifted left (circuraly) by one byte more than the row
    above it. So the first row is un-changed, the second row is shifted by one
    byte to the left, the third by 2 bytes, and the forth by 3 bytes.
    3. **[Mix columns][mix_cols]** - Each column of the message grid is considered
    as a column vector. We multiply that column vector by some matrix (in the
    Rijndael's Galois field) to get a new column to put in-place of the old one.
    The matrix is (I wish there was latex support on github...)

    |   |   |   |   |
    |---|---|---|---|
    | 2 | 3 | 1 | 1 |
    | 1 | 2 | 3 | 1 |
    | 1 | 1 | 2 | 3 |
    | 3 | 1 | 1 | 2 |

    What does it mean to be under the Rijndael's Galois field you ask? Well, I
    won't cover it in depth here (check the [official specification][specs]
    chapter 4) but here is a brief explenation.

    Think of a byte comprised of 8 bits, let's say 10010101 for example. Now think
    of each digit as the coefficient of a polynomial, the polynomial we get is
    (again, I wish I had latex math rendering here, but OK):

    P(x) = **1**x^7 + **0**x^6 + **0**x^5 + **1**x^4 +**0**x^3 +**1**x^2 + **0**x + **1**

    See how the bits of the byte come in the polynomial? Now this is our byte.
    If we want to add two bytes, we do so just like we would two polynomials
    (element-wise), but with one small twist. Instead of regular addition, we use
    the **XOR** operation, so **1 + 1** is not **2**, it's **0** (you can think
    of it like addition modulo 2).
    
    Multiplication is the interesting part, because we need to multiply all the
    elements by one another (we'll look at an example later). You may also ask
    yourself, what happens with stuff like **x^7** times **x^7**, it should be
    **x^14** but we only have 8 bits in our byte, so what would we do? The answer
    is that we do all the multiplication modulo (**x^4 - 1**), which I won't go
    into now (again, [check the specifications!][specs] chapter 4).

    Anyways, just remember that when you want to do something like **5*3** remember
    that that's not **15**! that should be **00000101** times **00000011** which
    is equivilent to the mulitplication of the polynomials:

    **5 = 00000101 = x^2 + 1**

    and

    **3 = 00000011 = x + 1**

    which if you carry out you would get

    **x^3 + x^2 = 00000110 = 6**

    So **5** times **3** is equal to **6** (weird, I know).

    By the way, the inverse of the mix columns matrix is

    |    |    |    |    |
    |----|----|----|----|
    | 14 | 11 | 13 |  9 |
    |  9 | 14 | 11 | 13 |
    | 13 |  9 | 14 | 11 |
    | 31 | 13 |  9 | 14 |

    If you were wondering...

    4. **Add key** - The message is XORed with another round key (different key
    each iteration,  generated from the **key expansion** step).
4. **Final round** (making 10 in total). The same as before but without the mix columns
step.
    1. **[Substitude bytes][s-box]**
    2. **Shift rows**
    3. **Add key**

[specs]: (https://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.197.pdf)
[wikipedia]: https://en.wikipedia.org/wiki/Advanced_Encryption_Standard
[mix_cols]: https://en.wikipedia.org/wiki/Rijndael_MixColumns
[key_schedule]: https://en.wikipedia.org/wiki/AES_key_schedule
[s-box]: https://en.wikipedia.org/wiki/Rijndael_S-box

<!-- ## 🔷 How to use this program?

**DON'T** (it's not like you were going to but OK)

This was made with purly educational puposes in mind. I was board this saturday.
I think it would be cool to make this into a character device file, this way you
can do something like `echo "some string" > AES` and it would encrypt your string. -->
