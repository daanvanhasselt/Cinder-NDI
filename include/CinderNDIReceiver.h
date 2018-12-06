#pragma once

#include <memory>
#include <windows.h>
#include <Processing.NDI.Lib.h>
#include "cinder/gl/Texture.h"

class CinderNDIReceiver{
	public:
		CinderNDIReceiver();
		~CinderNDIReceiver();

		void setup(std::string preferredSender = "");

		bool isReady();

		void update();
		std::pair<std::string, long long> getMetadata();
		std::pair<ci::gl::Texture2dRef, long long> getVideoTexture();

		int getCurrentSenderIndex();
		std::string getCurrentSenderName();
		int getNumberOfSendersFound();

		void switchSource(int index);

	private:
		int findSources();
		void initConnection(int index, bool waitForPreferredSender = false);

		int getIndexForSender(std::string name);

		bool mNdiInitialized;
		bool mReady;
		std::pair<ci::gl::Texture2dRef, long long> mVideoTexture;

		std::pair<std::string, long long> mMetadata;
		NDIlib_recv_instance_t mNdiReceiver;
		NDIlib_find_instance_t mNdiFinder;
		const NDIlib_source_t* mNdiSources = nullptr; // Owned by NDI.

		std::vector<std::string> NDIsenderNames;
		int currentIndex;
		std::string preferredSenderName;

};
