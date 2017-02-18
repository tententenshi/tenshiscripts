#ifndef __BITMAP_H_INCLUDED__
#define __BITMAP_H_INCLUDED__

#define FILEHEADERSIZE 14					//ファイルヘッダのサイズ
#define INFOHEADERSIZE 40					//情報ヘッダのサイズ
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

//取得に成功すればポインタを失敗すればNULLを返す
Image *Read_Bmp(char *filename);

//書き込みに成功すれば0を失敗すれば1を返す
int Write_Bmp(char *filename, Image *img);

//Imageを作成し、RGB情報もwidth*height分だけ動的に取得する。
//成功すればポインタを、失敗すればNULLを返す
Image *Create_Image(int width, int height);
//Imageを解法する
void Free_Image(Image *img);

//RGBそれぞれの値でそれぞれのグレースケールを作成
void RGBDivision(Image *colorimg, Image *Rimg, Image *Gimg, Image *Bimg);

//3つのBitmap画像から1つのカラー画像を作成
void RGBCombination(Image *Rimg, Image *Gimg, Image *Bimg, Image *colorimg);

//誤差拡散法
void Errordiffusion(Image *img);
#endif /*__BITMAP_H_INCLUDED__*/
