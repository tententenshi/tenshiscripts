/* MD5.H - MD5C.C�Τ���Υإå��ե�����
 */

/* Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.

�⤷���줽�줬�����Υ��եȥ������ޤ��Ϥ��ε�ǽ�˸��ڤ��뤫���ޤ��ϻ���
���դ��Ƥ��뤹�٤Ƥ��Ǻ�Ρ�RSA Data Security, Inc. MD5 ��å���������
�������ȥ��르�ꥺ��פȳ�ǧ�����ʤ�С����Υ��եȥ������򥳥ԡ�����
�Ѥ��뤿��˥饤���󥹤�Ϳ�����롣

�饤���󥹤ϡ�Ʊ��������줿����˸��ڤ��뤫���⤷���ϻ��Ȥ��դ�
��������Ƥˤ����ơ�RSA Data Security, Inc. MD5 ��å�����������������
���르�ꥺ�फ������줿�פ褦�ˡ����Τ褦�ʺ��ʤ���ǧ�����¤ꡢ����
�����Ż���Ԥäơ��Ȥ����Ȥ�ǧ����롣

RSA Data Security, Inc. �ϡ��ɤΤ褦���������Ū�Τ���ˤǤ⡢���Υ���
�ȥ������λ������ޤ��Ϥ��Υ��եȥ�������Ŭ�����ˤĤ��Ƥ������ϹԤʤ��
��������ϡ�������ɽ������ۤ��ݾڤʤ��� "����Τޤ�" ���뤵��롣

���������Τϡ����Υɥ�����ơ�����󤪤�ӡ��ޤ��ϥ��եȥ������Τ�
�Τ褦����ʬ�ΤɤΤ褦�ʥ��ԡ��ˤ����Ƥ��ݤ���ͤФʤ�ʤ���
*/

/* MD5 ����ط�. */
typedef struct 
{
	
	UINT4 state[4];
	/* state (ABCD) */
	UINT4 count[2];
	/* �ӥåȿ����⥸���2��64 (lsb first) */
	unsigned char buffer[64];
	/* input buffer */
}
MD5_CTX;


void MD5Init PROTO_LIST ((MD5_CTX *));

void MD5Update PROTO_LIST
((MD5_CTX *, unsigned char *, unsigned int));

void MD5Final PROTO_LIST ((unsigned char [16], MD5_CTX *));

