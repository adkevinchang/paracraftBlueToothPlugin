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

	//��ȡ��Ϣ
	std::string const & getMessage() {
		return msg;
	}

	//��ȡ��Ϣ
	int getEventType() {
		return type;
	}

private:
	//�¼���Ϣ����
	std::string const & msg;
	//�¼�����
	int type;
};

#endif // !
