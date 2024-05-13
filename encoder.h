#pragma once

#ifndef ENCODER_H
#define ENCODER_H

#include <cuda.h>
#include <cuda_runtime_api.h>
#include <iostream>
#include "logger.h"
#include "nvEncodeAPI.h"

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

		encoder(const CUdevice device, const CUcontext context);
		~encoder();

	private:

		bool InitializeNVEncoder(CUcontext context);
		bool SelectEncoderGUID();

		void* encodePointer;
		NV_ENCODE_API_FUNCTION_LIST functionList;
		NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS encodeParams;
		GUID encodeGUID;
		HMODULE hLibrary;

	};
}

#endif