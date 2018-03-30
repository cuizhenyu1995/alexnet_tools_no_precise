#pragma once
#include "layer5.h"
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
#define MODEL_GROUP 25
#define DATA_SIDE_LENGTH 57
#define RESULT_SIDE_LENGTH 29
#define POOLING_STEP_LENGTH 2
#define POOLING_RADIUS 3

char pre5[] = "weights";
char biase5[] = "biases";
char num5[NUMBER_LENGTH_MAX];
char last5[] = ".txt";
double *model5;
double *res5;
double *c1_data5;


namespace SystemVueModelBuilder {

#ifndef SV_CODE_MRA_RUN
	DEFINE_MODEL_INTERFACE(layer5)
	{
		SET_MODEL_NAME("layer5");
		SET_MODEL_DESCRIPTION("");
		SET_MODEL_CATEGORY("alexnet_simple_conv");
		ADD_MODEL_HEADER_FILE("layer5.h");
		model.SetModelCodeGenName("layer5");
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

	layer5::layer5()
	{
	}

	layer5::~layer5()
	{
	}

	bool layer5::Setup()
	{
		IN_N.SetRate(1);
		IN_D.SetRate(1);

		OUT_N.SetRate(1);
		OUT_D.SetRate(1);

		return true;
	};

	bool layer5::Initialize()
	{
		return true;
	}
	bool layer5::Run()
	{
		
		//为model5申请空间
		int fal = 0;
		model5 = new double[MODEL_WEIGHT*MODEL_GROUP*LENGTH_OF_SIDE*LENGTH_OF_SIDE];
		if (model5 == NULL) fal = 1;
		if (fal)
		{
			std::stringstream st;
			st << "Failed when getting space for model5!" << endl;
			POST_INFO(st.str().c_str());
			return true;
		}


		//读取model5----------------------------------------------------------------------------
		for (int i = 0; i < LENGTH_OF_SIDE*LENGTH_OF_SIDE; i++)
		{
			char s[FILE_LENGTH_MAX] = "";
			strcat(s, INPUT_FILE);
			strcat(s, pre5);
			mych(i + 1);
			strcat(s, num5);
			strcat(s, last5);

			//打开model5文件
			FILE *mf = fopen(s, "r");
			if (mf == NULL)
			{
				std::stringstream st;
				st << "Failed when opening model5 flie!" << endl;
				POST_INFO(st.str().c_str());
				return true;
			}

			//读取model5数据
			for (int j = 0; j < MODEL_WEIGHT; j++)
			{
				for (int k = 0; k < MODEL_GROUP; k++)
				{
					fscanf(mf, "%lf", &model5[j*MODEL_GROUP*LENGTH_OF_SIDE*LENGTH_OF_SIDE + k*LENGTH_OF_SIDE*LENGTH_OF_SIDE + i]);

				}
			}
			fclose(mf);
		}

		//为res5申请空间
		fal = 0;
		res5 = new double[MODEL_GROUP * DATA_SIDE_LENGTH * DATA_SIDE_LENGTH];
		if (res5 == NULL) fal = 1;
		if (fal)
		{
			std::stringstream st;
			st << "Failed when getting space for res5ult!" << endl;
			POST_INFO(st.str().c_str());
			return true;
		}

		for (int i = 0; i < MODEL_GROUP * DATA_SIDE_LENGTH * DATA_SIDE_LENGTH; i++) res5[i] = 0;

		//为data申请空间
		int DATA_WEIGHT = IN_N[0];

		fal = 0;
		c1_data5 = new double[DATA_WEIGHT * DATA_SIDE_LENGTH * DATA_SIDE_LENGTH];
		if (c1_data5 == NULL) fal = 1;
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
					c1_data5[k*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + i*DATA_SIDE_LENGTH + j] = IN_D[0](0, k*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + i*DATA_SIDE_LENGTH + j);


		//卷积操作-----------------------------------------------------------------------------
		for (int i = 0; i < MODEL_GROUP; i++)//group
		{
			for (int j = 0; j < MODEL_WEIGHT; j++)//weight
			{

				//virtual int getres5(double f[], double **mm, double **res5, int mph, int mpl, int R);
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
									tmp += model5[j*MODEL_GROUP*LENGTH_OF_SIDE*LENGTH_OF_SIDE + i*LENGTH_OF_SIDE*LENGTH_OF_SIDE + tp] * c1_data5[j*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + k*DATA_SIDE_LENGTH + l];
								}
								tp++;
							}
						}
						res5[i*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + ii*DATA_SIDE_LENGTH + jj] += tmp;
					}
				}
				//getres5(model5[j][i], c1_data5[j], res5[i], DATA_SIDE_LENGTH, DATA_SIDE_LENGTH, LENGTH_OF_SIDE)
			}
		}


		//读取biase5文件
		char s[FILE_LENGTH_MAX] = "";
		strcat(s, INPUT_FILE);
		strcat(s, biase5);
		strcat(s, last5);
		FILE *bf = fopen(s, "r");
		if (bf == NULL)
		{
			std::stringstream st;
			st << "Failed when opening biase5 file!" << endl;
			POST_INFO(st.str().c_str());
			return true;
		}


		//biase5操作---------------------------------------------------------------------
		for (int i = 0; i < MODEL_GROUP; i++)
		{
			double ttt;
			fscanf(bf, "%lf", &ttt);
			for (int ii = 0; ii < DATA_SIDE_LENGTH; ii++)
			{
				for (int jj = 0; jj < DATA_SIDE_LENGTH; jj++)
				{
					res5[i*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + ii*DATA_SIDE_LENGTH + jj] += ttt;
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
					res5[i*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + ii*DATA_SIDE_LENGTH + jj] = res5[i*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + ii*DATA_SIDE_LENGTH + jj] > 0 ? res5[i*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + ii*DATA_SIDE_LENGTH + jj] : 0;
				}
			}
		}

		//for (int i = 0; i < 10; i++)
		//{
		//	std::stringstream st;
		//	st <<"no."<<i<<"="<< res5[0][0][i] << endl;
		//	POST_INFO(st.str().c_str());
		//}

		//max-pooling操作(padding=same)&输出操作--------------------------------------------------
		out.Resize(1, MODEL_GROUP*RESULT_SIDE_LENGTH*RESULT_SIDE_LENGTH);
		out.Zero();
		int yu = 20;
		for (int i = 0; i < MODEL_GROUP; i++)
		{
			for (int ii = 0; ii < DATA_SIDE_LENGTH; ii += POOLING_STEP_LENGTH)
			{
				for (int jj = 0; jj < DATA_SIDE_LENGTH; jj += POOLING_STEP_LENGTH)
				{
					double now = res5[i*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + ii*DATA_SIDE_LENGTH + jj];
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
								now = now > res5[i*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + ni*DATA_SIDE_LENGTH + nj] ? now : res5[i*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + ni*DATA_SIDE_LENGTH + nj];
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
					//	st << "no."<< 19-yu<<"    " <<setprecision(30) <<  now << endl;
					//	POST_INFO(st.str().c_str());
					//}
					out(0, i*RESULT_SIDE_LENGTH*RESULT_SIDE_LENGTH + ii / POOLING_STEP_LENGTH*RESULT_SIDE_LENGTH + jj / POOLING_STEP_LENGTH) = now;
				}
			}
		}

		FILE* me = fopen("D:\\layer5.txt","w+");
		for (int i = 0; i < MODEL_GROUP; i++)
		{
			for (int j = 0; j < RESULT_SIDE_LENGTH; j++)
			{
				for (int k = 0; k < RESULT_SIDE_LENGTH; k++)
				{
					fprintf(me, "%.8f ", out(0, i*RESULT_SIDE_LENGTH*RESULT_SIDE_LENGTH + j*RESULT_SIDE_LENGTH+k));
				}
				fprintf(me, "\n");
			}
			fprintf(me, "\n");
		}
		fclose(me);

		//输出到缓冲区
		OUT_D[0] = out;
		OUT_N[0] = MODEL_GROUP;


		delete[] model5;//new和delete是成对出现的，这里也要多重循环释放掉空间
		model5 = NULL;
		delete[] res5;//new和delete是成对出现的，这里也要多重循环释放掉空间
		res5 = NULL;
		delete[] c1_data5;//new和delete是成对出现的，这里也要多重循环释放掉空间
		c1_data5 = NULL;

		std::stringstream stag;
		stag << "converting finished!" << endl;
		POST_INFO(stag.str().c_str());
		fclose(bf);
		return true;
	}

	bool layer5::Finalize()
	{

		return true;
	}

	void layer5::mych(int tmp)
	{
		memset(num5, '\0', sizeof(num5));
		int tp = 0;
		while (tmp)
		{
			num5[tp++] = '0' + (tmp % 10);
			tmp /= 10;
		}
		for (int i = 0; i<tp / 2; i++)
		{
			swap(num5[i], num5[tp - 1 - i]);
		}
		num5[tp++] = '\0';
	}

}