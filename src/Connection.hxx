#ifndef __Connection__
#define __Connection__

#include <FL/Fl_Widget.H>
#include <FL/Fl_Group.H>

class Connector;

class Connection:public Fl_Widget
{
friend class Connections;
protected:
	Connector *mFrom;
	Connector *mTo;
	Connector *mUpper; //used for unclutter
	Connector *mLower;
	
	int mLevel;
	bool mState;
	
public:
	Connector *From(void) { return mFrom; }
	Connector *To(void) { return mTo; }

	Connection(int X,int Y,int W,int H);
	
	void Set(Connector* a, Connector* b);

	void draw(void);

	int handle(int e);
};

class Connections:public Fl_Group
{
public:
	Connections(int X,int Y,int W,int H):Fl_Group(X,Y,W,H)
	{
		mIsRemoving = false;
		end();
	}
	void draw(void);
	
	int IsRemoving() { return mIsRemoving; }
	int Exists(Connector* a,Connector* b);
	Connection* Find(
		int fromClientId,int fromPortId,int toClientId,int toPortId);
	void RemoveClient(int id);

	void Unclutter(void);
	
	int DrawPhase(void) { return mDrawPhase; }
private:
	void ConflictTest(int i);
private:
	int mDrawPhase;
	int mIsRemoving;
};

#endif
