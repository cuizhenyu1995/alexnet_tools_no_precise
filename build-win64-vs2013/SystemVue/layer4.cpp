#pragma once
#include "layer4.h"
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
#define MODEL_WEIGHT 38
#define MODEL_GROUP 38
#define DATA_SIDE_LENGTH 57
#define RESULT_SIDE_LENGTH 57

char pre4[] = "weights";
char biase4[] = "biases";
char num4[NUMBER_LENGTH_MAX];
char last4[] = ".txt";
double *model4;
double *res4;
double *c1_data4;


namespace SystemVueModelBuilder {

#ifndef SV_CODE_MRA_RUN
	DEFINE_MODEL_INTERFACE(layer4)
	{
		SET_MODEL_NAME("layer4");
		SET_MODEL_DESCRIPTION("");
		SET_MODEL_CATEGORY("alexnet_simple_conv");
		ADD_MODEL_HEADER_FILE("layer4.h");
		model.SetModelCodeGenName("layer4");
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

	layer4::layer4()
	{
	}

	layer4::~layer4()
	{
	}

	bool layer4::Setup()
	{
		IN_N.SetRate(1);
		IN_D.SetRate(1);

		OUT_N.SetRate(1);
		OUT_D.SetRate(1);

		return true;
	};

	bool layer4::Initialize()
	{
		return true;
	}
	bool layer4::Run()
	{
		
		//为model4申请空间
		int fal = 0;
		model4 = new double[MODEL_WEIGHT*MODEL_GROUP*LENGTH_OF_SIDE*LENGTH_OF_SIDE];
		if (model4 == NULL) fal = 1;
		if (fal)
		{
			std::stringstream st;
			st << "Failed when getting space for model4!" << endl;
			POST_INFO(st.str().c_str());
			return true;
		}


		//读取model4----------------------------------------------------------------------------
		for (int i = 0; i < LENGTH_OF_SIDE*LENGTH_OF_SIDE; i++)
		{
			char s[FILE_LENGTH_MAX] = "";
			strcat(s, INPUT_FILE);
			strcat(s, pre4);
			mych(i + 1);
			strcat(s, num4);
			strcat(s, last4);

			//打开model4文件
			FILE *mf = fopen(s, "r");
			if (mf == NULL)
			{
				std::stringstream st;
				st << "Failed when opening model4 flie!" << endl;
				POST_INFO(st.str().c_str());
				return true;
			}

			//读取model4数据
			for (int j = 0; j < MODEL_WEIGHT; j++)
			{
				for (int k = 0; k < MODEL_GROUP; k++)
				{
					fscanf(mf, "%lf", &model4[j*MODEL_GROUP*LENGTH_OF_SIDE*LENGTH_OF_SIDE + k*LENGTH_OF_SIDE*LENGTH_OF_SIDE + i]);

				}
			}
			fclose(mf);
		}

		//为res4申请空间
		fal = 0;
		res4 = new double[MODEL_GROUP * DATA_SIDE_LENGTH * DATA_SIDE_LENGTH];
		if (res4 == NULL) fal = 1;
		if (fal)
		{
			std::stringstream st;
			st << "Failed when getting space for res4ult!" << endl;
			POST_INFO(st.str().c_str());
			return true;
		}

		for (int i = 0; i < MODEL_GROUP * DATA_SIDE_LENGTH * DATA_SIDE_LENGTH; i++) res4[i] = 0;

		//为data申请空间
		int DATA_WEIGHT = IN_N[0];

		fal = 0;
		c1_data4 = new double[DATA_WEIGHT * DATA_SIDE_LENGTH * DATA_SIDE_LENGTH];
		if (c1_data4 == NULL) fal = 1;
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
					c1_data4[k*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + i*DATA_SIDE_LENGTH + j] = IN_D[0](0, k*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + i*DATA_SIDE_LENGTH + j);


		//卷积操作-----------------------------------------------------------------------------
		for (int i = 0; i < MODEL_GROUP; i++)//group
		{
			for (int j = 0; j < MODEL_WEIGHT; j++)//weight
			{

				//virtual int getres4(double f[], double **mm, double **res4, int mph, int mpl, int R);
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
									tmp += model4[j*MODEL_GROUP*LENGTH_OF_SIDE*LENGTH_OF_SIDE + i*LENGTH_OF_SIDE*LENGTH_OF_SIDE + tp] * c1_data4[j*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + k*DATA_SIDE_LENGTH + l];
								}
								tp++;
							}
						}
						res4[i*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + ii*DATA_SIDE_LENGTH + jj] += tmp;
					}
				}
				//getres4(model4[j][i], c1_data4[j], res4[i], DATA_SIDE_LENGTH, DATA_SIDE_LENGTH, LENGTH_OF_SIDE)
			}
		}


		//读取biase4文件
		char s[FILE_LENGTH_MAX] = "";
		strcat(s, INPUT_FILE);
		strcat(s, biase4);
		strcat(s, last4);
		FILE *bf = fopen(s, "r");
		if (bf == NULL)
		{
			std::stringstream st;
			st << "Failed when opening biase4 file!" << endl;
			POST_INFO(st.str().c_str());
			return true;
		}


		//biase4操作---------------------------------------------------------------------
		for (int i = 0; i < MODEL_GROUP; i++)
		{
			double ttt;
			fscanf(bf, "%lf", &ttt);
			for (int ii = 0; ii < DATA_SIDE_LENGTH; ii++)
			{
				for (int jj = 0; jj < DATA_SIDE_LENGTH; jj++)
				{
					res4[i*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + ii*DATA_SIDE_LENGTH + jj] += ttt;
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
					res4[i*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + ii*DATA_SIDE_LENGTH + jj] = res4[i*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + ii*DATA_SIDE_LENGTH + jj] > 0 ? res4[i*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + ii*DATA_SIDE_LENGTH + jj] : 0;
				}
			}
		}

		//for (int i = 0; i < 10; i++)
		//{
		//	std::stringstream st;
		//	st <<"no."<<i<<"="<< res4[0][0][i] << endl;
		//	POST_INFO(st.str().c_str());
		//}

		//max-pooling操作(padding=same)&输出操作--------------------------------------------------
		out.Resize(1, MODEL_GROUP*RESULT_SIDE_LENGTH*RESULT_SIDE_LENGTH);
		out.Zero();
		for (int i = 0; i < MODEL_GROUP; i++)
		{
			for (int ii = 0; ii < DATA_SIDE_LENGTH; ii++)
			{
				for (int jj = 0; jj < DATA_SIDE_LENGTH; jj++)
				{
					double now = res4[i*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + ii*DATA_SIDE_LENGTH + jj];

					out(0, i*RESULT_SIDE_LENGTH*RESULT_SIDE_LENGTH + ii *RESULT_SIDE_LENGTH + jj) = now;
				}
			}
		}

		//输出到缓冲区
		OUT_D[0] = out;
		OUT_N[0] = MODEL_GROUP;


		delete[] model4;//new和delete是成对出现的，这里也要多重循环释放掉空间
		model4 = NULL;
		delete[] res4;//new和delete是成对出现的，这里也要多重循环释放掉空间
		res4 = NULL;
		delete[] c1_data4;//new和delete是成对出现的，这里也要多重循环释放掉空间
		c1_data4 = NULL;

		std::stringstream stag;
		stag << "converting finished!" << endl;
		POST_INFO(stag.str().c_str());
		fclose(bf);
		return true;
	}

	bool layer4::Finalize()
	{

		return true;
	}

	void layer4::mych(int tmp)
	{
		memset(num4, '\0', sizeof(num4));
		int tp = 0;
		while (tmp)
		{
			num4[tp++] = '0' + (tmp % 10);
			tmp /= 10;
		}
		for (int i = 0; i<tp / 2; i++)
		{
			swap(num4[i], num4[tp - 1 - i]);
		}
		num4[tp++] = '\0';
	}

}