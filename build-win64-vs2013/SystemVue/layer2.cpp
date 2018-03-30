#pragma once
#include "layer2.h"
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
#define MODEL_WEIGHT 9
#define MODEL_GROUP 25
#define DATA_SIDE_LENGTH 114
#define RESULT_SIDE_LENGTH 57
#define POOLING_STEP_LENGTH 2
#define POOLING_RADIUS 3

char pre2[] = "weights";
char biase2[] = "biases";
char num2[NUMBER_LENGTH_MAX];
char last2[] = ".txt";
double *model2;
double *res2;
double *c1_data2;


namespace SystemVueModelBuilder {

#ifndef SV_CODE_MRA_RUN
	DEFINE_MODEL_INTERFACE(layer2)
	{
		SET_MODEL_NAME("layer2");
		SET_MODEL_DESCRIPTION("");
		SET_MODEL_CATEGORY("alexnet_simple_conv");
		ADD_MODEL_HEADER_FILE("layer2.h");
		model.SetModelCodeGenName("layer2");
		model.SetModelNamespace("SystemVueModelBuilder");

		// Add parameters
		SystemVueModelBuilder::DFParam param = NULL;
		param = ADD_MODEL_PARAM(INPUT_FILE);
		param = ADD_MODEL_PARAM(LENGTH_OF_SIDE);
		param.SetDefaultValue("3");

		// Add input/output ports

		DFPort IN_DATA = ADD_MODEL_INPUT(IN_D);
		DFPort IN_NUM = ADD_MODEL_INPUT(IN_N);

		DFPort OUT_DATA = ADD_MODEL_OUTPUT(OUT_D);
		DFPort OUT_NUM = ADD_MODEL_OUTPUT(OUT_N);



		return true;
	}
#endif

	layer2::layer2()
	{
	}

	layer2::~layer2()
	{
	}

	bool layer2::Setup()
	{
		IN_N.SetRate(1);
		IN_D.SetRate(1);

		OUT_N.SetRate(1);
		OUT_D.SetRate(1);

		return true;
	};

