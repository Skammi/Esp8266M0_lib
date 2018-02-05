#include "esp8266M0_lib.h"
// General Methods
Esp8266::Esp8266 ()
{
	this->workingState = WIFI_IDLE;
	this->wifiMode = WIFI_MODE_STATION;
	this->isDebug = false;
}

void Esp8266::begin (Stream *serial)
{
	this->esp8266WiFi = serial;
}

void Esp8266::begin (Stream *serial, Stream *serialDebug)
{
	this->esp8266WiFi = serial;
	this->serialDebug = serialDebug;
	this->isDebug = true;
}

void Esp8266::debugPrintln (String str)
{
	this->serialDebug->println(str);
} // end debugPrintln

int Esp8266::getWorkingID()
{
	return this->workingID;
} // end getWorkingID

int Esp8266::getFailConnectID()
{
	return this->failConnectID;
} // end getFailConnectID

bool Esp8266::checkEsp8266()
{
	this->clearBuf();
	this->write("AT");

	if (not checkResponse ("OK"))
		return false;

	return true;

} //end checkEsp8266

bool Esp8266::resetEsp8266()
{
	unsigned long timeout = 7000;
	unsigned long t_start = 0;
	int buf[10];
	char index=0;

	this->clearBuf();
	this->write ("AT+RST");
	t_start = millis();
	while ((millis() - t_start) < timeout)
	{
		while (this->esp8266WiFi->available())
		{
			buf[index] = this->esp8266WiFi->read();
			if (buf[index]=='y' && buf[(index+9)%10]=='d' && buf[(index+8)%10]=='a' && buf[(index+7)%10]=='e' && buf[(index+6)%10]=='r') {
				return true;
			}
			index++;
			if (index==10)
				index = 0;
		}
	}
   	if (this->isDebug) this->debugPrintln("rest esp8266 timeout");

	return false;	
} // end resetEsp8266


// Connection related methods
bool Esp8266::connectAP (String ssid, String password)
{
	unsigned long timeout = 20000;
	unsigned long t_start = 0;
	int buf[10];
	char index=0;

	if (this->checkMode() == WIFI_MODE_STATION)
		this->wifiMode = WIFI_MODE_STATION;
	else if (setMode (WIFI_MODE_STATION))
		this->wifiMode = WIFI_MODE_STATION;
	else // non of the above no connectAP
	{
		if (this->isDebug) debugPrintln ("set mode to station false!");
		return false;
	}

	this->clearBuf ();
	this->esp8266WiFi->println ("AT+CWJAP=\""+ssid+"\",\""+password+"\"");

	t_start = millis ();
	while ((millis() - t_start) < timeout)
	{
		//while (available()>0) {
		while (this->esp8266WiFi->available() > 0)
		{
			buf[index] = this->esp8266WiFi->read();
			if (buf[index]=='K' && buf[(index+9)%10]=='O') {
				return true;
			}
			if (buf[index]=='L' && buf[(index+9)%10]=='I' && buf[(index+8)%10]=='A' && buf[(index+7)%10]=='F') {
				return false;
			}
			index++;
			if (index==10)
				index = 0;
		}
	}

	if (this->isDebug) debugPrintln ("connect AP timeout");
	return false;
} // end connectAP

bool Esp8266::enableAP (String ssid, String password)
{
	if (setMode (WIFI_MODE_AP))
	{
		this->write ("AT+CWSAP=\""+ssid+"\",\""+password+"\","+String(10)+String(4));
		String tmp;
		tmp = this->readData();
		if (tmp.indexOf ("OK") > 0)
			return true;
	}
	return false;
} // end EnableAP

String Esp8266::getIP()
{
	this->write ("AT+CIFSR");
	String tmp = this->readData();
	if (this->wifiMode == WIFI_MODE_STATION) {
		int index1 = tmp.indexOf("STAIP");
		int index2 = tmp.indexOf("+CIFSR:STAMAC");
		this->staIP =  tmp.substring(index1+7, index2-3);
		return this->staIP;
	} else {
		int index1 = tmp.indexOf("APIP");
		int index2 = tmp.indexOf("+CIFSR:APMAC");
		this->apIP =  tmp.substring(index1+6, index2-3);
		return this->apIP;
	}
} // end getIP

