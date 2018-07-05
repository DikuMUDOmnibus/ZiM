/*-
 * Copyright (c) 1998 fjoe <fjoe@iclub.nsu.ru>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id: charset.c 849 2006-04-22 13:08:54Z zsuzsu $
 */

unsigned char koi8_koi8[256] = {
	  0,   1,   2,   3,   4,   5,   6,   7,
	  8,   9,  10,  11,  12,  13,  14,  15,
	 16,  17,  18,  19,  20,  21,  22,  23,
	 24,  25,  26,  27,  28,  29,  30,  31,
	 32,  33,  34,  35,  36,  37,  38,  39,
	 40,  41,  42,  43,  44,  45,  46,  47,
	 48,  49,  50,  51,  52,  53,  54,  55,
	 56,  57,  58,  59,  60,  61,  62,  63,
	 64,  65,  66,  67,  68,  69,  70,  71,
	 72,  73,  74,  75,  76,  77,  78,  79,
	 80,  81,  82,  83,  84,  85,  86,  87,
	 88,  89,  90,  91,  92,  93,  94,  95,
	 96,  97,  98,  99, 100, 101, 102, 103,
	104, 105, 106, 107, 108, 109, 110, 111,
	112, 113, 114, 115, 116, 117, 118, 119,
	120, 121, 122, 123, 124, 125, 126, 127,
	128, 129, 130, 131, 132, 133, 134, 135,
	136, 137, 138, 139, 140, 141, 142, 143,
	144, 145, 146, 147, 148, 149, 150, 151,
	152, 153, 154, 155, 156, 157, 158, 159,
	160, 161, 162, 163, 164, 165, 166, 167,
	168, 169, 170, 171, 172, 173, 174, 175,
	176, 177, 178, 179, 180, 181, 182, 183,
	184, 185, 186, 187, 188, 189, 190, 191,
	192, 193, 194, 195, 196, 197, 198, 199,
	200, 201, 202, 203, 204, 205, 206, 207,
	208, 209, 210, 211, 212, 213, 214, 215,
	216, 217, 218, 219, 220, 221, 222, 223,
	224, 225, 226, 227, 228, 229, 230, 231,
	232, 233, 234, 235, 236, 237, 238, 239,
	240, 241, 242, 243, 244, 245, 246, 247,
	248, 249, 250, 251, 252, 253, 254, 255
};

unsigned char alt_koi8[256] = {
	  0,   1,   2,   3,   4,   5,   6,   7,
	  8,   9,  10,  11,  12,  13,  14,  15,
	 16,  17,  18,  19,  20,  21,  22,  23,
	 24,  25,  26,  27,  28,  29,  30,  31,
	 32,  33,  34,  35,  36,  37,  38,  39,
	 40,  41,  42,  43,  44,  45,  46,  47,
	 48,  49,  50,  51,  52,  53,  54,  55,
	 56,  57,  58,  59,  60,  61,  62,  63,
	 64,  65,  66,  67,  68,  69,  70,  71,
	 72,  73,  74,  75,  76,  77,  78,  79,
	 80,  81,  82,  83,  84,  85,  86,  87,
	 88,  89,  90,  91,  92,  93,  94,  95,
	 96,  97,  98,  99, 100, 101, 102, 103,
	104, 105, 106, 107, 108, 109, 110, 111,
	112, 113, 114, 115, 116, 117, 118, 119,
	120, 121, 122, 123, 124, 125, 126, 127,
	225, 226, 247, 231, 228, 229, 246, 250,
	233, 234, 235, 236, 237, 238, 239, 240,
	242, 243, 244, 245, 230, 232, 227, 254,
	251, 253, 255, 249, 248, 252, 224, 241,
	193, 194, 215, 199, 196, 197, 214, 218,
	201, 202, 203, 204, 205, 206, 207, 208,
	176, 177, 178, 182, 166, 181, 184, 185,
	164, 186, 187, 188, 189, 190, 167, 191,
	225, 226, 247, 231, 228, 229, 246, 250,
	233, 234, 235, 236, 237, 238, 239, 240,
	242, 243, 244, 245, 230, 232, 227, 254,
	251, 253, 255, 249, 248, 252, 224, 241,
	210, 211, 212, 213, 198, 200, 195, 222,
	219, 221, 223, 217, 216, 220, 192, 209,
	210, 211, 212, 213, 198, 200, 195, 222,
	219, 221, 223, 217, 216, 220, 192, 209
};

