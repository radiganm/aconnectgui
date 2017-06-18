#include "Connection.hxx"
#include "Connector.hxx"

#include <FL/fl_draw.H>

Connector::Connector(int ix,int iy,ConnectorType t)
:Fl_Widget(ix,iy,8,CONNECTOR_DISTANCE) {
	mType=t;
	mConnectedCount=0;
	mIsSelected=false;
}

void Connector::draw()
{
	int cx=x(),cy=y()+CONNECTOR_OFFSET;
	if (mType==kOutput) {
		fl_color(FL_BLACK);
		fl_yxline(cx,cy,cy+8);
		fl_line(cx+1,cy,cx+5,cy+4);
		fl_line(cx+5,cy+4,cx+1,cy+8);
		if (IsSelected()) 
			fl_color(FL_DARK1);
		else
			if (Connected()) 
				fl_color(FL_BLACK);			
			else 
				fl_color(FL_WHITE);
		for (int k=0;k<4;k++) {
			fl_xyline(cx+1,cy+k+1,cx+1+k);
			fl_xyline(cx+1,cy+7-k,cx+1+k);
		}			
	}else{
		fl_color(FL_BLACK);
		fl_yxline(cx+5,cy,cy+8);
		fl_line(cx+4,cy,cx,cy+4);
		fl_line(cx,cy+4,cx+4,cy+8);
		if (IsSelected()) 
			fl_color(FL_DARK1);
		else
			if (Connected()) 
				fl_color(FL_BLACK);			
			else 
				fl_color(FL_WHITE);
		for (int k=0;k<4;k++) {
			fl_xyline(cx+4-k,cy+k+1,cx+4);
			fl_xyline(cx+4-k,cy+7-k,cx+4);
		}					
	}
}

bool Connector::Contains(int ix,int iy)
{
	return (ix>=x() && ix<=x()+w() && iy>=y() && iy<=y()+h());
}

