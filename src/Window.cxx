#include "Window.hxx"
#include "Connector.hxx"
#include "Port.hxx"
#include <FL/Fl.H>
#include <FL/fl_ask.H>

#include "images/scicors.xpm"
#include "images/connect.xpm"
#include "images/question.xpm"
#include "images/alsalogo.xpm"

static Fl_Pixmap pixmap_scicors(scicors_xpm);
static Fl_Pixmap pixmap_connect(connect_xpm);
static Fl_Pixmap pixmap_question(question_xpm);
static Fl_Pixmap pixmap_alsalogo(alsalogo_xpm);

int aconnect_main(int argc,char** argv);

void Window::Connect(int client,int port,int clientB,int portB)
{
	Connector *a = 0,*b = 0;
	
	a = mClients->FindOutput(client,port);
	b = mClients->FindInput(clientB,portB);
//	if (a==0) printf("%d %d does not have an output\n",client,port);
//	if (b==0) printf("%d %d does not have an input\n",clientB,portB);
	if (a!=0 && b!=0) {
		Connect(a,b);
	}
}		

void Window::Disconnect(
	int fromClientId,int fromPortId,int toClientId,int toPortId)
{
	Connection* c = mConnections->Find(
		fromClientId,fromPortId,toClientId,toPortId);
/*
	if (!c)
		printf("could not find connection between %d:%d %d:%d\n",
			client,port,clientB,portB);
*/
	if (c)
	{
		Disconnect(c);
	}
}

void Window::Disconnect(Connection* c)
{
	mConnections->remove(c);
	mConnections->redraw();
	c->From()->Disconnect();
	c->To()->Disconnect();
	c->From()->redraw();
	c->To()->redraw();
	delete c;
	mConnections->Unclutter();
	mConnections->redraw();
}

void Window::Connect(Connector* a,Connector* b)
{
	a->Unselect();
	b->Unselect();

	Connection* c= new Connection(
		mConnections->x(),mConnections->y(),
		mConnections->w(),mConnections->h()
	);

	c->Set(a,b);

	a->Connect();
	b->Connect();

	a->redraw();
	b->redraw();

	mConnections->add(c);
	mConnections->Unclutter();
}

int Window::IsLegal(Connector* a,Connector* b)
{
	return a->Type()!=b->Type();
}

Client* Window::AddClient(snd_seq_client_info_t* cinfo)
{
	// some of this code should be in Window
	Client* c= mClients->FindClient(snd_seq_client_info_get_client(cinfo));
	
	if (c) {
		mCurClient = c;
	}else{
		mCurClient = new Client(
			mClients->x(),mClients->y()+mClients->h(),mClients->w(),20,
			strdup(snd_seq_client_info_get_name(cinfo)),
			snd_seq_client_info_get_client(cinfo)
		);
		int newh = mCurClient->y()+mCurClient->h()-mClients->y();
		
		mClients->size(mClients->w(),newh);
		mConnections->size(mConnections->w(),newh);
		mClients->add(mCurClient);
		size(w(),newh+33);
	}
	return mCurClient;
}
Port* Window::AddPort(snd_seq_port_info_t* pinfo)
{
	// some of this code should be in Window
	int dy = mCurClient->y()+mCurClient->h();
	mCurPort = mCurClient->AddPort(
		strdup(snd_seq_port_info_get_name(pinfo)),
	  snd_seq_port_info_get_port(pinfo),
		mCurType);
	dy = mCurClient->y()+mCurClient->h()-dy;
	for (int i = mClients->find(mCurClient)+1;i<mClients->children();i++)
	{
		mClients->child(i)->position(mClients->x(),mClients->child(i)->y()+dy);
	}
	int newh = mClients->child(mClients->children()-1)->y()+
		mClients->child(mClients->children()-1)->h()-
		mClients->y();
	mClients->size(mClients->w(),newh);
	mConnections->size(mConnections->w(),newh);
	size(w(),newh+33);
	return mCurPort;
}

