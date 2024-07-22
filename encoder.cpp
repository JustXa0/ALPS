#include "encoder.h"
#include <vector>

using namespace Microsoft::WRL;

Encoding::cuda::cuda()
{
	device = 0;
	context = NULL;
	
	cuInit(0);

	int deviceCount = 0;
	cuDeviceGetCount(&deviceCount);
	if (deviceCount == 0) {
		Logger::systemLogger.addLog(Logger::error, "No CUDA devices found.");
		return;
	}

	CUresult status = cuCtxCreate(&context, CU_CTX_SCHED_SPIN, device);
	if (status != CUDA_SUCCESS) {
		Logger::systemLogger.addLog(Logger::error, "Failed to create a CUDA context.");
		Logger::systemLogger.addLog(Logger::error, status);
		return;
	}
	Logger::systemLogger.addLog(Logger::info, "Created CUDA context sucessfully.");
}

Encoding::cuda::~cuda()
{
	CUresult result = cuCtxDestroy(context);
	if (result != CUDA_SUCCESS) {
		Logger::systemLogger.addLog(Logger::fatal, "Failed to destroy CUDA context");
	}
	else {
		Logger::systemLogger.addLog(Logger::info, "Successfully destroyed CUDA context.");
	}
}

// Begin of encoder class definitions

Encoding::encoder::encoder(CUdevice device, CUcontext context, Conversions::RectInts display) {

	encodePointer = nullptr;
	encodeParams = {};
	functionList.version = NV_ENCODE_API_FUNCTION_LIST_VER;
	encodeGUID = GUID_NULL;
	ready = true;
	inputBuffer = NULL;
	outputBuffer = NULL;
	inputPtr = nullptr;
	outputPtr = nullptr;

	hLibrary = LoadLibrary(L"nvEncodeAPI64.dll");

	if (hLibrary != NULL) {
		Logger::systemLogger.addLog(Logger::info, "Succesfully loaded nvEncodeAPI library.");

		if (context != NULL) {
			encodeParams.version = NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS_VER;
			encodeParams.deviceType = NV_ENC_DEVICE_TYPE_CUDA;
			encodeParams.device = context;
			encodeParams.reserved = 0;
			encodeParams.apiVersion = NVENCAPI_VERSION;

			typedef NVENCSTATUS(NVENCAPI* NvEncodeAPICreateInstancePtr)(NV_ENCODE_API_FUNCTION_LIST* functionList);
			NvEncodeAPICreateInstancePtr createInstanceFunc;

			createInstanceFunc = (NvEncodeAPICreateInstancePtr)GetProcAddress(hLibrary, "NvEncodeAPICreateInstance");

			if (createInstanceFunc) {
				NVENCSTATUS status = createInstanceFunc(&functionList);
				if (status != NV_ENC_SUCCESS) {
					Logger::systemLogger.addLog(Logger::error, "Failed to create NVEncode function list.");
					Logger::systemLogger.addLog(Logger::error, status);
					ready = false;
				}
			}
		}
		else {
			Logger::systemLogger.addLog(Logger::error, "CUDA context not properly initialized.");
			ready = false;
		}
	}
	else {
		Logger::systemLogger.addLog(Logger::error, "Failed to load nvEncodeAPI library.");
		ready = false;
	}
}

