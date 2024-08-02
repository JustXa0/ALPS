#pragma once

#ifndef ENCODER_H
#define ENCODER_H

#include <cuda.h>
#include <cuda_runtime_api.h>
#include <iostream>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>

#include "logger.h"
#include "nvEncodeAPI.h"
#include "conversions.h"

using namespace Microsoft::WRL;

namespace Encoding {

	class cuda {
	
	public:

		cuda();
		~cuda();

		CUdevice device;
		CUcontext context;
	};

	class encoder {

	public:

		encoder();
		encoder(const CUdevice device, const CUcontext context, const Conversions::RectInts);
		bool InitializeNVEncoder(CUcontext context, Conversions::RectInts display);
		bool AllocateBuffers(Conversions::RectInts display, uint32_t numBFrames);
		bool DeallocateBuffers();

		bool StartEncoding();
		bool EndEncoding();

		bool GetStatus();
		~encoder();

	private:

		bool SelectEncoderGUID();
		bool SelectInputFormat();
		//bool CreateInputBuffers(Conversions::RectInts display, uint32_t numBuffers);
		//bool CreateOutputBuffers(Conversions::RectInts display, uint32_t numBuffers);

		// Helper functions
		void writeErrorMessage(std::string, NVENCSTATUS);
		
		bool ready;
		bool encoding;
		void* encodePointer;
		NV_ENCODE_API_FUNCTION_LIST functionList;
		NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS encodeParams;
		GUID encodeGUID;
		HMODULE hLibrary;
		CUdeviceptr inputBuffer;
		CUdeviceptr outputBuffer;
		NV_ENC_REGISTERED_PTR inputPtr;
		NV_ENC_REGISTERED_PTR outputPtr;

	};

	class capture {
		
	public:
	
	};
}

#endif