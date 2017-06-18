#ifndef __Port__
#define __Port__

#include <FL/Fl_Group.H>
#include "Connector.hxx"
#include "Client.hxx"
#include <FL/Fl_Box.H>

class Connector;

#include <stdio.h>

class Port:public Fl_Group {
protected:
	Fl_Box*    mLabelBox;

	Connector* mInput;
	Connector* mOutput;

	int mPortId;
public:	
	void draw(void)
	{
		Fl_Group::draw();
	}

	Port(int ix,int iy,int iw,int ih,char* il,int portId,ConnectorType t);

	const char* Name(void) { return mLabelBox->label(); }

	int PortId(void) { return mPortId; }
	int ClientId(void) { return Owner()->ClientId(); }	
		
	Connector* Input(void) { return mInput; }
	Connector* Output(void) { return mOutput; }
			
	Client* Owner(void) { return (Client*) parent()->parent(); }
		
	Connector* FindConnector(int ix,int iy);	
};

#endif
