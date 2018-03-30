#pragma once
#include "test_fc.h"
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


using namespace std;
#include <iostream>
#include <cstdio>
#include <algorithm>
#include <cmath>

using std::sort;
using std::fabs;

#define MODEL_WEIGHT 3
#define MODEL_GROUP 2
#define MODEL_SIZE 114*114
#define FILE_LENGTH_MAX 255
#define DATA_SIDE_LENGTH 114
#define DATA_WEIGHT 3

char fpre6[] = "weights";
char fbiase6[] = "biases";
char flast6[] = ".txt";
double *fmodel6;
double *ffc1_data6;
double *fres6;


namespace SystemVueModelBuilder {

#ifndef SV_CODE_MRA_RUN
	DEFINE_MODEL_INTERFACE(test_fc)
	{
		SET_MODEL_NAME("test_fc");
		SET_MODEL_DESCRIPTION("");
		SET_MODEL_CATEGORY("alexnet_simple_fully_connected");
		ADD_MODEL_HEADER_FILE("test_fc.h");
		model.SetModelCodeGenName("test_fc");
		model.SetModelNamespace("SystemVueModelBuilder");

		// Add parameters
		SystemVueModelBuilder::DFParam param = NULL;
		param = ADD_MODEL_PARAM(INPUT_FILE);

		// Add input/output ports
		DFPort IN_DATA = ADD_MODEL_INPUT(IN_D);
		DFPort IN_NUM = ADD_MODEL_INPUT(IN_N);

		DFPort OUT1 = ADD_MODEL_OUTPUT(OUT_1);
		DFPort OUTN = ADD_MODEL_OUTPUT(OUT_N);


		return true;
	}
#endif

	test_fc::test_fc()
	{
	}

	test_fc::~test_fc()
	{
	}

	bool test_fc::Setup()
	{
		IN_N.SetRate(1);
		IN_D.SetRate(1);

		OUT_N.SetRate(1);
		OUT_1.SetRate(1);

		return true;
	};