bool Encoding::encoder::InitializeNVEncoder(CUcontext context, Conversions::RectInts display) {

	if (!ready) {
		Logger::systemLogger.addLog(Logger::error, "Cannot call method, functionList not created.");
		return false;
	}
	encodeParams.version = NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS_VER;
	encodeParams.deviceType = NV_ENC_DEVICE_TYPE_CUDA;
	encodeParams.device = context;
	encodeParams.reserved = 0;
	encodeParams.apiVersion = NVENCAPI_VERSION;
	NVENCSTATUS status = functionList.nvEncOpenEncodeSessionEx(&encodeParams, &encodePointer);
	if (status != NV_ENC_SUCCESS) {
		Logger::systemLogger.addLog(Logger::error, "Failed to open encode session.");
		Logger::systemLogger.addLog(Logger::error, functionList.nvEncGetLastErrorString(encodePointer));
		Logger::systemLogger.addLog(Logger::error, status);
		return false;
	}
	
	if (!SelectEncoderGUID()) {
		Logger::systemLogger.addLog(Logger::error, "Aborting initialization of NVEncoder due to error in selection of EncoderGUID.");
		return false;
	}

	if (!SelectInputFormat()) {
		Logger::systemLogger.addLog(Logger::error, "Aborting initalization of NVEncoder due to error in selection of InputFormat.");
		return false;
	}

	Logger::systemLogger.addLog(Logger::info, "Initializing hardware encoder session.");
	NV_ENC_INITIALIZE_PARAMS encParams = {};

	encParams.version = NV_ENC_INITIALIZE_PARAMS_VER;
	encParams.encodeGUID = encodeGUID;
	encParams.encodeWidth = display.right;
	encParams.encodeHeight = display.bottom;
	encParams.frameRateNum = 60;
	encParams.frameRateDen = 1;
	encParams.bufferFormat = NV_ENC_BUFFER_FORMAT_NV12;

	status = functionList.nvEncInitializeEncoder(encodePointer, &encParams);
	if (status != NV_ENC_SUCCESS) {
		writeErrorMessage("Failed to initialize encoder", status);
		return false;
	}

	Logger::systemLogger.addLog(Logger::info, "Initialized hardware encoder session.");
	return true;
}

bool Encoding::encoder::AllocateBuffers(Conversions::RectInts display, uint32_t numBFrames) {
	if (numBFrames != 0) {
		Logger::systemLogger.addLog(Logger::warning, "Currently, there is no support for B Frames in an effort to reduce latency. This may be looked at in the future. Total number of buffers will be 1.");
		numBFrames = 0;
	}

	// Calculate input buffer size
	size_t inputBufferSize = static_cast<size_t>(display.bottom * display.right * 3 / 2);
	CUdeviceptr inputBuffer;

	// Calculate output buffer size (for now this will just be the same size as the input buffer)
	size_t outputBufferSize = inputBufferSize;
	CUdeviceptr outputBuffer;

	cuInit(0);
	CUresult error = cuMemAlloc(&inputBuffer, inputBufferSize);
	if (error != CUDA_SUCCESS) {
		Logger::systemLogger.addLog(Logger::error, "Failed to malloc input buffer.");
		return false;
	}

	
	NV_ENC_REGISTER_RESOURCE resource = {};
	resource.version = NV_ENC_REGISTER_RESOURCE_VER;
	resource.resourceType = NV_ENC_INPUT_RESOURCE_TYPE_CUDADEVICEPTR;
	resource.width = display.right;
	resource.height = display.bottom;
	resource.pitch = (uint32_t)display.right;
	resource.subResourceIndex = 0;
	resource.resourceToRegister = reinterpret_cast<void*>(inputBuffer);
	resource.registeredResource = inputPtr;
	resource.bufferFormat = NV_ENC_BUFFER_FORMAT_NV12;
	resource.bufferUsage = NV_ENC_INPUT_IMAGE;

	NVENCSTATUS status = functionList.nvEncRegisterResource(encodePointer, &resource);
	if (status != NV_ENC_SUCCESS) {
		writeErrorMessage("Failed to register input buffer.", status);
		cuMemFree(inputBuffer);
		return false;
	}

	error = cuMemAlloc(&outputBuffer, outputBufferSize);
	if (error != CUDA_SUCCESS) {
		Logger::systemLogger.addLog(Logger::error, "Failed to malloc output buffer.");
		functionList.nvEncUnregisterResource(encodePointer, inputPtr);
		cuMemFree(inputBuffer);
	}
	resource.resourceToRegister = reinterpret_cast<void*>(outputBuffer);
	resource.registeredResource = outputPtr;
	resource.bufferUsage = NV_ENC_OUTPUT_BITSTREAM;

	status = functionList.nvEncRegisterResource(encodePointer, &resource);
	if (status != NV_ENC_SUCCESS) {
		writeErrorMessage("Failed to register output buffer.", status);
		cuMemFree(outputBuffer);
		functionList.nvEncUnregisterResource(encodePointer, inputPtr);
		cuMemFree(inputBuffer);
		return false;
	}

	Logger::systemLogger.addLog(Logger::info, "Succesfully created and registered CUDA input/output buffers with NVENC API.");
	return true;
}

