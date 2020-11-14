#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#define c_shift(byte, shift) ((byte << shift) | (byte >> (8 - shift))) // Circular shift


int gen_key(char buffer[17])
{
  char key[17] = {0};

  // TODO: Implement!

  return 0;
}


char** key_expansion(char* key)
{
  // TODO: Implement!!
}


void fwd_s_box(char* byte)
{
  *byte ^= c_shift(*byte, 1) ^ c_shift(*byte, 2) ^ c_shift(*byte, 3) ^ c_shift(*byte, 4) ^ 0x63;
}


void inv_s_box(char* byte)
{
  *byte = c_shift(*byte, 1) ^ c_shift(*byte, 3) ^ c_shift(*byte, 6) ^ 0x05;
}


void sub_bytes(char* bytes)
{
  for (size_t i = 0; i < 16; i++)
    fwd_s_box(bytes + i);
}


// Inverse of sub_bytes()
void unsub_bytes(char* bytes)
{
  for (size_t i = 0; i < 16; i++)
    inv_s_box(bytes + i);
}


void rotate_left_by_one(char* bytes)
{
  char tmp = bytes[0];
  bytes[0] = bytes[1];
  bytes[1] = bytes[2];
  bytes[2] = bytes[3];
  bytes[3] = tmp;
}


void rotate_right_by_one(char* bytes)
{
  char tmp = bytes[3];
  bytes[3] = bytes[2];
  bytes[2] = bytes[1];
  bytes[1] = bytes[0];
  bytes[0] = tmp;
}


void rotate_word(char* word, uint8_t shift)
{
  shift = shift % 4;
  if (shift >= 0) {
    for (; shift>0 ; --shift)
      rotate_left_by_one(word);
  } else {
    for (; shift>0 ; --shift)
      rotate_right_by_one(word);
  }
}


void shift_rows(char* bytes)
{
  for (size_t i = 0; i < 4; i++)
    rotate_word(bytes+4*i, i);
}

// Inverse of shift_rows()
void unshift_rows(char* bytes)
{
  for (size_t i = 0; i < 4; i++)
    rotate_word(bytes+4*i, -i);
}

// =========================== Test vectors ===========================
//           Hexadecimal	                   Decimal
//      Before	       After	            Before	           After
//   db 13 53 45	 8e 4d a1 bc	      219 19 83 69	    142 77 161 188
//   f2 0a 22 5c	 9f dc 58 9d	      242 10 34 92	    159 220 88 157
//   01 01 01 01	 01 01 01 01	      1   1  1  1	      1   1   1  1
//   c6 c6 c6 c6	 c6 c6 c6 c6	      198 198 198 198	  198 198 198 198
//   d4 d4 d4 d5	 d5 d5 d7 d6	      212 212 212 213	  213 213 215 214
//   2d 26 31 4c	 4d 7e bd f8	      45  38  49  76	  77  126 189 248
//=====================================================================
void mix_column(unsigned char* bytes)
{
  // Matrix multiplication under the Rijndael's Galois field
  // | b0 | = | 2 3 1 1 | | a0 |
  // | b1 | = | 1 2 3 1 | | a1 |
  // | b2 | = | 1 1 2 3 | | a2 |
  // | b3 | = | 3 1 1 2 | | a3 |

  unsigned char tmp[4];   // A copy of the original byte array
  unsigned char tmp2[4];  // The byte array multiplied by 2 in the Rijndael's Galois field
  unsigned char h;
  for (unsigned char i = 0; i < 4; ++i){
    tmp[i] = bytes[i];

    h = (unsigned char)((signed char)bytes[i] >> 7);
    tmp2[i] = bytes[i] << 1;
    tmp2[i] ^= 0x1B & h;
  }
  
  // bytes[0] = ((2*tmp[0]) ^ (3*tmp[1]) ^ (1*tmp[2]) ^ (1*tmp[3]));
  // bytes[1] = ((1*tmp[0]) ^ (2*tmp[1]) ^ (3*tmp[2]) ^ (1*tmp[3]));
  // bytes[2] = ((1*tmp[0]) ^ (1*tmp[1]) ^ (2*tmp[2]) ^ (3*tmp[3]));
  // bytes[3] = ((3*tmp[0]) ^ (1*tmp[1]) ^ (1*tmp[2]) ^ (2*tmp[3]));

  // 2 = 10 = x
  // 3 = 11 = x + 1 = x*P  1
  bytes[0] = (tmp2[0] ^ tmp[3] ^ tmp[2] ^ tmp2[1] ^ tmp[1]);
  bytes[1] = (tmp2[1] ^ tmp[0] ^ tmp[3] ^ tmp2[2] ^ tmp[2]);
  bytes[2] = (tmp2[2] ^ tmp[1] ^ tmp[0] ^ tmp2[3] ^ tmp[3]);
  bytes[3] = (tmp2[3] ^ tmp[2] ^ tmp[1] ^ tmp2[0] ^ tmp[0]);

}


