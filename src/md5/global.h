/* GLOBAL.H - RSAREF�����פ����
 */

/* �⤷����ѥ��餬�ե��󥯥��������ץ�ȥ����ԥ󥰤򥵥ݡ��Ȥ����
  �顢PROTOTYPES �Ϥ�������ꤵ���٤��Ǥ��롣
���줬C����ѥ���ե饰�ˤ�äƤޤ��������Ƥ��ʤ��ä��ʤ�С��ǥե�
��Ȥ� PROTOTYPES �� 0 �ˤʤ롣
*/
#ifndef PROTOTYPES
#define PROTOTYPES 0
#endif

/* POINTER�ϰ���Ū�ʥݥ��󥿥����פ�������� */
typedef unsigned char *POINTER;


/* UINT2 ��2�Х��ȥ�ɤ�������� */
typedef unsigned short int UINT2;


/* UINT4 ��4�Х��ȥ�ɤ�������� */
typedef unsigned long int UINT4;


/* PROTO_LIST �ϡ�PROTOTYPES �������˱������������롣
PROTOTYPES ��ȤäƤ���С�PROTO_LIST �ϥꥹ�Ȥ��֤�������ʤ��С�
����϶��Υꥹ�Ȥ��֤���
*/
#if PROTOTYPES
#define PROTO_LIST(list) list
#else
#define PROTO_LIST(list) ()
#endif