bool Window::HandleConnect(Connector* a,Connector* b)
{
	int argc;
	char* argv[16];
	char from[16];
	char to[16];
	
	sprintf(from,"%d:%d",a->Owner()->ClientId(),a->Owner()->PortId());
	sprintf(to,"%d:%d",b->Owner()->ClientId(),b->Owner()->PortId());
	
	argc = 0;
	argv[argc++] = "aconnect";
	argv[argc++] = from;
	argv[argc++] = to;

	return (aconnect_main(argc,argv)==0);
}

bool Window::HandleDisconnect(Connection* c)
{
	int argc;
	char* argv[16];
	char from[16];
	char to[16];
	
	Connector* a = c->From();
	Connector* b = c->To();
	
	sprintf(from,"%d:%d",a->Owner()->ClientId(),a->Owner()->PortId());
	sprintf(to,"%d:%d",b->Owner()->ClientId(),b->Owner()->PortId());
	
	argc = 0;
	argv[argc++] = "aconnect";
	argv[argc++] = "-d";
	argv[argc++] = from;
	argv[argc++] = to;
	
	return (aconnect_main(argc,argv)==0);
}

Window::Window():Fl_Window(300,33,"ALSA Sequencer")
{
	mClients = new Clients(0,33,140,0);
	mConnections = new Connections(140,33,160,0);
	int cx = 0;
	
	Fl_Group* g = new Fl_Group(0,0,300,24);
	{
		Fl_Radio_Button* b = new Fl_Radio_Button(cx,0,24,24);
		b->box(FL_THIN_UP_BOX);
		b->down_box(FL_FLAT_BOX);
		pixmap_scicors.label(b);
		mCutButton = b;
		cx+=24;
	}
	{
		Fl_Radio_Button* b = new Fl_Radio_Button(cx,0,24,24);
		b->box(FL_THIN_UP_BOX);
		b->down_box(FL_FLAT_BOX);
		pixmap_connect.label(b);
		mConnectButton = b;
		cx+=24;
	}
	if (0) {
		Fl_Radio_Button* b = new Fl_Radio_Button(cx,0,24,24);
		b->box(FL_THIN_UP_BOX);
		b->down_box(FL_FLAT_BOX);
		pixmap_question.label(b);
		mInfoButton = b;
		cx+=24;
	}
	{
		Fl_Box* b = new Fl_Box(cx,0,300-cx,24);
		b->box(FL_THIN_UP_BOX);
		b->align(FL_ALIGN_INSIDE|FL_ALIGN_RIGHT);
		pixmap_alsalogo.label(b);
		cx+=24;
	}
	{
		Fl_Box* b = new Fl_Box(0,24,300,1);
		b->color(FL_WHITE);
		b->box(FL_FLAT_BOX);
	}
	g->end();

	mCurClient = 0;
	mCurPort = 0;
	mCurType = kNone;
	mHandle = 0;
	if (snd_seq_open(&mHandle,"hw",SND_SEQ_OPEN_INPUT,0)<0)
	{
		fl_alert("Error opening sequencer\n");
		exit(-1);
	}
	mClientId = snd_seq_client_id(mHandle);
	if(mClientId<0)
	{
		fl_alert("Error opening sequencer\n");
		exit(-1);
	}
	snd_seq_set_client_name(mHandle,"aconnectgui");
	unsigned int caps = SND_SEQ_PORT_CAP_WRITE;
	int port =  snd_seq_create_simple_port(mHandle, "Notify", caps,
		SND_SEQ_PORT_TYPE_MIDI_GENERIC |
		SND_SEQ_PORT_TYPE_APPLICATION);
		
	if (port<0)
	{
		fl_alert("Error creating port\n");
		exit(-1);
	}
	int err;
	if ((err=snd_seq_connect_from(mHandle,port,0,1))<0)
	{
		fl_alert("Error subscribing to System::Announce port [%d]\n",err);
		exit(-1);
	}

	mPollMax = snd_seq_poll_descriptors_count(mHandle,POLLIN);
	mPollFds = (struct pollfd *) calloc(mPollMax, sizeof(struct pollfd));

	snd_seq_nonblock(mHandle, 1);
}

