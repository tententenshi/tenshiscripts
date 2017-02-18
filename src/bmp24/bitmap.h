#ifndef __BITMAP_H_INCLUDED__
#define __BITMAP_H_INCLUDED__

#define FILEHEADERSIZE 14					//�ե�����إå��Υ�����
#define INFOHEADERSIZE 40					//����إå��Υ�����
#define HEADERSIZE (FILEHEADERSIZE+INFOHEADERSIZE)

typedef struct{
	unsigned char b;
	unsigned char g;
	unsigned char r;
}Rgb;

typedef struct{
	unsigned int h;
	unsigned char s;
	unsigned char v;
}Hsv;

typedef struct{
	unsigned int height;
	unsigned int width;
	Rgb *rgb;
	Hsv *hsv;
}Image;

//��������������Хݥ��󥿤��Ԥ����NULL���֤�
Image *Read_Bmp(char *filename);

//�񤭹��ߤ����������0���Ԥ����1���֤�
int Write_Bmp(char *filename, Image *img);

//Image���������RGB�����width*heightʬ����ưŪ�˼������롣
//��������Хݥ��󥿤򡢼��Ԥ����NULL���֤�
Image *Create_Image(int width, int height);
//Image���ˡ����
void Free_Image(Image *img);

//RGB���줾����ͤǤ��줾��Υ��졼������������
void RGBDivision(Image *colorimg, Image *Rimg, Image *Gimg, Image *Bimg);

//3�Ĥ�Bitmap��������1�ĤΥ��顼���������
void RGBCombination(Image *Rimg, Image *Gimg, Image *Bimg, Image *colorimg);

//���Ȼ�ˡ
void Errordiffusion(Image *img);
#endif /*__BITMAP_H_INCLUDED__*/
