#pragma once
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

namespace SystemVueModelBuilder
{
	class test_fc : public SystemVueModelBuilder::DFModel
	{
	public:
		DECLARE_MODEL_INTERFACE(test_fc);
		test_fc();
		virtual ~test_fc();

		virtual bool Setup();
		virtual bool Initialize();
		virtual bool Run();
		virtual bool Finalize();



		/// define parameters
		char* INPUT_FILE;

		/// define ports

		IntCircularBuffer IN_N;
		DoubleMatrixCircularBuffer IN_D;

		IntCircularBuffer OUT_N;
		DoubleMatrixCircularBuffer OUT_1;


	protected:

		DoubleMatrix in_tmp;
		DoubleMatrix out;

	};
}