unsigned char koi8_alt[256] = {
	  0,   1,   2,   3,   4,   5,   6,   7,
	  8,   9,  10,  11,  12,  13,  14,  15,
	 16,  17,  18,  19,  20,  21,  22,  23,
	 24,  25,  26,  27,  28,  29,  30,  31,
	 32,  33,  34,  35,  36,  37,  38,  39,
	 40,  41,  42,  43,  44,  45,  46,  47,
	 48,  49,  50,  51,  52,  53,  54,  55,
	 56,  57,  58,  59,  60,  61,  62,  63,
	 64,  65,  66,  67,  68,  69,  70,  71,
	 72,  73,  74,  75,  76,  77,  78,  79,
	 80,  81,  82,  83,  84,  85,  86,  87,
	 88,  89,  90,  91,  92,  93,  94,  95,
	 96,  97,  98,  99, 100, 101, 102, 103,
	104, 105, 106, 107, 108, 109, 110, 111,
	112, 113, 114, 115, 116, 117, 118, 119,
	120, 121, 122, 123, 124, 125, 126, 127,
	128, 129, 130, 131, 132, 133, 134, 135,
	136, 137, 138, 139, 140, 141, 142, 143,
	144, 145, 146, 147, 148, 149, 150, 151,
	152, 153, 154, 155, 156, 157, 158, 159,
	160, 161, 162, 168, 186, 165, 179, 191,
	168, 169, 170, 171, 172, 173, 174, 175,
	176, 177, 178, 168, 170, 181, 178, 175,
	184, 185, 186, 187, 188, 189, 190, 191,
	238, 160, 161, 230, 164, 165, 228, 163,
	229, 168, 169, 170, 171, 172, 173, 174,
	175, 239, 224, 225, 226, 227, 166, 162,
	236, 235, 167, 232, 237, 233, 231, 234,
	158, 128, 129, 150, 132, 133, 148, 131,
	149, 136, 137, 138, 139, 140, 141, 142,
	143, 159, 144, 145, 146, 147, 134, 130,
	156, 155, 135, 152, 157, 153, 151, 154
};

unsigned char win_koi8[256] = {
	  0,   1,   2,   3,   4,   5,   6,   7,
	  8,   9,  10,  11,  12,  13,  14,  15,
	 16,  17,  18,  19,  20,  21,  22,  23,
	 24,  25,  26,  27,  28,  29,  30,  31,
	 32,  33,  34,  35,  36,  37,  38,  39,
	 40,  41,  42,  43,  44,  45,  46,  47,
	 48,  49,  50,  51,  52,  53,  54,  55,
	 56,  57,  58,  59,  60,  61,  62,  63,
	 64,  65,  66,  67,  68,  69,  70,  71,
	 72,  73,  74,  75,  76,  77,  78,  79,
	 80,  81,  82,  83,  84,  85,  86,  87,
	 88,  89,  90,  91,  92,  93,  94,  95,
	 96,  97,  98,  99, 100, 101, 102, 103,
	104, 105, 106, 107, 108, 109, 110, 111,
	112, 113, 114, 115, 116, 117, 118, 119,
	120, 121, 122, 123, 124, 125, 126, 127,	/* 127 */
	128, 129, 130, 131, 132, 133, 134, 135,
	136, 137, 138, 139, 140, 141, 142, 143,
	144, 145, 146, 147, 148, 149, 150, 151,
	152, 153, 154, 155, 156, 157, 158, 159, /* 159 */
	160, 161, 162, 165, 163, 168, 179, 169, /* 167 */
	179, 180, 171, 172, 173, 174, 175, 183, /* 175 */
	176, 177, 178, 182, 166, 181, 184, 185, /* 183 */
	163, 186, 187, 188, 189, 190, 167, 191, /* 191 */
	225, 226, 247, 231, 228, 229, 246, 250,
	233, 234, 235, 236, 237, 238, 239, 240,
	242, 243, 244, 245, 230, 232, 227, 254,
	251, 253, 255, 249, 248, 252, 224, 241,
	193, 194, 215, 199, 196, 197, 214, 218,
	201, 202, 203, 204, 205, 206, 207, 208,
	210, 211, 212, 213, 198, 200, 195, 222,
	219, 221, 223, 217, 216, 220, 192, 209	/* 255 */
};

