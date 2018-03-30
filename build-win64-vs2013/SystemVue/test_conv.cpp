#pragma once
#include "test_conv.h"
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
#include <iomanip>

using namespace std;
#include <iostream>
#include <cstdio>
#include <algorithm>
#include <cmath>

using std::sort;
using std::fabs;

#define FILE_LENGTH_MAX 255
#define NUMBER_LENGTH_MAX 10
#define MODEL_WEIGHT 3
#define MODEL_GROUP 3
#define DATA_SIDE_LENGTH 227
#define RESULT_SIDE_LENGTH 114
#define POOLING_STEP_LENGTH 2
#define POOLING_RADIUS 2

char tpre1[] = "weights";
char tbiase1[] = "biases";
char tnum1[NUMBER_LENGTH_MAX];
char tlast1[] = ".txt";
double *tmodel1;
double *tres1;
double *tc1_data1;


namespace SystemVueModelBuilder {

#ifndef SV_CODE_MRA_RUN
	DEFINE_MODEL_INTERFACE(test_conv)
	{
		SET_MODEL_NAME("test_conv");
		SET_MODEL_DESCRIPTION("");
		SET_MODEL_CATEGORY("alexnet_simple_conv");
		ADD_MODEL_HEADER_FILE("test_conv.h");
		model.SetModelCodeGenName("test_conv");
		model.SetModelNamespace("SystemVueModelBuilder");

		// Add parameters
		SystemVueModelBuilder::DFParam param = NULL;
		param = ADD_MODEL_PARAM(INPUT_FILE);
		param = ADD_MODEL_PARAM(LENGTH_OF_SIDE);
		param.SetDefaultValue("11");

		// Add input/output ports

		DFPort IN_DATA = ADD_MODEL_INPUT(IN_D);
		DFPort IN_NUM = ADD_MODEL_INPUT(IN_N);

		DFPort OUT_DATA = ADD_MODEL_OUTPUT(OUT_D);
		DFPort OUT_NUM = ADD_MODEL_OUTPUT(OUT_N);



		return true;
	}
#endif

	test_conv::test_conv()
	{
	}

	test_conv::~test_conv()
	{
	}

	bool test_conv::Setup()
	{
		IN_N.SetRate(1);
		IN_D.SetRate(1);

		OUT_N.SetRate(1);
		OUT_D.SetRate(1);

		return true;
	};

