
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define HARDWARE
#define SOFTWARE

#define DEBUG

#define PIPO64_128
//#define PIPO64_256

#ifdef PIPO64_128
#define ROUND 13
#define SIZE 2				//64 = 32 * 2
#define INT_NUM 2			//64 = 32 * 2
#define MASTER_KEY_SIZE 2	//128 = 64 * 2
#elif defined PIPO64_256
#define ROUND 15
#define SIZE 2
#define INT_NUM 2
#define MASTER_KEY_SIZE 4	//256 = 64 * 2
#endif

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

u32 MASTER_KEY[MASTER_KEY_SIZE * INT_NUM] = { 0, };
u32 ROUND_KEY[(ROUND + 1) * INT_NUM] = { 0, };
u32 PLAIN_TEXT[SIZE] = { 0, };
u32 CIPHER_TEXT[SIZE] = { 0, };

void keyadd(u8* val, u8* rk)
{
	val[0] ^= rk[0];
	val[1] ^= rk[1];
	val[2] ^= rk[2];
	val[3] ^= rk[3];
	val[4] ^= rk[4];
	val[5] ^= rk[5];
	val[6] ^= rk[6];
	val[7] ^= rk[7];
}

void sbox(u8 *X)
{
	u8 T[3] = { 0, };
	//(MSB: x[7], LSB: x[0]) 
	// Input: x[7], x[6], x[5], x[4], x[3], x[2], x[1], x[0] 
	//S5_1
	X[5] ^= (X[7] & X[6]);
	X[4] ^= (X[3] & X[5]);
	X[7] ^= X[4];
	X[6] ^= X[3];
	X[3] ^= (X[4] | X[5]);
	X[5] ^= X[7];
	X[4] ^= (X[5] & X[6]);
	//S3
	X[2] ^= X[1] & X[0];
	X[0] ^= X[2] | X[1];
	X[1] ^= X[2] | X[0];
	X[2] = ~X[2];
	// Extend XOR
	X[7] ^= X[1];	X[3] ^= X[2];	X[4] ^= X[0];
	//S5_2
	T[0] = X[7];	T[1] = X[3];	T[2] = X[4];
	X[6] ^= (T[0] & X[5]);
	T[0] ^= X[6];
	X[6] ^= (T[2] | T[1]);
	T[1] ^= X[5];
	X[5] ^= (X[6] | T[2]);
	T[2] ^= (T[1] & T[0]);
	// Truncate XOR and bit change
	X[2] ^= T[0];	T[0] = X[1] ^ T[2];	X[1] = X[0]^T[1];	X[0] = X[7];	X[7] = T[0];
	T[1] = X[3];	X[3] = X[6];	X[6] = T[1];
	T[2] = X[4];	X[4] = X[5];	X[5] = T[2];
	// Output: (MSb) x[7], x[6], x[5], x[4], x[3], x[2], x[1], x[0] (LSb)
}

void inv_sbox(u8 *X)
{	//(MSB: x[7], LSB: x[0]) 
	// Input: x[7], x[6], x[5], x[4], x[3], x[2], x[1], x[0] 

	u8 T[3] = { 0, };

	T[0] = X[7]; X[7] = X[0]; X[0] = X[1]; X[1] = T[0];
	T[0] = X[7];	T[1] = X[6];	T[2] = X[5];
	// S52 inv
	X[4] ^= (X[3] | T[2]);
	X[3] ^= (T[2] | T[1]);
	T[1] ^= X[4];
	T[0] ^= X[3];
	T[2] ^= (T[1] & T[0]);
	X[3] ^= (X[4] & X[7]);
	//  Extended XOR
	X[0] ^= T[1]; X[1] ^= T[2]; X[2] ^= T[0];	
	T[0] = X[3]; X[3] = X[6]; X[6] = T[0];
	T[0] = X[5]; X[5] = X[4]; X[4] = T[0];
	//  Truncated XOR
	X[7] ^= X[1];	X[3] ^= X[2];	X[4] ^= X[0];
	// Inv_S5_1
	X[4] ^= (X[5] & X[6]);
	X[5] ^= X[7];
	X[3] ^= (X[4] | X[5]);
	X[6] ^= X[3];
	X[7] ^= X[4];
	X[4] ^= (X[3] & X[5]);
	X[5] ^= (X[7] & X[6]);
	// Inv_S3
	X[2] = ~X[2];
	X[1] ^= X[2] | X[0];
	X[0] ^= X[2] | X[1];
	X[2] ^= X[1] & X[0];
	 // Output: x[7], x[6], x[5], x[4], x[3], x[2], x[1], x[0]
}

//left rotation: (0,7,4,3,6,5,1,2)
void pbox(u8* X)
{
	X[1] = ((X[1] << 7)) | ((X[1] >> 1));
	X[2] = ((X[2] << 4)) | ((X[2] >> 4));
	X[3] = ((X[3] << 3)) | ((X[3] >> 5));
	X[4] = ((X[4] << 6)) | ((X[4] >> 2));
	X[5] = ((X[5] << 5)) | ((X[5] >> 3));
	X[6] = ((X[6] << 1)) | ((X[6] >> 7));
	X[7] = ((X[7] << 2)) | ((X[7] >> 6));

}

//left rotation(inverse): (0,1,4,5,2,3,7,6)
void inv_pbox(u8* X)
{
	X[1] = ((X[1] << 1)) | ((X[1] >> 7));
	X[2] = ((X[2] << 4)) | ((X[2] >> 4));
	X[3] = ((X[3] << 5)) | ((X[3] >> 3));
	X[4] = ((X[4] << 2)) | ((X[4] >> 6));
	X[5] = ((X[5] << 3)) | ((X[5] >> 5));
	X[6] = ((X[6] << 7)) | ((X[6] >> 1));
	X[7] = ((X[7] << 6)) | ((X[7] >> 2));
}

