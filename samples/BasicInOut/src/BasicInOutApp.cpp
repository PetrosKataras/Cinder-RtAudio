#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "CinderRtAudio.h"
#include <queue>

using namespace ci;
using namespace ci::app;

// We'll create a new Cinder Application by deriving from the App class.
class BasicInOutApp : public App {
  public:
  	void setup() override;
	void mouseDrag( MouseEvent event ) override;

	void keyDown( KeyEvent event ) override;
	void keyUp( KeyEvent event ) override;

	void draw() override;
	/**** 

	The audioIn/Out cb's will be fired from the RtAudio streaming thread.
	If you plan to use the buffers in the outside world ( e.g update() / draw() etc. )
	you will need to take care of properly guarding them against concurrent access.

	****/
	void audioIn( cinder::audio::Buffer& outputBuffer ){ 
		if( mRecording )
			audioQueue.push( outputBuffer ); 
	};

	void audioOut( cinder::audio::Buffer& outputBuffer ){ 
		if( ! mRecording && audioQueue.size() > 0 ) {
			auto& inputBuffer = audioQueue.front();
			outputBuffer.copy( inputBuffer, inputBuffer.getNumFrames() );
			audioQueue.pop();
		}
	};

  private:
  	  std::queue<cinder::audio::Buffer> audioQueue;
  	  bool mRecording = false;
  	  pk::CinderRtAudio mAudioStream;
};

void prepareSettings( BasicInOutApp::Settings* settings )
{
}

void BasicInOutApp::setup()
{
	mAudioStream.parseDevices();

	// Called from the streaming thread while the stream is running.
	mAudioStream.setAudioInCb( std::bind( &BasicInOutApp::audioIn, this, std::placeholders::_1 ) );
	// Called from the streaming thread while the stream is running.
	mAudioStream.setAudioOutCb( std::bind( &BasicInOutApp::audioOut, this, std::placeholders::_1 ) );

	pk::CinderRtAudio::StreamOptions streamOptions;
	streamOptions.setNumInputChannels( 2 )
		.setNumOutputChannels( 2 );

	mAudioStream.openStream( streamOptions );
	mAudioStream.startStream();

}

void BasicInOutApp::mouseDrag( MouseEvent event )
{
}

void BasicInOutApp::keyUp( KeyEvent event )
{
	if( event.getChar() == 'r' ) {
		mRecording = false;
	}
}

void BasicInOutApp::keyDown( KeyEvent event )
{
	if( event.getChar() == 'f' ) {
		setFullScreen( ! isFullScreen() );
	}
	else if( event.getChar() == 'r' ) {
		mRecording = true;
	}
	else if( event.getCode() == KeyEvent::KEY_SPACE ) {
	}
	else if( event.getCode() == KeyEvent::KEY_ESCAPE ) {
		if( isFullScreen() )
			setFullScreen( false );
		else
			quit();
	}
}

void BasicInOutApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
}

// This line tells Cinder to actually create and run the application.
CINDER_APP( BasicInOutApp, RendererGl, prepareSettings )
