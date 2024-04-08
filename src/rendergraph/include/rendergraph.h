#pragma once

#include "cgpu/api.h"

namespace RenderGraph
{
	class Recorder
	{
	public:
		int declareTexture(const char* name, uint16_t width, uint16_t height, ECGPUFormat format);
		int declareColorTexture(const char* name, uint16_t width, uint16_t height, ECGPUFormat format);
		int declareDepthTexture(const char* name, uint16_t width, uint16_t height, int depthBits, bool needStencil);
		int importTexture(const char* name, CGPUTextureView texture);
		int addPass();
		void present();

	private:
	};

	class Compiler
	{

	};

	class Executor
	{

	};
}