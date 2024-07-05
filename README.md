# NI-USB6000-DAQ
1. 该说明主要针对WIndows平台Matlab Simulink
2. Matlab2024a版本的Simulink Desktop Real-Time模块不支持NI USB系列

## 使用Data Acquisition Toolbox

安装对应Toolbox及驱动即可使用，但在Real-Time Synchronization中高采样频率会影响电机通讯效率

## 基于DAQmx二次开发

参考.\src中的c文件使用C-mex S-function可以实现较低开销的高频率采样

具体函数接口定义请参考NI官网 [NI-DAQmx C Reference Help - NI](https://www.ni.com/docs/zh-CN/bundle/ni-daqmx-c-api-ref/page/cdaqmx/help_file_title.html)

```shell
mex Callback_SFunction.c -I"..\include" -L"..\lib64\msvc" -lNIDAQmx
mex Read_SFunction.c -I"..\include" -L"..\lib64\msvc" -lNIDAQmx
```

