#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "cinder/Serial.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class MouseApp : public App {
  public:
	static void prepare(Settings* settings);
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;

	SerialRef                   mSerial;
	uint8_t						mBytes[4];

	int X;
	int Y;
};

void MouseApp::prepare(Settings* settings)
{
	settings->setTitle("This Damn Thesis");
	settings->setWindowSize(800, 800);
	settings->setHighDensityDisplayEnabled();
	//    settings->disableFrameRate();
}

void MouseApp::setup()
{
	// -------------------------------- Serial Communication
	for (const auto& dev : Serial::getDevices())
		console() << "Device: " << dev.getName() << endl;

	try {
		Serial::Device dev = Serial::Device("cu.usbmodem14603");
		//Serial::Device dev = Serial::Device("dev/");
		mSerial = Serial::create(dev, 115200);
	}
	catch (SerialExc & exc) {
		console() << "coult not initialize the serial device " << endl;
		exit(-1);
	}

//	uint8_t bytes[2] = { 0, 0 };
//	mSerial->writeBytes(bytes, 2);
//	while (true) {
//		if (mSerial->getNumBytesAvailable() > 0){
//			mSerial->readBytes(mBytes, 4);
//			uint16_t pos1 = (mBytes[1] << 8) | mBytes[0];
//			uint16_t pos2 = (mBytes[3] << 8) | mBytes[2];
//			X = pos1;
//			Y = pos2;
//			break;
//		}
		//console() << pos1 << "   " << pos2 << endl;
		//mSerial->flush();
//	}
}

void MouseApp::mouseDown( MouseEvent event )
{
}

void MouseApp::update()
{
	uint8_t bytes[2] = {0, 0};
	mSerial->writeBytes(bytes, 2);
	if (mSerial->getNumBytesAvailable() > 0) {
		mSerial->readBytes(mBytes, 4);
		uint16_t pos1 = (mBytes[1] << 8) | mBytes[0];
		uint16_t pos2 = (mBytes[3] << 8) | mBytes[2];
		//if (abs(pos1 - X) < 20) X = pos1;
		//if (abs(pos2 - Y) < 20) Y = pos2;
		X = pos1;
		Y = pos2;
//		console() << pos1 << "   " << pos2 << endl;
//		mSerial->flush();
	}
}

void MouseApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
	gl::drawSolidCircle(vec2(X, Y), 10, 30);
}

CINDER_APP( MouseApp, RendererGl )
