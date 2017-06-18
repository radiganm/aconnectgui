#include "Connection.hxx"
#include "Connector.hxx"
#include "Port.hxx"
#include "Window.hxx"

#include <FL/fl_draw.H>
#include <FL/Fl.H>

Connection::Connection(int X,int Y,int W,int H)
:Fl_Widget(X,Y,W,H)
{
	mFrom=0;
	mTo=0;
	mLevel=0;
	mState=false;
}

void Connection::Set(Connector* a, Connector* b) 
{
	mFrom = a;
	mTo = b;
	
	mLower = mUpper = 0;
}

int Connection::handle(int e)
{
	Window* window = (Window*) parent()->parent();
	
	if (e==FL_PUSH) {
		if (window->IsDisconnecting()) {
			int Y1 = mFrom->y()/*-parent()->y()*/+4+CONNECTOR_OFFSET, 
				X2 = x()+mLevel*10+10, 
				Y2 = mTo->y()/*-parent()->y()*/+4+CONNECTOR_OFFSET;
				
			if (Y1>Y2)
			{
				int tmp = Y1; Y1 = Y2; Y2 = tmp;
			}
			if (Fl::event_x()>X2-3 && Fl::event_x()<X2+3 &&
				Fl::event_y()>=Y1 && Fl::event_y()<=Y2) {
				mState = true;
				parent()->redraw();	
				return 1;
			}
		}
	}
	if (e==FL_RELEASE) {
		int Y1 = mFrom->y()/*-parent()->y()*/+4+CONNECTOR_OFFSET, 
			X2 = x()+mLevel*10+10, 
			Y2 = mTo->y()/*-parent()->y()*/+4+CONNECTOR_OFFSET;
		if (Y1>Y2)
		{
			int tmp = Y1; Y1 = Y2; Y2 = tmp;
		}
		if (Fl::event_x()>X2-3 && Fl::event_x()<X2+3 &&
			Fl::event_y()>=Y1 && Fl::event_y()<=Y2) {
			if (window->IsDisconnecting()) {
				window->HandleDisconnect(this);
				return 1;
			}
			mState = false; 
			return 1;
		}else{
			mState = false;
			window->GetConnections()->redraw();	
		}
	}
	return 0;
}

void Connection::draw(void) {
	int draw_phase = ((Connections*)parent())->DrawPhase();
	
	int X1 = x(), Y1 = mFrom->y()/*-parent()->y()*/+4+CONNECTOR_OFFSET, 
		X2 = x()+mLevel*10+10, Y2 = mTo->y()/*-parent()->y()*/+4+CONNECTOR_OFFSET;

	if (Y1>Y2)
	{
		int tmp = Y1; Y1 = Y2; Y2 = tmp;
	}
	if (draw_phase==1) {
		if (mState==1) {
			fl_color(FL_DARK3);
		}else{
			fl_color(FL_BLACK);		
		}
		fl_xyline(X1,Y1,X2-5);
		fl_xyline(X2-4,Y1+1,X2-3);
		fl_xyline(X2-2,Y1+2,X2-2);
		fl_yxline(X2-1,Y1+3,Y1+4);

		fl_yxline(X2-1,Y2-4,Y2-3);
		fl_xyline(X2-2,Y2-2,X2-2);
		fl_xyline(X2-4,Y2-1,X2-3);
		fl_xyline(X1,Y2,X2-5);

		fl_yxline(X2,Y1+5,Y2-5);
	} else {
		fl_color(color());
		fl_yxline(X2-2,Y1+5,Y2-5);
		fl_yxline(X2-1,Y1+5,Y2-5);
		fl_yxline(X2+1,Y1+5,Y2-5);
		fl_yxline(X2+2,Y1+5,Y2-5);
	}
}

void Connections::draw(void) {
	fl_color(color());
	fl_rectf(x(),y(),w(),h());

	mDrawPhase = 1;
	Fl_Group::draw();
	mDrawPhase = 0;
	Fl_Group::draw();
}


void Connections::ConflictTest(int i) 
{
	Connection** connections = (Connection**) array();
	int nconnections = children();

	for (int j=0;j<nconnections;j++) {
		if (i!=j) {
			if (connections[i]->mLevel >= connections[j]->mLevel) {
				if (
					connections[i]->mUpper->y() > connections[j]->mUpper->y()
					||
					(connections[i]->mUpper->y() == connections[j]->mUpper->y() &&
					connections[i]->mLower->y() < connections[j]->mLower->y())
					
				) {
					if (connections[i]->mUpper->y() < connections[j]->mLower->y())
					{
						connections[j]->mLevel=connections[i]->mLevel+1;
						ConflictTest(j);
					}
				}
			}
		}
	}
}

void Connections::Unclutter(void)
{
	Connection** connections = (Connection**) array();
	int nconnections = children();
		
	for (int i=0;i<nconnections;i++) {
		connections[i]->mLevel = 1;
		if (connections[i]->mFrom->y() > connections[i]->mTo->y()) {
			connections[i]->mLower= connections[i]->mFrom;
			connections[i]->mUpper= connections[i]->mTo;
		}else{
			connections[i]->mUpper= connections[i]->mFrom;
			connections[i]->mLower= connections[i]->mTo;
		}
	}

	for (int i=0;i<nconnections;i++) {
		ConflictTest(i);
	}
}

int Connections::Exists(Connector* a,Connector* b)
{
	Connection** connections = (Connection**) array();
	int nconnections = children();

	for (int i=0;i<nconnections;i++) {
		if (
			connections[i]->mFrom== a && connections[i]->mTo== b ||
			connections[i]->mFrom== b && connections[i]->mTo== a) {
			return 1;
		}
	}
	return 0;
}

Connection* Connections::Find(
	int fromClientId,int fromPortId,
	int toClientId,int toPortId)
{
	int nconnections = children();

	for (int i=0;i<nconnections;i++)
	{
		if (
			((Connection*)child(i))->mFrom->Owner()->ClientId() == fromClientId &&
			((Connection*)child(i))->mFrom->Owner()->PortId() == fromPortId &&
			((Connection*)child(i))->mFrom->Owner()->Output() &&
			((Connection*)child(i))->mTo->Owner()->ClientId() == toClientId &&
			((Connection*)child(i))->mTo->Owner()->PortId() == toPortId &&
			((Connection*)child(i))->mTo->Owner()->Input())
			return (Connection*)child(i);
	}
	return 0;
}

void Connections::RemoveClient(int id)
{
	int nconnections = children();
			printf("remove_client %d / %d\n",id,children());

	for (int i=0;i<children();)
	{
		printf("%d %d\n",
			((Connection*)child(i))->mFrom->Owner()->ClientId(),
			((Connection*)child(i))->mTo->Owner()->ClientId()	
		);
		if (
			((Connection*)child(i))->mFrom->Owner()->ClientId() == id ||
			((Connection*)child(i))->mTo->Owner()->ClientId() == id)
		{
			printf("removing now\n");
			((Connection*)child(i))->mFrom->Disconnect();
			((Connection*)child(i))->mTo->Disconnect();
			remove(child(i));
		}else
		{
			i++;
		}
	}
}

