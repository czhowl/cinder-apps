#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "cinder/Serial.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class MouseApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;

	SerialRef                   mSerial;
	char						mBytes[4];
};

void MouseApp::setup()
{
	// -------------------------------- Serial Communication
	for (const auto& dev : Serial::getDevices())
		console() << "Device: " << dev.getName() << endl;

	//try {
	//	Serial::Device dev = Serial::findDeviceByNameContains("COM5");
	//	mSerial = Serial::create(dev, 250000);
	//}
	//catch (SerialExc & exc) {
	//	console() << "coult not initialize the serial device " << endl;
	//	//exit(-1);
	//}
}

void MouseApp::mouseDown( MouseEvent event )
{
}

void MouseApp::update()
{
	//if (mSerial->getNumBytesAvailable() > 0) {
	//	mSerial->readBytes(mBytes, 4);
	//	int pos1 = mBytes[1] << 8 + mBytes[0];
	//	int pos2 = mBytes[3] << 8 + mBytes[2];
	//	console() << pos1 << "   " << pos2 << endl;
	//}
	for (const auto& dev : Serial::getDevices())
		console() << "Device: " << dev.getName() << endl;
}

void MouseApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP( MouseApp, RendererGl )