// Inverse of mix_column() - TODO: Doesn't work yet
void unmix_column(unsigned char* bytes)
{
  // Matrix multiplication under the Rijndael's Galois field
  // | a0 | = | 14 11 13 9 | | b0 |
  // | a1 | = | 9 14 11 13 | | b1 |
  // | a2 | = | 13 9 14 11 | | b2 |
  // | a3 | = | 11 13 9 14 | | b3 |

  unsigned char tmp[4]; // Copy of bytes
  unsigned char x[4];   // Bytes times x
  unsigned char x2[4];  // Bytes times x^2
  unsigned char x3[4];  // Bytes times x^3

  // I could do without these but it is more readable this way and I don't care about performance
  unsigned char tmp14[4];  // 14 = 1110 = x3 + x2 +  x
  unsigned char tmp13[4];  // 13 = 1101 = x3 + x2 + 1
  unsigned char tmp11[4];  // 11 = 1011 = x3 + x  + 1
  unsigned char tmp9[4];   // 9  = 1001 = x3 + 1       

  unsigned char h;         // High bits
  for (size_t i = 0; i < 4; ++i) {
    tmp[i] = bytes[i];

    x[i] = bytes[i] << 1;
    x2[i] = bytes[i] << 2;
    x3[i] = bytes[i] << 3;

    h = (unsigned char)((signed char)bytes[i] >> 7);
    x[i] ^= 0x1B & h;

    h = (unsigned char)((signed char)bytes[i] >> 6);
    x2[i] ^= 0x1B & h;

    h = (unsigned char)((signed char)bytes[i] >> 5);
    x3[i] ^= 0x1B & h;

    tmp14[i] = x3[i] ^ x2[i] ^ x[i];
    tmp13[i] = x3[i] ^ x2[i] ^ tmp[i];
    tmp11[i] = x3[i] ^ x[i]  ^ tmp[i];
    tmp9[i]  = x3[i] ^ tmp[i];
  }

  bytes[0] = tmp14[0] ^ tmp11[1] ^ tmp13[2] ^ tmp9[3];
  bytes[0] = tmp9[0]  ^ tmp14[1] ^ tmp11[2] ^ tmp13[3];
  bytes[0] = tmp13[0] ^ tmp9[1]  ^ tmp14[2] ^ tmp11[3];
  bytes[0] = tmp11[0] ^ tmp13[1] ^ tmp9[2]  ^ tmp14[3];

  // bytes[0] = (14*tmp[0] ^ 11*tmp[1] ^ 13*tmp[2] ^  9*tmp[3]);
  // bytes[1] = ( 9*tmp[0] ^ 14*tmp[1] ^ 11*tmp[2] ^ 13*tmp[3]);
  // bytes[2] = (13*tmp[0] ^  9*tmp[1] ^ 14*tmp[2] ^ 11*tmp[3]);
  // bytes[3] = (11*tmp[0] ^ 13*tmp[1] ^  9*tmp[2] ^ 14*tmp[3]);
}