unsigned char koi8_win[256] = {
	  0,   1,   2,   3,   4,   5,   6,   7,
	  8,   9,  10,  11,  12,  13,  14,  15,
	 16,  17,  18,  19,  20,  21,  22,  23,
	 24,  25,  26,  27,  28,  29,  30,  31,
	 32,  33,  34,  35,  36,  37,  38,  39,
	 40,  41,  42,  43,  44,  45,  46,  47,
	 48,  49,  50,  51,  52,  53,  54,  55,
	 56,  57,  58,  59,  60,  61,  62,  63,
	 64,  65,  66,  67,  68,  69,  70,  71,
	 72,  73,  74,  75,  76,  77,  78,  79,
	 80,  81,  82,  83,  84,  85,  86,  87,
	 88,  89,  90,  91,  92,  93,  94,  95,
	 96,  97,  98,  99, 100, 101, 102, 103,
	104, 105, 106, 107, 108, 109, 110, 111,
	112, 113, 114, 115, 116, 117, 118, 119,
	120, 121, 122, 123, 124, 125, 126, 127, /* 127 */
	128, 129, 130, 131, 132, 133, 134, 135, /* 135 */
	136, 137, 138, 139, 140, 141, 142, 143, /* 143 */
	144, 145, 146, 147, 148, 149, 150, 151, /* 151 */
	152, 153, 154, 155, 156, 157, 158, 159, /* 159 */
	160, 161, 162, 184, 186, 165, 179, 191, /* 167 */
	168, 169, 170, 171, 172, 173, 174, 175, /* 175 */
	176, 177, 178, 168, 170, 181, 178, 175, /* 183 */
	184, 185, 186, 187, 188, 189, 190, 191, /* 191 */
	254, 224, 225, 246, 228, 229, 244, 227,
	245, 232, 233, 234, 235, 236, 237, 238,
	239, 255, 240, 241, 242, 243, 230, 226,
	252, 251, 231, 248, 253, 249, 247, 250,
	222, 192, 193, 214, 196, 197, 212, 195,
	213, 200, 201, 202, 203, 204, 205, 206,
	207, 223, 208, 209, 210, 211, 198, 194,
	220, 219, 199, 216, 221, 217, 215, 218
};

unsigned char iso_koi8[256] = {
	  0,   1,   2,   3,   4,   5,   6,   7,
	  8,   9,  10,  11,  12,  13,  14,  15,
	 16,  17,  18,  19,  20,  21,  22,  23,
	 24,  25,  26,  27,  28,  29,  30,  31,
	 32,  33,  34,  35,  36,  37,  38,  39,
	 40,  41,  42,  43,  44,  45,  46,  47,
	 48,  49,  50,  51,  52,  53,  54,  55,
	 56,  57,  58,  59,  60,  61,  62,  63,
	 64,  65,  66,  67,  68,  69,  70,  71,
	 72,  73,  74,  75,  76,  77,  78,  79,
	 80,  81,  82,  83,  84,  85,  86,  87,
	 88,  89,  90,  91,  92,  93,  94,  95,
	 96,  97,  98,  99, 100, 101, 102, 103,
	104, 105, 106, 107, 108, 109, 110, 111,
	112, 113, 114, 115, 116, 117, 118, 119,
	120, 121, 122, 123, 124, 125, 126, 127,
	128, 129, 130, 131, 132, 133, 134, 135,
	136, 137, 138, 139, 140, 141, 142, 143,
	144, 145, 146, 147, 148, 149, 150, 151,
	152, 153, 154, 155, 156, 157, 158, 159,
	160, 161, 162, 163, 164, 165, 166, 167,
	168, 169, 170, 171, 172, 173, 174, 175,
	225, 226, 247, 231, 228, 229, 246, 250,
	233, 234, 235, 236, 237, 238, 239, 240,
	242, 243, 244, 245, 230, 232, 227, 254,
	251, 253, 255, 249, 248, 252, 224, 241,
	193, 194, 215, 199, 196, 197, 214, 218,
	201, 202, 203, 204, 205, 206, 207, 208,
	210, 211, 212, 213, 198, 200, 195, 222,
	219, 221, 223, 217, 216, 220, 192, 209,
	176, 177, 178, 179, 180, 181, 182, 183,
	184, 185, 186, 187, 188, 189, 190, 191
};

