#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Log.h"

#include "CinderNDIReceiver.h"

using namespace ci;
using namespace ci::app;

class BasicReceiverApp : public App {
  public:
	BasicReceiverApp();
	void update() override;
	void draw() override;
	void keyDown(KeyEvent event) override;

  private:
	CinderNDIReceiver mReceiver;
};



BasicReceiverApp::BasicReceiverApp()
	: mReceiver{}
{

}

void BasicReceiverApp::update()
{
	mReceiver.update();
	int senderIndex = mReceiver.getCurrentSenderIndex();
	std::string senderName = mReceiver.getCurrentSenderName();
	getWindow()->setTitle( "#" + std::to_string(senderIndex) + " " + senderName + " @ " + std::to_string( (int) getAverageFps() ) + " FPS" );

}

void BasicReceiverApp::draw()
{
	gl::clear( ColorA::black() );
	auto meta = mReceiver.getMetadata();
	auto tex = mReceiver.getVideoTexture();
	if( tex.first ) {
		Rectf centeredRect = Rectf( tex.first->getBounds() ).getCenteredFit( getWindowBounds(), true );
		gl::draw( tex.first, centeredRect );
	}
	CI_LOG_I( " Frame: " << tex.second << ", metadata: " << meta.first << " : " << meta.second );
}

void BasicReceiverApp::keyDown(KeyEvent event) {
	if (event.getCode() == KeyEvent::KEY_UP) {

	}
}

void prepareSettings( BasicReceiverApp::Settings* settings )
{
}

// This line tells Cinder to actually create and run the application.
CINDER_APP( BasicReceiverApp, RendererGl, prepareSettings )
