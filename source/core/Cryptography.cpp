//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "Common.h"
#include "PuttyIntf.h"
#include "Cryptography.h"
#include <process.h>

/*
 ---------------------------------------------------------------------------
 Copyright (c) 2002, Dr Brian Gladman <brg@gladman.me.uk>, Worcester, UK.
 All rights reserved.

 LICENSE TERMS

 The free distribution and use of this software in both source and binary
 form is allowed (with or without changes) provided that:

   1. distributions of this source code include the above copyright
      notice, this list of conditions and the following disclaimer;

   2. distributions in binary form include the above copyright
      notice, this list of conditions and the following disclaimer
      in the documentation and/or other associated materials;

   3. the copyright holder's name is not used to endorse products
      built using this software without specific written permission.

 ALTERNATIVELY, provided that this notice is retained in full, this product
 may be distributed under the terms of the GNU General Public License (GPL),
 in which case the provisions of the GPL apply INSTEAD OF those given above.

 DISCLAIMER

 This software is provided 'as is' with no explicit or implied warranties
 in respect of its properties, including, but not limited to, correctness
 and/or fitness for purpose.
 -------------------------------------------------------------------------

 This file implements password based file encryption and authentication
 using AES in CTR mode, HMAC-SHA1 authentication and RFC2898 password
 based key derivation.

 This is an implementation of HMAC, the FIPS standard keyed hash function

*/

#include <memory.h>

#define sha1_ctx                  SHA_State
#define sha1_begin(ctx)           putty_SHA_Init(ctx)
#define sha1_hash(buf, len, ctx)  SHA_Bytes(ctx, buf, len)
#define sha1_end(dig, ctx)        putty_SHA_Final(ctx, dig)

#define IN_BLOCK_LENGTH     64
#define OUT_BLOCK_LENGTH    20
#define HMAC_IN_DATA        0xffffffff

typedef struct
{   unsigned char   key[IN_BLOCK_LENGTH];
    sha1_ctx        ctx[1];
    unsigned int    klen;
} hmac_ctx;

/* initialise the HMAC context to zero */
static void hmac_sha1_begin(hmac_ctx cx[1])
{
    memset(cx, 0, sizeof(hmac_ctx));
}

/* input the HMAC key (can be called multiple times)    */
static void hmac_sha1_key(const unsigned char key[], unsigned long key_len, hmac_ctx cx[1])
{
    if(cx->klen + key_len > IN_BLOCK_LENGTH)    /* if the key has to be hashed  */
    {
        if(cx->klen <= IN_BLOCK_LENGTH)         /* if the hash has not yet been */
        {                                       /* started, initialise it and   */
            sha1_begin(cx->ctx);                /* hash stored key characters   */
            sha1_hash(cx->key, cx->klen, cx->ctx);
        }

        sha1_hash(const_cast<unsigned char *>(key), key_len, cx->ctx);       /* hash long key data into hash */
    }
    else                                        /* otherwise store key data     */
        memcpy(cx->key + cx->klen, key, key_len);

    cx->klen += key_len;                        /* update the key length count  */
}

/* input the HMAC data (can be called multiple times) - */
/* note that this call terminates the key input phase   */
static void hmac_sha1_data(const unsigned char data[], unsigned long data_len, hmac_ctx cx[1])
{   unsigned int i;

    if(cx->klen != HMAC_IN_DATA)                /* if not yet in data phase */
    {
        if(cx->klen > IN_BLOCK_LENGTH)          /* if key is being hashed   */
        {                                       /* complete the hash and    */
            sha1_end(cx->key, cx->ctx);         /* store the result as the  */
            cx->klen = OUT_BLOCK_LENGTH;        /* key and set new length   */
        }

        /* pad the key if necessary */
        memset(cx->key + cx->klen, 0, IN_BLOCK_LENGTH - cx->klen);

        /* xor ipad into key value  */
        for(i = 0; i < (IN_BLOCK_LENGTH >> 2); ++i)
            ((unsigned long*)cx->key)[i] ^= 0x36363636;

        /* and start hash operation */
        sha1_begin(cx->ctx);
        sha1_hash(cx->key, IN_BLOCK_LENGTH, cx->ctx);

        /* mark as now in data mode */
        cx->klen = HMAC_IN_DATA;
    }

    /* hash the data (if any)       */
    if(data_len)
        sha1_hash(const_cast<unsigned char *>(data), data_len, cx->ctx);
}

