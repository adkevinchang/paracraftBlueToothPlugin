//-----------------------------------------------------------------------------
// Class:	LogitowBleControler
// Authors:	kevin
// Emails:	kevin@paracra.com
// Company: paracra
// Date:	2017.12.1
//-----------------------------------------------------------------------------
#pragma warning (disable: 4068)
#include "LogitowBleControler.h"
#include <iostream>
#include <string>
#include "atlstr.h"
#include <stdio.h>
#include <tchar.h>
#include <setupapi.h>
#include <devguid.h>
#include <regstr.h>
#include <bthdef.h>
#include <bluetoothleapis.h>
#include <bthledef.h>

#include <iostream>
#include <sstream>
#include <string>
#include <locale>
#include "btle_helpers.h"
#include <atlbase.h>
#include <atlconv.h>
#include "event\Event.hpp"
#include "event\EventBus.hpp"
#include "BleEvent.hpp"

using namespace LogitowApi;
using namespace std;
using namespace btle;

namespace util
{
	std::string to_narrow(const wchar_t *s, char def = '?', const std::locale& loc = std::locale())
	{
		std::ostringstream stm;
		while (*s != L'\0')
		{
			stm << std::use_facet< std::ctype<wchar_t> >(loc).narrow(*s++, def);
		}
		return stm.str();
	}
}

//
//蓝牙数据通知处理函数
//_In_ BTH_LE_GATT_EVENT_TYPE EventType,
//_In_ PVOID EventOutParameter,
//_In_opt_ PVOID Context
//

/**
* \brief Demo class showing off some functionality of the EventBus
*/
class EventBusCenter : public Object
{

public:
	EventBusCenter() {

	}

	virtual ~EventBusCenter() { }

	string dec2hex(int i) //将int转成16进制字符串  
	{
		stringstream ioss; //定义字符串流  
		string s_temp; //存放转化后字符  
		ioss << setiosflags(ios::uppercase) << hex << i; //以十六制(大写)形式输出  
														 //ioss << resetiosflags(ios::uppercase) << hex << i; //以十六制(小写)形式输出//取消大写的设置  
		ioss >> s_temp;
		return s_temp;
	}

	/**
	*广播蓝牙积木链接时数据
	* 
	*/
	void BroadcastBleData(int tnum[],int len) {
		//整数转16进制字符串  000000031004B8
		unsigned char* cbuf = (unsigned char*)"";
		std::string keepstr;

		for (int i = 0; i<len; i++) {
			printf("%0x",tnum[i]);
			std::string str = dec2hex(tnum[i]);
			if (strlen(str.c_str())<2)
			{
				str = "0"+ str;
			}
			LogitowApi::LogitowBleControler::GetInstance()->ShowLog("bledata:%d \r\n",tnum[i]);
			keepstr += str;
		}
		if (strcmp(LogitowApi::LogitowBleControler::GetInstance()->prebledata.c_str(), keepstr.c_str()) != 0)
		{
			BleEvent evt(this, keepstr, BLE_NOTICE_EVENT_TYPE);
			EventBus::FireEvent(evt);
			LogitowApi::LogitowBleControler::GetInstance()->prebledata = keepstr;
		}

	}

	/**
	*广播蓝牙状态 0未连接1已连接
	*
	*/
	void BroadcastBleStatus(int sta) {
		//整数转16进制字符串
		std::string keepstr = "0";
		if (sta == 0)
		{
			keepstr = "0";
		}
		else
		{
			keepstr = "1";
		}
		BleEvent evt(this, keepstr, BLE_STATUS_EVENT_TYPE);
		EventBus::FireEvent(evt);
	}




private:

};

VOID CALLBACK onValueChangedCallback(BTH_LE_GATT_EVENT_TYPE type,PVOID param,PVOID context)
{
	//LogitowApi::LogitowBleControler::OutputDebugPrintf("notification obtained ");
	PBLUETOOTH_GATT_VALUE_CHANGED_EVENT ValueChangedEventParameters = (PBLUETOOTH_GATT_VALUE_CHANGED_EVENT)param;

	EventBusCenter blecenter;

	HRESULT hr;
	if (0 == ValueChangedEventParameters->CharacteristicValue->DataSize) {
		hr = E_FAIL;//失败
	}
	else {
		int pnum[100];//成功并且解析数据
		int indexnum = ValueChangedEventParameters->CharacteristicValue->DataSize;

		for(int i=0; i<indexnum;i++) {
			pnum[i] = ValueChangedEventParameters->CharacteristicValue->Data[i];
		}

		blecenter.BroadcastBleData(pnum,indexnum);
		// if the first bit is set, then the value is the next 2 bytes.  If it is clear, the value is in the next byte
		//The Heart Rate Value Format bit (bit 0 of the Flags field) indicates if the data format of 
		//the Heart Rate Measurement Value field is in a format of UINT8 or UINT16. 
		//When the Heart Rate Value format is sent in a UINT8 format, the Heart Rate Value 
		//Format bit shall be set to 0. When the Heart Rate Value format is sent in a UINT16 
		//format, the Heart Rate Value Format bit shall be set to 1
		//from this PDF https://www.bluetooth.org/docman/handlers/downloaddoc.ashx?doc_id=239866
		//unsigned heart_rate;
		//if (0x01 == (ValueChangedEventParameters->CharacteristicValue->Data[0] & 0x01)) {
			//heart_rate = ValueChangedEventParameters->CharacteristicValue->Data[1] * 256 + ValueChangedEventParameters->CharacteristicValue->Data[2];
		//}
		//else {
		//	heart_rate = ValueChangedEventParameters->CharacteristicValue->Data[1];
		//}
		//printf("%d\n", heart_rate);
		//std::string str = std::to_string(heart_rate);
		//demo.Demo1(str.c_str());
	}
}

