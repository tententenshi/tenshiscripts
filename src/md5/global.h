/* GLOBAL.H - RSAREFタイプと定数
 */

/* もしコンパイラがファンクション引数プロトタイピングをサポートするな
  ら、PROTOTYPES はそれに設定されるべきである。
それがCコンパイラフラグによってまだ定義されていなかったならば、デフォ
ルトで PROTOTYPES は 0 になる。
*/
#ifndef PROTOTYPES
#define PROTOTYPES 0
#endif

/* POINTERは一般的なポインタタイプを定義する */
typedef unsigned char *POINTER;


/* UINT2 は2バイトワードを定義する */
typedef unsigned short int UINT2;


/* UINT4 は4バイトワードを定義する */
typedef unsigned long int UINT4;


/* PROTO_LIST は、PROTOTYPES の定義上に応じて定義される。
PROTOTYPES を使っていれば、PROTO_LIST はリストを返す、さもなくば、
それは空のリストを返す。
*/
#if PROTOTYPES
#define PROTO_LIST(list) list
#else
#define PROTO_LIST(list) ()
#endif
