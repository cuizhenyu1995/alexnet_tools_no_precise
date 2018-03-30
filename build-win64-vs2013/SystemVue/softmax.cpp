#pragma once
#include "softmax.h"
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

double *softmax_data;
double *softmax_res;


namespace SystemVueModelBuilder {

#ifndef SV_CODE_MRA_RUN
	DEFINE_MODEL_INTERFACE(softmax)
	{
		SET_MODEL_NAME("softmax");
		SET_MODEL_DESCRIPTION("");
		SET_MODEL_CATEGORY("alexnet_simple_fully_connected");
		ADD_MODEL_HEADER_FILE("softmax.h");
		model.SetModelCodeGenName("softmax");
		model.SetModelNamespace("SystemVueModelBuilder");

		// Add parameters
		SystemVueModelBuilder::DFParam param = NULL;

		// Add input/output ports
		DFPort IN_DATA = ADD_MODEL_INPUT(IN_D);
		DFPort IN_NUM = ADD_MODEL_INPUT(IN_N);


		DFPort OUT_DATA = ADD_MODEL_OUTPUT(OUT_D);
		DFPort OUT_NUM = ADD_MODEL_OUTPUT(OUT_N);


		return true;
	}
#endif

	softmax::softmax()
	{
	}

	softmax::~softmax()
	{
	}

	bool softmax::Setup()
	{
		IN_N.SetRate(1);
		IN_D.SetRate(1);

		OUT_N.SetRate(1);
		OUT_D.SetRate(1);

		return true;
	};

	bool softmax::Initialize()
	{
		return true;
	}
	bool softmax::Run()
	{

		std::stringstream sut;
		sut << "softmax start!" << endl;
		POST_INFO(sut.str().c_str());

		//为softmax_res申请空间
		int fail = 0;
		softmax_res = new double[IN_N[0]];
		if (softmax_res == NULL) fail = 1;
		if (fail)
		{
			std::stringstream st;
			st << "Failed when getting space for softmax_result!" << endl;
			POST_INFO(st.str().c_str());
			return true;
		}
		else
		{
			std::stringstream st;
			st << "Success when getting space for softmax_result!" << endl;
			POST_INFO(st.str().c_str());
		}

		for (int i = 0; i < IN_N[0]; i++)
			softmax_res[i] = 0;

		//为读取上一层分量申请空间
		fail = 0;
		softmax_data = new double[IN_N[0]];
		if (softmax_data == NULL) fail = 1;
		if (fail)
		{
			std::stringstream st;
			st << "Failed when getting space for data!" << endl;
			POST_INFO(st.str().c_str());
			return true;
		}
		else
		{
			std::stringstream st;
			st << "Success when getting space for data!" << endl;
			POST_INFO(st.str().c_str());
		}

		//读取上一层分量------------------------------------------------------------
		double sum = 0;
		for (int i = 0; i < IN_N[0]; i++)
		{
			softmax_data[i] = (double)(IN_D[0](0, i));
			sum+=exp(softmax_data[i]);
		}

		//softmax-----------------------------------------------------------------------
		for (int i = 0; i < IN_N[0]; i++)
		{
			softmax_res[i] = exp(softmax_data[i])/sum;
		}

		//偏置操作&放入matrix
		out.Resize(1, IN_N[0]);
		out.Zero();
		for (int i = 0; i < IN_N[0]; i++)
		{
			out(0, i) = softmax_res[i];
		}

		//放入输出缓存
		OUT_N[0] = IN_N[0];
		OUT_D[0] = out;

		//释放空间
		delete[] softmax_data;//new和delete是成对出现的，这里也要多重循环释放掉空间
		softmax_data = NULL;
		delete[] softmax_res;
		softmax_res = NULL;

		std::stringstream stag;
		stag << "softmax finished!" << endl;
		POST_INFO(stag.str().c_str());
		return true;
	}

	bool softmax::Finalize()
	{

		return true;
	}
}