	bool test_conv::Initialize()
	{
		return true;
	}
	bool test_conv::Run()
	{

		//为tmodel1申请空间
		int fal = 0;
		tmodel1 = new double[MODEL_WEIGHT*MODEL_GROUP*LENGTH_OF_SIDE*LENGTH_OF_SIDE];
		if (tmodel1 == NULL) fal = 1;
		if (fal)
		{
			std::stringstream st;
			st << "Failed when getting space for tmodel1!" << endl;
			POST_INFO(st.str().c_str());
			return true;
		}

		//读取tmodel1----------------------------------------------------------------------------
		for (int i = 0; i < LENGTH_OF_SIDE*LENGTH_OF_SIDE; i++)
		{
			char s[FILE_LENGTH_MAX] = "";
			strcat(s, INPUT_FILE);
			strcat(s, tpre1);
			mych(i + 1);
			strcat(s, tnum1);
			strcat(s, tlast1);

			//打开tmodel1文件
			FILE *mf = fopen(s, "r");
			if (mf == NULL)
			{
				std::stringstream st;
				st << "Failed when opening tmodel1 flie!" << endl;
				POST_INFO(st.str().c_str());
				return true;
			}
			//读取tmodel1数据
			for (int j = 0; j < MODEL_WEIGHT; j++)
			{
				for (int k = 0; k < MODEL_GROUP; k++)
				{
					fscanf(mf, "%lf", &tmodel1[j*MODEL_GROUP*LENGTH_OF_SIDE*LENGTH_OF_SIDE + k*LENGTH_OF_SIDE*LENGTH_OF_SIDE + i]);

				}
			}
			fclose(mf);
		}

		//为tres1申请空间
		fal = 0;
		tres1 = new double[MODEL_GROUP * DATA_SIDE_LENGTH * DATA_SIDE_LENGTH];
		if (tres1 == NULL) fal = 1;
		if (fal)
		{
			std::stringstream st;
			st << "Failed when getting space for tres1ult!" << endl;
			POST_INFO(st.str().c_str());
			return true;
		}
		for (int i = 0; i < MODEL_GROUP * DATA_SIDE_LENGTH * DATA_SIDE_LENGTH; i++) tres1[i] = 0;

		//为data申请空间
		int DATA_WEIGHT = IN_N[0];

		fal = 0;
		tc1_data1 = new double[DATA_WEIGHT * DATA_SIDE_LENGTH * DATA_SIDE_LENGTH];
		if (tc1_data1 == NULL) fal = 1;
		if (fal)
		{
			std::stringstream st;
			st << "Failed when getting space for data!" << endl;
			POST_INFO(st.str().c_str());
			return true;
		}

		//获取数据-----------------------------------------------------------------------------
		for (int i = 0; i < DATA_SIDE_LENGTH; i++)
			for (int j = 0; j < DATA_SIDE_LENGTH; j++)
				for (int k = 0; k < DATA_WEIGHT; k++)
					tc1_data1[k*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + i*DATA_SIDE_LENGTH + j] = IN_D[0](0, k*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + i*DATA_SIDE_LENGTH + j);


		//卷积操作-----------------------------------------------------------------------------
		for (int i = 0; i < MODEL_GROUP; i++)//group
		{
			for (int j = 0; j < MODEL_WEIGHT; j++)//weight
			{

				//virtual int gettres1(double f[], double **mm, double **tres1, int mph, int mpl, int R);
				int mph = DATA_SIDE_LENGTH, mpl = DATA_SIDE_LENGTH, R = LENGTH_OF_SIDE;

				for (int ii = 0; ii<mph; ii++)
				{
					for (int jj = 0; jj<mpl; jj++)
					{
						int ll = jj - R / 2, rr = jj + R / 2;
						int nn = ii - R / 2, ss = ii + R / 2;

						int tp = 0;
						double tmp = 0;
						for (int k = nn; k <= ss; k++)
						{
							for (int l = ll; l <= rr; l++)
							{
								if (k >= 0 && k<mph && l >= 0 && l<mpl)
								{
									tmp += tmodel1[j*MODEL_GROUP*LENGTH_OF_SIDE*LENGTH_OF_SIDE + i*LENGTH_OF_SIDE*LENGTH_OF_SIDE + tp] * tc1_data1[j*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + k*DATA_SIDE_LENGTH + l];
								}
								tp++;
							}
						}
						tres1[i*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + ii*DATA_SIDE_LENGTH + jj] += tmp;
					}
				}
				//gettres1(tmodel1[j][i], tc1_data1[j], tres1[i], DATA_SIDE_LENGTH, DATA_SIDE_LENGTH, LENGTH_OF_SIDE)
			}
		}


		//读取tbiase1文件
		char s[FILE_LENGTH_MAX] = "";
		strcat(s, INPUT_FILE);
		strcat(s, tbiase1);
		strcat(s, tlast1);
		FILE *bf = fopen(s, "r");
		if (bf == NULL)
		{
			std::stringstream st;
			st << "Failed when opening tbiase1 file!" << endl;
			POST_INFO(st.str().c_str());
			return true;
		}


		//tbiase1操作---------------------------------------------------------------------
		for (int i = 0; i < MODEL_GROUP; i++)
		{
			double ttt;
			fscanf(bf, "%lf", &ttt);
			for (int ii = 0; ii < DATA_SIDE_LENGTH; ii++)
			{
				for (int jj = 0; jj < DATA_SIDE_LENGTH; jj++)
				{
					tres1[i*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + ii*DATA_SIDE_LENGTH + jj] += ttt;
				}
			}
		}

		//ReLU操作-------------------------------------------------------------------
		for (int i = 0; i < MODEL_GROUP; i++)
		{
			for (int ii = 0; ii < DATA_SIDE_LENGTH; ii++)
			{
				for (int jj = 0; jj < DATA_SIDE_LENGTH; jj++)
				{
					tres1[i*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + ii*DATA_SIDE_LENGTH + jj] = tres1[i*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + ii*DATA_SIDE_LENGTH + jj] > 0.0 ? tres1[i*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + ii*DATA_SIDE_LENGTH + jj] : 0.0;
				}
			}
		}

		//for (int i = 0; i < 10; i++)
		//{
		//	std::stringstream st;
		//	st <<"no."<<i<<"="<< tres1[0][0][i] << endl;
		//	POST_INFO(st.str().c_str());
		//}

		//max-pooling操作(padding=same)&输出操作--------------------------------------------------
		out.Resize(1, MODEL_GROUP*RESULT_SIDE_LENGTH*RESULT_SIDE_LENGTH);
		out.Zero();
		for (int i = 0; i < MODEL_GROUP; i++)
		{
			for (int ii = 0; ii < DATA_SIDE_LENGTH; ii += POOLING_STEP_LENGTH)
			{
				for (int jj = 0; jj < DATA_SIDE_LENGTH; jj += POOLING_STEP_LENGTH)
				{
					double now = tres1[i*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + ii*DATA_SIDE_LENGTH + jj];
					for (int di = 0; di < POOLING_RADIUS; di++)
					{
						for (int dj = 0; dj < POOLING_RADIUS; dj++)
						{
							if (ii + di < DATA_SIDE_LENGTH&&jj + dj<DATA_SIDE_LENGTH) now = now > tres1[i*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + (ii + di)*DATA_SIDE_LENGTH + (jj + dj)] ? now : tres1[i*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + (ii + di)*DATA_SIDE_LENGTH + (jj + dj)];
							else now = max(now, 0.0);
						}
					}

					out(0, i*RESULT_SIDE_LENGTH*RESULT_SIDE_LENGTH + ii / POOLING_STEP_LENGTH*RESULT_SIDE_LENGTH + jj / POOLING_STEP_LENGTH) = now;
				}
			}
		}

		FILE* me = fopen("D:\\test_conv.txt", "w+");
		for (int i = 0; i < MODEL_GROUP; i++)
		{
			for (int j = 0; j < RESULT_SIDE_LENGTH; j++)
			{
				for (int k = 0; k < RESULT_SIDE_LENGTH; k++)
				{
					fprintf(me, "%.8f ", out(0, i*RESULT_SIDE_LENGTH*RESULT_SIDE_LENGTH + j*RESULT_SIDE_LENGTH + k));
				}
				fprintf(me, "\n");
			}
			fprintf(me, "\n");
		}
		fclose(me);

		//输出到缓冲区
		OUT_D[0] = out;
		OUT_N[0] = MODEL_GROUP;


		delete[] tmodel1;//new和delete是成对出现的，这里也要多重循环释放掉空间
		tmodel1 = NULL;
		delete[] tres1;//new和delete是成对出现的，这里也要多重循环释放掉空间
		tres1 = NULL;
		delete[] tc1_data1;//new和delete是成对出现的，这里也要多重循环释放掉空间
		tc1_data1 = NULL;

		std::stringstream stag;
		stag << "converting finished!" << endl;
		POST_INFO(stag.str().c_str());
		fclose(bf);
		return true;
	}

	bool test_conv::Finalize()
	{

		return true;
	}

	void test_conv::mych(int tmp)
	{
		memset(tnum1, '\0', sizeof(tnum1));
		int tp = 0;
		while (tmp)
		{
			tnum1[tp++] = '0' + (tmp % 10);
			tmp /= 10;
		}
		for (int i = 0; i<tp / 2; i++)
		{
			swap(tnum1[i], tnum1[tp - 1 - i]);
		}
		tnum1[tp++] = '\0';
	}

}