#include "encoder.h"
#include <vector>

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

Encoding::encoder::encoder(CUdevice device, CUcontext context) {

	encodePointer = nullptr;
	encodeParams = {};
	functionList.version = NV_ENCODE_API_FUNCTION_LIST_VER;

	hLibrary = LoadLibrary(L"nvEncodeAPI64.dll");

	if (hLibrary != NULL) {
		Logger::systemLogger.addLog(Logger::info, "Succesfully loaded nvEncodeAPI library.");

		if (context != NULL) {
			encodeParams.version = NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS_VER;
			encodeParams.deviceType = NV_ENC_DEVICE_TYPE_CUDA;
			encodeParams.device = context;
			encodeParams.reserved = 0;
			encodeParams.apiVersion = NVENCAPI_VERSION;
			if (Encoding::encoder::InitializeNVEncoder(context)) {
				Encoding::encoder::SelectEncoderGUID();
			}
			else {
				Logger::systemLogger.addLog(Logger::error, "Unable to initialize NV Encoder");
			}
		}
		else {
			Logger::systemLogger.addLog(Logger::error, "CUDA context not properly initialized.");
		}
	}
	else {
		Logger::systemLogger.addLog(Logger::error, "Failed to load nvEncodeAPI library.");
	}

}

bool Encoding::encoder::InitializeNVEncoder(CUcontext context) {

	typedef NVENCSTATUS(NVENCAPI* NvEncodeAPICreateInstancePtr)(NV_ENCODE_API_FUNCTION_LIST* functionList);
	NvEncodeAPICreateInstancePtr createInstanceFunc;

	createInstanceFunc = (NvEncodeAPICreateInstancePtr)GetProcAddress(hLibrary, "NvEncodeAPICreateInstance");

	if (!createInstanceFunc) {
		Logger::systemLogger.addLog(Logger::error, "Failed to retrieve NvEncodeAPICreateInstance function.");
		return false;
	}
	
	NVENCSTATUS status = createInstanceFunc(&functionList);

	if (status != NV_ENC_SUCCESS) {
		Logger::systemLogger.addLog(Logger::error, "Failed to create NVEncode function list.");
		Logger::systemLogger.addLog(Logger::error, status);
		return false;
	}
	encodeParams.version = NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS_VER;
	encodeParams.deviceType = NV_ENC_DEVICE_TYPE_CUDA;
	encodeParams.device = context;
	encodeParams.reserved = 0;
	encodeParams.apiVersion = NVENCAPI_VERSION;
	status = functionList.nvEncOpenEncodeSessionEx(&encodeParams, &encodePointer);
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
			Logger::systemLogger.addLog(Logger::error, functionList.nvEncGetLastErrorString);
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
			Logger::systemLogger.addLog(Logger::error, functionList.nvEncGetLastErrorString);
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
	//		Logger::systemLogger.addLog(Logger::error, functionList.nvEncGetLastErrorString);
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
	//		Logger::systemLogger.addLog(Logger::error, functionList.nvEncGetLastErrorString);
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

Encoding::encoder::~encoder() {

	if (hLibrary != NULL) {
		FreeLibrary(hLibrary);
		Logger::systemLogger.addLog(Logger::info, "Successfully deloaded nvEncodeAPI library.");
	}
	Logger::systemLogger.addLog(Logger::info, "Succesfully closed encoder class.");
}
