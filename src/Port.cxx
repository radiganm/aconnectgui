#include "Port.hxx"
#include "Connector.hxx"
#include <FL/Fl_Box.H>

Port::Port(int ix,int iy,int iw,int ih,char* il,int portId,ConnectorType t)
:Fl_Group(ix,iy,iw,ih)
{
	mLabelBox = new Fl_Box(
		x(),y()+CONNECTOR_OFFSET,
		w()-8,h()-CONNECTOR_OFFSET-PORT_EXTRA
	);
	mLabelBox->box(FL_FLAT_BOX);
	mLabelBox->label(il);
	mLabelBox->labelsize(10);
	mLabelBox->align(FL_ALIGN_INSIDE|FL_ALIGN_RIGHT);

	mPortId = portId;

	int cy=y();
	
	mInput = 0;
	mOutput = 0;
	
	if (t&kInput) {
		add(mInput = new Connector(x()+w()-5,cy,kInput));

		cy+=CONNECTOR_DISTANCE;				
	}
	if (t&kOutput) {
		add(mOutput = new Connector(x()+w()-5,cy,kOutput));
	}
	
	resizable(0);
}

Connector* Port::FindConnector(int ix,int iy)
{
	if (ix>=x() && ix<=x()+w() && iy>=y() && iy<=y()+h()) {
		if (mInput && mInput->Contains(ix,iy)) {
			return mInput;
		}
		if (mOutput && mOutput->Contains(ix,iy)) {
			return mOutput;
		}
	}
	return 0;
}	