bool Esp8266::setSingleConnect()
{
	this->connectID = 0;
	this->multiFlag = false;
	return this->setMux (0);
} // end setSingleConnect

bool Esp8266::setMultiConnect()
{
	this->connectID = 0;
	this->multiFlag = true;
	return this->setMux (1);
} // end setMultiConnect

bool Esp8266::connectTCPServer (String serverIP, String serverPort) {
	unsigned long timeout = 5000;
	unsigned long t_start = 0;
	unsigned char buf[10];
	unsigned char index=0;

	this->clearBuf();
	if (!this->multiFlag)
	{
		this->write ("AT+CIPSTART=\"TCP\",\"" + serverIP + "\"," + serverPort);
		t_start = millis();
		while ((millis()) - t_start < timeout)
		{
			while (this->esp8266WiFi->available())
			{
				buf[index] = this->esp8266WiFi->read();
				if (buf[index]=='T' && buf[(index+9)%10]=='C' && buf[(index+8)%10]=='E' && buf[(index+7)%10]=='N'
									&& buf[(index+6)%10]=='N' && buf[(index+5)%10]=='O' && buf[(index+4)%10]=='C')
					return true;

				index++;
				if (index==10)
					index = 0;			
			}
		}
		if (this->isDebug)
			this->debugPrintln ("connectTCPServer timeout");
		return false;
	} else {
		write("AT+CIPSTART="+ String(this->connectID) + ",\"TCP\",\"" + serverIP + "\"," + serverPort);
		t_start = millis();
		while ((millis())-t_start < timeout)
		{
			while (this->esp8266WiFi->available())
			{
				buf[index] = this->esp8266WiFi->read();
				if (buf[index]=='T' && buf[(index+9)%10]=='C' && buf[(index+8)%10]=='E' && buf[(index+7)%10]=='N'
									&& buf[(index+6)%10]=='N' && buf[(index+5)%10]=='O' && buf[(index+4)%10]=='C') {
					this->connectID++;
					return true;
				}
				index++;
				if (index==10)
					index = 0;			
			}
		}
		if (this->isDebug)
			this->debugPrintln("connectTCPServer timeout");
		return false;		
	}
} // end connectTCPServer

bool Esp8266::openTCPServer(int port, int timeout)
/*
 * Returns true if successful if both commands send 'OK'
 * Returns false in all other cases.
 */
{
	if (this->setMux(1))
	{
		String str="";
		this->write ("AT+CIPSERVER=1,"+String(port));
		str = this->readData();
		if (str.indexOf("OK"))
		{
			this->write ("AT+CIPSTO="+String(timeout));
			str = this->readData();
			if (str.indexOf ("OK"))
				return true;
		}
	}
	return false;
} // end openTCPServer


// State related methods
void Esp8266::setState(int state)
{
	this->workingState = state;
} // end setState

int Esp8266::getState()
{
	return this->workingState;
} // end getState


// Message related methods
int Esp8266::checkMessage()
{
	//this->debugPrintln ("checkMessage before read data");
	String tmp="";
	tmp = this->readData();
	//this->debugPrintln ("checkMessage tmp =-"+tmp+"-");
	if (tmp!="")
	{
		if (tmp.substring(2, 6) == "+IPD") {
			if (!(this->multiFlag)) {
				int index = tmp.indexOf(":");
				int length = tmp.substring(7, index+1).toInt();
				this->message = tmp.substring(index+1, index+length+1);
				return WIFI_NEW_MESSAGE;
			} else {
				int id = 0, length=0, index=0; 
				id = tmp.substring(7, 8).toInt();
				index = tmp.indexOf(":");
				length = tmp.substring(9, index+1).toInt();
				this->workingID = id;
				this->message = tmp.substring(index+1, index+length+1);
				return WIFI_NEW_MESSAGE;
			}
		} else if (tmp.substring(0,6) == "CLOSED" || (tmp.charAt(1)==',' && tmp.substring(2,8)=="CLOSED")) {
			if (!(this->multiFlag)) {
				return WIFI_CLOSED;
			} else {
				this->failConnectID = tmp.charAt(0)-'0';
				return WIFI_CLOSED;
			}
		} else if (tmp.substring(1,9) == ",CONNECT") {
			int index = tmp.charAt(0)-'0';
			this->workingID = index;
			return WIFI_CLIENT_ON;
		} else if (this->isPureDataMode) {
			this->message = tmp;
			return WIFI_NEW_MESSAGE;
		} else {
			return WIFI_IDLE;
		}
	}
	return this->workingState;
} // end checkMessage

