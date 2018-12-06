#pragma once

#include <memory>
#include <windows.h>
#include <Processing.NDI.Lib.h>
#include "cinder/gl/Texture.h"

class CinderNDIReceiver{
	public:
		CinderNDIReceiver(int index = 0);
		CinderNDIReceiver(std::string senderName);
		~CinderNDIReceiver();

		void update();
		std::pair<std::string, long long> getMetadata();
		std::pair<ci::gl::Texture2dRef, long long> getVideoTexture();
	private:
		void initConnection(int index);

		int getIndexForSender(std::string name);

		bool mNdiInitialized = false;
		bool mReadyToReceive = false;
		std::pair<ci::gl::Texture2dRef, long long> mVideoTexture;

		std::pair<std::string, long long> mMetadata;
		NDIlib_recv_instance_t mNdiReceiver;
		NDIlib_find_instance_t mNdiFinder;
		const NDIlib_source_t* mNdiSources = nullptr; // Owned by NDI.

		std::vector<std::string> NDIsenderNames;
		int currentIndex;

};