//蓝牙处理函数对象
HANDLE GetBLEHandle(__in GUID svcGuid)
{
	HDEVINFO hDevInfo;
	SP_DEVICE_INTERFACE_DATA devIntfData;
	SP_DEVINFO_DATA devInfoData;
	GUID deviceGUID = svcGuid;
	HANDLE hFind = NULL;

	hDevInfo = SetupDiGetClassDevs(&deviceGUID, NULL, NULL, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);

	if (hDevInfo == INVALID_HANDLE_VALUE) return NULL;

	devIntfData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
	devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

	for (DWORD i = 0; SetupDiEnumDeviceInterfaces(hDevInfo, NULL, &deviceGUID, i, &devIntfData); i++)
	{
		SP_DEVICE_INTERFACE_DETAIL_DATA DeviceInterfaceDetailData;

		DeviceInterfaceDetailData.cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

		DWORD size = 0;

		if (!SetupDiGetDeviceInterfaceDetail(hDevInfo, &devIntfData, NULL, 0, &size, 0))
		{
			int err = GetLastError();

			if (err == ERROR_NO_MORE_ITEMS) break;

			PSP_DEVICE_INTERFACE_DETAIL_DATA pInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)GlobalAlloc(GPTR, size);

			pInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

			if (!SetupDiGetDeviceInterfaceDetail(hDevInfo, &devIntfData, pInterfaceDetailData, size, &size, &devInfoData))
				break;

			hFind = CreateFile(
				pInterfaceDetailData->DevicePath,
				GENERIC_WRITE | GENERIC_READ,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL,
				OPEN_EXISTING,
				0,
				NULL);

			GlobalFree(pInterfaceDetailData);
		}
	}

	SetupDiDestroyDeviceInfoList(hDevInfo);
	return hFind;
}

//打印函数如何通用
void OutputDebugPrintf(const char* strOutputString, ...)
{
	char strBuffer[4096] = { 0 };
	va_list vlArgs;
	va_start(vlArgs, strOutputString);
	_vsnprintf_s(strBuffer, sizeof(strBuffer) - 1, strOutputString, vlArgs);
	//vsprintf(strBuffer,strOutputString,vlArgs);
	va_end(vlArgs);
	OutputDebugString(CA2W(strBuffer));
}

LogitowApi::LogitowBleControler::LogitowBleControler()
	:connected(false),prebledata("")
{
	OutputDebugPrintf("LogitowBleControler Init.\n");
}

LogitowApi::LogitowBleControler::~LogitowBleControler()
{
	OutputDebugPrintf("LogitowBleControler destroy.\n");
}

/*
* Comments:功能描述
* Param hehe * hehe:参数
* @Return bool:成功返回true，否则返回false
*/
LogitowBleControler* LogitowApi::LogitowBleControler::GetInstance()
{
	static LogitowBleControler s_singleton;
	return &s_singleton;
}


//
//Comments:搜索蓝牙设备
//Param
//@void
//