/* compute and output the MAC value */
static void hmac_sha1_end(unsigned char mac[], unsigned long mac_len, hmac_ctx cx[1])
{   unsigned char dig[OUT_BLOCK_LENGTH];
    unsigned int i;

    /* if no data has been entered perform a null data phase        */
    if(cx->klen != HMAC_IN_DATA)
        hmac_sha1_data((const unsigned char*)0, 0, cx);

    sha1_end(dig, cx->ctx);         /* complete the inner hash      */

    /* set outer key value using opad and removing ipad */
    for(i = 0; i < (IN_BLOCK_LENGTH >> 2); ++i)
        ((unsigned long*)cx->key)[i] ^= 0x36363636 ^ 0x5c5c5c5c;

    /* perform the outer hash operation */
    sha1_begin(cx->ctx);
    sha1_hash(cx->key, IN_BLOCK_LENGTH, cx->ctx);
    sha1_hash(dig, OUT_BLOCK_LENGTH, cx->ctx);
    sha1_end(dig, cx->ctx);

    /* output the hash value            */
    for(i = 0; i < mac_len; ++i)
        mac[i] = dig[i];
}

#define BLOCK_SIZE  16

void aes_set_encrypt_key(const unsigned char in_key[], unsigned int klen, void * cx)
{
  call_aes_setup(cx, BLOCK_SIZE, const_cast<unsigned char *>(in_key), klen);
}

void aes_encrypt_block(const unsigned char in_blk[], unsigned char out_blk[], void * cx)
{
  int Index;
  memmove(out_blk, in_blk, BLOCK_SIZE);
  for (Index = 0; Index < 4; Index++)
  {
    unsigned char t;
    t = out_blk[Index * 4 + 0];
    out_blk[Index * 4 + 0] = out_blk[Index * 4 + 3];
    out_blk[Index * 4 + 3] = t;
    t = out_blk[Index * 4 + 1];
    out_blk[Index * 4 + 1] = out_blk[Index * 4 + 2];
    out_blk[Index * 4 + 2] = t;
  }
  call_aes_encrypt(cx, reinterpret_cast<unsigned int*>(out_blk));
  for (Index = 0; Index < 4; Index++)
  {
    unsigned char t;
    t = out_blk[Index * 4 + 0];
    out_blk[Index * 4 + 0] = out_blk[Index * 4 + 3];
    out_blk[Index * 4 + 3] = t;
    t = out_blk[Index * 4 + 1];
    out_blk[Index * 4 + 1] = out_blk[Index * 4 + 2];
    out_blk[Index * 4 + 2] = t;
  }
}

typedef struct
{   unsigned char   nonce[BLOCK_SIZE];          /* the CTR nonce          */
    unsigned char   encr_bfr[BLOCK_SIZE];       /* encrypt buffer         */
    void *          encr_ctx;                   /* encryption context     */
    hmac_ctx        auth_ctx;                   /* authentication context */
    unsigned int    encr_pos;                   /* block position (enc)   */
    unsigned int    pwd_len;                    /* password length        */
    unsigned int    mode;                       /* File encryption mode   */
} fcrypt_ctx;

#define MAX_KEY_LENGTH        32
#define KEYING_ITERATIONS   1000
#define PWD_VER_LENGTH         2

/*
    Field lengths (in bytes) versus File Encryption Mode (0 < mode < 4)

    Mode Key Salt  MAC Overhead
       1  16    8   10       18
       2  24   12   10       22
       3  32   16   10       26

   The following macros assume that the mode value is correct.
*/

#define KEY_LENGTH(mode)        (8 * (mode & 3) + 8)
#define SALT_LENGTH(mode)       (4 * (mode & 3) + 4)
#define MAC_LENGTH(mode)        (10)

/* subroutine for data encryption/decryption    */
/* this could be speeded up a lot by aligning   */
/* buffers and using 32 bit operations          */

