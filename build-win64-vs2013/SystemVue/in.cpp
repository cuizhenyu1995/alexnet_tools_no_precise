#include "in.h"
#include "SystemVue/ModelBuilder.h"
#include "SystemVue/CircularBuffer.h"
#include "SystemVue/Matrix.h"
#include "SystemVue/MatrixCircularBuffer.h"
#include "SystemVue/DFParam.h"

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <malloc.h>
#define DATA_SIDE_LENGTH 227
#define FILE_LENGTH_MAX 255
#define CONST_BIASE_R 124
#define CONST_BIASE_G 117
#define CONST_BIASE_B 104

#define WIDTHBYTES(bits) (((bits)+31)/32*4)//����ʹͼ������ռ�ֽ���Ϊ4byte�ı���
#define MYDRAW_HEIGHT 227  //Ŀ��ͼ��߶�
#define MYDRAW_WIDTH 227  //Ŀ��ͼ����
#define DATA_WEIGHT 3

using namespace std;
typedef unsigned long       DWORD;
typedef int                 BOOL;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef long                LONG;

typedef struct tagBITMAPFILEHEADER {
	DWORD  bfSize;          //�ļ���С
	WORD   bfReserved1;     //�����֣�������
	WORD   bfReserved2;     //�����֣�ͬ��
	DWORD  bfOffBits;       //ʵ��λͼ���ݵ�ƫ���ֽ�������ǰ�������ֳ���֮��
} BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER {
	//public:
	DWORD   biSize;             //ָ���˽ṹ��ĳ��ȣ�Ϊ40
	LONG    biWidth;            //λͼ��
	LONG    biHeight;           //λͼ��
	WORD    biPlanes;           //ƽ������Ϊ1
	WORD    biBitCount;         //������ɫλ����������1��2��4��8��16��24���µĿ�����32
	DWORD   biCompression;      //ѹ����ʽ��������0��1��2������0��ʾ��ѹ��
	DWORD   biSizeImage;        //ʵ��λͼ����ռ�õ��ֽ���
	LONG    biXPelsPerMeter;    //X����ֱ���
	LONG    biYPelsPerMeter;    //Y����ֱ���
	DWORD   biClrUsed;          //ʹ�õ���ɫ�������Ϊ0�����ʾĬ��ֵ(2^��ɫλ��)
	DWORD   biClrImportant;     //��Ҫ��ɫ�������Ϊ0�����ʾ������ɫ������Ҫ��
} BITMAPINFOHEADER;

BITMAPINFOHEADER bih;
BYTE *Buffer = NULL;
long LineByteWidth;

namespace SystemVueModelBuilder {

#ifndef SV_CODE_MRA_IN
	DEFINE_MODEL_INTERFACE(in)
	{
		SET_MODEL_NAME("in");
		SET_MODEL_DESCRIPTION("");
		SET_MODEL_CATEGORY("in");
		ADD_MODEL_HEADER_FILE("in.h");
		model.SetModelCodeGenName("in");
		model.SetModelNamespace("SystemVueModelBuilder");

		// Add parameters
		SystemVueModelBuilder::DFParam param = NULL;

		param = ADD_MODEL_PARAM(INPUT_FILE);
		param.SetParamAsFile();

		// Add input/output ports
		DFPort OUT_DATA = ADD_MODEL_OUTPUT(OUT_D);
		OUT_DATA.SetDescription("Output DATA");
		DFPort OUT_NUM = ADD_MODEL_OUTPUT(OUT_N);
		OUT_NUM.SetDescription("Output NUMBER");

		return true;
	}
#endif

	in::in()
	{
	}

	in::~in()
	{
	}

	bool in::Setup()
	{
		//setRate��������Ե���ÿһ���˿ڣ�����˿ں�����˿ڶ�Ҫ�������ã������õĻ�Ĭ��Ϊ1
		//�˿ڿ�����Ϊ��һ��Buffer, ���ε�����˿ڻ´�����ε�����˿ڵ����ݣ��ÿ���Ϊֵ��������˵�����
		OUT_N.SetRate(1);
		OUT_D.SetRate(1);

		return true;
	};

	bool in::Initialize()
	{
		return true;
	}

