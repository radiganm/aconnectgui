#ifndef __Fl_FrameGroup__
#define __Fl_FrameGroup__

#include <FL/Fl_Group.H>
#include <FL/fl_draw.H>

class FrameGroup:public Fl_Group
{
public:
	FrameGroup(int X,int Y,int W,int H,char* L):
	Fl_Group(X,Y,W,H,L) 
	{
		labeltype(FL_NO_LABEL);
	}
	void draw(void) { 
		fl_draw_box(FL_ENGRAVED_FRAME,x(),y()+5,w(),h()-5,FL_BLACK);
		fl_color(color());
		fl_font(labelfont(),labelsize());
		fl_rectf(x()+10,y(),int(fl_width(label()))+4,10);
		fl_color(labelcolor());
		fl_draw(label(),x()+12,y()+10);
		Fl_Group::draw();
	}
};

#endif