void ENC(u32* PLAIN_TEXT, u32* ROUND_KEY, u32* CIPHER_TEXT) {
	int i = 0;
	u8* P = (u8*)PLAIN_TEXT;
	u8* RK = (u8*)ROUND_KEY;

	keyadd(P, RK);

#ifdef DEBUG
	printf("\n  WK Add: %02X%02X%02X%02X, %02X%02X%02X%02X", P[7], P[6], P[5], P[4], P[3], P[2], P[1], P[0]);
#endif
	for (i = 1; i < ROUND+1; i++)
	{
		//printf("\n  S Before : %02X %02X %02X %02X, %02X %02X %02X %02X", P[7], P[6], P[5], P[4], P[3], P[2], P[1], P[0]);
		sbox(P);
		//printf("\n  S After : %02X %02X %02X %02X, %02X %02X %02X %02X", P[7], P[6], P[5], P[4], P[3], P[2], P[1], P[0]);
		pbox(P);
		//printf("\n  R After : %02X %02X %02X %02X, %02X %02X %02X %02X", P[7], P[6], P[5], P[4], P[3], P[2], P[1], P[0]);
		keyadd(P, RK + (i * 8));
		//printf("\n  K Add: %02X %02X %02X %02X, %02X %02X %02X %02X", P[7], P[6], P[5], P[4], P[3], P[2], P[1], P[0]);
#ifdef DEBUG
		printf("\nROUND %02i: %02X%02X%02X%02X, %02X%02X%02X%02X", i, P[7], P[6], P[5], P[4], P[3], P[2], P[1], P[0]);
#endif
	}
}

void DEC(u32* CIPHER_TEXT, u32* ROUND_KEY, u32* PLAIN_TEXT) {
	int i = 0;
	u8* C = (u8*)CIPHER_TEXT;
	u8* RK = (u8*)ROUND_KEY;
	
	for (i = ROUND; i > 0; i--)
	{
		keyadd(C, RK + (i * 8));
		inv_pbox(C);
		inv_sbox(C);
#ifdef DEBUG
		printf("\nROUND %02i: %02X%02X%02X%02X, %02X%02X%02X%02X", i, C[7], C[6], C[5], C[4], C[3], C[2], C[1], C[0]);
#endif
	}
	keyadd(C, RK);
}

void ROUND_KEY_GEN() {
	u32 i, j;
	u32 RCON = 0;
	srand(time(NULL));
	
	printf("==PLAIN_TEXT==\n");
	for (i = 0; i<SIZE; i++) 
		PLAIN_TEXT[i] = rand() | (rand() << 16);
	
	//PIPO-64/128,PIPO-64/256 test vector
	PLAIN_TEXT[0] = 0x1E270026;
	PLAIN_TEXT[1] = 0x098552F6;
		
	for (i = SIZE; i>0; i--)
		printf("0x%08X, \t", PLAIN_TEXT[i-1]);	
	
	printf("\n==MASTER_KEY==\n");
	for (i = 0; i < MASTER_KEY_SIZE; i++) 
		for (j = 0; j < INT_NUM; j++) 
			MASTER_KEY[INT_NUM*i + j] = rand() | (rand() << 16);	
	//PIPO-64/128 test vector
	MASTER_KEY[0] = 0x2E152297;
	MASTER_KEY[1] = 0x7E1D20AD;
	MASTER_KEY[2] = 0x779428D2;
	MASTER_KEY[3] = 0x6DC416DD;


	////PIPO-64/256 test vector
	//MASTER_KEY[7] = 0x34386a09;
	//MASTER_KEY[6] = 0x43116e68;
	//MASTER_KEY[5] = 0x25c471ff;
	//MASTER_KEY[4] = 0x72e5709c;
	//MASTER_KEY[3] = 0x6dc416dd;
	//MASTER_KEY[2] = 0x779428d2;
	//MASTER_KEY[1] = 0x7e1d20ad;
	//MASTER_KEY[0] = 0x2e152297;

	for (i = MASTER_KEY_SIZE; i >0; i--) {
		for (j = INT_NUM; j >0; j--) {
			printf("0x%08X, \t", MASTER_KEY[INT_NUM*(i-1) + (j-1)]);
		}
	}

	printf("\n==ROUND_KEY==\n");
	for (i = 0; i < ROUND + 1; i++) {
		for (j = 0; j < INT_NUM; j++) 
			ROUND_KEY[INT_NUM*i + j] = MASTER_KEY[(INT_NUM*i + j) % (MASTER_KEY_SIZE*INT_NUM)];		
		ROUND_KEY[INT_NUM*i] ^= RCON;
		RCON++;
		for (j = INT_NUM; j >0; j--)
			printf("0x%08X, \t", ROUND_KEY[INT_NUM*i + (j-1)]);
	
		printf("\n");
	}	
}

int main(void) {

	ROUND_KEY_GEN();

	ENC(PLAIN_TEXT, ROUND_KEY, CIPHER_TEXT);
	
	printf("\n\n==CIPHER_TEXT==\n");
	printf("%08X, %08X\n", PLAIN_TEXT[1], PLAIN_TEXT[0]);
	
	DEC(PLAIN_TEXT, ROUND_KEY, CIPHER_TEXT);
	
	printf("\n==PLAIN_TEXT==\n");
	printf("%08X, %08X\n", PLAIN_TEXT[1], PLAIN_TEXT[0]);

	return EXIT_SUCCESS;
}
