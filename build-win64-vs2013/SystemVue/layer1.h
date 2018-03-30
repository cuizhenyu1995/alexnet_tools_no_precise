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
	class layer1 : public SystemVueModelBuilder::DFModel
	{
	public:
		DECLARE_MODEL_INTERFACE(layer1);
		layer1();
		virtual ~layer1();

		virtual bool Setup();
		virtual bool Initialize();
		virtual bool Run();
		virtual bool Finalize();

		virtual void mych(int tmp);

		/// define parameters
		int LENGTH_OF_SIDE;
		char* INPUT_FILE;

		/// define ports
		IntCircularBuffer IN_N;
		DoubleMatrixCircularBuffer IN_D;

		IntCircularBuffer OUT_N;
		DoubleMatrixCircularBuffer OUT_D;


	protected:

		DoubleMatrix in_tmp;
		DoubleMatrix out;

	};
}