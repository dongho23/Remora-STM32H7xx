#ifndef COMMSINTERFACE_H
#define COMMSINTERFACE_H

class CommsInterface {
private:

public:
	CommsInterface();

	virtual void init();
	virtual void start();
	virtual void tasks();
};

#endif
