#ifndef __MyWindow__
#define __MyWindow__

#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Radio_Button.H>
#include <FL/Fl_Pixmap.H>

#include "Connection.hxx"
#include "Port.hxx"
#include "Client.hxx"

#include <alsa/asoundlib.h>

class MyWindow:public Fl_Window
{
private:
	Connections* mConnections;
	Clients*     mClients;
	Fl_Radio_Button *mCutButton;
	Fl_Radio_Button *mInfoButton;
	Fl_Radio_Button *mConnectButton;

	Client* mCurClient;
	Port*   mCurPort;
	ConnectorType mCurType;

	struct pollfd *mPollFds;
	int mPollMax;

	snd_seq_t* mHandle; // handle and client for system notification events
	int mClientId;

public:
	MyWindow();
	~MyWindow();

	Connections* GetConnections(void) { return mConnections; }

	bool IsConnecting(void) { return mConnectButton->value(); }
	bool IsDisconnecting(void) { return mCutButton->value(); }

	void Connect(int client,int port,int clientB,int portB);
	void Disconnect(int client,int port,int clientB,int portB);

	void Connect(Connector* a,Connector* b);
	void Disconnect(Connection* c);

	Client* AddClient(snd_seq_client_info_t* cinfo);
	Port* AddPort(snd_seq_port_info_t* pinfo);
	
	void SetCurType(ConnectorType t)
	{
		mCurType = t;
	}
	ConnectorType GetCurType(void)
	{
		return mCurType;
	}
	Client* GetCurClient(void) { return mCurClient; }
	Port* GetCurPort(void) { return mCurPort; }
	
	snd_seq_t* GetHandle(void) { return mHandle; }
	
	Connector* FindOutput(int clientId,int portId)
	{
		mClients->FindOutput(clientId,portId);
	}
	
	bool HandleConnect(Connector* a,Connector* b);
	bool HandleDisconnect(Connection* c);

	int IsLegal(Connector* a,Connector* b);
	
	static void TimeoutStatic(void* ptr)
	{
		((MyWindow*)ptr)->Timeout();
	}
	
	void Timeout(void);
};

#endif