	bool test_fc::Initialize()
	{
		return true;
	}
	bool test_fc::Run()
	{

		//为model获取空间
		int fail = 0;
		fmodel6 = new double[MODEL_WEIGHT*MODEL_GROUP*MODEL_SIZE];
		if (fmodel6 == NULL) fail = 1;
		if (fail)
		{
			std::stringstream st;
			st << "Failed when getting space for model!" << endl;
			POST_INFO(st.str().c_str());
			return true;
		}

		//打开model文件
		char s[FILE_LENGTH_MAX] = "";
		strcat(s, INPUT_FILE);
		strcat(s, fpre6);
		strcat(s, flast6);
		FILE *mf = fopen(s, "r");
		if (mf == NULL)
		{
			std::stringstream st;
			st << "Failed when opening model file!" << endl;
			POST_INFO(st.str().c_str());
			return true;
		}


		//读取model信息-------------------------------------------------------------------------
		for (int i = 0; i < MODEL_WEIGHT; i++)
		{
			for (int j = 0; j < MODEL_SIZE; j++)
			{
				for (int k = 0; k < MODEL_GROUP; k++)
				{
					fscanf(mf, "%lf", &fmodel6[i*MODEL_GROUP*MODEL_SIZE + k*MODEL_SIZE + j]);
				}
			}
		}
		//FILE *TEST = fopen("D:\\momo.txt", "w+");
		//for (int i = 0; i < MODEL_WEIGHT; i++)
		//{
		//	for (int j = 0; j < MODEL_SIZE; j++)
		//	{
		//		for (int k = 0; k < MODEL_GROUP; k++)
		//		{
		//			fprintf(TEST, "%f ", fmodel6[i*MODEL_GROUP*MODEL_SIZE + k*MODEL_SIZE + j]);
		//		}
		//	}
		//}
		//fclose(TEST);
		//
		//for (int i = 0; i < MODEL_GROUP*MODEL_WEIGHT; i++) 


		//为fres6申请空间
		fail = 0;
		fres6 = new double[MODEL_WEIGHT*MODEL_GROUP];
		if (fres6 == NULL) fail = 1;
		if (fail)
		{
			std::stringstream st;
			st << "Failed when getting space for result!" << endl;
			POST_INFO(st.str().c_str());
			return true;
		}

		for (int i = 0; i < MODEL_WEIGHT; i++)
			for (int j = 0; j < MODEL_GROUP; j++)
				fres6[i*MODEL_GROUP + j] = 0;

		//为读取上一层分量申请空间
		fail = 0;
		ffc1_data6 = new double[DATA_WEIGHT*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH];
		if (ffc1_data6 == NULL) fail = 1;
		if (fail)
		{
			std::stringstream st;
			st << "Failed when getting space for data!" << endl;
			POST_INFO(st.str().c_str());
			return true;
		}

		//读取上一层分量------------------------------------------------------------
		for (int k = 0; k < DATA_WEIGHT; k++)
		{
			for (int i = 0; i < DATA_SIDE_LENGTH; i++)
			{
				for (int j = 0; j < DATA_SIDE_LENGTH; j++)
				{
					
					ffc1_data6[k*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + i*DATA_SIDE_LENGTH + j] = (double)(IN_D[0](0, k*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + i*DATA_SIDE_LENGTH + j));
				}
			}
		}

		//全连接操作-----------------------------------------------------------------------
		for (int i = 0; i < MODEL_GROUP; i++)
		{
			for (int j = 0; j < MODEL_WEIGHT; j++)
			{

				//virtual int getres(double f[], double **mm, double *res, int mph, int mpl);
				int tp = 0, mph = DATA_SIDE_LENGTH, mpl = DATA_SIDE_LENGTH;
				for (int ii = 0; ii<mph; ii++)
				{
					for (int jj = 0; jj<mpl; jj++)
					{
						fres6[j*MODEL_GROUP + i] += fmodel6[j*MODEL_GROUP*MODEL_SIZE + i*MODEL_SIZE + tp] * ffc1_data6[j*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + ii*DATA_SIDE_LENGTH + jj];

						tp++;
					}
				}


				//getres(fmodel6[j][i], ffc1_data6[j], &res[j][i], DATA_SIDE_LENGTH, DATA_SIDE_LENGTH);
			}
		}

		//打开偏置文件
		char bis[FILE_LENGTH_MAX] = "";
		strcat(bis, INPUT_FILE);
		strcat(bis, fbiase6);
		strcat(bis, flast6);
		FILE *bf = fopen(bis, "r");
		if (bf == NULL)
		{
			std::stringstream st;
			st << "Failed when opening biase file!" << endl;
			POST_INFO(st.str().c_str());
			return true;
		}

		//FILE *TEST = fopen("D:\\daan77777.txt", "w+");
		//for (int i = 0; i < MODEL_GROUP*MODEL_WEIGHT; i++) fprintf(TEST, "%f ", fres6[i]);


		//偏置操作&放入matrix
		out.Resize(1, MODEL_GROUP);
		out.Zero();
		for (int i = 0; i < MODEL_GROUP; i++)
		{
			double ttt = 0;
			fscanf(bf, "%lf", &ttt);
			for (int j = 0; j < MODEL_WEIGHT; j++)
				ttt += fres6[j*MODEL_GROUP + i];
			out(0, i) = ttt ;
		}

		//to_test
		//FILE *TEST = fopen("D:\\res0007.txt", "w+");
		//for (int i = 0; i < MODEL_GROUP; i++)
		//{
		//	fprintf(TEST, "%f ", out(0, i));
		//	if (i == 0)
		//	{
		//		std::stringstream bii;
		//		bii << "write success!" << endl;
		//		POST_INFO(bii.str().c_str());
		//	}
		//}
		//fclose(TEST);

		//放入输出缓存
		OUT_1[0] = out;
		OUT_N[0] = MODEL_GROUP;

		//释放空间
		delete[] fmodel6;//new和delete是成对出现的，这里也要多重循环释放掉空间
		fmodel6 = NULL;
		delete[] ffc1_data6;//new和delete是成对出现的，这里也要多重循环释放掉空间
		ffc1_data6 = NULL;
		delete[] fres6;
		fres6 = NULL;

		std::stringstream stag;
		stag << "fully connected finished!" << endl;
		POST_INFO(stag.str().c_str());
		fclose(mf);
		fclose(bf);
		return true;
	}

	bool test_fc::Finalize()
	{

		return true;
	}
}