	bool layer2::Initialize()
	{
		return true;
	}
	bool layer2::Run()
	{

		//为model2申请空间
		int fal = 0;
		model2 = new double[MODEL_WEIGHT*MODEL_GROUP*LENGTH_OF_SIDE*LENGTH_OF_SIDE];
		if (model2 == NULL) fal = 1;
		if (fal)
		{
			std::stringstream st;
			st << "Failed when getting space for model2!" << endl;
			POST_INFO(st.str().c_str());
			return true;
		}

		//读取model2----------------------------------------------------------------------------
		for (int i = 0; i < LENGTH_OF_SIDE*LENGTH_OF_SIDE; i++)
		{
			char s[FILE_LENGTH_MAX] = "";
			strcat(s, INPUT_FILE);
			strcat(s, pre2);
			mych(i + 1);
			strcat(s, num2);
			strcat(s, last2);

			//打开model2文件
			FILE *mf = fopen(s, "r");
			if (mf == NULL)
			{
				std::stringstream st;
				st << "Failed when opening model2 flie!" << endl;
				POST_INFO(st.str().c_str());
				return true;
			}
			//读取model2数据
			for (int j = 0; j < MODEL_WEIGHT; j++)
			{
				for (int k = 0; k < MODEL_GROUP; k++)
				{
					fscanf(mf, "%lf", &model2[j*MODEL_GROUP*LENGTH_OF_SIDE*LENGTH_OF_SIDE + k*LENGTH_OF_SIDE*LENGTH_OF_SIDE + i]);

				}
			}
			fclose(mf);
		}

		//为res2申请空间
		fal = 0;
		res2 = new double[MODEL_GROUP * DATA_SIDE_LENGTH * DATA_SIDE_LENGTH];
		if (res2 == NULL) fal = 1;
		if (fal)
		{
			std::stringstream st;
			st << "Failed when getting space for res2ult!" << endl;
			POST_INFO(st.str().c_str());
			return true;
		}
		for (int i = 0; i < MODEL_GROUP * DATA_SIDE_LENGTH * DATA_SIDE_LENGTH; i++) res2[i] = 0;

		//为data申请空间
		int DATA_WEIGHT = IN_N[0];

		fal = 0;
		c1_data2 = new double[DATA_WEIGHT * DATA_SIDE_LENGTH * DATA_SIDE_LENGTH];
		if (c1_data2 == NULL) fal = 1;
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
					c1_data2[k*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + i*DATA_SIDE_LENGTH + j] = IN_D[0](0, k*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + i*DATA_SIDE_LENGTH + j);
		

		//卷积操作-----------------------------------------------------------------------------
		for (int i = 0; i < MODEL_GROUP; i++)//group
		{
			for (int j = 0; j < MODEL_WEIGHT; j++)//weight
			{

				//virtual int getres2(double f[], double **mm, double **res2, int mph, int mpl, int R);
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
									tmp += model2[j*MODEL_GROUP*LENGTH_OF_SIDE*LENGTH_OF_SIDE + i*LENGTH_OF_SIDE*LENGTH_OF_SIDE + tp] * c1_data2[j*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + k*DATA_SIDE_LENGTH + l];
								}
								tp++;
							}
						}
						res2[i*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + ii*DATA_SIDE_LENGTH + jj] += tmp;
					}
				}
				//getres2(model2[j][i], c1_data2[j], res2[i], DATA_SIDE_LENGTH, DATA_SIDE_LENGTH, LENGTH_OF_SIDE)
			}
		}
		

		//读取biase2文件
		char s[FILE_LENGTH_MAX] = "";
		strcat(s, INPUT_FILE);
		strcat(s, biase2);
		strcat(s, last2);
		FILE *bf = fopen(s, "r");
		if (bf == NULL)
		{
			std::stringstream st;
			st << "Failed when opening biase2 file!" << endl;
			POST_INFO(st.str().c_str());
			return true;
		}

		//biase2操作---------------------------------------------------------------------
		for (int i = 0; i < MODEL_GROUP; i++)
		{
			double ttt;
			fscanf(bf, "%lf", &ttt);
			for (int ii = 0; ii < DATA_SIDE_LENGTH; ii++)
			{
				for (int jj = 0; jj < DATA_SIDE_LENGTH; jj++)
				{
					res2[i*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + ii*DATA_SIDE_LENGTH + jj] += ttt;
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
					res2[i*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + ii*DATA_SIDE_LENGTH + jj] = res2[i*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + ii*DATA_SIDE_LENGTH + jj] > 0 ? res2[i*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + ii*DATA_SIDE_LENGTH + jj] : 0;
				}
			}
		}

		//for (int i = 0; i < 10; i++)
		//{
		//	std::stringstream st;
		//	st <<"no."<<i<<"="<< res2[0][0][i] << endl;
		//	POST_INFO(st.str().c_str());
		//}

		//max-pooling操作(padding=same)&输出操作--------------------------------------------------
		out.Resize(1, MODEL_GROUP*RESULT_SIDE_LENGTH*RESULT_SIDE_LENGTH);
		out.Zero();
		//int yu = 20;
		for (int i = 0; i < MODEL_GROUP; i++)
		{
			for (int ii = 0; ii < DATA_SIDE_LENGTH; ii += POOLING_STEP_LENGTH)
			{
				for (int jj = 0; jj < DATA_SIDE_LENGTH; jj += POOLING_STEP_LENGTH)
				{
					double now = res2[i*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + ii*DATA_SIDE_LENGTH + jj];
					//if (yu > 0)
					//{
					//	std::stringstream xin;
					//	xin << "xin == (" << ii << "," << jj << ")" << "min=" << -(POOLING_RADIUS / 2) << "  max=" << (POOLING_RADIUS - 1) / 2  << endl;
					//	POST_INFO(xin.str().c_str());
					//	std::stringstream yst;
					//	yst <<"{{{{{{{{{{{{{{{{{{{{"<<yu << endl;
					//	POST_INFO(yst.str().c_str());
					//}
					//int cnt = 0;
					for (int di = -(POOLING_RADIUS / 2); di <= (POOLING_RADIUS - 1) / 2; di++)
					{
						for (int dj = -(POOLING_RADIUS / 2); dj <= (POOLING_RADIUS - 1) / 2; dj++)
						{
							//cnt ++;
							int ni = ii + di, nj = jj + dj;
							if (0 <= ni && ni < DATA_SIDE_LENGTH && 0 <= nj && nj<DATA_SIDE_LENGTH)
								now = now > res2[i*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + ni*DATA_SIDE_LENGTH + nj] ? now : res2[i*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + ni*DATA_SIDE_LENGTH + nj];
							else now = now > 0 ? now : 0;

							//std::stringstream st;
							//st <<"++++++++++++++++"<< (yu > 0) << endl;
							//POST_INFO(st.str().c_str());

							//if (yu > 0)
							//{
							//	std::stringstream st;
							//	st << "--"<<cnt<<"--("<<ni<<","<<nj<<")" << endl;
							//	POST_INFO(st.str().c_str());
							//}
						}
					}

					//if (yu > 0)
					//{
					//	yu--;
					//	std::stringstream st;
					//	st << "no.---"<<cnt<<"--"<< 19-yu<<" )))))" <<setprecision(30) <<  now << endl;
					//	POST_INFO(st.str().c_str());
					//}
					out(0, i*RESULT_SIDE_LENGTH*RESULT_SIDE_LENGTH + ii / POOLING_STEP_LENGTH*RESULT_SIDE_LENGTH + jj / POOLING_STEP_LENGTH) = now;
				}
			}
		}

		

		//输出到缓冲区
		OUT_D[0] = out;
		OUT_N[0] = MODEL_GROUP;


		delete[] model2;//new和delete是成对出现的，这里也要多重循环释放掉空间
		model2 = NULL;
		delete[] res2;//new和delete是成对出现的，这里也要多重循环释放掉空间
		res2 = NULL;
		delete[] c1_data2;//new和delete是成对出现的，这里也要多重循环释放掉空间
		c1_data2 = NULL;

		std::stringstream stag;
		stag << "converting finished!" << endl;
		POST_INFO(stag.str().c_str());
		fclose(bf);
		return true;
	}

	bool layer2::Finalize()
	{

		return true;
	}

	void layer2::mych(int tmp)
	{
		memset(num2, '\0', sizeof(num2));
		int tp = 0;
		while (tmp)
		{
			num2[tp++] = '0' + (tmp % 10);
			tmp /= 10;
		}
		for (int i = 0; i<tp / 2; i++)
		{
			swap(num2[i], num2[tp - 1 - i]);
		}
		num2[tp++] = '\0';
	}

}