void LogitowApi::LogitowBleControler::ConnectDevice()
{
	OutputDebugPrintf("SearchBleDevice start.\n");
	GUID svcGuid;
	CLSIDFromString(TEXT(TO_DATA_DEVICE_UUID), &svcGuid);

	HANDLE hLEDevice = GetBLEHandle(svcGuid);

	//printf("Find Handle: 0x%x \r\n", hLEDevice);

	OutputDebugPrintf("Find Handle: 0x%x \r\n", hLEDevice);
	//-------------------------------------------
	// 注册数据处理的事件  /连接处理/断开连接处理/数据通讯处理

	//-------------------------------------------  
	//设备服务缓冲区数量
	USHORT serviceBufferCount;

	HRESULT hr = BluetoothGATTGetServices(
		hLEDevice,
		0,
		NULL,
		&serviceBufferCount,
		BLUETOOTH_GATT_FLAG_NONE);

	if (hr != HRESULT_FROM_WIN32(ERROR_MORE_DATA)) {
		OutputDebugPrintf("BluetoothGATTGetServices - Need Alloc Buffer Size: %d \r\n", serviceBufferCount);

	}

	//处理服务的缓存区对象。
	PBTH_LE_GATT_SERVICE pServiceBuffer = (PBTH_LE_GATT_SERVICE)
		malloc(sizeof(BTH_LE_GATT_SERVICE) * serviceBufferCount);

	if (pServiceBuffer == NULL) {
		OutputDebugPrintf("pServiceBuffer out of memory\r\n");
	}
	else {
		RtlZeroMemory(pServiceBuffer,
			sizeof(BTH_LE_GATT_SERVICE) * serviceBufferCount);
	}

	//搜索已连接的设备个数
	USHORT numServices;
	hr = BluetoothGATTGetServices(
		hLEDevice,
		serviceBufferCount,
		pServiceBuffer,
		&numServices,
		BLUETOOTH_GATT_FLAG_NONE);

	if (hr == S_OK) {
		OutputDebugPrintf("BluetoothGATTGetServices - Service Count %d \r\n", numServices);
	}

	//-------------------------------------------
	//获取特征体 并输出特征体缓存区大小
	USHORT charBufferSize;
	hr = BluetoothGATTGetCharacteristics(
		hLEDevice,
		pServiceBuffer,
		0,
		NULL,
		&charBufferSize,
		BLUETOOTH_GATT_FLAG_NONE);
	if (HRESULT_FROM_WIN32(ERROR_MORE_DATA) == hr) {
		OutputDebugPrintf("BluetoothGATTGetCharacteristics - Buffer Size %d \r\n", charBufferSize);
	}
	
	//处理特征体的缓存区对象。
	PBTH_LE_GATT_CHARACTERISTIC pCharBuffer;
	if (charBufferSize > 0)
	{
		pCharBuffer = (PBTH_LE_GATT_CHARACTERISTIC)
			malloc(charBufferSize * sizeof(BTH_LE_GATT_CHARACTERISTIC));

		if (pCharBuffer == NULL) {
			OutputDebugPrintf("pCharBuffer out of memory\r\n");
		}
		else {
			RtlZeroMemory(pCharBuffer,
				charBufferSize * sizeof(BTH_LE_GATT_CHARACTERISTIC));
		}
		//获取该服务下面的所有特征体数量
		USHORT numChars;
		hr = BluetoothGATTGetCharacteristics(
			hLEDevice,
			pServiceBuffer,
			charBufferSize,
			pCharBuffer,
			&numChars,
			BLUETOOTH_GATT_FLAG_NONE);

		if (hr == S_OK) {
			OutputDebugPrintf("BluetoothGATTGetCharacteristics - Actual Data %d  %d \r\n", numChars, charBufferSize);
		}

		if (numChars != charBufferSize) {
			OutputDebugPrintf("buffer size and buffer size actual size mismatch\r\n");
		}
	}

	// 激活数据服务的特征体的描述对象通知
	// Step 4: now get the list of descriptors. note how the pCharBuffer is required from step 3
	// descriptors are required as we descriptors that are notification based will have to be written
	// once IsSubcribeToNotification set to true, we set the appropriate callback function
	// need for setting descriptors for notification according to
	//http://social.msdn.microsoft.com/Forums/en-US/11d3a7ce-182b-4190-bf9d-64fefc3328d9/windows-bluetooth-le-apis-event-callbacks?forum=wdk
	PBTH_LE_GATT_CHARACTERISTIC currGattChar;
	for (int ii = 0; ii <charBufferSize; ii++) {
		currGattChar = &pCharBuffer[ii];
		USHORT charValueDataSize;
		PBTH_LE_GATT_CHARACTERISTIC_VALUE pCharValueBuffer;

		OutputDebugPrintf("遍历体征 %s \r\n", BTH_LE_UUID_TO_STRING(currGattChar->CharacteristicUuid).c_str());
		///////////////////////////////////////////////////////////////////////////
		// Determine Descriptor Buffer Size
		////////////////////////////////////////////////////////////////////////////
		USHORT descriptorBufferSize;
		hr = BluetoothGATTGetDescriptors(
			hLEDevice,
			currGattChar,
			0,
			NULL,
			&descriptorBufferSize,
			BLUETOOTH_GATT_FLAG_NONE);

		if (HRESULT_FROM_WIN32(ERROR_MORE_DATA) != hr) {
			OutputDebugPrintf("BluetoothGATTGetDescriptors - Buffer Size %d\n", hr);
		}

		PBTH_LE_GATT_DESCRIPTOR pDescriptorBuffer;
		if (descriptorBufferSize > 0) {
			pDescriptorBuffer = (PBTH_LE_GATT_DESCRIPTOR)
				malloc(descriptorBufferSize
					* sizeof(BTH_LE_GATT_DESCRIPTOR));

			if (NULL == pDescriptorBuffer) {
				OutputDebugPrintf("pDescriptorBuffer out of memory\n");
			}
			else {
				RtlZeroMemory(pDescriptorBuffer, descriptorBufferSize);
			}

			////////////////////////////////////////////////////////////////////////////
			// Retrieve Descriptors
			////////////////////////////////////////////////////////////////////////////

			USHORT numDescriptors;
			hr = BluetoothGATTGetDescriptors(
				hLEDevice,
				currGattChar,
				descriptorBufferSize,
				pDescriptorBuffer,
				&numDescriptors,
				BLUETOOTH_GATT_FLAG_NONE);

			if (S_OK != hr) {
				OutputDebugPrintf("BluetoothGATTGetDescriptors - Actual Data %d\n", hr);
			}

			if (numDescriptors != descriptorBufferSize) {
				OutputDebugPrintf("buffer size and buffer size actual size mismatch\n");
			}

			for (int kk = 0; kk<numDescriptors; kk++) {
				PBTH_LE_GATT_DESCRIPTOR  currGattDescriptor = &pDescriptorBuffer[kk];
				////////////////////////////////////////////////////////////////////////////
				// Determine Descriptor Value Buffer Size
				////////////////////////////////////////////////////////////////////////////

				OutputDebugPrintf("遍历描述对象 %s %d \r\n", BTH_LE_UUID_TO_STRING(currGattDescriptor->DescriptorUuid).c_str(),numDescriptors);

				USHORT descValueDataSize;
				hr = BluetoothGATTGetDescriptorValue(
					hLEDevice,
					currGattDescriptor,
					0,
					NULL,
					&descValueDataSize,
					BLUETOOTH_GATT_FLAG_NONE);

				if (HRESULT_FROM_WIN32(ERROR_MORE_DATA) != hr)
				{
					OutputDebugPrintf("BluetoothGATTGetDescriptorValue - Buffer Size %d\n", hr);
				}

				PBTH_LE_GATT_DESCRIPTOR_VALUE pDescValueBuffer = (PBTH_LE_GATT_DESCRIPTOR_VALUE)malloc(descValueDataSize);

				if (NULL == pDescValueBuffer)
				{
					OutputDebugPrintf("pDescValueBuffer out of memory\n");
				}
				else 
				{
					RtlZeroMemory(pDescValueBuffer, descValueDataSize);
				}

				////////////////////////////////////////////////////////////////////////////
				// Retrieve the Descriptor Value
				////////////////////////////////////////////////////////////////////////////

				hr = BluetoothGATTGetDescriptorValue(
					hLEDevice,
					currGattDescriptor,
					(ULONG)descValueDataSize,
					pDescValueBuffer,
					NULL,
					BLUETOOTH_GATT_FLAG_NONE);
				if (S_OK != hr) {
					OutputDebugPrintf("BluetoothGATTGetDescriptorValue - Actual Data %d\n", hr);
				}
				// you may also get a descriptor that is read (and not notify) andi am guessing the attribute handle is out of limits
				// we set all descriptors that are notifiable to notify us via IsSubstcibeToNotification
				if (currGattDescriptor->AttributeHandle < 255) {
					BTH_LE_GATT_DESCRIPTOR_VALUE newValue;

					RtlZeroMemory(&newValue, sizeof(newValue));

					newValue.DescriptorType = ClientCharacteristicConfiguration;
					newValue.ClientCharacteristicConfiguration.IsSubscribeToNotification = TRUE;

					hr = BluetoothGATTSetDescriptorValue(
						hLEDevice,
						currGattDescriptor,
						&newValue,
						BLUETOOTH_GATT_FLAG_NONE);
					if (HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER) == hr) {
						OutputDebugPrintf("BluetoothGATTSetDescriptorValue - ERROR_INVALID_PARAMETER %d\n", hr);
					}
					if (HRESULT_FROM_WIN32(ERROR_BAD_NET_RESP) == hr) {
						OutputDebugPrintf("BluetoothGATTSetDescriptorValue - ERROR_BAD_NET_RESP %d\n", hr);
					}
					if (HRESULT_FROM_WIN32(ERROR_SEM_TIMEOUT) == hr) {
						OutputDebugPrintf("BluetoothGATTSetDescriptorValue - ERROR_SEM_TIMEOUT %d\n", hr);
					}
					if (HRESULT_FROM_WIN32(ERROR_NO_SYSTEM_RESOURCES) == hr) {
						OutputDebugPrintf("BluetoothGATTSetDescriptorValue - ERROR_NO_SYSTEM_RESOURCESa %d\n", hr);
					}
					if (HRESULT_FROM_WIN32(ERROR_INVALID_FUNCTION) == hr) {
						OutputDebugPrintf("BluetoothGATTSetDescriptorValue - ERROR_INVALID_FUNCTION %d\n", hr);
					}
					if (S_OK  == hr) {
						this->connected = true;
						EventBusCenter blecenter;
						if(this->connected)
						{
							blecenter.BroadcastBleStatus(1);
						}
						else
						{
							blecenter.BroadcastBleStatus(0);
						}

						OutputDebugPrintf("BluetoothGATTSetDescriptorValue - S_OK %d\n", hr);
						//ConnectModelDevice();
					}
					else
					{

					}
					if (S_OK != hr) {
						OutputDebugPrintf("BluetoothGATTSetDescriptorValue - S_OK != hr %d\n", hr);
					}
					
				}

			}
		}

		// set the appropriate callback function when the descriptor change value 
		// _In_ BTH_LE_GATT_EVENT_TYPE EventType,
		//_In_ PVOID EventOutParameter,
		//	_In_opt_ PVOID Context
		BLUETOOTH_GATT_EVENT_HANDLE EventHandle;

		if (currGattChar->IsNotifiable) {
			OutputDebugPrintf("Setting Notification for ServiceHandle %d %s\n", currGattChar->ServiceHandle, BTH_LE_UUID_TO_STRING(currGattChar->CharacteristicUuid).c_str());
			BTH_LE_GATT_EVENT_TYPE EventType = CharacteristicValueChangedEvent;

			BLUETOOTH_GATT_VALUE_CHANGED_EVENT_REGISTRATION EventParameterIn;
			EventParameterIn.Characteristics[0] = *currGattChar;
			EventParameterIn.NumCharacteristics = 1;

			hr = BluetoothGATTRegisterEvent(
				hLEDevice,
				EventType,
				&EventParameterIn,
				reinterpret_cast<PFNBLUETOOTH_GATT_EVENT_CALLBACK>(onValueChangedCallback),
				this,
				&EventHandle,
				BLUETOOTH_GATT_FLAG_NONE);

			if (S_OK == hr) {
				OutputDebugPrintf("BluetoothGATTRegisterEvent - ok %d\n", hr);
			}
			if (HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER) == hr) {
				OutputDebugPrintf("BluetoothGATTRegisterEvent - invalid parameter\n");
			}
		}
		else if (currGattChar->IsReadable) {
			////////////////////////////////////////////////////////////////////////////
			// Determine Characteristic Value Buffer Size
			////////////////////////////////////////////////////////////////////////////
			hr = BluetoothGATTGetCharacteristicValue(
				hLEDevice,
				currGattChar,
				0,
				NULL,
				&charValueDataSize,
				BLUETOOTH_GATT_FLAG_NONE);

			if (HRESULT_FROM_WIN32(ERROR_MORE_DATA) != hr) {
				OutputDebugPrintf("BluetoothGATTGetCharacteristicValue - Buffer Size %d\n", hr);
			}

			pCharValueBuffer = (PBTH_LE_GATT_CHARACTERISTIC_VALUE)malloc(charValueDataSize);

			if (NULL == pCharValueBuffer) {
				OutputDebugPrintf("pCharValueBuffer out of memory\n");
			}
			else {
				RtlZeroMemory(pCharValueBuffer, charValueDataSize);
			}

			////////////////////////////////////////////////////////////////////////////
			// Retrieve the Characteristic Value
			////////////////////////////////////////////////////////////////////////////

			hr = BluetoothGATTGetCharacteristicValue(
				hLEDevice,
				currGattChar,
				(ULONG)charValueDataSize,
				pCharValueBuffer,
				NULL,
				BLUETOOTH_GATT_FLAG_NONE);

			if (S_OK != hr) {
				OutputDebugPrintf("BluetoothGATTGetCharacteristicValue - Actual Data %d\n", hr);
			}

			// print the characeteristic Value
			OutputDebugPrintf("\n Printing a read characterstic ");
			for (int iii = 0; iii < (int)pCharValueBuffer->DataSize; iii++) {// ideally check ->DataSize before printing
				printf("%d ", pCharValueBuffer->Data[iii]);
			}
			OutputDebugPrintf("\n");

			// Free before going to next iteration, or memory leak.
			free(pCharValueBuffer);
			pCharValueBuffer = NULL;
		}

	}

	// go into an inf loop that sleeps. you will ideally see notifications from the HR device
	/*while (1) {
		OutputDebugPrintf("sleep\n");
		Sleep(1000);
	}*/

	//CloseHandle(hLEDevice);

	if (GetLastError() != NO_ERROR &&
		GetLastError() != ERROR_NO_MORE_ITEMS)
	{
		// Insert error handling here.
		//return 1;
	}

	//return 0;
	// 激活模块服务的特征体的描述对象通知

	// 写入数据获取电量

}


