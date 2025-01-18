#include "commsHandler.h"
#include "../../remora.h"

CommsHandler::CommsHandler() : data(false), noDataCount(0), status(false) {

}

CommsHandler::~CommsHandler() {}

void CommsHandler::update() {
	if (data)
	{
		noDataCount = 0;
		status = true;
	}
	else
	{
		noDataCount++;
	}

	if (noDataCount > Config::dataErrMax)
	{
		noDataCount = 0;
		status = false;
	}

	data = false;
}

void CommsHandler::commsTasks() {

}