	//Run�������ռ�Ŀ�ľ��ǲ������ݣ�����ֵ����Ӧ�Ķ˿�
	bool in::Run()
	{
		
		//����ӿڳ�ʼ��
		OUT_d.Resize(1, DATA_WEIGHT*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH);
		OUT_d.Zero();


		//��ȡbmp�ļ�
		//if (ReadBmp(INPUT_FILE) == 0)
		//{
		//	std::stringstream sut;
		//	sut << "Failed when reading data!" << endl;
		//	POST_INFO(sut.str().c_str());
		//    return true;
		//}

		//if (bih.biWidth != DATA_SIDE_LENGTH || bih.biHeight != DATA_SIDE_LENGTH)
		{
			BITMAPFILEHEADER bitHead, writebitHead;
			BITMAPINFOHEADER bitInfoHead, writebitInfoHead;
			FILE* pfile;//�����ļ�
			FILE* wfile;//����ļ�

			//char strFile[FILE_LENGTH_MAX] = INPUT_FILE;//��ͼ��·����BMPͼ�����Ϊ24λ���ɫ��ʽ
			char strFile[FILE_LENGTH_MAX];
			int leng = strlen(INPUT_FILE);
			for (int i = 0; i < leng; i++) strFile[i] = INPUT_FILE[i];
			strFile[leng] = '\0';
			char strFilesave[FILE_LENGTH_MAX];//�����ͼ��洢·��
			strcpy(strFilesave, strFile);
			strFilesave[leng - 4] = '_';
			strFilesave[leng - 3] = '.';
			strFilesave[leng - 2] = 'b';
			strFilesave[leng - 1] = 'm';
			strFilesave[leng - 0] = 'p';
			strFilesave[leng + 1] = '\0';



			pfile = fopen(strFile, "rb");
			//fopen_s(&pfile, strFile, "rb");//�ļ���ͼ��
			wfile = fopen(strFilesave, "wb");
			//fopen_s(&wfile, strFilesave, "wb");//���ļ�Ϊ�洢�޸ĺ�ͼ����׼��
			//��ȡλͼ�ļ�ͷ��Ϣ.

			//std::stringstream sut;
			//sut << "JINAAAAAAAAAAAAAAAAA!" << endl;
			//POST_INFO(sut.str().c_str());

			WORD fileType;
			fread(&fileType, 1, sizeof(WORD), pfile);
			fwrite(&fileType, 1, sizeof(WORD), wfile);
			//    if (fileType != 0x4d42)
			//    {
			//        printf("file is not .bmp file!");
			//        return 0;
			//    }
			//std::stringstream sgt;
			//sgt << "JINBBBBBBBBBBBBBBBBB!" << endl;
			//POST_INFO(sgt.str().c_str());
			//��ȡλͼ�ļ�ͷ��Ϣ
			fread(&bitHead, 1, sizeof(tagBITMAPFILEHEADER), pfile);
			writebitHead = bitHead;//���ڽ�ȡͼ��ͷ��Դ�ļ�ͷ���ƣ������Ƚ�Դ�ļ�ͷ���ݸ����ȡ�ļ�ͷ
			//��ȡλͼ��Ϣͷ��Ϣ
			fread(&bitInfoHead, 1, sizeof(BITMAPINFOHEADER), pfile);
			writebitInfoHead = bitInfoHead;//ͬλͼ�ļ�ͷ����

			writebitInfoHead.biHeight = MYDRAW_HEIGHT;//Ϊ��ȡ�ļ���дλͼ�߶�
			writebitInfoHead.biWidth = MYDRAW_WIDTH;//Ϊ��ȡ�ļ���дλͼ���
			int mywritewidth = WIDTHBYTES(writebitInfoHead.biWidth*writebitInfoHead.biBitCount);//BMPͼ��ʵ��λͼ�������Ŀ��Ϊ4byte�ı������ڴ˼���ʵ�����������
			writebitInfoHead.biSizeImage = mywritewidth*writebitInfoHead.biHeight;//����λͼʵ����������С

			writebitHead.bfSize = 54 + writebitInfoHead.biSizeImage;//λͼ�ļ�ͷ��СΪλͼ��������С����54byte
			fwrite(&writebitHead, 1, sizeof(tagBITMAPFILEHEADER), wfile);//д��λͼ�ļ�ͷ��Ϣ������ļ�
			fwrite(&writebitInfoHead, 1, sizeof(BITMAPINFOHEADER), wfile);//д��λͼ��Ϣͷ��Ϣ������ļ�

			int width = bitInfoHead.biWidth;
			int height = bitInfoHead.biHeight;
			//�����ڴ�ռ��Դͼ�����ڴ�
			int l_width = WIDTHBYTES(width*bitInfoHead.biBitCount);//����λͼ��ʵ�ʿ�Ȳ�ȷ����Ϊ4byte�ı���
			int write_width = WIDTHBYTES(writebitInfoHead.biWidth*writebitInfoHead.biBitCount);//����дλͼ��ʵ�ʿ�Ȳ�ȷ����Ϊ4byte�ı���

			BYTE    *pColorData = (BYTE *)malloc(height*l_width);//�����ڴ�ռ�洢ͼ������
			memset(pColorData, 0, height*l_width);

			BYTE    *pColorDataMid = (BYTE *)malloc(mywritewidth*MYDRAW_HEIGHT);//�����ڴ�ռ�洢ͼ����֮������
			memset(pColorDataMid, 0, mywritewidth*MYDRAW_HEIGHT);

			long nData = height*l_width;
			long write_nData = mywritewidth*MYDRAW_HEIGHT;//��ȡ��λͼ���������ȶ���

			//��λͼ������Ϣ����������
			fread(pColorData, 1, nData, pfile);//ͼ�����ͨ�������ⲿ�����ݼ���ʵ��


			//˫���Բ�ֵ

			for (int hnum = 0; hnum < MYDRAW_HEIGHT; hnum++)
				for (int wnum = 0; wnum < MYDRAW_WIDTH; wnum++)
				{
					double d_original_img_hnum = hnum*height / (double)MYDRAW_HEIGHT;
					double d_original_img_wnum = wnum*width / (double)MYDRAW_WIDTH;
					int i_original_img_hnum = d_original_img_hnum;
					int i_original_img_wnum = d_original_img_wnum;
					double distance_to_a_x = d_original_img_wnum - i_original_img_wnum;//��ԭͼ������a���ˮƽ����
					double distance_to_a_y = d_original_img_hnum - i_original_img_hnum;//��ԭͼ������a��Ĵ�ֱ����

					int original_point_a = i_original_img_hnum*l_width + i_original_img_wnum * 3;//����λ��ƫ��������Ӧ��ͼ��ĸ����ص�RGB�����,�൱�ڵ�A
					int original_point_b = i_original_img_hnum*l_width + (i_original_img_wnum + 1) * 3;//����λ��ƫ��������Ӧ��ͼ��ĸ����ص�RGB�����,�൱�ڵ�B
					int original_point_c = (i_original_img_hnum + 1)*l_width + i_original_img_wnum * 3;//����λ��ƫ��������Ӧ��ͼ��ĸ����ص�RGB�����,�൱�ڵ�C
					int original_point_d = (i_original_img_hnum + 1)*l_width + (i_original_img_wnum + 1) * 3;//����λ��ƫ��������Ӧ��ͼ��ĸ����ص�RGB�����,�൱�ڵ�D
					if (i_original_img_hnum + 1 == MYDRAW_HEIGHT - 1)
					{
						original_point_c = original_point_a;
						original_point_d = original_point_b;
					}
					if (i_original_img_wnum + 1 == MYDRAW_WIDTH - 1)
					{
						original_point_b = original_point_a;
						original_point_d = original_point_c;
					}

					int pixel_point = hnum*write_width + wnum * 3;//ӳ��߶ȱ任ͼ������λ��ƫ����
					pColorDataMid[pixel_point] =
						pColorData[original_point_a] * (1 - distance_to_a_x)*(1 - distance_to_a_y) +
						pColorData[original_point_b] * distance_to_a_x*(1 - distance_to_a_y) +
						pColorData[original_point_c] * distance_to_a_y*(1 - distance_to_a_x) +
						pColorData[original_point_c] * distance_to_a_y*distance_to_a_x;
					pColorDataMid[pixel_point + 1] =
						pColorData[original_point_a + 1] * (1 - distance_to_a_x)*(1 - distance_to_a_y) +
						pColorData[original_point_b + 1] * distance_to_a_x*(1 - distance_to_a_y) +
						pColorData[original_point_c + 1] * distance_to_a_y*(1 - distance_to_a_x) +
						pColorData[original_point_c + 1] * distance_to_a_y*distance_to_a_x;
					pColorDataMid[pixel_point + 2] =
						pColorData[original_point_a + 2] * (1 - distance_to_a_x)*(1 - distance_to_a_y) +
						pColorData[original_point_b + 2] * distance_to_a_x*(1 - distance_to_a_y) +
						pColorData[original_point_c + 2] * distance_to_a_y*(1 - distance_to_a_x) +
						pColorData[original_point_c + 2] * distance_to_a_y*distance_to_a_x;

				}
			//˫���Բ�ֵ

			fwrite(pColorDataMid, 1, write_nData, wfile);   //��������ͼ��������д���ļ�
			fclose(pfile);
			fclose(wfile);

			strcpy(INPUT_FILE, strFilesave);

			fclose(pfile);
			fclose(wfile);
		}

		if (ReadBmp(INPUT_FILE) == 0)
		{
			std::stringstream sut;
			sut << "Failed when reading data!" << endl;
			POST_INFO(sut.str().c_str());
			return true;
		}
		//std::stringstream sutk;
		//sutk << "w=" << bih.biWidth<< " h="<<bih.biHeight << endl;
		//POST_INFO(sutk.str().c_str());
		//��ȡbmp����&����matrix----------------------------------------------------------------
		BYTE r, g, b;
		for (int x = 0; x<DATA_SIDE_LENGTH; x++)
		{
			for (int y = 0; y<DATA_SIDE_LENGTH; y++)
			{
				GetDIBColor(y, x, &r, &g, &b);
				OUT_d(0, 0 * DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + x*DATA_SIDE_LENGTH + y) = (double)b - CONST_BIASE_B;
				OUT_d(0, 1 * DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + x*DATA_SIDE_LENGTH + y) = (double)g - CONST_BIASE_G;
				OUT_d(0, 2 * DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + x*DATA_SIDE_LENGTH + y) = (double)r - CONST_BIASE_R;
			}
		}
		//for (int i = 0; i < 20; i++)
		//{
		//	std::stringstream rrr;
		//	rrr << OUT_b(0, i) - 104 << " " << OUT_g(0, i) - 117 << " " << OUT_r(0, i) - 124 << "  no." << i << endl;
		//	POST_INFO(rrr.str().c_str());
		//}
		//���������
		OUT_N[0] = DATA_WEIGHT;
		OUT_D[0] = OUT_d;

		//�ͷſռ�
		free(Buffer);


		return true;
	}