String Esp8266::getMessage()
{
	return this->message;
} // end getMessage

bool Esp8266::sendMessage (String str)
{
	// SingleConnect variant
	int len = str.length();

	this->write ("AT+CIPSEND="+String(len));
	if (not checkResponse (">"))
		return false;

	this->esp8266WiFi->print(str);
	return (checkResponse ("SEND OK"));
} // end sendMessage SingleConnect variant

bool Esp8266::sendMessage (int index, String str)
{
	// MultiConnect variant
	int len = str.length ();

	this->write("AT+CIPSEND="+String(index)+","+String(len));

	if (not checkResponse (">"))
		return false;

	this->esp8266WiFi->print(str);
	return (checkResponse ("SEND OK"));
}// end sendMessage MultiConnect variant

bool Esp8266::setPureDataMode ()
{
	this->write ("AT+CIPMODE=1");
	if (not checkResponse ("OK"))
		return false;
	//this->debugPrintln ("setPureDataMode after wait for OK");

	this->write ("AT+CIPSEND");
	if (not checkResponse (">"))
		return false;
	//this->debugPrintln ("setPureDataMode after wait for >");

	this->isPureDataMode = true;
	return true;
} // end setPureDataMode

bool Esp8266::sendPureDataMessage (const uint8_t *buffer, size_t length)
{
	if ((length > 2048) || (!this->isPureDataMode))
		return false;
	return (this->esp8266WiFi->write (buffer, length) != length);
} // end sendPureDataMessage

bool Esp8266::resetPureDataMode()
{
	this->esp8266WiFi->print ("+++");
	if (not checkResponse ("+++"))
		return false;

	this->write ("AT");
	if (not checkResponse ("OK"))
		return false;
	this->isPureDataMode = false;
	return true;
} // end resetPureDataMode


// Private Methods
void Esp8266::clearBuf()
{
	while (this->esp8266WiFi->available() > 0)
    this->esp8266WiFi->read();
} // end clearBuf

void Esp8266::write (String str)
{
	this->esp8266WiFi->println(str);
	this->esp8266WiFi->flush();
} // end write

String Esp8266::readData ()
{
	unsigned long timeout = 100;
	unsigned long t = millis();
	String data = "";

	while (millis() - t < timeout)
	{
		if (this->esp8266WiFi->available() > 0)
		{
			char r = esp8266WiFi->read();
			data += r;
			t = millis ();
		}
	}

	return data;
} // end readData

bool Esp8266::checkResponse (String response)
{
	bool isOK = false;
	String tmp = "";
	do
	{	// readData will timeout after 100ms
		tmp = this->readData();
		isOK = tmp.indexOf (response) > 0;
		if (tmp =="")
			return false;
	}
	while (not isOK);
	return true;
}	// end checkResponse


char Esp8266::checkMode()
{
	this->clearBuf ();
	this->write ("AT+CWMODE?");

	String str = this->readData();

	if (str.indexOf('1') > 0 )
		return '1';
	else if (str.indexOf('2') > 0)
		return '2';
	else if (str.indexOf('3') > 0)
		return '3';
	else
		return '0';
} // end checkMode

bool Esp8266::setMode(char mode)
{
	// <mode>1 means Station mode
	// <mode>2 means softAP mode
	// <mode>3 means AP + Station mode

	this->clearBuf();
	this->write ("AT+CWMODE="+String(mode));

	if (checkResponse ("no change"))
		return true;
	else
	{
		if (this->resetEsp8266())
		{
			this->wifiMode = mode;
			return true;
		}
	}
	return false;
} // end setMode

bool Esp8266::setMux (int flag)
{
	// <mode>0  single connection
	// <mode>1  multiple connection
	bool isOK = false;
	String tmp = "";

	this->clearBuf();
	this->write ("AT+CIPMUX="+String(flag));
	do
	{	// readData will timeout after 100ms
		tmp = this->readData();
		isOK = (tmp.indexOf("OK")>0 || tmp.indexOf("link is builded")>0);
		if (tmp =="")
			return false;
	}
	while (not isOK);
	return true;
} // end setMux

//---[ end of file Esp8266M0_lib.cpp ]---
