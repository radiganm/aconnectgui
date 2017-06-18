#ifndef __Connector__
#define __Connector__

#include <FL/Fl_Widget.H>
#include "ConnectorType.hxx"
#include "Layout.hxx"

class Connection;
class Port;

class Connector:public Fl_Widget {
private:
	unsigned char mConnectedCount;
	unsigned char mIsSelected;
	ConnectorType mType;

public:
	Connector(int ix,int iy,ConnectorType t);

	void draw(void);

	void Select()   { mIsSelected = true;  }
	void Unselect() { mIsSelected = false; }
	unsigned char IsSelected(void) { return mIsSelected; }

	void Connect() { mConnectedCount++; }
	void Disconnect() { if (mConnectedCount) mConnectedCount--;	}
	unsigned char Connected(void) { return mConnectedCount; }
	
	
	ConnectorType Type(void) { return mType; }
	
	Port* Owner(void) { return (Port*) parent(); }
	
	bool Contains(int ix,int iy);
};

#endif
