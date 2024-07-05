#define S_FUNCTION_NAME  Callback_SFunction
#define S_FUNCTION_LEVEL 2

#include "simstruc.h"
#include <NIDAQmx.h>

#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) return; else

// Task handle for DAQmx
TaskHandle taskHandle = 0;
int32 error = 0;
char errBuff[2048] = {'\0'};
float64 data[20];  // 4 channels * 5 samples = 20
int32 read;
int totalRead = 0;

// Function to handle DAQmx errors
void handleError() {
    if (DAQmxFailed(error)) {
        DAQmxGetExtendedErrorInfo(errBuff, 2048);
        if (taskHandle != 0) {
            DAQmxStopTask(taskHandle);
            DAQmxClearTask(taskHandle);
        }
        ssPrintf("DAQmx Error: %s\n", errBuff);
    }
}

// Function to be called every N samples
int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData) {
    DAQmxErrChk(DAQmxReadAnalogF64(taskHandle, 5, 10.0, DAQmx_Val_GroupByScanNumber, data, 20, &read, NULL));
    if (read > 0) {
        totalRead += read;
    }
    return 0;

    Error:
    handleError();
    return -1;
}

// Initialization sizes
static void mdlInitializeSizes(SimStruct *S) {
    ssSetNumSFcnParams(S, 0);  // Number of expected parameters
    ssSetNumContStates(S, 0);
    ssSetNumDiscStates(S, 0);

    if (!ssSetNumInputPorts(S, 0)) return;
    if (!ssSetNumOutputPorts(S, 1)) return;
    ssSetOutputPortWidth(S, 0, 4);  // 4 channels * 5 samples = 20

    ssSetNumSampleTimes(S, 1);
    ssSetOptions(S, SS_OPTION_EXCEPTION_FREE_CODE);
}

// Initialization sample times
static void mdlInitializeSampleTimes(SimStruct *S) {
    ssSetSampleTime(S, 0, CONTINUOUS_SAMPLE_TIME);
    ssSetOffsetTime(S, 0, 0.0);
}

// Function to be executed at the start of the simulation
#define MDL_START
static void mdlStart(SimStruct *S) {
    DAQmxErrChk(DAQmxCreateTask("", &taskHandle));
    DAQmxErrChk(DAQmxCreateAIVoltageChan(taskHandle, "Dev1/ai1,Dev1/ai2,Dev1/ai3,Dev1/ai4", "", DAQmx_Val_Cfg_Default, -10.0, 10.0, DAQmx_Val_Volts, NULL));
    DAQmxErrChk(DAQmxCfgSampClkTiming(taskHandle, "", 5000.0, DAQmx_Val_Rising, DAQmx_Val_ContSamps, 500));
    DAQmxErrChk(DAQmxRegisterEveryNSamplesEvent(taskHandle, DAQmx_Val_Acquired_Into_Buffer, 5, 0, EveryNCallback, NULL));
    DAQmxErrChk(DAQmxStartTask(taskHandle));
}

// Function to be executed at each time step
static void mdlOutputs(SimStruct *S, int_T tid) {
    real_T *y = ssGetOutputPortRealSignal(S, 0);
    for (int i = 0; i < 4; ++i) {
        y[i] = (data[i] + data[i+4] + data[i+8] + data[i+12] + data[i+16])/5;
    }
}

// Function to handle termination
static void mdlTerminate(SimStruct *S) {
    if (taskHandle != 0) {
        DAQmxStopTask(taskHandle);
        DAQmxClearTask(taskHandle);
    }
    handleError();
}

#ifdef  MATLAB_MEX_FILE
#include "simulink.c"
#else
#include "cg_sfun.h"
#endif