//VOID CALLBACK onValueChangedCallback(BTH_LE_GATT_EVENT_TYPE type,
//	PVOID param,
//	PVOID context);

//reinterpret_cast<PFNBLUETOOTH_GATT_EVENT_CALLBACK>(onValueChangedCallback)

//连接模块服务器
//
void LogitowApi::LogitowBleControler::ConnectModelDevice()
{
	OutputDebugPrintf("ConnectModelDevice start.\n");
	GUID svcGuid;
	CLSIDFromString(TEXT(TO_MODE_DEVICE_UUID), &svcGuid);

	HANDLE hLEDevice = GetBLEHandle(svcGuid);

	//printf("Find Handle: 0x%x \r\n", hLEDevice);

	OutputDebugPrintf("Model Find Handle: 0x%x \r\n", hLEDevice);
	//-------------------------------------------
	// 注册数据处理的事件  /连接处理/断开连接处理/数据通讯处理

	//-------------------------------------------  
	//设备服务缓冲区数量
	USHORT serviceBufferCount;

	HRESULT hr = BluetoothGATTGetServices(
		hLEDevice,
		0,
		NULL,
		&serviceBufferCount,
		BLUETOOTH_GATT_FLAG_NONE);

	if (hr != HRESULT_FROM_WIN32(ERROR_MORE_DATA)) {
		OutputDebugPrintf("Model BluetoothGATTGetServices - Need Alloc Buffer Size: %d \r\n", serviceBufferCount);

	}

	//处理服务的缓存区对象。
	PBTH_LE_GATT_SERVICE pServiceBuffer = (PBTH_LE_GATT_SERVICE)
		malloc(sizeof(BTH_LE_GATT_SERVICE) * serviceBufferCount);

	if (pServiceBuffer == NULL) {
		OutputDebugPrintf("Model pServiceBuffer out of memory\r\n");
	}
	else {
		RtlZeroMemory(pServiceBuffer,
			sizeof(BTH_LE_GATT_SERVICE) * serviceBufferCount);
	}

	//搜索已连接的设备个数
	USHORT numServices;
	hr = BluetoothGATTGetServices(
		hLEDevice,
		serviceBufferCount,
		pServiceBuffer,
		&numServices,
		BLUETOOTH_GATT_FLAG_NONE);

	if (hr == S_OK) {
		OutputDebugPrintf("Model BluetoothGATTGetServices - Service Count %d \r\n", numServices);
	}

	//-------------------------------------------
	//获取特征体 并输出特征体缓存区大小
	USHORT charBufferSize;
	hr = BluetoothGATTGetCharacteristics(
		hLEDevice,
		pServiceBuffer,
		0,
		NULL,
		&charBufferSize,
		BLUETOOTH_GATT_FLAG_NONE);
	if (HRESULT_FROM_WIN32(ERROR_MORE_DATA) == hr) {
		OutputDebugPrintf("Model BluetoothGATTGetCharacteristics - Buffer Size %d \r\n", charBufferSize);
	}

	//处理特征体的缓存区对象。
	PBTH_LE_GATT_CHARACTERISTIC pCharBuffer;
	if (charBufferSize > 0)
	{
		pCharBuffer = (PBTH_LE_GATT_CHARACTERISTIC)
			malloc(charBufferSize * sizeof(BTH_LE_GATT_CHARACTERISTIC));

		if (pCharBuffer == NULL) {
			OutputDebugPrintf("Model pCharBuffer out of memory\r\n");
		}
		else {
			RtlZeroMemory(pCharBuffer,
				charBufferSize * sizeof(BTH_LE_GATT_CHARACTERISTIC));
		}
		//获取该服务下面的所有特征体数量
		USHORT numChars;
		hr = BluetoothGATTGetCharacteristics(
			hLEDevice,
			pServiceBuffer,
			charBufferSize,
			pCharBuffer,
			&numChars,
			BLUETOOTH_GATT_FLAG_NONE);

		if (hr == S_OK) {
			OutputDebugPrintf("Model BluetoothGATTGetCharacteristics - Actual Data %d \r\n", numChars);
		}

		if (numChars != charBufferSize) {
			OutputDebugPrintf("Model buffer size and buffer size actual size mismatch\r\n");
		}
	}

	// 激活数据服务的特征体的描述对象通知
	// Step 4: now get the list of descriptors. note how the pCharBuffer is required from step 3
	// descriptors are required as we descriptors that are notification based will have to be written
	// once IsSubcribeToNotification set to true, we set the appropriate callback function
	// need for setting descriptors for notification according to
	//http://social.msdn.microsoft.com/Forums/en-US/11d3a7ce-182b-4190-bf9d-64fefc3328d9/windows-bluetooth-le-apis-event-callbacks?forum=wdk
	PBTH_LE_GATT_CHARACTERISTIC currGattChar;
	for (int ii = 0; ii <charBufferSize; ii++) {
		currGattChar = &pCharBuffer[ii];
		USHORT charValueDataSize;
		PBTH_LE_GATT_CHARACTERISTIC_VALUE pCharValueBuffer;

		///////////////////////////////////////////////////////////////////////////
		// Determine Descriptor Buffer Size
		////////////////////////////////////////////////////////////////////////////
		USHORT descriptorBufferSize;
		hr = BluetoothGATTGetDescriptors(
			hLEDevice,
			currGattChar,
			0,
			NULL,
			&descriptorBufferSize,
			BLUETOOTH_GATT_FLAG_NONE);

		if (HRESULT_FROM_WIN32(ERROR_MORE_DATA) != hr) {
			OutputDebugPrintf("Model BluetoothGATTGetDescriptors - Buffer Size %d\n", hr);
		}

		PBTH_LE_GATT_DESCRIPTOR pDescriptorBuffer;
		if (descriptorBufferSize > 0) {
			pDescriptorBuffer = (PBTH_LE_GATT_DESCRIPTOR)
				malloc(descriptorBufferSize
					* sizeof(BTH_LE_GATT_DESCRIPTOR));

			if (NULL == pDescriptorBuffer) {
				OutputDebugPrintf("Model pDescriptorBuffer out of memory\n");
			}
			else {
				RtlZeroMemory(pDescriptorBuffer, descriptorBufferSize);
			}

			////////////////////////////////////////////////////////////////////////////
			// Retrieve Descriptors
			////////////////////////////////////////////////////////////////////////////

			USHORT numDescriptors;
			hr = BluetoothGATTGetDescriptors(
				hLEDevice,
				currGattChar,
				descriptorBufferSize,
				pDescriptorBuffer,
				&numDescriptors,
				BLUETOOTH_GATT_FLAG_NONE);

			if (S_OK != hr) {
				OutputDebugPrintf("Model BluetoothGATTGetDescriptors - Actual Data %d\n", hr);
			}

			if (numDescriptors != descriptorBufferSize) {
				OutputDebugPrintf("Model buffer size and buffer size actual size mismatch\n");
			}

			for (int kk = 0; kk<numDescriptors; kk++) {
				PBTH_LE_GATT_DESCRIPTOR  currGattDescriptor = &pDescriptorBuffer[kk];
				////////////////////////////////////////////////////////////////////////////
				// Determine Descriptor Value Buffer Size
				////////////////////////////////////////////////////////////////////////////
				USHORT descValueDataSize;
				hr = BluetoothGATTGetDescriptorValue(
					hLEDevice,
					currGattDescriptor,
					0,
					NULL,
					&descValueDataSize,
					BLUETOOTH_GATT_FLAG_NONE);

				if (HRESULT_FROM_WIN32(ERROR_MORE_DATA) != hr) {
					OutputDebugPrintf("Model BluetoothGATTGetDescriptorValue - Buffer Size %d\n", hr);
				}

				PBTH_LE_GATT_DESCRIPTOR_VALUE pDescValueBuffer = (PBTH_LE_GATT_DESCRIPTOR_VALUE)malloc(descValueDataSize);

				if (NULL == pDescValueBuffer) {
					OutputDebugPrintf("Model pDescValueBuffer out of memory\n");
				}
				else {
					RtlZeroMemory(pDescValueBuffer, descValueDataSize);
				}

				////////////////////////////////////////////////////////////////////////////
				// Retrieve the Descriptor Value
				////////////////////////////////////////////////////////////////////////////

				hr = BluetoothGATTGetDescriptorValue(
					hLEDevice,
					currGattDescriptor,
					(ULONG)descValueDataSize,
					pDescValueBuffer,
					NULL,
					BLUETOOTH_GATT_FLAG_NONE);
				if (S_OK != hr) {
					OutputDebugPrintf("Model BluetoothGATTGetDescriptorValue - Actual Data %d\n", hr);
				}
				// you may also get a descriptor that is read (and not notify) andi am guessing the attribute handle is out of limits
				// we set all descriptors that are notifiable to notify us via IsSubstcibeToNotification
				if (currGattDescriptor->AttributeHandle < 255) {
					BTH_LE_GATT_DESCRIPTOR_VALUE newValue;

					RtlZeroMemory(&newValue, sizeof(newValue));

					newValue.DescriptorType = ClientCharacteristicConfiguration;
					newValue.ClientCharacteristicConfiguration.IsSubscribeToNotification = TRUE;

					hr = BluetoothGATTSetDescriptorValue(
						hLEDevice,
						currGattDescriptor,
						&newValue,
						BLUETOOTH_GATT_FLAG_NONE);
					if (S_OK != hr) {
						OutputDebugPrintf("Model BluetoothGATTGetDescriptorValue - Actual Data %d\n", hr);
					}
				}

			}
		}

		// set the appropriate callback function when the descriptor change value 
		// _In_ BTH_LE_GATT_EVENT_TYPE EventType,
		//_In_ PVOID EventOutParameter,
		//	_In_opt_ PVOID Context
		BLUETOOTH_GATT_EVENT_HANDLE EventHandle;

		if (currGattChar->IsNotifiable) {
			OutputDebugPrintf("Model Setting Notification for ServiceHandle %d\n", currGattChar->ServiceHandle);
			BTH_LE_GATT_EVENT_TYPE EventType = CharacteristicValueChangedEvent;

			BLUETOOTH_GATT_VALUE_CHANGED_EVENT_REGISTRATION EventParameterIn;
			EventParameterIn.Characteristics[0] = *currGattChar;
			EventParameterIn.NumCharacteristics = 1;

			hr = BluetoothGATTRegisterEvent(
				hLEDevice,
				EventType,
				&EventParameterIn,
				reinterpret_cast<PFNBLUETOOTH_GATT_EVENT_CALLBACK>(onValueChangedCallback),
				this,
				&EventHandle,
				BLUETOOTH_GATT_FLAG_NONE);

			if (S_OK == hr) {
				OutputDebugPrintf("Model BluetoothGATTRegisterEvent - Actual Data %d\n", hr);
			}
			if (HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER) == hr) {
				OutputDebugPrintf("Model BluetoothGATTRegisterEvent - invalid parameter\n");
			}
		}
		else if (currGattChar->IsReadable) {
			////////////////////////////////////////////////////////////////////////////
			// Determine Characteristic Value Buffer Size
			////////////////////////////////////////////////////////////////////////////
			hr = BluetoothGATTGetCharacteristicValue(
				hLEDevice,
				currGattChar,
				0,
				NULL,
				&charValueDataSize,
				BLUETOOTH_GATT_FLAG_NONE);

			if (HRESULT_FROM_WIN32(ERROR_MORE_DATA) != hr) {
				OutputDebugPrintf("Model BluetoothGATTGetCharacteristicValue - Buffer Size %d\n", hr);
			}

			pCharValueBuffer = (PBTH_LE_GATT_CHARACTERISTIC_VALUE)malloc(charValueDataSize);

			if (NULL == pCharValueBuffer) {
				OutputDebugPrintf("Model pCharValueBuffer out of memory\n");
			}
			else {
				RtlZeroMemory(pCharValueBuffer, charValueDataSize);
			}

			////////////////////////////////////////////////////////////////////////////
			// Retrieve the Characteristic Value
			////////////////////////////////////////////////////////////////////////////

			hr = BluetoothGATTGetCharacteristicValue(
				hLEDevice,
				currGattChar,
				(ULONG)charValueDataSize,
				pCharValueBuffer,
				NULL,
				BLUETOOTH_GATT_FLAG_NONE);

			if (S_OK != hr) {
				OutputDebugPrintf("Model BluetoothGATTGetCharacteristicValue - Actual Data %d\n", hr);
			}

			// print the characeteristic Value
			OutputDebugPrintf("\nModel  Printing a read characterstic ");
			for (int iii = 0; iii < (int)pCharValueBuffer->DataSize; iii++) {// ideally check ->DataSize before printing
				printf("%d ", pCharValueBuffer->Data[iii]);
			}
			OutputDebugPrintf("\n");

			// Free before going to next iteration, or memory leak.
			free(pCharValueBuffer);
			pCharValueBuffer = NULL;
		}

	}

	// go into an inf loop that sleeps. you will ideally see notifications from the HR device
	/*while (1) {
	OutputDebugPrintf("sleep\n");
	Sleep(1000);
	}*/

	//CloseHandle(hLEDevice);

	if (GetLastError() != NO_ERROR &&
		GetLastError() != ERROR_NO_MORE_ITEMS)
	{
		// Insert error handling here.
		//return 1;
	}

	//return 0;
	// 激活模块服务的特征体的描述对象通知

	// 写入数据获取电量

}

