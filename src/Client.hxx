#ifndef __Client__
#define __Client__

#include <FL/Fl_Group.H>
#include "FrameGroup.hxx"
#include "ConnectorType.hxx"

#include <stdio.h>

class Port;
class Connector;

class Client:public FrameGroup {
private:
	Fl_Group* mPorts;
	int mClientId;
public:
	Client(int X,int Y,int W,int H,char* L,int C);
	Port* AddPort(char* il,int portId,ConnectorType t);

	const char* Name(void)
	{
		return label();
	}
	int ClientId(void) { return mClientId; }

	Port* FindPort(int clientId,int portId);
	Connector* FindInput(int clientId,int portId);
	Connector* FindOutput(int clientId,int portId);

	Connector* FindConnector(int ix,int iy);

	void draw(void)
	{
		FrameGroup::draw();	
	}
	
};

class Clients:public Fl_Group
{
public:
	Clients(int X,int Y,int W,int H);

	Client*    FindClient(int clientId);
	Port*      FindPort(int clientId,int portId);
	Connector* FindInput(int clientId,int portId);
	Connector* FindOutput(int clientId,int portId);
	Connector* FindConnector(int ix,int iy);
	
	int handle(int e);
	
	void draw(void)
	{
		for (int i=0;i<children();i++)
		{
			child(i)->draw();
		}
		Fl_Group::draw();
	}
};

#endif
