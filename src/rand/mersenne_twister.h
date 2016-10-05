/*==============================================================
 *
 * mersenne_twister.h	random value generator
 *
 *==============================================================
 */
#ifndef _mersenne_twister_h_
#define _mersenne_twister_h_

#ifdef __cplusplus
extern "C" {
#endif

void init_genrand(unsigned long s);
void init_by_array(unsigned long init_key[], int key_length);

/* generates a random number on [0,0xffffffff]-interval */
unsigned long genrand_int32(void);

/* generates a random number on [0,0x7fffffff]-interval */
long genrand_int31(void);

/* generates a random number on [0,1]-real-interval */
double genrand_real1(void);

/* generates a random number on [0,1)-real-interval */
double genrand_real2(void);

/* generates a random number on (0,1)-real-interval */
double genrand_real3(void);

/* generates a random number on [0,1) with 53-bit resolution*/
double genrand_res53(void);

#ifdef __cplusplus
}
#endif

#endif	/* _mersenne_twister_h_ */