// 搜索蓝牙 0找到设备，1未找到设备
int LogitowApi::LogitowBleControler::ScanBLEDevices()
{
	HDEVINFO hDI;
	SP_DEVINFO_DATA did;
	DWORD i;

	// Create a HDEVINFO with all present devices.为所有的设备创建信息对象
	hDI = SetupDiGetClassDevs(&GUID_DEVCLASS_BLUETOOTH, NULL, NULL, DIGCF_PRESENT);

	if (hDI == INVALID_HANDLE_VALUE)
	{
		return 1;
	}

	// Enumerate through all devices in Set. 枚举所有设备
	did.cbSize = sizeof(SP_DEVINFO_DATA);
	for (i = 0; SetupDiEnumDeviceInfo(hDI, i, &did); i++)
	{
		bool hasError = false;
		//获取设备名
		DWORD nameData;
		LPTSTR nameBuffer = NULL;
		DWORD nameBufferSize = 0;

		while (!SetupDiGetDeviceRegistryProperty(
			hDI,
			&did,
			SPDRP_FRIENDLYNAME,
			&nameData,
			(PBYTE)nameBuffer,
			nameBufferSize,
			&nameBufferSize))
		{
			if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
			{
				if (nameBuffer) delete(nameBuffer);
				nameBuffer = new wchar_t[nameBufferSize * 2];
			}
			else
			{
				hasError = true;
				break;
			}
		}
		//获取设备唯一地址
		DWORD addressData;
		LPTSTR addressBuffer = NULL;
		DWORD addressBufferSize = 0;

		while (!SetupDiGetDeviceRegistryProperty(
			hDI,
			&did,
			SPDRP_HARDWAREID,
			&addressData,
			(PBYTE)addressBuffer,
			addressBufferSize,
			&addressBufferSize))
		{
			if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
			{
				if (addressBuffer) delete(addressBuffer);
				addressBuffer = new wchar_t[addressBufferSize * 2];
			}
			else
			{
				hasError = true;
				break;
			}
		}
		//获取设备唯一ID
		LPTSTR deviceIdBuffer = NULL;
		DWORD deviceIdBufferSize = 0;

		while (!SetupDiGetDeviceInstanceId(
			hDI,
			&did,
			deviceIdBuffer,
			deviceIdBufferSize,
			&deviceIdBufferSize))
		{
			if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
			{
				if (deviceIdBuffer) delete(deviceIdBuffer);
				deviceIdBuffer = new wchar_t[deviceIdBufferSize * 2];
			}
			else
			{
				hasError = true;
				break;
			}
		}

		if (hasError)
		{
			continue;
		}
		
		if (currname == "")
		{
			currname = util::to_narrow(nameBuffer);
			curraddress = util::to_narrow(addressBuffer);
			currdeviceId = util::to_narrow(deviceIdBuffer);
			OutputDebugPrintf("BluetoothGATTGetCharacteristics - Actual Data %s \r\n", currname.c_str());
			OutputDebugPrintf("BluetoothGATTGetCharacteristics - Actual Data %s \r\n", curraddress.c_str());
			OutputDebugPrintf("BluetoothGATTGetCharacteristics - Actual Data %s \r\n", currdeviceId.c_str());
		}
	}
	if (currname == "")
	{
		return 1;
	}
	return 0;
}


// 获取蓝牙设备电量
void LogitowApi::LogitowBleControler::GetDevicePower()
{
	//特征体写入数值获取变量


}

//显示日志
void LogitowApi::LogitowBleControler::ShowLog(const char* strOutputString, ...)
{
	char strBuffer[4096] = { 0 };
	va_list vlArgs;
	va_start(vlArgs, strOutputString);
	_vsnprintf_s(strBuffer, sizeof(strBuffer) - 1, strOutputString, vlArgs);
	//vsprintf(strBuffer,strOutputString,vlArgs);
	va_end(vlArgs);
	OutputDebugString(CA2W(strBuffer));
}


//检测蓝牙连接和断开的状态事件。

//int _tmain(int argc, _tchar* argv[])
//{
//	int result = 0;// connectbledevice();
//	std::cin.get();
//	return result;
//}