Window::~Window()
{
	snd_seq_close(mHandle);
}


void Window::Timeout(void)
{
	int count;
	snd_seq_event_t *ev;

	do
	{
		snd_seq_event_input(mHandle, &ev);
		if (ev)
		{
			snd_seq_free_event(ev);
			if (ev->type==SND_SEQ_EVENT_PORT_SUBSCRIBED)
			{
				if (
					ev->data.connect.sender.client!=mClientId &&
					ev->data.connect.dest.client!=mClientId)
				{
					Connect(
						ev->data.connect.sender.client,
						ev->data.connect.sender.port,
						ev->data.connect.dest.client,
						ev->data.connect.dest.port);
					mConnections->redraw();
				}
			}
			if (ev->type==SND_SEQ_EVENT_PORT_UNSUBSCRIBED)
			{
				if (
					ev->data.connect.sender.client!=mClientId &&
					ev->data.connect.dest.client!=mClientId)
				{
					Disconnect(
						ev->data.connect.sender.client,
						ev->data.connect.sender.port,
						ev->data.connect.dest.client,
						ev->data.connect.dest.port);
					mConnections->redraw();
				}
			}
			if (ev->type==SND_SEQ_EVENT_CLIENT_START)
			{
				if (ev->data.addr.client != snd_seq_client_id(mHandle))
				{
					snd_seq_client_info_t *cinfo;
					snd_seq_port_info_t *pinfo;
					snd_seq_client_info_alloca(&cinfo);
					snd_seq_port_info_alloca(&pinfo);
					if (snd_seq_get_any_client_info(mHandle, ev->data.addr.client, cinfo) >= 0) {
						AddClient(cinfo);
						/* reset query info */
						count = 0;
						if (snd_seq_get_any_port_info(mHandle, 
							ev->data.addr.client,
							 ev->data.addr.port,
							pinfo) >= 0) {
							if (snd_seq_port_info_get_capability(pinfo) & (SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE))
							{
								mCurType = kInput;
								AddPort(pinfo);
							}
							if (snd_seq_port_info_get_capability(pinfo) & (SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ))
							{
								mCurType = kOutput;
								AddPort(pinfo);
							}
						}
					}
					fflush(stdout);
				}	
			}
			if (ev->type==SND_SEQ_EVENT_CLIENT_EXIT)
			{
				Client* c= mClients->FindClient(ev->data.addr.client);
				int newh = mClients->h()-c->h();
				mConnections->RemoveClient(ev->data.addr.client);
				mConnections->size(mConnections->w(),newh);
				mClients->remove(c);
				mClients->size(mClients->w(),newh);
				size(w(),newh+33);
			}
			snd_seq_free_event(ev);
		}
	} while (ev);

	Fl::add_timeout(0.1,Window::TimeoutStatic,this);
}

Window* patchbay = 0;

main()
{
	int argc;
	char* argv[16];
	
	Fl::get_system_colors();
	
	patchbay = new Window;

	/* NB the concept of input of aconnect and the  classes is reverse...
	*/

	patchbay->SetCurType(kOutput);
	argc = 0;
	argv[argc++] = "aconnect";
	argv[argc++] = "-i";
	argv[argc++] = "-l";
	aconnect_main(argc,argv);

	patchbay->SetCurType(kInput);
	argc = 0;
	argv[argc++] = "aconnect";
	argv[argc++] = "-o";
	argv[argc++] = "-l";
	aconnect_main(argc,argv);

	patchbay->show();

	Fl::add_timeout(0.1,Window::TimeoutStatic,patchbay);

	Fl::run();
}
