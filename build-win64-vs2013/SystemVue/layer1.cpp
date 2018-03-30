#pragma once
#include "layer1.h"
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
#define MODEL_GROUP 9
#define DATA_SIDE_LENGTH 227
#define RESULT_SIDE_LENGTH 114
#define POOLING_STEP_LENGTH 2
#define POOLING_RADIUS 3

char pre1[] = "weights";
char biase1[] = "biases";
char num1[NUMBER_LENGTH_MAX];
char last1[] = ".txt";
double *model1;
double *res1;
double *c1_data1;


namespace SystemVueModelBuilder {

#ifndef SV_CODE_MRA_RUN
	DEFINE_MODEL_INTERFACE(layer1)
	{
		SET_MODEL_NAME("layer1");
		SET_MODEL_DESCRIPTION("");
		SET_MODEL_CATEGORY("alexnet_simple_conv");
		ADD_MODEL_HEADER_FILE("layer1.h");
		model.SetModelCodeGenName("layer1");
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

	layer1::layer1()
	{
	}

	layer1::~layer1()
	{
	}

	bool layer1::Setup()
	{
		IN_N.SetRate(1);
		IN_D.SetRate(1);

		OUT_N.SetRate(1);
		OUT_D.SetRate(1);

		return true;
	};

	bool layer1::Initialize()
	{
		return true;
	}
	bool layer1::Run()
	{
		
		//为model1申请空间
		int fal = 0;
		model1 = new double[MODEL_WEIGHT*MODEL_GROUP*LENGTH_OF_SIDE*LENGTH_OF_SIDE];
		if (model1 == NULL) fal = 1;
		if (fal)
		{
			std::stringstream st;
			st << "Failed when getting space for model1!" << endl;
			POST_INFO(st.str().c_str());
			return true;
		}

		//读取model1----------------------------------------------------------------------------
		for (int i = 0; i < LENGTH_OF_SIDE*LENGTH_OF_SIDE; i++)
		{
			char s[FILE_LENGTH_MAX] = "";
			strcat(s, INPUT_FILE);
			strcat(s, pre1);
			mych(i + 1);
			strcat(s, num1);
			strcat(s, last1);

			//打开model1文件
			FILE *mf = fopen(s, "r");
			if (mf == NULL)
			{
				std::stringstream st;
				st << "Failed when opening model1 flie!" << endl;
				POST_INFO(st.str().c_str());
				return true;
			}
			//读取model1数据
			for (int j = 0; j < MODEL_WEIGHT; j++)
			{
				for (int k = 0; k < MODEL_GROUP; k++)
				{
					fscanf(mf, "%lf", &model1[j*MODEL_GROUP*LENGTH_OF_SIDE*LENGTH_OF_SIDE + k*LENGTH_OF_SIDE*LENGTH_OF_SIDE + i]);

				}
			}
			
			fclose(mf);
		}
		//std::stringstream sut;
		//sut <<setprecision(30)<< "model1[2][3][6]" << model1[1 * MODEL_GROUP*LENGTH_OF_SIDE*LENGTH_OF_SIDE + 2 * LENGTH_OF_SIDE*LENGTH_OF_SIDE + 5] << endl;
		//POST_INFO(sut.str().c_str());

		//为res1申请空间
		fal = 0;
		res1 = new double[MODEL_GROUP * DATA_SIDE_LENGTH * DATA_SIDE_LENGTH];
		if (res1 == NULL) fal = 1;
		if (fal)
		{
			std::stringstream st;
			st << "Failed when getting space for res1ult!" << endl;
			POST_INFO(st.str().c_str());
			return true;
		}
		for (int i = 0; i < MODEL_GROUP * DATA_SIDE_LENGTH * DATA_SIDE_LENGTH; i++) res1[i] = 0;

		//为data申请空间
		int DATA_WEIGHT = IN_N[0];

		fal = 0;
		c1_data1 = new double[DATA_WEIGHT * DATA_SIDE_LENGTH * DATA_SIDE_LENGTH];
		if (c1_data1 == NULL) fal = 1;
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
					c1_data1[k*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + i*DATA_SIDE_LENGTH + j] = IN_D[0](0, k*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + i*DATA_SIDE_LENGTH + j);

		
		//卷积操作-----------------------------------------------------------------------------
		for (int i = 0; i < MODEL_GROUP; i++)//group
		{
			for (int j = 0; j < MODEL_WEIGHT; j++)//weight
			{
				
				//virtual int getres1(double f[], double **mm, double **res1, int mph, int mpl, int R);
				int mph = DATA_SIDE_LENGTH, mpl = DATA_SIDE_LENGTH, R = LENGTH_OF_SIDE;
				
				for (int ii = 0; ii<mph; ii++)
				{
					for (int jj = 0; jj<mpl; jj++)
					{
						double tmp = 0;
						int ll = jj - R / 2, rr = jj + R / 2;
						int nn = ii - R / 2, ss = ii + R / 2;

						int tp = 0;
						
						for (int k = nn; k <= ss; k++)
						{
							for (int l = ll; l <= rr; l++)
							{
								if (k >= 0 && k<mph && l >= 0 && l<mpl)
								{
									tmp += model1[j*MODEL_GROUP*LENGTH_OF_SIDE*LENGTH_OF_SIDE + i*LENGTH_OF_SIDE*LENGTH_OF_SIDE + tp] * c1_data1[j*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + k*DATA_SIDE_LENGTH + l];
								}
								tp++;
							}
						}
						
						res1[i*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + ii*DATA_SIDE_LENGTH + jj] += tmp;
					}
				}
				
				//getres1(model1[j][i], c1_data1[j], res1[i], DATA_SIDE_LENGTH, DATA_SIDE_LENGTH, LENGTH_OF_SIDE)
				
			}
		}


		//读取biase1文件
		char s[FILE_LENGTH_MAX] = "";
		strcat(s, INPUT_FILE);
		strcat(s, biase1);
		strcat(s, last1);
		FILE *bf = fopen(s, "r");
		if (bf == NULL)
		{
			std::stringstream st;
			st << "Failed when opening biase1 file!" << endl;
			POST_INFO(st.str().c_str());
			return true;
		}


		//biase1操作---------------------------------------------------------------------
		for (int i = 0; i < MODEL_GROUP; i++)
		{
			double ttt;
			fscanf(bf, "%lf", &ttt);
			for (int ii = 0; ii < DATA_SIDE_LENGTH; ii++)
			{
				for (int jj = 0; jj < DATA_SIDE_LENGTH; jj++)
				{
					res1[i*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + ii*DATA_SIDE_LENGTH + jj] += ttt;
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
					res1[i*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + ii*DATA_SIDE_LENGTH + jj] = res1[i*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + ii*DATA_SIDE_LENGTH + jj] > 0 ? res1[i*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + ii*DATA_SIDE_LENGTH + jj] : 0;

				}
			}
		}

		for (int i = 0; i < 20; i++)
		{
			std::stringstream st;
			st <<"no."<<i<<"="<< res1[i] << endl;
			POST_INFO(st.str().c_str());
		}

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
					double now = res1[i*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + ii*DATA_SIDE_LENGTH + jj];
					//if (yu > 0)
					//{
					//	std::stringstream xin;
					//	xin << "xin == (" << ii << "," << jj << ")" << "min=" << -(POOLING_RADIUS / 2) << "  max=" << (POOLING_RADIUS - 1) / 2  << endl;
					//	POST_INFO(xin.str().c_str());
					//	std::stringstream yst;
					//	yst <<"{{{{{{{{{{{{{{{{{{{{"<<yu << endl;
					//	POST_INFO(yst.str().c_str());
					//}
					int cnt = 0;
					for (int di = -(POOLING_RADIUS / 2); di <= (POOLING_RADIUS-1) / 2; di++)
					{
						for (int dj = -(POOLING_RADIUS / 2); dj <= (POOLING_RADIUS-1) / 2; dj++)
						{
							//cnt ++;
							int ni = ii + di, nj = jj + dj;
							if (0<=ni && ni < DATA_SIDE_LENGTH && 0<=nj && nj<DATA_SIDE_LENGTH)
								now = now > res1[i*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + ni*DATA_SIDE_LENGTH + nj] ? now : res1[i*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + ni*DATA_SIDE_LENGTH + nj];
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
					//	st << "-----no."<< 19-yu<<"     " <<setprecision(30) <<  now << endl;
					//	POST_INFO(st.str().c_str());
					//}
					out(0, i*RESULT_SIDE_LENGTH*RESULT_SIDE_LENGTH + ii / POOLING_STEP_LENGTH*RESULT_SIDE_LENGTH + jj / POOLING_STEP_LENGTH) = now;
				}
			}
		}

		//输出到缓冲区
		OUT_D[0] = out;
		OUT_N[0] = MODEL_GROUP;


		delete[] model1;//new和delete是成对出现的，这里也要多重循环释放掉空间
		model1 = NULL;
		delete[] res1;//new和delete是成对出现的，这里也要多重循环释放掉空间
		res1 = NULL;
		delete[] c1_data1;//new和delete是成对出现的，这里也要多重循环释放掉空间
		c1_data1 = NULL;

		std::stringstream stag;
		stag << "converting finished!" << endl;
		POST_INFO(stag.str().c_str());
		fclose(bf);
		return true;
	}

	bool layer1::Finalize()
	{

		return true;
	}

	void layer1::mych(int tmp)
	{
		memset(num1, '\0', sizeof(num1));
		int tp = 0;
		while (tmp)
		{
			num1[tp++] = '0' + (tmp % 10);
			tmp /= 10;
		}
		for (int i = 0; i<tp / 2; i++)
		{
			swap(num1[i], num1[tp - 1 - i]);
		}
		num1[tp++] = '\0';
	}

}