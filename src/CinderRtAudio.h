#pragma once
#include "RtAudio.h"
#include <string>
#include <map>
#include <iostream>
#include "cinder/audio/Buffer.h"

namespace pk{

using AudioCb = std::function<void( cinder::audio::Buffer& )>;

class CinderRtAudio {
	public:
		struct StreamOptions {
			public:
				StreamOptions() = default;
				StreamOptions& setBufferSize( unsigned int bufferSize );
				StreamOptions& setInputDeviceId( unsigned int inputDeviceId );
				StreamOptions& setOutputDeviceId( unsigned int outputDeviceId );
				StreamOptions& setNumInputChannels( unsigned int numInputChannels );
				StreamOptions& setNumOutputChannels( unsigned int numOutputChannels );
				StreamOptions& setSampleRate( unsigned int sampleRate );
				StreamOptions& setAudioApi( RtAudio::Api api );

				const unsigned int& 	getBufferSize() const;
				const int& 				getInputDeviceId() const;
				const int& 				getOutputDeviceId() const;
				const unsigned int& 	getNumInputChannels() const;
				const unsigned int& 	getNumOutputChannels() const;
				const unsigned int& 	getSampleRate() const;
				const RtAudio::Api&		getAudioApi() const;
			private:
				int							mInputDeviceId{ -1 };
				int							mOutputDeviceId{ -1 };
				unsigned int 				mBufferSize{ 256 };
				unsigned int 				mNumInputChannels{ 0 };
				unsigned int 				mNumOutputChannels{ 0 };
				unsigned int 				mSampleRate{ 44100 };
				RtAudio::Api				mAudioApi{ RtAudio::Api::UNSPECIFIED }; // forces the use of the default API. Consult the RtAudio doc for more API options.
		};
	public:
		CinderRtAudio() = default;
		CinderRtAudio( const CinderRtAudio& ) = delete;
		CinderRtAudio& operator=( const RtAudio& ) = delete;


		void setAudioInCb( AudioCb audioInCb );
		void setAudioOutCb( AudioCb audioOutCb );
		void openStream( const StreamOptions& stream );
		void startStream();
		void stopStream();
		void closeStream();

		bool audioInputEnabled() const
		{
			return mStreamOptions.getNumInputChannels() > 0;
		}

		bool audioOutputEnabled() const
		{
			return mStreamOptions.getNumOutputChannels() > 0;
		}

		void parseDevices();

		unsigned int findDeviceIdByName( const std::string& deviceName );
	private:
		struct Device {
			unsigned int 				outputChannels 	= 0;
			unsigned int 				inputChannels 	= 0;
			unsigned int 				duplexChannels	= 0;
			unsigned int 				id				= 0;
			std::string					name;
			bool						isDefaultOutput	= false;
			bool						isDefaultInput	= false;
			std::vector<unsigned int> 	sampleRates;
			unsigned int 				preferredSampleRate	= 0;

		};
		friend std::ostream& operator<<( std::ostream& os, const Device& deviceInfo );
		static int rtAudioInOutCb( void* outputBuffer, void* inputBuffer, unsigned int nBufferSize, double streamTime, RtAudioStreamStatus status, void* data );
	private:
		std::string rtAudioFormatToString( RtAudioFormat audioFormat );

		using DeviceInfoMap = std::map<std::string, Device>;
	private:
		std::unique_ptr<RtAudio>					mRtAudioStream;
		DeviceInfoMap								mDeviceInfoMap;
		AudioCb										mAudioInputCb; 	// Called from the RtAudio streaming thread. 
		AudioCb										mAudioOutputCb;	// Called from the RtAudio streaming thread. 
		StreamOptions								mStreamOptions;
		cinder::audio::Buffer						mOutputBuffer; 	// Passed from the RtAudio streaming thread via mAudioOutputCb to a listener. Careful.
		cinder::audio::Buffer						mInputBuffer;	// Passed from the RtAudio streaming thread via mAudioInputCb to a listener. Careful.
};


} // namespace pk
