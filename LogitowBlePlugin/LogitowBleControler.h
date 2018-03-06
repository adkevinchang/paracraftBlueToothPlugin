#pragma once
#include "malloc.h"
#include <stdlib.h>
#include <stdio.h>  
#include <windows.h>  
#include <setupapi.h>  
#include <devguid.h>  
#include <regstr.h>  
#include <bthdef.h>  
#include <iostream>
#include <string>
#include <Bluetoothleapis.h>  
#pragma comment(lib, "SetupAPI")  
#pragma comment(lib, "BluetoothApis.lib")  

#define TO_DATA_DEVICE_UUID "{69400001-b5a3-f393-e0a9-e50e24dcca99}" 
#define TO_WRITE_DATA_CHAR_UUID "{69400003-b5a3-f393-e0a9-e50e24dcca99}" 
#define TO_MODE_DEVICE_UUID "{7f510004-b5a3-f393-e0a9-e50e24dcca9e}" 
#define TO_WRITE_MODE_CHAR_UUID "{7f510005-b5a3-f393-e0a9-e50e24dcca9e}"
#define DEVICE_NAME "LOGITOW"

namespace LogitowApi
{
	typedef void(*BleCallBack)(void);

	class LogitowBleControler
	{
	public:

		LogitowBleControler();
		~LogitowBleControler();
		static LogitowBleControler* GetInstance();
		void ConnectDevice();
		void ConnectModelDevice();
		int ScanBLEDevices();
		void GetDevicePower();
		void ShowLog(const char* strOutputString, ...);
		//void connect();
		//void disconnect();
		//void close();
		//void setCharacteristicNotification();
		//void readCharacteristic();
		//void writeCharacteristic();
		bool connected; 
		std::string currname;
		std::string curraddress;//绑定功能待定
		std::string currdeviceId;
		std::string prebledata;//同时两个相同的数据不处理

	protected:
		/**
		*
		*/
		//bool OptimizeRectFaceModelInPlace(XFile::Scene& scene);
		//void RemoveUntexturedFaces(XFile::Scene& scene);
	protected:
		// static model files 
		//std::map<std::string, XFile::Scene*> m_Xfiles;
	};
}
