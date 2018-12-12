#pragma once

#include <memory>
#include <windows.h>
#include <Processing.NDI.Lib.h>
#include <thread>
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
		void initConnection(int index);

		int getIndexForSender(std::string name);

		bool mNdiInitialized;
		bool mReady;
		std::pair<ci::gl::Texture2dRef, long long> mVideoTexture;
		bool mNewFrame = false;
		bool getIsNewFrame();

		std::pair<std::string, long long> mMetadata;
		NDIlib_recv_instance_t mNdiReceiver;
		NDIlib_find_instance_t mNdiFinder;
		const NDIlib_source_t* mNdiSources = nullptr; // Owned by NDI.

		std::vector<std::string> mNDIsenderNames;
		int mCurrentIndex;
		std::string mPreferredSenderName;
		bool shouldWaitForPreferredSender();

		std::shared_ptr<std::thread> mSourceFindingThread;
		bool mQuitSourceFindingThread;
		void threadedSourceFind();
};