unsigned char koi8_iso[256] = {
	  0,   1,   2,   3,   4,   5,   6,   7,
	  8,   9,  10,  11,  12,  13,  14,  15,
	 16,  17,  18,  19,  20,  21,  22,  23,
	 24,  25,  26,  27,  28,  29,  30,  31,
	 32,  33,  34,  35,  36,  37,  38,  39,
	 40,  41,  42,  43,  44,  45,  46,  47,
	 48,  49,  50,  51,  52,  53,  54,  55,
	 56,  57,  58,  59,  60,  61,  62,  63,
	 64,  65,  66,  67,  68,  69,  70,  71,
	 72,  73,  74,  75,  76,  77,  78,  79,
	 80,  81,  82,  83,  84,  85,  86,  87,
	 88,  89,  90,  91,  92,  93,  94,  95,
	 96,  97,  98,  99, 100, 101, 102, 103,
	104, 105, 106, 107, 108, 109, 110, 111,
	112, 113, 114, 115, 116, 117, 118, 119,
	120, 121, 122, 123, 124, 125, 126, 127,
	128, 129, 130, 131, 132, 133, 134, 135,
	136, 137, 138, 139, 140, 141, 142, 143,
	144, 145, 146, 147, 148, 149, 150, 151,
	152, 153, 154, 155, 156, 157, 158, 159,
	160, 161, 162, 163, 164, 165, 166, 167,
	168, 169, 170, 171, 172, 173, 174, 175,
	240, 241, 242, 243, 244, 245, 246, 247,
	248, 249, 250, 251, 252, 253, 254, 255,
	238, 208, 209, 230, 212, 213, 228, 211,
	229, 216, 217, 218, 219, 220, 221, 222,
	223, 239, 224, 225, 226, 227, 214, 210,
	236, 235, 215, 232, 237, 233, 231, 234,
	206, 176, 177, 198, 180, 181, 196, 179,
	197, 184, 185, 186, 187, 188, 189, 190,
	191, 207, 192, 193, 194, 195, 182, 178,
	204, 203, 183, 200, 205, 201, 199, 202
};

unsigned char mac_koi8[256] = {
	  0,   1,   2,   3,   4,   5,   6,   7,
	  8,   9,  10,  11,  12,  13,  14,  15,
	 16,  17,  18,  19,  20,  21,  22,  23,
	 24,  25,  26,  27,  28,  29,  30,  31,
	 32,  33,  34,  35,  36,  37,  38,  39,
	 40,  41,  42,  43,  44,  45,  46,  47,
	 48,  49,  50,  51,  52,  53,  54,  55,
	 56,  57,  58,  59,  60,  61,  62,  63,
	 64,  65,  66,  67,  68,  69,  70,  71,
	 72,  73,  74,  75,  76,  77,  78,  79,
	 80,  81,  82,  83,  84,  85,  86,  87,
	 88,  89,  90,  91,  92,  93,  94,  95,
	 96,  97,  98,  99, 100, 101, 102, 103,
	104, 105, 106, 107, 108, 109, 110, 111,
	112, 113, 114, 115, 116, 117, 118, 119,
	120, 121, 122, 123, 124, 125, 126, 127,
	 48,  49,  50,  51,  52,  53,  54,  55,
	 56,  57,  33,  64,  35,  36,  37,  94,
	 38,  42, 215, 199, 196, 210, 195, 222,
	221, 223, 217, 220, 192, 209, 200, 241,
	 60,  62,  63,  47,  33,  97,  98,  99,
	100, 101, 102, 103,  42, 104, 105, 219,
	107, 108, 109, 110, 111, 112, 113, 194,
	216, 116, 117, 118,  65,  66,  67,  68,
	 69, 218, 197,  72, 225, 226, 204, 247,
	202, 231, 214, 201, 206, 203, 204, 205,
	 95, 228, 211, 207, 208,  54, 229,  95,
	234, 198, 212, 213, 246,  54,  55,  56,
	233, 250, 234, 236, 235, 237, 214, 238,
	240, 239, 242, 243, 245, 244, 230, 232,
	210, 227, 251, 254, 253, 249, 255, 222,
	219, 252, 248, 224, 241, 193, 209, 209
};