bool Encoding::encoder::DeallocateBuffers() {
	if (inputBuffer != NULL && inputPtr != nullptr) {
		NVENCSTATUS status = functionList.nvEncUnregisterResource(encodePointer, inputPtr);
		if (status != NV_ENC_SUCCESS) {
			writeErrorMessage("Failed to unregister input buffer.", status);
			return false;
		}
		inputPtr = nullptr;
		cuMemFree(inputBuffer);
		inputBuffer = NULL;
	}
	if (outputBuffer != NULL && outputPtr != nullptr) {
		NVENCSTATUS status = functionList.nvEncUnregisterResource(encodePointer, outputPtr);
		if (status != NV_ENC_SUCCESS) {
			writeErrorMessage("Failed to unregister output buffer.", status);
			return false;
		}
		outputPtr = nullptr;
		cuMemFree(outputBuffer);
		outputBuffer = NULL;
	}
	Logger::systemLogger.addLog(Logger::info, "Successfully deallocated input/output buffers.");
	return true;
}


bool Encoding::encoder::SelectEncoderGUID() {
	uint32_t numGUID;
	NVENCSTATUS status = functionList.nvEncGetEncodeGUIDCount(encodePointer, &numGUID);
	if (status != NV_ENC_SUCCESS) {
		Logger::systemLogger.addLog(Logger::error, "Failed to get GUID count.");
		Logger::systemLogger.addLog(Logger::error, functionList.nvEncGetLastErrorString(encodePointer));
		Logger::systemLogger.addLog(Logger::error, status);
		return false;
	}
	GUID* guidArray = (GUID*)malloc(sizeof(GUID) * numGUID);
	if (guidArray == NULL) {
		Logger::systemLogger.addLog(Logger::error, "Failed to allocate heap space for guidArray");
		return false;
	}
	uint32_t supportedGUIDs;
	status = functionList.nvEncGetEncodeGUIDs(encodePointer, guidArray, numGUID, &supportedGUIDs);
	if (status != NV_ENC_SUCCESS) {
		Logger::systemLogger.addLog(Logger::error, "Failed to get GUID array.");
		Logger::systemLogger.addLog(Logger::error, functionList.nvEncGetLastErrorString(encodePointer));
		Logger::systemLogger.addLog(Logger::error, status);
		free(guidArray);
		return false;
	}

	NV_ENC_CAPS_PARAM capsParam = { NV_ENC_CAPS_PARAM_VER, NV_ENC_CAPS_NUM_MAX_BFRAMES, 0 };
	int capsVal = 0;
	std::vector<int> possibleList;

	for (int i = 0; i < (int)supportedGUIDs; i++) {
		status = functionList.nvEncGetEncodeCaps(encodePointer, guidArray[i], &capsParam, &capsVal);
		if (status != NV_ENC_SUCCESS) {
			Logger::systemLogger.addLog(Logger::error, "Failed to check capabilities of encode presets.");
			Logger::systemLogger.addLog(Logger::error, functionList.nvEncGetLastErrorString(encodePointer));
			Logger::systemLogger.addLog(Logger::error, status);
			free(guidArray);
			return false;
		}
		// If capsVal is greater than 0, then the encoding profile supports B-Frames, which improves compression rates. See the wiki link below for more info:
		// https://en.wikipedia.org/wiki/Video_compression_picture_types#Bi-directional_predicted_(B)_frames/slices_(macroblocks)
		if (capsVal > 0) {
			possibleList.push_back(i);
		}
	}

	capsParam.capsToQuery = NV_ENC_CAPS_SUPPORTED_RATECONTROL_MODES;
	int i = 0;
	while (i < possibleList.size()) {
		status = functionList.nvEncGetEncodeCaps(encodePointer, guidArray[possibleList.at(i)], &capsParam, &capsVal);
		if (status != NV_ENC_SUCCESS) {
			Logger::systemLogger.addLog(Logger::error, "Failed to check capabilities of encode presets.");
			Logger::systemLogger.addLog(Logger::error, functionList.nvEncGetLastErrorString(encodePointer));
			Logger::systemLogger.addLog(Logger::error, status);
			free(guidArray);
			return false;
		}
		// If capsVal is greater than 1, then the encoding profile supports rate controlling, which allows tuning of bitrates
		if (capsVal < 1) {
			possibleList.erase(possibleList.begin() + i);
		}
		else {
			i++;
		}
	}

	capsParam.capsToQuery = NV_ENC_CAPS_SUPPORT_MEONLY_MODE;
	i = 0;
	while (i < possibleList.size()) {
		status = functionList.nvEncGetEncodeCaps(encodePointer, guidArray[possibleList.at(i)], &capsParam, &capsVal);
		if (status != NV_ENC_SUCCESS) {
			Logger::systemLogger.addLog(Logger::error, "Failed to check capabilities of encode presets.");
			Logger::systemLogger.addLog(Logger::error, functionList.nvEncGetLastErrorString(encodePointer));
			Logger::systemLogger.addLog(Logger::error, status);
			free(guidArray);
			return false;
		}
		// If capsVal is equal to 2, then the encoding profile supports only motion estimation encoding for all types of frames, including B-Frames. 
		// This mode only encodes the motion captured, reduing total packet size? Needs further research.
		if (capsVal == 2) {
			possibleList.erase(possibleList.begin() + i);
		}
		else {
			i++;
		}
	}

	//capsParam.capsToQuery = NV_ENC_CAPS_SUPPORT_LOOKAHEAD;
	//i = 0;
	//while (i < possibleList.size()) {
	//	status = functionList.nvEncGetEncodeCaps(encodePointer, guidArray[possibleList.at(i)], &capsParam, &capsVal);
	//	if (status != NV_ENC_SUCCESS) {
	//		Logger::systemLogger.addLog(Logger::error, "Failed to check capabilities of encode presets.");
	//		Logger::systemLogger.addLog(Logger::error, functionList.nvEncGetLastErrorString(encodePointer));
	//		Logger::systemLogger.addLog(Logger::error, status);
	//		free(guidArray);
	//		return false;
	//	}
	//	// If capsVal is equal to 1, then the encoding profile supports lookahead encoding, which can predict future frames and prestart encoding processes?
	//	// Needs more research to check this claim^. 
	//	if (capsVal == 1) {
	//		possibleList.erase(possibleList.begin() + i);
	//	}
	//	else {
	//		i++;
	//	}
	//}

	//capsParam.capsToQuery = NV_ENC_CAPS_SUPPORT_TEMPORAL_AQ;
	//i = 0;
	//while (i < possibleList.size()) {
	//	status = functionList.nvEncGetEncodeCaps(encodePointer, guidArray[possibleList.at(i)], &capsParam, &capsVal);
	//	if (status != NV_ENC_SUCCESS) {
	//		Logger::systemLogger.addLog(Logger::error, "Failed to check capabilities of encode presets.");
	//		Logger::systemLogger.addLog(Logger::error, functionList.nvEncGetLastErrorString(encodePointer));
	//		Logger::systemLogger.addLog(Logger::error, status);
	//		free(guidArray);
	//		return false;
	//	}
	//	// If capsVal is equal to 1, thent he encoding profile supports temporal AQ, which improves video quality at lower bitrates.
	//	// Check the wiki to learn more: https://docs.nvidia.com/video-technologies/video-codec-sdk/12.1/nvenc-video-encoder-api-prog-guide/index.html#temporal-aq
	//	if (capsVal == 1) {
	//		possibleList.erase(possibleList.begin() + i);
	//	}
	//	else {
	//		i++;
	//	}
	//}

	// If there are remaining encoding profiles after all filters, pick based off of higher B-Frame support?
	capsParam.capsToQuery = NV_ENC_CAPS_NUM_MAX_BFRAMES;
	i = 0;
	int max = 0;
	int maxPos = i;
	while (i < possibleList.size()) {
		status = functionList.nvEncGetEncodeCaps(encodePointer, guidArray[possibleList.at(i)], &capsParam, &capsVal);
		if (status != NV_ENC_SUCCESS) {
			Logger::systemLogger.addLog(Logger::error, "Failed to check capabilities of encode presets.");
			Logger::systemLogger.addLog(Logger::error, functionList.nvEncGetLastErrorString);
			Logger::systemLogger.addLog(Logger::error, status);
			free(guidArray);
			return false;
		}
		if (capsVal > max) {
			max = capsVal;
			maxPos = i;
		}
		i++;
	}

	encodeGUID = guidArray[maxPos];
	free(guidArray);
	Logger::systemLogger.addLog(Logger::info, "Successfully selected Encoder GUID and freed all dynamic memory.");
	return true;
}

