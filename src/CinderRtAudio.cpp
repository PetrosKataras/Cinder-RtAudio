#include "CinderRtAudio.h"

#include "cinder/Log.h"
#include "cinder/app/AppBase.h"

namespace pk {
///////////////////////////////////
// ------------------------- Format
CinderRtAudio::StreamOptions& CinderRtAudio::StreamOptions::setBufferSize( unsigned int bufferSize )
{
	mBufferSize = bufferSize;
	return *this;
}

CinderRtAudio::StreamOptions& CinderRtAudio::StreamOptions::setInputDeviceId( unsigned int inputDeviceId )
{
	mInputDeviceId = inputDeviceId;
	return *this;
}

CinderRtAudio::StreamOptions& CinderRtAudio::StreamOptions::setOutputDeviceId( unsigned int outputDeviceId )
{
	mOutputDeviceId = outputDeviceId;
	return *this;
}

CinderRtAudio::StreamOptions& CinderRtAudio::StreamOptions::setNumInputChannels( unsigned int numInputChannels )
{
	mNumInputChannels = numInputChannels;
	return *this;
}

CinderRtAudio::StreamOptions& CinderRtAudio::StreamOptions::setNumOutputChannels( unsigned int numOutputChannels )
{
	mNumOutputChannels = numOutputChannels;
	return *this;
}

CinderRtAudio::StreamOptions& CinderRtAudio::StreamOptions::setAudioApi( RtAudio::Api api )
{
	mAudioApi = api;
	return *this;
}

const unsigned int& CinderRtAudio::StreamOptions::getBufferSize() const
{
	return mBufferSize;
}

const int& CinderRtAudio::StreamOptions::getInputDeviceId() const
{
	return mInputDeviceId;
}

const int& CinderRtAudio::StreamOptions::getOutputDeviceId() const
{
	return mOutputDeviceId;
}

const unsigned int& CinderRtAudio::StreamOptions::getNumInputChannels() const
{
	return mNumInputChannels;
}

const unsigned int& CinderRtAudio::StreamOptions::getNumOutputChannels() const
{
	return mNumOutputChannels;
}

const unsigned int& CinderRtAudio::StreamOptions::getSampleRate() const
{
	return mSampleRate;
}

const RtAudio::Api& CinderRtAudio::StreamOptions::getAudioApi() const
{
	return mAudioApi;
}
////////////////////////////////////////////
// --------------------------- CinderRtAudio
void CinderRtAudio::setAudioInCb( AudioCb audioInCb )
{
	mAudioInputCb = audioInCb;
}

void CinderRtAudio::setAudioOutCb( AudioCb audioOutCb )
{
	mAudioOutputCb = audioOutCb;
}

void CinderRtAudio::openStream( const StreamOptions& streamOptions )
{
	// Close ( if any ) currently open stream.
	if( mRtAudioStream )
		closeStream();

	mStreamOptions = streamOptions;

	mRtAudioStream.reset( new RtAudio( mStreamOptions.getAudioApi() ) );

	auto bufferSize = mStreamOptions.getBufferSize();
	auto sampleRate	= mStreamOptions.getSampleRate(); 

	RtAudio::StreamOptions rtAudioStreamOptions;
	rtAudioStreamOptions.flags = RTAUDIO_NONINTERLEAVED | RTAUDIO_SCHEDULE_REALTIME;
	RtAudio::StreamParameters outputParams, inputParams;
	
	auto setRtAudioStreamParams = []( RtAudio::StreamParameters& streamParams, unsigned int deviceId, int numChannels ) {
		streamParams.deviceId 	= deviceId;
		streamParams.nChannels 	= numChannels; 
	};

	// Input
	auto deviceId = mStreamOptions.getInputDeviceId(); 
	if( deviceId == -1 ) {
		deviceId = mRtAudioStream->getDefaultInputDevice();
	}
	auto numChannels = mStreamOptions.getNumInputChannels();
	if( numChannels > 0 ) {
		setRtAudioStreamParams( inputParams, deviceId, numChannels );
		mInputBuffer = cinder::audio::Buffer( bufferSize, numChannels );
	}

	// Output
	deviceId = mStreamOptions.getOutputDeviceId(); 
	if( deviceId == -1 ) {
		deviceId = mRtAudioStream->getDefaultOutputDevice();
	}
	numChannels = mStreamOptions.getNumOutputChannels();	
	if( numChannels > 0 ) {
		setRtAudioStreamParams( outputParams, deviceId, numChannels );
		mOutputBuffer = cinder::audio::Buffer( bufferSize, numChannels );
	}

	try {
		mRtAudioStream->openStream( outputParams.nChannels > 0 ? &outputParams : nullptr
				,inputParams.nChannels > 0 ? &inputParams : nullptr
				, RTAUDIO_FLOAT32
				, sampleRate
				, &bufferSize
				, &rtAudioInOutCb
				, this
				, &rtAudioStreamOptions );
	}
	catch( std::exception& e ) {
		mRtAudioStream.reset();
		CI_LOG_EXCEPTION( " Failed to open RtAudio stream ! ", e );
	}
}

void CinderRtAudio::startStream()
{
	if( ! mRtAudioStream ) {
		CI_LOG_W( "RtAudioStream is not open !" );
		return;
	}

	if( ! mRtAudioStream->isStreamRunning() ) {
		try {
			mRtAudioStream->startStream();
		}
		catch( std::exception& e ) {
			CI_LOG_EXCEPTION( "Failed to start RtAudio stream ! ", e );
		}
	}
}

void CinderRtAudio::stopStream()
{
	if( ! mRtAudioStream ) {
		CI_LOG_W( "RtAudioStream is not open !" );
		return;
	}

	if( mRtAudioStream->isStreamRunning() ) {
		try {
			mRtAudioStream->stopStream();
		}
		catch( std::exception& e ) {
			CI_LOG_EXCEPTION( "Failed to stop RtAudio stream ! ", e );
		}
	}
}

void CinderRtAudio::closeStream()
{
	if( ! mRtAudioStream ) {
		CI_LOG_W( "RtAudioStream is not open !" );
		return;
	}

	if( mRtAudioStream->isStreamOpen() ) {
		try {
			mRtAudioStream->closeStream();
		}
		catch( std::exception& e ) {
			CI_LOG_EXCEPTION( "Failed to close RtAudio stream ! ", e );
		}
	}
}

int CinderRtAudio::rtAudioInOutCb( void* outputBuffer, void* inputBuffer, unsigned int nBufferSize, double streamTime, RtAudioStreamStatus status, void* data )
{
	auto stream = static_cast<CinderRtAudio*>( data );
	
	if( stream->audioInputEnabled() ) {
		if( stream->mAudioInputCb ) {
			std::memcpy( stream->mInputBuffer.getData(), (float*)inputBuffer, sizeof( float ) * nBufferSize * stream->mStreamOptions.getNumInputChannels() ); 
			stream->mAudioInputCb( stream->mInputBuffer );
		}
	}

	if( stream->audioOutputEnabled() ) {
		if( stream->mAudioOutputCb ) {
			stream->mAudioOutputCb( stream->mOutputBuffer );
			std::memcpy( (float*)outputBuffer, stream->mOutputBuffer.getData(), sizeof( float ) * nBufferSize * stream->mStreamOptions.getNumOutputChannels() ); 
			stream->mOutputBuffer.zero();
		}
	}
	return 0;
}

unsigned int CinderRtAudio::findDeviceIdByName( const std::string& deviceName )
{
	for( const auto& device : mDeviceInfoMap ) {
		if( device.first.compare( deviceName ) == 0 ) {
			return device.second.id;
		}
	}
	return -1;
}

void CinderRtAudio::parseDevices()
{
	RtAudio audioDevices;
	unsigned int devicesCount = audioDevices.getDeviceCount();
	CI_LOG_I( "Found : " << std::to_string( devicesCount ) << " audio devices.. " );

	Device device;
	for( unsigned int deviceIndex = 0; deviceIndex < devicesCount; ++deviceIndex ) {
		RtAudio::DeviceInfo rtAudioDeviceInfo = audioDevices.getDeviceInfo( deviceIndex );
		// If we probed the device successfully parse it.
		if( rtAudioDeviceInfo.probed ) {
			device.name					= rtAudioDeviceInfo.name;
			device.id					= deviceIndex;
			device.outputChannels		= rtAudioDeviceInfo.outputChannels;
			device.inputChannels		= rtAudioDeviceInfo.inputChannels;
			device.duplexChannels		= rtAudioDeviceInfo.duplexChannels;
			device.isDefaultOutput		= rtAudioDeviceInfo.isDefaultOutput;
			device.isDefaultInput		= rtAudioDeviceInfo.isDefaultInput;
			device.sampleRates			= rtAudioDeviceInfo.sampleRates;
			device.preferredSampleRate	= rtAudioDeviceInfo.preferredSampleRate;

			auto deviceAlreadyParsed = [ this ]( std::string name ) -> bool {
				auto search = mDeviceInfoMap.find( name );
				if( search != mDeviceInfoMap.end() ) {
					return true;
				}
				return false;
			};
			// If we haven't added to our device map yet add it.
			if( ! deviceAlreadyParsed( device.name ) ) {
				mDeviceInfoMap[ device.name ] = device;
			}

			CI_LOG_I( device );
		}
	}
}

std::string CinderRtAudio::rtAudioFormatToString( RtAudioFormat audioFormat )
{
	if( audioFormat & RTAUDIO_SINT8 ) {
		return "8-bit int";
	}
	else if( audioFormat & RTAUDIO_SINT16 ) {
		return "16-bit int";
	}
	else if( audioFormat & RTAUDIO_SINT24 ) {
		return "24-bit int";
	}
	else if( audioFormat & RTAUDIO_SINT32 ) {
		return "32-bit int";
	}
	else if( audioFormat & RTAUDIO_FLOAT32 ) {
		return "32-bit float";
	}
	else if( audioFormat & RTAUDIO_FLOAT64 ) {
		return "64-bit float";
	}
	else {
		return "UNKNOWN";
	}
}

std::ostream& operator<<( std::ostream& os, const CinderRtAudio::Device& device )
{
	os << "Device name : " << device.name << "\n";
	os << "Num output channels : " << device.outputChannels << "\n";
	os << "Num input channels : " << device.inputChannels << "\n";
	os << "Num duplex channels : " << device.duplexChannels << "\n";
	os << "Is default output : " << ( device.isDefaultOutput ? "yes" : "no" ) << "\n";
	os << "Is default input : " << ( device.isDefaultInput ? "yes" : "no" ) << "\n";
	for( const auto& supportedSamplerate : device.sampleRates ) {
		os << "Supported samplerate : " << supportedSamplerate << "\n";
	}
	os << "Preferred samplerate : " << device.preferredSampleRate << "\n";
	return os;
}
} // namespace pk


