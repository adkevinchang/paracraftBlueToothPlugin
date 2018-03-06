#ifndef _EVENT_BLE_EVENT_HPP_
#define _EVENT_BLE_EVENT_HPP_

#include "event\Event.hpp"
#include <string>

#define BLE_STATUS_EVENT_TYPE 100
#define BLE_NOTICE_EVENT_TYPE 101
#define BLE_POWER_EVENT_TYPE 102

class BleEvent : public Event
{
public:
	BleEvent(Object* sender,std::string const & msg,int _type) :
	Event(sender),
	msg(msg),
	type(_type){

	}

	virtual ~BleEvent()
	{

	}

	//获取消息
	std::string const & getMessage() {
		return msg;
	}

	//获取消息
	int getEventType() {
		return type;
	}

private:
	//事件消息内容
	std::string const & msg;
	//事件类型
	int type;
};

#endif // !