static void derive_key(const unsigned char pwd[],  /* the PASSWORD     */
               unsigned int pwd_len,        /* and its length   */
               const unsigned char salt[],  /* the SALT and its */
               unsigned int salt_len,       /* length           */
               unsigned int iter,   /* the number of iterations */
               unsigned char key[], /* space for the output key */
               unsigned int key_len)/* and its required length  */
{
    unsigned int    i, j, k, n_blk;
    unsigned char uu[OUT_BLOCK_LENGTH], ux[OUT_BLOCK_LENGTH];
    hmac_ctx c1[1], c2[1], c3[1];

    /* set HMAC context (c1) for password               */
    hmac_sha1_begin(c1);
    hmac_sha1_key(pwd, pwd_len, c1);

    /* set HMAC context (c2) for password and salt      */
    memmove(c2, c1, sizeof(hmac_ctx));
    hmac_sha1_data(salt, salt_len, c2);

    /* find the number of SHA blocks in the key         */
    n_blk = 1 + (key_len - 1) / OUT_BLOCK_LENGTH;

    for(i = 0; i < n_blk; ++i) /* for each block in key */
    {
        /* ux[] holds the running xor value             */
        memset(ux, 0, OUT_BLOCK_LENGTH);

        /* set HMAC context (c3) for password and salt  */
        memmove(c3, c2, sizeof(hmac_ctx));

        /* enter additional data for 1st block into uu  */
        uu[0] = (unsigned char)((i + 1) >> 24);
        uu[1] = (unsigned char)((i + 1) >> 16);
        uu[2] = (unsigned char)((i + 1) >> 8);
        uu[3] = (unsigned char)(i + 1);

        /* this is the key mixing iteration         */
        for(j = 0, k = 4; j < iter; ++j)
        {
            /* add previous round data to HMAC      */
            hmac_sha1_data(uu, k, c3);

            /* obtain HMAC for uu[]                 */
            hmac_sha1_end(uu, OUT_BLOCK_LENGTH, c3);

            /* xor into the running xor block       */
            for(k = 0; k < OUT_BLOCK_LENGTH; ++k)
                ux[k] ^= uu[k];

            /* set HMAC context (c3) for password   */
            memmove(c3, c1, sizeof(hmac_ctx));
        }

        /* compile key blocks into the key output   */
        j = 0; k = i * OUT_BLOCK_LENGTH;
        while(j < OUT_BLOCK_LENGTH && k < key_len)
            key[k++] = ux[j++];
    }
}

static void encr_data(unsigned char data[], unsigned long d_len, fcrypt_ctx cx[1])
{
    unsigned long i = 0, pos = cx->encr_pos;

    while(i < d_len)
    {
        if(pos == BLOCK_SIZE)
        {   unsigned int j = 0;
            /* increment encryption nonce   */
            while(j < 8 && !++cx->nonce[j])
                ++j;
            /* encrypt the nonce to form next xor buffer    */
            aes_encrypt_block(cx->nonce, cx->encr_bfr, cx->encr_ctx);
            pos = 0;
        }

        data[i++] ^= cx->encr_bfr[pos++];
    }

    cx->encr_pos = pos;
}

static void fcrypt_init(
    int mode,                               /* the mode to be used (input)          */
    const unsigned char pwd[],              /* the user specified password (input)  */
    unsigned int pwd_len,                   /* the length of the password (input)   */
    const unsigned char salt[],             /* the salt (input)                     */
    unsigned char pwd_ver[PWD_VER_LENGTH],  /* 2 byte password verifier (output)    */
    fcrypt_ctx      cx[1])                  /* the file encryption context (output) */
{
    unsigned char kbuf[2 * MAX_KEY_LENGTH + PWD_VER_LENGTH];

    cx->mode = mode;
    cx->pwd_len = pwd_len;

    /* derive the encryption and authetication keys and the password verifier   */
    derive_key(pwd, pwd_len, salt, SALT_LENGTH(mode), KEYING_ITERATIONS,
                        kbuf, 2 * KEY_LENGTH(mode) + PWD_VER_LENGTH);

    /* initialise the encryption nonce and buffer pos   */
    cx->encr_pos = BLOCK_SIZE;
    /* if we need a random component in the encryption  */
    /* nonce, this is where it would have to be set     */
    memset(cx->nonce, 0, BLOCK_SIZE * sizeof(unsigned char));

    /* initialise for encryption using key 1            */
    cx->encr_ctx = call_aes_make_context();
    aes_set_encrypt_key(kbuf, KEY_LENGTH(mode), cx->encr_ctx);

    /* initialise for authentication using key 2        */
    hmac_sha1_begin(&cx->auth_ctx);
    hmac_sha1_key(kbuf + KEY_LENGTH(mode), KEY_LENGTH(mode), &cx->auth_ctx);

    if (pwd_ver != NULL)
    {
      memmove(pwd_ver, kbuf + 2 * KEY_LENGTH(mode), PWD_VER_LENGTH);
    }
}

