#pragma once
#include "layer8.h"
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

#define MODEL_WEIGHT 100
#define MODEL_GROUP 2
#define MODEL_SIZE 1*1
#define FILE_LENGTH_MAX 255
#define DATA_SIDE_LENGTH 1
#define DATA_WEIGHT 100

char pre8[] = "weights";
char biase8[] = "biases";
char last8[] = ".txt";
double *model8;
double *fc1_data8;
double *res8;


namespace SystemVueModelBuilder {

#ifndef SV_CODE_MRA_RUN
	DEFINE_MODEL_INTERFACE(layer8)
	{
		SET_MODEL_NAME("layer8");
		SET_MODEL_DESCRIPTION("");
		SET_MODEL_CATEGORY("alexnet_simple_fully_connected");
		ADD_MODEL_HEADER_FILE("layer8.h");
		model.SetModelCodeGenName("layer8");
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

	layer8::layer8()
	{
	}

	layer8::~layer8()
	{
	}

	bool layer8::Setup()
	{
		IN_N.SetRate(1);
		IN_D.SetRate(1);

		OUT_N.SetRate(1);
		OUT_1.SetRate(1);

		return true;
	};

	bool layer8::Initialize()
	{
		return true;
	}
	bool layer8::Run()
	{

		//Ϊmodel��ȡ�ռ�
		int fail = 0;
		model8 = new double[MODEL_WEIGHT*MODEL_GROUP*MODEL_SIZE];
		if (model8 == NULL) fail = 1;
		if (fail)
		{
			std::stringstream st;
			st << "Failed when getting space for model!" << endl;
			POST_INFO(st.str().c_str());
			return true;
		}

		//��model�ļ�
		char s[FILE_LENGTH_MAX] = "";
		strcat(s, INPUT_FILE);
		strcat(s, pre8);
		strcat(s, last8);
		FILE *mf = fopen(s, "r");
		if (mf == NULL)
		{
			std::stringstream st;
			st << "Failed when opening model file!" << endl;
			POST_INFO(st.str().c_str());
			return true;
		}

		//��ȡmodel��Ϣ-------------------------------------------------------------------------
		for (int i = 0; i < MODEL_WEIGHT; i++)
			for (int j = 0; j < MODEL_SIZE; j++)
				for (int k = 0; k < MODEL_GROUP; k++)
				{
					fscanf(mf, "%lf", &model8[i*MODEL_GROUP*MODEL_SIZE + k*MODEL_SIZE + j]);
				}

		//Ϊres8����ռ�
		fail = 0;
		res8 = new double[MODEL_WEIGHT*MODEL_GROUP];
		if (res8 == NULL) fail = 1;
		if (fail)
		{
			std::stringstream st;
			st << "Failed when getting space for result!" << endl;
			POST_INFO(st.str().c_str());
			return true;
		}

		for (int i = 0; i < MODEL_WEIGHT; i++)
			for (int j = 0; j < MODEL_GROUP; j++)
				res8[i*MODEL_GROUP + j] = 0;

		//Ϊ��ȡ��һ���������ռ�
		fail = 0;
		fc1_data8 = new double[DATA_WEIGHT*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH];
		if (fc1_data8 == NULL) fail = 1;
		if (fail)
		{
			std::stringstream st;
			st << "Failed when getting space for data!" << endl;
			POST_INFO(st.str().c_str());
			return true;
		}

		//��ȡ��һ�����------------------------------------------------------------
		for (int k = 0; k < DATA_WEIGHT; k++)
		{
			for (int i = 0; i < DATA_SIDE_LENGTH; i++)
			{
				for (int j = 0; j < DATA_SIDE_LENGTH; j++)
				{
					fc1_data8[k*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + i*DATA_SIDE_LENGTH + j] = (double)(IN_D[0](0, k*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + i*DATA_SIDE_LENGTH + j));
				}
			}
		}

		//ȫ���Ӳ���-----------------------------------------------------------------------
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
						res8[j*MODEL_GROUP + i] += model8[j*MODEL_GROUP*MODEL_SIZE + i*MODEL_SIZE + tp] * fc1_data8[j*DATA_SIDE_LENGTH*DATA_SIDE_LENGTH + ii*DATA_SIDE_LENGTH + jj];

						tp++;
					}
				}

				//getres(model8[j][i], fc1_data8[j], &res[j][i], DATA_SIDE_LENGTH, DATA_SIDE_LENGTH);
			}
		}

		//��ƫ���ļ�
		char bis[FILE_LENGTH_MAX] = "";
		strcat(bis, INPUT_FILE);
		strcat(bis, biase8);
		strcat(bis, last8);
		FILE *bf = fopen(bis, "r");
		if (bf == NULL)
		{
			std::stringstream st;
			st << "Failed when opening biase file!" << endl;
			POST_INFO(st.str().c_str());
			return true;
		}

		//ƫ�ò���&����matrix
		out.Resize(1, MODEL_GROUP);
		out.Zero();
		for (int i = 0; i < MODEL_GROUP; i++)
		{
			double ttt = 0;
			fscanf(bf, "%lf", &ttt);
			for (int j = 0; j < MODEL_WEIGHT; j++)
				ttt += res8[j*MODEL_GROUP + i];
			out(0, i) = ttt;
		}

		//�����������
		OUT_1[0] = out;
		OUT_N[0] = MODEL_GROUP;

		//�ͷſռ�
		delete[] model8;//new��delete�ǳɶԳ��ֵģ�����ҲҪ����ѭ���ͷŵ��ռ�
		model8 = NULL;
		delete[] fc1_data8;//new��delete�ǳɶԳ��ֵģ�����ҲҪ����ѭ���ͷŵ��ռ�
		fc1_data8 = NULL;
		delete[] res8;
		res8 = NULL;

		std::stringstream stag;
		stag << "fully connected finished!" << endl;
		POST_INFO(stag.str().c_str());
		fclose(mf);
		fclose(bf);
		return true;
	}

	bool layer8::Finalize()
	{

		return true;
	}
}