void mix_columns(char* bytes)
{
  for (size_t i = 0; i < 4; ++i)
    mix_column(bytes+4*i);
}


// Inverse of mix_columns()
void unmix_columns(char* bytes)
{
  for (size_t i = 0; i < 4; ++i)
    unmix_column(bytes+4*i);
}


void add_round_key(char* bytes, char* key)
{
  for (size_t i = 0; i < 16; i++)
    bytes[i] = bytes[i];
}


void print_block(char* bytes, size_t len)
{
  for (size_t i = 0; i < len; i++)
    printf("%x\t", (uint8_t)bytes[i]);
  printf("\n");
}


int encrypt(char* msg, char* key)
{
  // 1. Key expansion stage - create multiple keys from the original key using the AES key schedule
  char* keys[] = {key,key,key,key,key,key,key,key,key,key,key};  // TODO: Implement key scheduale

  // 2. Initial round key addition - XOR the first key with the message
  add_round_key(msg, keys[0]);

  // 3. Cycles (9 rounds)
  for (size_t i = 1; i < 10; ++i)
  {
    sub_bytes(msg);
    shift_rows(msg);
    mix_columns(msg);
    add_round_key(msg, keys[i]);
  }

  // 4. Final round (making 10 in total)
  sub_bytes(msg);
  shift_rows(msg);
  mix_columns(msg);
  add_round_key(msg, keys[10]);
  return 0;  // TODO: Implement!
}


int decrypt(char* msg, char* key)
{
  // 1. Key expansion stage - create multiple keys from the original key using the AES key schedule
  char* keys[] = {key,key,key,key,key,key,key,key,key,key,key};  // TODO: Implement key scheduale

  // 4. Final round (making 10 in total)
  add_round_key(msg, keys[10]);
  unmix_columns(msg);
  unshift_rows(msg);
  unsub_bytes(msg);

  // 3. Cycles (9 rounds)
  for (size_t i = 1; i < 10; ++i)
  {
    add_round_key(msg, keys[i]);
    unmix_columns(msg);
    unshift_rows(msg);
    unsub_bytes(msg);
  }

  // 2. Initial round key addition - XOR the first key with the message
  add_round_key(msg, keys[0]);


  return 0;  // TODO: Implement!
}


int main()
{
  // Please do not store secret keys like this!
  char key[16] = "H2a76899IZIoC5xC";  // 128-bit key
  char msg[16] = "1234123412341234";


  // Test mix_column() and unmix_column()
  char test_vector[4] = {0xdb, 0x13, 0x53, 0x45};
  print_block(test_vector, 4);

  mix_column(test_vector);    // Should be:  8e 4d a1 bc
  unmix_column(test_vector);  // Should be:  db 13 53 45 (same as original test_vector)

  print_block(test_vector, 4);

  // Print original message bytes as hex
  printf("\33[1m* Message:\33[35m    ");
  print_block(msg, 16);

  // Print encrypted message bytes as hex
  printf("\33[0m\33[1m* Encrypted:\33[35m  ");
  encrypt(msg, key);
  print_block(msg, 16);

  // Print decrypted message bytes as hex (should be equal to the original message)
  printf("\33[0m\33[1m* Decrypted:\33[35m  ");
  decrypt(msg, key);
  print_block(msg, 16);

  return 0;
}

  /* =========== Status ===========
   * -- Encrypt --
   * add_round_key - Working :)
   * mix_columns   - Working :)
   * shift_rows    - Working :)
   * sub_bytes     - Working :)
   * 
   * -- Decrypt --
   * unmix_columns - NOT WORKING :(     <--------
   * unshift_rows  - Working :)
   * unsub_bytes   - Working :)
   * 
   * -- Other --
   * gen_key       - NOT IMPLEMENTED    <--------
   * key_expantion - NOT IMPLEMENTED    <--------
   * multi-block   - not even close :(  <--------
   */