bool Encoding::encoder::SelectInputFormat() {
	uint32_t formatCount;
	NVENCSTATUS status = functionList.nvEncGetInputFormatCount(encodePointer, encodeGUID, &formatCount);
	if (status != NV_ENC_SUCCESS) {
		writeErrorMessage("Failed to obtain input format count.", status);
		return false;
	}
	NV_ENC_BUFFER_FORMAT* buffer = (NV_ENC_BUFFER_FORMAT*) malloc(sizeof(NV_ENC_BUFFER_FORMAT) * formatCount);
	if (buffer == NULL) {
		Logger::systemLogger.addLog(Logger::error, "Failed to allocated buffer size for buffer formats.");
		return false;
	}

	status = functionList.nvEncGetInputFormats(encodePointer, encodeGUID, buffer, formatCount, &formatCount);
	if (status != NV_ENC_SUCCESS) {
		writeErrorMessage("Failed to obtain input format buffer.", status);
		free(buffer);
		return false;
	}
	bool supported = false;
	for (uint32_t i = 0; i < formatCount; i++) {
		if (buffer[i] & NV_ENC_BUFFER_FORMAT_NV12) {
			supported = true;
			break;
		}
	}

	free(buffer);

	if (supported) {
		Logger::systemLogger.addLog(Logger::info, "NV12 Format supported, continuing");
		return true;
	}
	Logger::systemLogger.addLog(Logger::error, "NV12 Format not supported. Closing all processes");
	return false;
}

//bool Encoding::encoder::CreateInputBuffers(Conversions::RectInts display, uint32_t numBuffers);


void Encoding::encoder::writeErrorMessage(std::string message, NVENCSTATUS status) {
	Logger::systemLogger.addLog(Logger::error, message);
	Logger::systemLogger.addLog(Logger::error, status);
	Logger::systemLogger.addLog(Logger::error, functionList.nvEncGetLastErrorString(encodePointer));
}

Encoding::encoder::~encoder() {

	DeallocateBuffers();
	// For some reason this doesn't work and gives a memory exception? Not sure why..
	/*if (encodepointer != nullptr) {
		nvencstatus status = functionlist.nvencdestroyencoder(encodepointer);
		if (status != nv_enc_success) {
			writeerrormessage("failed to destroy encoder.", status);
		}
	}*/
	if (hLibrary != NULL) {
		FreeLibrary(hLibrary);
		Logger::systemLogger.addLog(Logger::info, "Successfully deloaded nvEncodeAPI library.");
	}
	
	Logger::systemLogger.addLog(Logger::info, "Succesfully closed encoder class.");
}

// Beginning of capture class definitions

