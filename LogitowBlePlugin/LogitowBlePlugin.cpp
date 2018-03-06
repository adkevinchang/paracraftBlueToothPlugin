#include "stdafx.h"
#include "LogitowBle.h"
#include "LogitowBleControler.h"
#include <assert.h>
#include <atlbase.h>
#include <atlconv.h>
#include "BleEvent.hpp"
#include "event\EventBus.hpp"

#ifdef WIN32
#define CORE_EXPORT_DECL    __declspec(dllexport)
#else
#define CORE_EXPORT_DECL
#endif

// forward declare of exported functions. 
#ifdef __cplusplus
extern "C" {
#endif
	CORE_EXPORT_DECL const char* LibDescription();
	CORE_EXPORT_DECL int LibNumberClasses();
	CORE_EXPORT_DECL unsigned long LibVersion();
	CORE_EXPORT_DECL ParaEngine::ClassDescriptor* LibClassDesc(int i);
	CORE_EXPORT_DECL void LibInit();
	CORE_EXPORT_DECL void LibActivate(int nType, void* pVoid);
#ifdef __cplusplus
}   /* extern "C" */
#endif
 
HINSTANCE Instance = NULL;
NPL::INPLRuntimeState* pState;

using namespace ParaEngine;
using namespace LogitowApi;
using namespace std;

ClassDescriptor* LogitowBlePlugin_GetClassDesc();
typedef ClassDescriptor* (*GetClassDescMethod)();

GetClassDescMethod Plugins[] = 
{
	LogitowBlePlugin_GetClassDesc,
};

/** This has to be unique, change this id for each new plugin.
*/
#define LogitowBle_CLASS_ID Class_ID(0x2e905e29, 0x47b409af)

class LogitowBlePluginDesc:public ClassDescriptor
{
public:
	void* Create(bool loading = FALSE)
	{
		return new CLogitowBle();
	}

	const char* ClassName()
	{
		return "ILogitowBle";
	}

	SClass_ID SuperClassID()
	{
		return OBJECT_MODIFIER_CLASS_ID;
	}

	Class_ID ClassID()
	{
		return LogitowBle_CLASS_ID;
	}

	const char* Category() 
	{ 
		return "LogitowBle"; 
	}

	const char* InternalName() 
	{ 
		return "LogitowBle"; 
	}

	HINSTANCE HInstance() 
	{
		extern HINSTANCE Instance;
		return Instance; 
	}
};

ClassDescriptor* LogitowBlePlugin_GetClassDesc()
{
	static LogitowBlePluginDesc s_desc;
	return &s_desc;
}

CORE_EXPORT_DECL const char* LibDescription()
{
	return "ParaEngine LogitowBle Ver 1.0.0";
}

CORE_EXPORT_DECL unsigned long LibVersion()
{
	return 1;
}

CORE_EXPORT_DECL int LibNumberClasses()
{
	return sizeof(Plugins)/sizeof(Plugins[0]);
}

CORE_EXPORT_DECL ClassDescriptor* LibClassDesc(int i)
{
	if (i < LibNumberClasses() && Plugins[i])
	{
		return Plugins[i]();
	}
	else
	{
		return NULL;
	}
}

CORE_EXPORT_DECL void LibInit()
{

}

#ifdef WIN32
BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved)
#else
void __attribute__ ((constructor)) DllMain()
#endif
{
	// TODO: dll start up code here
#ifdef WIN32
	Instance = hinstDLL;				// Hang on to this DLL's instance handle.
	return (TRUE);
#endif
}

/**
*
*/
class BleListener : public EventHandler<BleEvent>
{
public:
	BleListener() { }

	virtual ~BleListener() { }


