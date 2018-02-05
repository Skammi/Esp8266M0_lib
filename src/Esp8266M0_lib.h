#ifndef _ESP8266M0_H_
#define _ESP8266M0_H_

#include <Arduino.h>
#include <Stream.h>
#include <vector>
#include <avr/pgmspace.h>
#include <IPAddress.h>

#define	WIFI_IDLE			1
#define WIFI_NEW_MESSAGE	2
#define WIFI_CLOSED			3
#define WIFI_CLIENT_ON		4

#define WIFI_MODE_STATION			'1'
#define WIFI_MODE_AP				'2'
#define WIFI_MODE_BOTH				'3'

class Esp8266
{
public:
	// General Methods
	Esp8266 ();
	void begin (Stream *serial);
	void begin (Stream *serial, Stream *serialDebug);
	void debugPrintln (String str);
	int getWorkingID ();
	int getFailConnectID ();
	bool checkEsp8266 ();
	bool resetEsp8266 ();

	// Access Point related methods
	bool connectAP (String ssid, String password);
	bool enableAP (String ssid, String password);
	String getIP ();
	bool setSingleConnect ();
	bool setMultiConnect ();

	bool connectTCPServer (String serverIP, String serverPort);
	bool openTCPServer (int port, int timeout);

	// State related methods
	void setState (int state);
	int getState ();

	// Message related methods
	int	 checkMessage ();
	String getMessage ();
	bool sendMessage (String str);
	bool sendMessage (int index, String str);
	bool setPureDataMode ();
	bool sendPureDataMessage (const uint8_t *buffer, size_t length);
	bool resetPureDataMode ();
	//void comSend (); No method defined in Esp8266*.cpp
	unsigned long nop (unsigned long timeout);

private:
	void clearBuf ();
	void write (String str);
	String readData ();
	bool checkResponse (String str);
	char checkMode ();
	bool setMode (char mode);
	bool setMux (int flag);

private:
	Stream *esp8266WiFi;
	Stream *serialDebug;
	int connectID=0;
	int workingID=0;
	int failConnectID=0;
	bool multiFlag=false;
	int workingState=0;
	String message;
	//String message="";
	char wifiMode;
	String staIP;
	String apIP;
	bool isDebug=false;
	bool isPureDataMode=false;
};

#endif /*** _ESP8266M0_H_ ***/