/* perform 'in place' encryption and authentication */

static void fcrypt_encrypt(unsigned char data[], unsigned int data_len, fcrypt_ctx cx[1])
{
    encr_data(data, data_len, cx);
    hmac_sha1_data(data, data_len, &cx->auth_ctx);
}

/* perform 'in place' authentication and decryption */

static void fcrypt_decrypt(unsigned char data[], unsigned int data_len, fcrypt_ctx cx[1])
{
    hmac_sha1_data(data, data_len, &cx->auth_ctx);
    encr_data(data, data_len, cx);
}

/* close encryption/decryption and return the MAC value */

static int fcrypt_end(unsigned char mac[], fcrypt_ctx cx[1])
{
    hmac_sha1_end(mac, MAC_LENGTH(cx->mode), &cx->auth_ctx);
    call_aes_free_context(cx->encr_ctx);
    return MAC_LENGTH(cx->mode);    /* return MAC length in bytes   */
}
//---------------------------------------------------------------------------
#define PASSWORD_MANAGER_AES_MODE 3
//---------------------------------------------------------------------------
static void __fastcall FillBufferWithRandomData(char * Buf, int Len)
{
  while (Len > 0)
  {
    *Buf = static_cast<char>((rand() >> 7) & 0xFF);
    Buf++;
    Len--;
  }
}
//---------------------------------------------------------------------------
static RawByteString __fastcall AES256Salt()
{
  RawByteString Result;
  Result.SetLength(SALT_LENGTH(PASSWORD_MANAGER_AES_MODE));
  FillBufferWithRandomData(Result.c_str(), Result.Length());
  return Result;
}
//---------------------------------------------------------------------------
void __fastcall AES256EncyptWithMAC(RawByteString Input, UnicodeString Password,
  RawByteString & Salt, RawByteString & Output, RawByteString & Mac)
{
  fcrypt_ctx aes;
  if (Salt.IsEmpty())
  {
    Salt = AES256Salt();
  }
  assert(Salt.Length() == SALT_LENGTH(PASSWORD_MANAGER_AES_MODE));
  UTF8String UtfPassword = Password;
  fcrypt_init(PASSWORD_MANAGER_AES_MODE,
    reinterpret_cast<const unsigned char *>(UtfPassword.c_str()), UtfPassword.Length(),
    reinterpret_cast<const unsigned char *>(Salt.c_str()), NULL, &aes);
  Output = Input;
  Output.Unique();
  fcrypt_encrypt(reinterpret_cast<unsigned char *>(Output.c_str()), Output.Length(), &aes);
  Mac.SetLength(MAC_LENGTH(PASSWORD_MANAGER_AES_MODE));
  fcrypt_end(reinterpret_cast<unsigned char *>(Mac.c_str()), &aes);
}
//---------------------------------------------------------------------------
void __fastcall AES256EncyptWithMAC(RawByteString Input, UnicodeString Password,
  RawByteString & Output)
{
  RawByteString Salt;
  RawByteString Encrypted;
  RawByteString Mac;
  AES256EncyptWithMAC(Input, Password, Salt, Encrypted, Mac);
  Output = Salt + Encrypted + Mac;
}
//---------------------------------------------------------------------------
bool __fastcall AES256DecryptWithMAC(RawByteString Input, UnicodeString Password,
  RawByteString Salt, RawByteString & Output, RawByteString Mac)
{
  fcrypt_ctx aes;
  assert(Salt.Length() == SALT_LENGTH(PASSWORD_MANAGER_AES_MODE));
  UTF8String UtfPassword = Password;
  fcrypt_init(PASSWORD_MANAGER_AES_MODE,
    reinterpret_cast<const unsigned char *>(UtfPassword.c_str()), UtfPassword.Length(),
    reinterpret_cast<const unsigned char *>(Salt.c_str()), NULL, &aes);
  Output = Input;
  Output.Unique();
  fcrypt_decrypt(reinterpret_cast<unsigned char *>(Output.c_str()), Output.Length(), &aes);
  RawByteString Mac2;
  Mac2.SetLength(MAC_LENGTH(PASSWORD_MANAGER_AES_MODE));
  assert(Mac.Length() == Mac2.Length());
  fcrypt_end(reinterpret_cast<unsigned char *>(Mac2.c_str()), &aes);
  return (Mac2 == Mac);
}
//---------------------------------------------------------------------------
bool __fastcall AES256DecryptWithMAC(RawByteString Input, UnicodeString Password,
  RawByteString & Output)
{
  bool Result =
    Input.Length() > SALT_LENGTH(PASSWORD_MANAGER_AES_MODE) + MAC_LENGTH(PASSWORD_MANAGER_AES_MODE);
  if (Result)
  {
    RawByteString Salt = Input.SubString(1, SALT_LENGTH(PASSWORD_MANAGER_AES_MODE));
    RawByteString Encrypted =
      Input.SubString(SALT_LENGTH(PASSWORD_MANAGER_AES_MODE) + 1,
        Input.Length() - SALT_LENGTH(PASSWORD_MANAGER_AES_MODE) - MAC_LENGTH(PASSWORD_MANAGER_AES_MODE));
    RawByteString Mac =
      Input.SubString(Input.Length() - MAC_LENGTH(PASSWORD_MANAGER_AES_MODE) + 1,
        MAC_LENGTH(PASSWORD_MANAGER_AES_MODE));
    Result = AES256DecryptWithMAC(Encrypted, Password, Salt, Output, Mac);
  }
  return Result;
}
//---------------------------------------------------------------------------
void __fastcall AES256CreateVerifier(UnicodeString Input, RawByteString & Verifier)
{
  RawByteString Salt = AES256Salt();
  RawByteString Dummy = AES256Salt();

  RawByteString Encrypted;
  RawByteString Mac;
  AES256EncyptWithMAC(Dummy, Input, Salt, Encrypted, Mac);

  Verifier = Salt + Dummy + Mac;
}
//---------------------------------------------------------------------------
bool __fastcall AES256Verify(UnicodeString Input, RawByteString Verifier)
{
  int SaltLength = SALT_LENGTH(PASSWORD_MANAGER_AES_MODE);
  RawByteString Salt = Verifier.SubString(1, SaltLength);
  RawByteString Dummy = Verifier.SubString(SaltLength + 1, SaltLength);
  RawByteString Mac = Verifier.SubString(SaltLength + SaltLength + 1, MAC_LENGTH(PASSWORD_MANAGER_AES_MODE));

  RawByteString Encrypted;
  RawByteString Mac2;
  AES256EncyptWithMAC(Dummy, Input, Salt, Encrypted, Mac2);

  assert(Mac2.Length() == Mac.Length());

  return (Mac == Mac2);
}
//---------------------------------------------------------------------------
unsigned char SScrambleTable[256] =
{
    0, 223, 235, 233, 240, 185,  88, 102,  22, 130,  27,  53,  79, 125,  66, 201,
   90,  71,  51,  60, 134, 104, 172, 244, 139,  84,  91,  12, 123, 155, 237, 151,
  192,   6,  87,  32, 211,  38, 149,  75, 164, 145,  52, 200, 224, 226, 156,  50,
  136, 190, 232,  63, 129, 209, 181, 120,  28,  99, 168,  94, 198,  40, 238, 112,
   55, 217, 124,  62, 227,  30,  36, 242, 208, 138, 174, 231,  26,  54, 214, 148,
   37, 157,  19, 137, 187, 111, 228,  39, 110,  17, 197, 229, 118, 246, 153,  80,
   21, 128,  69, 117, 234,  35,  58,  67,  92,   7, 132, 189,   5, 103,  10,  15,
  252, 195,  70, 147, 241, 202, 107,  49,  20, 251, 133,  76, 204,  73, 203, 135,
  184,  78, 194, 183,   1, 121, 109,  11, 143, 144, 171, 161,  48, 205, 245,  46,
   31,  72, 169, 131, 239, 160,  25, 207, 218, 146,  43, 140, 127, 255,  81,  98,
   42, 115, 173, 142, 114,  13,   2, 219,  57,  56,  24, 126,   3, 230,  47, 215,
    9,  44, 159,  33, 249,  18,  93,  95,  29, 113, 220,  89,  97, 182, 248,  64,
   68,  34,   4,  82,  74, 196, 213, 165, 179, 250, 108, 254,  59,  14, 236, 175,
   85, 199,  83, 106,  77, 178, 167, 225,  45, 247, 163, 158,   8, 221,  61, 191,
  119,  16, 253, 105, 186,  23, 170, 100, 216,  65, 162, 122, 150, 176, 154, 193,
  206, 222, 188, 152, 210, 243,  96,  41,  86, 180, 101, 177, 166, 141, 212, 116
};
//---------------------------------------------------------------------------
unsigned char * ScrambleTable;
unsigned char * UnscrambleTable;
//---------------------------------------------------------------------------
RawByteString __fastcall ScramblePassword(UnicodeString Password)
{
  #define SCRAMBLE_LENGTH_EXTENSION 50
  UTF8String UtfPassword = Password;
  int Len = UtfPassword.Length();
  char * Buf = new char[Len + SCRAMBLE_LENGTH_EXTENSION];
  int Padding = (((Len + 3) / 17) * 17 + 17) - 3 - Len;
  for (int Index = 0; Index < Padding; Index++)
  {
    int P = 0;
    while ((P <= 0) || (P > 255) || IsDigit(static_cast<wchar_t>(P)))
    {
      P = (int)((double)rand() / ((double)RAND_MAX / 256.0));
    }
    Buf[Index] = (unsigned char)P;
  }
  Buf[Padding] = (char)('0' + (Len % 10));
  Buf[Padding + 1] = (char)('0' + ((Len / 10) % 10));
  Buf[Padding + 2] = (char)('0' + ((Len / 100) % 10));
  strcpy(Buf + Padding + 3, UtfPassword.c_str());
  char * S = Buf;
  int Last = 31;
  while (*S != '\0')
  {
    Last = (Last + (unsigned char)*S) % 255 + 1;
    *S = ScrambleTable[Last];
    S++;
  }
  RawByteString Result = Buf;
  memset(Buf, 0, Len + SCRAMBLE_LENGTH_EXTENSION);
  delete[] Buf;
  return Result;
}
//---------------------------------------------------------------------------
bool __fastcall UnscramblePassword(RawByteString Scrambled, UnicodeString & Password)
{
  Scrambled.Unique();
  char * S = Scrambled.c_str();
  int Last = 31;
  while (*S != '\0')
  {
    int X = (int)UnscrambleTable[(unsigned char)*S] - 1 - (Last % 255);
    if (X <= 0)
    {
      X += 255;
    }
    *S = (char)X;
    Last = (Last + X) % 255 + 1;
    S++;
  }

  S = Scrambled.c_str();
  while ((*S != '\0') && ((*S < '0') || (*S > '9')))
  {
    S++;
  }
  bool Result = false;
  if (strlen(S) >= 3)
  {
    int Len = (S[0] - '0') + 10 * (S[1] - '0') + 100 * (S[2] - '0');
    int Total = (((Len + 3) / 17) * 17 + 17);
    if ((Len >= 0) && (Total == Scrambled.Length()) && (Total - (S - Scrambled.c_str()) - 3 == Len))
    {
      Scrambled.Delete(1, Scrambled.Length() - Len);
      Result = true;
    }
  }
  if (Result)
  {
    Password = UTF8String(Scrambled.c_str(), Scrambled.Length());
  }
  else
  {
    Password = "";
  }
  return Result;
}
//---------------------------------------------------------------------------
void __fastcall CryptographyInitialize()
{
  ScrambleTable = SScrambleTable;
  UnscrambleTable = new unsigned char[256];
  for (int Index = 0; Index < 256; Index++)
  {
    UnscrambleTable[SScrambleTable[Index]] = (unsigned char)Index;
  }
  srand((unsigned int)time(NULL) ^ (unsigned int)getpid());
}
//---------------------------------------------------------------------------
void __fastcall CryptographyFinalize()
{
  delete[] UnscrambleTable;
  UnscrambleTable = NULL;
  ScrambleTable = NULL;
}
//---------------------------------------------------------------------------
int __fastcall PasswordMaxLength()
{
  return 128;
}
//---------------------------------------------------------------------------
int __fastcall IsValidPassword(UnicodeString Password)
{
  if (Password.IsEmpty() || (Password.Length() > PasswordMaxLength()))
  {
    return -1;
  }
  else
  {
    int A = 0;
    int B = 0;
    int C = 0;
    int D = 0;
    for (int Index = 1; Index <= Password.Length(); Index++)
    {
      if (IsLowerCaseLetter(Password[Index]))
      {
        A = 1;
      }
      else if (IsUpperCaseLetter(Password[Index]))
      {
        B = 1;
      }
      else if (IsDigit(Password[Index]))
      {
        C = 1;
      }
      else
      {
        D = 1;
      }
    }
    return (Password.Length() >= 6) && ((A + B + C + D) >= 2);
  }
}
