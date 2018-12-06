#include "CinderNDIReceiver.h"
#include <cstdio>

#include "cinder/Log.h"
#include "cinder/Surface.h"
#include <Processing.NDI.Recv.h>

CinderNDIReceiver::CinderNDIReceiver()
	: mNdiSources{ nullptr }
{
	if( ! NDIlib_is_supported_CPU() ) {
		CI_LOG_E( "Failed to initialize NDI because of unsupported CPU!" );
	}

	if( ! NDIlib_initialize() ) {
		CI_LOG_E( "Failed to initialize NDI!" );
	}

	const NDIlib_find_create_t NDI_find_create_desc = { true, nullptr };

	mNdiFinder = NDIlib_find_create( &NDI_find_create_desc );
	if( !mNdiFinder ) {
		CI_LOG_E( "Failed to create NDI finder!" );
	}
	;
	mNdiInitialized = false;
	mReady = false;
}

void CinderNDIReceiver::setup(std::string name) {
	int no_sources = 0;
	const NDIlib_source_t* p_sources = nullptr;
	while (!no_sources) {
		mNdiSources = NDIlib_find_get_sources(mNdiFinder, &no_sources, 1000);
		no_sources = findSources();
	}

	currentIndex = 0;
	preferredSenderName = name;
	if (shouldWaitForPreferredSender()) {
		CI_LOG_I("waiting for sender with name " << preferredSenderName);
	}
	initConnection(currentIndex);

	mNdiInitialized = true;
}

CinderNDIReceiver::~CinderNDIReceiver()
{
	if( mNdiFinder ) {
		NDIlib_find_destroy( mNdiFinder );	
	}
	if( mNdiReceiver ) {
		NDIlib_recv_destroy( mNdiReceiver );
	}

	NDIlib_destroy();
	mNdiInitialized = false;
}


int CinderNDIReceiver::getIndexForSender(std::string name) {
	if (name.empty()) return -1;

	if (NDIsenderNames.size() > 0) {
		for (int i = 0; i<(int)NDIsenderNames.size(); i++) {
			std::string test = NDIsenderNames.at(i);
			if (test.find(name) != std::string::npos) {
				return i;
			}
		}
	}
	return -1;
}

bool CinderNDIReceiver::shouldWaitForPreferredSender() {
	return !preferredSenderName.empty();
}

void CinderNDIReceiver::initConnection(int index)
{
	if (index != currentIndex) {
		currentIndex = index;
	}
	if( mNdiSources ) {
		
		NDIlib_recv_create_t NDI_recv_create_desc = 
		{
			mNdiSources[index],
			NDIlib_recv_color_format_e::NDIlib_recv_color_format_e_BGRX_BGRA,
			NDIlib_recv_bandwidth_highest,
			TRUE
		};

		mNdiReceiver = NDIlib_recv_create2( &NDI_recv_create_desc );
		if( ! mNdiReceiver ) {
			CI_LOG_E( "Failed to create NDI receiver!" );
		} else {
			CI_LOG_I("Connected to sender at index #" << std::to_string(index) << " OK");
		}


		const NDIlib_tally_t tally_state = { true, false };
		NDIlib_recv_set_tally( mNdiReceiver, &tally_state);

		if (shouldWaitForPreferredSender() && !mReady) {
			int preferredIndex = getIndexForSender(preferredSenderName);
			if (preferredIndex > -1) {
				mReady = true;
				switchSource(preferredIndex);
			} else {
				mReady = false;
			}
		} else {
			mReady = true;
		}
	}
}

int CinderNDIReceiver::findSources() {
	int numberOfSources = 0;
	NDIsenderNames.clear();
	const NDIlib_source_t* p_sources = nullptr;
	mNdiSources = NDIlib_find_get_sources(mNdiFinder, &numberOfSources, 0);

	for (int i = 0; i < numberOfSources; i++) {
		mNdiSources = NDIlib_find_get_sources(mNdiFinder, &numberOfSources, 1000);
		if (mNdiSources[i].p_ndi_name && mNdiSources[i].p_ndi_name[0]) {
			NDIsenderNames.push_back(mNdiSources[i].p_ndi_name);
		}
	}

	return numberOfSources;
}

bool CinderNDIReceiver::isReady() {
	return mReady;
}


void CinderNDIReceiver::update()
{
	if (!mNdiInitialized) {
		return;
	}

	// Check if we have at least one source
	int numberOfSources = 0;
	const NDIlib_source_t* p_sources = nullptr;
	mNdiSources = NDIlib_find_get_sources( mNdiFinder, &numberOfSources, 0 );

	if( numberOfSources == 0 || numberOfSources != getNumberOfSendersFound() ) {
		mReady = false;
		// Connections might take a while.. Wait for 10secs..
		numberOfSources = findSources();
	} else {
		// If we are here it means that the source has changed
		// so we need to rebuild our receiver.
		if( ! mReady ) {
			if( mNdiReceiver ) NDIlib_recv_destroy( mNdiReceiver );
			initConnection(currentIndex);
		}
	}

	if( mReady ) {

		for( int i = 0; i < 2; ++i ) {
			NDIlib_video_frame_t video_frame;
			//NDIlib_audio_frame_t audio_frame;
			NDIlib_metadata_frame_t metadata_frame;

			switch( NDIlib_recv_capture( mNdiReceiver, &video_frame, NULL, &metadata_frame, 0 ) )
			{
				// No data
			case NDIlib_frame_type_none:
			{
				//CI_LOG_I( "No data received. " );
				break;
			}

			// Video data
			case NDIlib_frame_type_video:
			{
				//CI_LOG_I( "Video data received with width: " << video_frame.xres << " and height: " << video_frame.yres );
				auto surface = ci::Surface::create( video_frame.p_data, video_frame.xres, video_frame.yres, video_frame.line_stride_in_bytes, ci::SurfaceChannelOrder::BGRA );
				mVideoTexture.first = ci::gl::Texture::create( *surface );
				mVideoTexture.first->setTopDown( true );
				mVideoTexture.second = video_frame.timecode;
				NDIlib_recv_free_video( mNdiReceiver, &video_frame );
				break;
			}

			// Audio data
			case NDIlib_frame_type_audio:
			{
				//CI_LOG_I( "Audio data received with " << audio_frame.no_samples << " number of samples" );
				//NDIlib_recv_free_audio( mNdiReceiver, &audio_frame );
				break;
			}

			// Meta data
			case NDIlib_frame_type_metadata:
			{
				CI_LOG_I( "Meta data received." );
				mMetadata.first = metadata_frame.p_data;
				mMetadata.second = metadata_frame.timecode;
				NDIlib_recv_free_metadata( mNdiReceiver, &metadata_frame );
				break;
			}
			}
		}
	}
}

int CinderNDIReceiver::getCurrentSenderIndex() {
	return currentIndex;
}

int CinderNDIReceiver::getNumberOfSendersFound() {
	return NDIsenderNames.size();
}

void CinderNDIReceiver::switchSource(int index) {
	initConnection(index);
}

std::string CinderNDIReceiver::getCurrentSenderName() {
	if (NDIsenderNames.size() == 0) {
		return "none";
	} else {
		return NDIsenderNames[currentIndex];
	}
}


std::pair<std::string, long long> CinderNDIReceiver::getMetadata()
{
	return mMetadata;
}

std::pair<ci::gl::Texture2dRef, long long> CinderNDIReceiver::getVideoTexture()
{
	return mVideoTexture;
}