	/**
	*
	* @param e The BleEvent event
	*/
	virtual void onEvent(BleEvent * e) override {

		// Ignore the event if it's already been canceled
		if (e->getCanceled()) {
			return;
		}

		if (e->getEventType() == BLE_NOTICE_EVENT_TYPE)
		{
			NPLInterface::NPLObjectProxy output_msg;
			output_msg["type"] = (double)(BLE_NOTICE_EVENT_TYPE);
			//output_msg["sample_number_output"] = (double)(1234567);
			output_msg["result"] = e->getMessage();

			std::string output;
			NPLInterface::NPLHelper::NPLTableToString("bodyinfo", output_msg, output);
			pState->activate("script/apps/Aries/Creator/Game/BleController.lua", output.c_str(), output.size());

			LogitowApi::LogitowBleControler::GetInstance()->ShowLog(e->getMessage().c_str());
			LogitowApi::LogitowBleControler::GetInstance()->ShowLog("\r\n");
			//showLog("BLE_NOTICE_EVENT");
		}
		else if (e->getEventType() == BLE_STATUS_EVENT_TYPE)
		{
			NPLInterface::NPLObjectProxy output_msg;
			output_msg["type"] = (double)(BLE_STATUS_EVENT_TYPE);
			//output_msg["sample_number_output"] = (double)(1234567);
			output_msg["result"] = e->getMessage();

			std::string output;
			NPLInterface::NPLHelper::NPLTableToString("bodyinfo", output_msg, output);
			pState->activate("script/apps/Aries/Creator/Game/BleController.lua", output.c_str(), output.size());

			LogitowApi::LogitowBleControler::GetInstance()->ShowLog(e->getMessage().c_str());
			LogitowApi::LogitowBleControler::GetInstance()->ShowLog("\r\n");
			//showLog("BLE_STATUS_EVENT");
		}
		else if (e->getEventType() == BLE_POWER_EVENT_TYPE)
		{
			NPLInterface::NPLObjectProxy output_msg;
			output_msg["type"] = (double)(BLE_POWER_EVENT_TYPE);
			//output_msg["sample_number_output"] = (double)(1234567);
			output_msg["result"] = e->getMessage();

			std::string output;
			NPLInterface::NPLHelper::NPLTableToString("bodyinfo", output_msg, output);
			pState->activate("script/apps/Aries/Creator/Game/BleController.lua", output.c_str(), output.size());

			LogitowApi::LogitowBleControler::GetInstance()->ShowLog(e->getMessage().c_str());
			LogitowApi::LogitowBleControler::GetInstance()->ShowLog("\r\n");
			//showLog("BLE_POWER_EVENT");
		}
		

	}

private:
	//static const int BORDER_SIZE = 500;

};

/** this is the main activate function to be called, when someone calls NPL.activate("this_file.dll", msg); 
*/
CORE_EXPORT_DECL void LibActivate(int nType, void* pVoid)
{
	if(nType == ParaEngine::PluginActType_STATE)
	{
		pState = (NPL::INPLRuntimeState*)pVoid;
		const char* sMsg = pState->GetCurrentMsg();
		int nMsgLength = pState->GetCurrentMsgLength();

		NPLInterface::NPLObjectProxy input_msg = NPLInterface::NPLHelper::MsgStringToNPLTable(sMsg);
		const std::string& sCmd = input_msg["cmd"];

		//开始启动蓝牙
		if (sCmd == "startble")
		{

			BleListener* blehand = new BleListener();
			HandlerRegistration* reg = EventBus::AddHandler<BleEvent>(blehand);

			int res = LogitowApi::LogitowBleControler::GetInstance()->ScanBLEDevices();
			if (res == 0)
			{
				LogitowApi::LogitowBleControler::GetInstance()->ConnectDevice();
				//showLog("logitow Bluetooth ConnectDevice！");
			}
			else
			{
				//未找到逻辑塔蓝牙设备
				//showLog("No logitow Bluetooth device found!");
			}
		}

		//写入获取电量
		if (sCmd == "getPower")
		{

		}


	}
}

//lua中打印日志
/*
void showLog(const char* stroutputstring)
{
	NPLInterface::NPLObjectProxy output_msg;
	output_msg["succeed"] = "true";
	output_msg["sample_number_output"] = (double)(1);
	output_msg["result"] = stroutputstring;
	std::string output;
	NPLInterface::NPLHelper::NPLTableToString("msg", output_msg, output);
	pState->activate("script/test/echo.lua", output.c_str(), output.size());
}*/


//打印函数如何通用
//void outputdebugprintf(const char* stroutputstring, ...)
//{
//	char strbuffer[4096] = { 0 };
//	va_list vlargs;
//	va_start(vlargs, stroutputstring);
//	_vsnprintf_s(strbuffer, sizeof(strbuffer) - 1, stroutputstring, vlargs);
//	//vsprintf(strbuffer,stroutputstring,vlargs);
//	va_end(vlargs);
//	outputdebugstring(ca2w(strbuffer));
//}