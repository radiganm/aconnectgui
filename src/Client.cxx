#include "MyWindow.hxx"
#include "Client.hxx"
#include "Connection.hxx"
#include "Port.hxx"
#include "Layout.hxx"
#include <FL/Fl.H>

Client::Client(int X,int Y,int W,int H,char* L,int C)
:FrameGroup(X,Y,W,H,L)
{
	labelsize(10);
	mPorts = new Fl_Group(X+2,Y+14,W-7,0);
	mPorts->end();
	mPorts->resizable(0);
	end();
	resizable(0);
	mClientId = C;
}

Port* Client::FindPort(int clientId,int portId)
{
	if (mClientId!=clientId) return 0;
	
	int n = mPorts->children();
	for (int i=0;i<n;i++) {
		Port* p = (Port*) (mPorts->child(i));
		if (p->PortId()==portId) {
			return p;
		}
	}
	return 0;
}

Connector* Client::FindInput(int clientId,int portId)
{
	if (mClientId!=clientId) return 0;
	
	int n = mPorts->children();
	for (int i=0;i<n;i++) {
		Port* p = (Port*) (mPorts->child(i));
		if (p->PortId()==portId && p->Input()) {
			return p->Input();
		}
	}
	return 0;
}

Connector* Client::FindOutput(int clientId,int portId)
{
	if (mClientId!=clientId) return 0;
	
	int n = mPorts->children();
	for (int i=0;i<n;i++) {
		Port* p = (Port*) (mPorts->child(i));
		if (p->PortId()==portId && p->Output()) {
			return p->Output();
		}
	}
	return 0;
}

Connector* Client::FindConnector(int ix,int iy)
{
	if (ix>=x() && ix<=x()+w() && iy>=y() && iy<=y()+h()) {
		int n = mPorts->children();
		for (int i=0;i<n;i++) {
			Port* p = (Port*) (mPorts->child(i));
			Connector* c = p->FindConnector(ix,iy);
			if (c) return c;
		}
	}
	return 0;
}

Clients::Clients(int X,int Y,int W,int H):Fl_Group(X,Y,W,H)
{ 
	resizable(0);	
	end(); 
}

Port*      Clients::FindPort(int clientId,int portId)
{
	int n = children();
	for (int i=0;i<n;i++) {
		Client* c = (Client*) (child(i));
		Port* p = c->FindPort(clientId,portId);
		if (p) return p;
	}
	return 0;
}

Connector* Clients::FindInput(int clientId,int portId)
{
	int n = children();
	for (int i=0;i<n;i++) {
		Client* c = (Client*) (child(i));
		Connector* r = c->FindInput(clientId,portId);
		if (r) return r;
	}
	return 0;
}

Connector* Clients::FindOutput(int clientId,int portId)
{
	int n = children();
	for (int i=0;i<n;i++) {
		Client* c = (Client*) (child(i));
		Connector* r = c->FindOutput(clientId,portId);
		if (r) return r;
	}
	return 0;
}

Client*    Clients::FindClient(int clientId)
{
	int n = children();
	for (int i=0;i<n;i++) {
		Client* c = (Client*) (child(i));
		if (c->ClientId()==clientId) return c;
	}
	return 0;
}

Connector* Clients::FindConnector(int ix,int iy)
{
	int n = children();
	for (int i=0;i<n;i++) {
		Client* c = (Client*) (child(i));
		Connector* con = c->FindConnector(ix,iy);
		if (con) return con;
	}
	return 0;
}

Port* Client::AddPort(char* il,int portId,ConnectorType t)
{
	Port* port;
	int ph=0;
	if (t&kInput) {
		ph+=CONNECTOR_DISTANCE;				
	}
	if (t&kOutput) {
		ph+=CONNECTOR_DISTANCE;
	}
	ph+=PORT_EXTRA;

	port = new Port(mPorts->x(),mPorts->y()+mPorts->h(),mPorts->w(),ph,il,portId,t);
	mPorts->size(mPorts->w(),port->y()+port->h()-mPorts->y());
	mPorts->add(port);
	size(w(),mPorts->y()+mPorts->h()-y()+4);
	mPorts->color(FL_RED);
	mPorts->redraw();
	return port;
}

int Clients::handle(int e)
{
	static Connector* a,*b,*prev;
	static Connection* c=0;

	MyWindow* patchbay = ((MyWindow*)parent());
	Connections* connections = patchbay->GetConnections();

	if (e==FL_PUSH && patchbay->IsConnecting()) {
		a = 0; b = 0; c = 0; prev = 0;
		a = FindConnector(Fl::event_x(),Fl::event_y());
		if (a) {
			a->Select();
			a->redraw();
		}
	}
	if (e==FL_DRAG && a) {
		b = FindConnector(Fl::event_x(),Fl::event_y());
		if (prev!=b && a!=b) {
			if (prev) {
				prev->Unselect();
				prev->redraw();
			}
			if (b) {
				if (a!=b) {
					if (!connections->Exists(a,b)) {
						if (patchbay->IsLegal(a,b)) {
							c= new Connection(
								connections->x(),connections->y(),
								connections->w(),connections->h()
							);
							b->Select();
							b->redraw();
							c->Set(a,b);
							connections->add(c);
							connections->redraw();
						}
					}
				}
			}else{
				if (c) {
					connections->remove(c);
					delete c;
					c = 0;
					connections->redraw();
				}
			}
			prev=b;
		}
	}
	if (e==FL_RELEASE) {
		if (c) {
			connections->remove(c);
			delete c;
			c = 0;
		}
		if (a && b) {
			bool flag = 0;
			if (a->Type()==kOutput)
			{
				flag = patchbay->HandleConnect(a,b);
			}else{
				flag = patchbay->HandleConnect(b,a);
			}
			if (!flag)
			{
				a->Unselect();
				a->redraw();
				b->Unselect();
				b->redraw();
			}
		}else{
			if (a) {
				a->Unselect();
				a->redraw();
			}
		}
	}
	return 1;
}