	bool in::Finalize() {

		return true;
	}
	//��ȡbmp�ļ�
	int in::ReadBmp(const char* szFileName){
		FILE *file;
		WORD bfh[7];
		long dpixeladd;
		if (NULL == (file = fopen(szFileName, "rb")))
		{
			fclose(file);
			return 0;
		}

		fread(&bfh, sizeof(WORD), 7, file);
		if (bfh[0] != (WORD)(((WORD)'B') | ('M' << 8)))
		{
			fclose(file);
			return 0;
		}
		fread(&bih, sizeof(BITMAPINFOHEADER), 1, file);
		if (bih.biBitCount < 24)
		{
			fclose(file);
			return 0;
		}
		dpixeladd = bih.biBitCount / 8;
		LineByteWidth = bih.biWidth * (dpixeladd);
		if ((LineByteWidth % 4) != 0)
			LineByteWidth += 4 - (LineByteWidth % 4);
		if ((Buffer = (BYTE*)malloc(sizeof(BYTE)* LineByteWidth * bih.biHeight)) != NULL)
		{
			fread(Buffer, LineByteWidth * bih.biHeight, 1, file);
			fclose(file);
			return 1;
		}
		fclose(file);
		return 0;
	}

	//��ȡbmp����
	int in::GetDIBColor(int X, int Y, BYTE *r, BYTE *g, BYTE *b){
		int dpixeladd;
		BYTE *ptr;
		if (X < 0 || X >= bih.biWidth || Y < 0 || Y >= bih.biHeight)
		{
			return 0;
		}
		dpixeladd = bih.biBitCount / 8;
		ptr = Buffer + X * dpixeladd + (bih.biHeight - 1 - Y) * LineByteWidth;
		*b = *ptr;
		*g = *(ptr + 1);
		*r = *(ptr + 2);
		return 1;
	}
}