unsigned char koi8_mac[256] = {
	  0,   1,   2,   3,   4,   5,   6,   7,
	  8,   9,  10,  11,  12,  13,  14,  15,
	 16,  17,  18,  19,  20,  21,  22,  23,
	 24,  25,  26,  27,  28,  29,  30,  31,
	 32,  33,  34,  35,  36,  37,  38,  39,
	 40,  41,  42,  43,  44,  45,  46,  47,
	 48,  49,  50,  51,  52,  53,  54,  55,
	 56,  57,  58,  59,  60,  61,  62,  63,
	 64,  65,  66,  67,  68,  69,  70,  71,
	 72,  73,  74,  75,  76,  77,  78,  79,
	 80,  81,  82,  83,  84,  85,  86,  87,
	 88,  89,  90,  91,  92,  93,  94,  95,
	 96,  97,  98,  99, 100, 101, 102, 103,
	104, 105, 106, 107, 108, 109, 110, 111,
	112, 113, 114, 115, 116, 117, 118, 119,
	120, 121, 122, 123, 124, 125, 126, 127,
	128, 129, 130, 131, 132, 133, 134, 135,
	136, 137, 138, 139, 140, 141, 142, 143,
	144, 145, 146, 147, 148, 149, 150, 151,
	152, 153, 154, 155, 156, 157, 158, 159,
	224, 225, 226, 227, 228, 229, 230, 231,
	232, 233, 234, 235, 236, 237, 238, 239,
	176, 177, 178, 179, 180, 181, 182, 183,
	184, 185, 186, 187, 188, 189, 190, 191,
	254, 224, 225, 246, 228, 229, 244, 227,
	245, 232, 233, 234, 235, 236, 237, 238,
	239, 223, 240, 241, 242, 243, 230, 226,
	252, 251, 231, 248, 253, 249, 247, 250,
	158, 128, 129, 150, 132, 133, 148, 131,
	149, 136, 137, 138, 139, 140, 141, 142,
	143, 159, 144, 145, 146, 147, 134, 130,
	156, 155, 135, 152, 157, 153, 151, 154
};

unsigned char koi8_vola[256] = {
	  0,   1,   2,   3,   4,   5,   6,   7,
	  8,   9,  10,  11,  12,  13,  14,  15,
	 16,  17,  18,  19,  20,  21,  22,  23,
	 24,  25,  26,  27,  28,  29,  30,  31,
	 32,  33,  34,  35,  36,  37,  38,  39,
	 40,  41,  42,  43,  44,  45,  46,  47,
	 48,  49,  50,  51,  52,  53,  54,  55,
	 56,  57,  58,  59,  60,  61,  62,  63,
	 64,  65,  66,  67,  68,  69,  70,  71,
	 72,  73,  74,  75,  76,  77,  78,  79,
	 80,  81,  82,  83,  84,  85,  86,  87,
	 88,  89,  90,  91,  92,  93,  94,  95,
	 96,  97,  98,  99, 100, 101, 102, 103,
	104, 105, 106, 107, 108, 109, 110, 111,
	112, 113, 114, 115, 116, 117, 118, 119,
	120, 121, 122, 123, 124, 125, 126, 127,
	128, 129, 130, 131, 132, 133, 134, 135,
	136, 137, 138, 139, 140, 141, 142, 143,
	144, 145, 146, 147, 148, 149, 150, 151,
	152, 153, 154, 155, 156, 157, 158, 159,
	160, 161, 162, 'e', 164, 165, 166, 167,
	168, 169, 170, 171, 172, 173, 174, 175,
	176, 177, 178, 'E', 180, 181, 182, 183,
	184, 185, 186, 187, 188, 189, 190, 191,
	'u', 'a', 'b', 'c', 'd', 'e', 'f', 'g',
	'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
	'p', 'y', 'r', 's', 't', 'u', 'j', 'v',
	'\'', 'y', 'z', 's', 'e', 's', 'c', '\'',
	'U', 'A', 'B', 'C', 'D', 'E', 'F', 'G',
	'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
	'P', 'Y', 'R', 'S', 'T', 'U', 'J', 'V',
	'\'', 'Y', 'Z', 'S', 'E', 'S', 'C', '\''
};

