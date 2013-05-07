/* MD5.H - MD5C.Cのためのヘッダファイル
 */

/* Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.

もしそれそれが、このソフトウェアまたはこの機能に言及するか、または参照
を付けているすべての素材の「RSA Data Security, Inc. MD5 メッセージダイ
ジェストアルゴリズム」と確認されるならば、このソフトウェアをコピーし、
用いるためにライセンスは与えられる。

ライセンスは、同じく得られたワークに言及するか、もしくは参照を付け
る材料全てにおいて「RSA Data Security, Inc. MD5 メッセージダイジェスト
アルゴリズムから得られた」ように、そのような作品が確認される限り、派生
した仕事を行って、使うことを認められる。

RSA Data Security, Inc. は、どのような特定の目的のためにでも、このソフ
トウェアの市販性またはこのソフトウェアの適合性についての説明は行なわな
い。それは、あらゆる表現や暗黙の保証なしで "ありのまま" 供給される。

これらの通知は、このドキュメンテーションおよび／またはソフトウェアのど
のような部分のどのようなコピーにおいても保たれねばならない。
*/

/* MD5 前後関係. */
typedef struct 
{
	
	UINT4 state[4];
	/* state (ABCD) */
	UINT4 count[2];
	/* ビット数、モジュロ2＾64 (lsb first) */
	unsigned char buffer[64];
	/* input buffer */
}
MD5_CTX;


void MD5Init PROTO_LIST ((MD5_CTX *));

void MD5Update PROTO_LIST
((MD5_CTX *, unsigned char *, unsigned int));

void MD5Final PROTO_LIST ((unsigned char [16], MD5_CTX *));

