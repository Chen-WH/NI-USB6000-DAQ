#define S_FUNCTION_NAME  Read_SFunction
#define S_FUNCTION_LEVEL 2

#include "simstruc.h"
#include "NIDAQmx.h"

TaskHandle taskHandle = 0;

static void mdlInitializeSizes(SimStruct *S)
{
    ssSetNumSFcnParams(S, 0);  // No parameters
    ssSetNumContStates(S, 0);
    ssSetNumDiscStates(S, 0);

    if (!ssSetNumInputPorts(S, 0)) return;
    if (!ssSetNumOutputPorts(S, 1)) return;
    ssSetOutputPortWidth(S, 0, 4); // 4 channels

    ssSetNumSampleTimes(S, 1);
    ssSetNumRWork(S, 0);
    ssSetNumIWork(S, 0);
    ssSetNumPWork(S, 0);
    ssSetNumModes(S, 0);
    ssSetNumNonsampledZCs(S, 0);

    ssSetOptions(S, 0);
}

static void mdlInitializeSampleTimes(SimStruct *S)
{
    ssSetSampleTime(S, 0, 0.001); // 0.001 seconds
    ssSetOffsetTime(S, 0, 0.0);
}

#define MDL_START
static void mdlStart(SimStruct *S)
{
    int32 error = 0;
    char errBuff[2048] = {'\0'};

    error = DAQmxCreateTask("", &taskHandle);
    if (DAQmxFailed(error)) goto Error;

    error = DAQmxCreateAIVoltageChan(taskHandle, "Dev1/ai1,Dev1/ai2,Dev1/ai3,Dev1/ai4", "", DAQmx_Val_Cfg_Default, -10.0, 10.0, DAQmx_Val_Volts, NULL);
    if (DAQmxFailed(error)) goto Error;

    error = DAQmxCfgSampClkTiming(taskHandle, "", 5000.0, DAQmx_Val_Rising, DAQmx_Val_ContSamps, 500);
    if (DAQmxFailed(error)) goto Error;

    error = DAQmxStartTask(taskHandle);
    if (DAQmxFailed(error)) goto Error;

    return;

Error:
    if (DAQmxFailed(error)) {
        DAQmxGetExtendedErrorInfo(errBuff, 2048);
        ssSetErrorStatus(S, errBuff);
    }
}

static void mdlOutputs(SimStruct *S, int_T tid)
{
    real_T *y = ssGetOutputPortRealSignal(S, 0);
    int32 error = 0;
    int32 read;
    float64 data[20]; // 4 channels * 5 samples each
    char errBuff[2048] = {'\0'};

    error = DAQmxReadAnalogF64(taskHandle, -1, 10.0, DAQmx_Val_GroupByScanNumber, data, 20, &read, NULL);
    if (DAQmxFailed(error)) goto Error;

    for (int i = 0; i < 4; ++i) {
        y[i] = (data[i] + data[i+4] + data[i+8] + data[i+12] + data[i+16])/5;
    }

    return;

Error:
    if (DAQmxFailed(error)) {
        DAQmxGetExtendedErrorInfo(errBuff, 2048);
        ssSetErrorStatus(S, errBuff);
    }
}

static void mdlTerminate(SimStruct *S)
{
    if (taskHandle != 0) {
        DAQmxStopTask(taskHandle);
        DAQmxClearTask(taskHandle);
    }
}

#ifdef  MATLAB_MEX_FILE
#include "simulink.c"      /* MEX-file interface mechanism */
#else
#include "cg_sfun.h"       /* Code generation registration function */
#endif