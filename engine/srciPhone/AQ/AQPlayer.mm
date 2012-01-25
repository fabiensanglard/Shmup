/*
 
    File: AQPlayer.mm
Abstract: n/a
 Version: 2.0

Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
Inc. ("Apple") in consideration of your agreement to the following
terms, and your use, installation, modification or redistribution of
this Apple software constitutes acceptance of these terms.  If you do
not agree with these terms, please do not use, install, modify or
redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and
subject to these terms, Apple grants you a personal, non-exclusive
license, under Apple's copyrights in this original Apple software (the
"Apple Software"), to use, reproduce, modify and redistribute the Apple
Software, with or without modifications, in source and/or binary forms;
provided that if you redistribute the Apple Software in its entirety and
without modifications, you must retain this notice and the following
text and disclaimers in all such redistributions of the Apple Software.
Neither the name, trademarks, service marks or logos of Apple Inc. may
be used to endorse or promote products derived from the Apple Software
without specific prior written permission from Apple.  Except as
expressly stated in this notice, no other rights or licenses, express or
implied, are granted by Apple herein, including but not limited to any
patent rights that may be infringed by your derivative works or by other
works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

Copyright (C) 2009 Apple Inc. All Rights Reserved.

 
*/


#include "AQPlayer.h"
#include "AudioToolbox/AudioFile.h"

void AQPlayer::AQBufferCallback(void *					inUserData,
								AudioQueueRef			inAQ,
								AudioQueueBufferRef		inCompleteAQBuffer) 
{
	AQPlayer *THIS = (AQPlayer *)inUserData;

	//printf("[AQBufferCallback] START starting mCurrentPacket=: %d for obj=%p. \n",(int)THIS->mCurrentPacket,THIS);
	
	if (THIS->mIsDone) return;

	UInt32 numBytes;
	UInt32 nPackets = THIS->GetNumPacketsToRead();
	OSStatus result = AudioFileReadPackets(THIS->GetAudioFileID(), false, &numBytes, inCompleteAQBuffer->mPacketDescriptions, THIS->GetCurrentPacket(), &nPackets, inCompleteAQBuffer->mAudioData);
	if (result)
		printf("AudioFileReadPackets failed: %ld", result);
	if (nPackets > 0) {
		inCompleteAQBuffer->mAudioDataByteSize = numBytes;		
		inCompleteAQBuffer->mPacketDescriptionCount = nPackets;		
		AudioQueueEnqueueBuffer(inAQ, inCompleteAQBuffer, 0, NULL);
		THIS->mCurrentPacket = (THIS->GetCurrentPacket() + nPackets);
	//	printf("[AQBufferCallback] THIS->mCurrentPacket = (THIS->GetCurrentPacket() + nPackets);\n");
	} 
	
	else 
	{
		if (THIS->IsLooping())
		{
			THIS->mCurrentPacket = 0;
	//		printf("[AQBufferCallback] THIS->mCurrentPacket = 0;\n");
			AQBufferCallback(inUserData, inAQ, inCompleteAQBuffer);
		}
		else
		{
			// stop
			THIS->mIsDone = true;
			AudioQueueStop(inAQ, false);
		}
	}
	
	//printf("[AQBufferCallback] END starting packet: %d.\n",(int)THIS->mCurrentPacket);
}

void AQPlayer::isRunningProc (  void *              inUserData,
								AudioQueueRef           inAQ,
								AudioQueuePropertyID    inID)
{
	AQPlayer *THIS = (AQPlayer *)inUserData;
	UInt32 size = sizeof(THIS->mIsRunning);
	OSStatus result = AudioQueueGetProperty (inAQ, kAudioQueueProperty_IsRunning, &THIS->mIsRunning, &size);
	
	if ((result == noErr) && (!THIS->mIsRunning))
		[[NSNotificationCenter defaultCenter] postNotificationName: @"playbackQueueStopped" object: nil];
}

void AQPlayer::CalculateBytesForTime (CAStreamBasicDescription & inDesc, UInt32 inMaxPacketSize, Float64 inSeconds, UInt32 *outBufferSize, UInt32 *outNumPackets)
{
	// we only use time here as a guideline
	// we're really trying to get somewhere between 16K and 64K buffers, but not allocate too much if we don't need it
	static const int maxBufferSize = 0x10000; // limit size to 64K
	static const int minBufferSize = 0x4000; // limit size to 16K
	
	if (inDesc.mFramesPerPacket) {
		Float64 numPacketsForTime = inDesc.mSampleRate / inDesc.mFramesPerPacket * inSeconds;
		*outBufferSize = numPacketsForTime * inMaxPacketSize;
	} else {
		// if frames per packet is zero, then the codec has no predictable packet == time
		// so we can't tailor this (we don't know how many Packets represent a time period
		// we'll just return a default buffer size
		*outBufferSize = maxBufferSize > inMaxPacketSize ? maxBufferSize : inMaxPacketSize;
	}
	
	// we're going to limit our size to our default
	if (*outBufferSize > maxBufferSize && *outBufferSize > inMaxPacketSize)
		*outBufferSize = maxBufferSize;
	else {
		// also make sure we're not too small - we don't want to go the disk for too small chunks
		if (*outBufferSize < minBufferSize)
			*outBufferSize = minBufferSize;
	}
	*outNumPackets = *outBufferSize / inMaxPacketSize;
}

AQPlayer::AQPlayer() :
	mQueue(0),
	mAudioFile(0),
	mFilePath(NULL),
	mIsRunning(false),
	mIsInitialized(false),
	mNumPacketsToRead(0),
	mCurrentPacket(0),
	mIsDone(false),
	mIsLooping(false) 
{
	printf("[CreateQueueForFile] Constructor   mCurrentPacket=%d for object %p.\n",(int)this->mCurrentPacket,this);
}

AQPlayer::~AQPlayer() 
{
	DisposeQueue(true);
}

OSStatus AQPlayer::CheckResult(OSStatus result,const char* methodName)
{
	
	char error = 1;
	
	switch (result) {
		case kAudioSessionNoError:		error=0;																		break;
		case kAudioSessionNotInitialized:			printf("AQPlayer error: kAudioSessionNotInitialized.");			break;
		case kAudioSessionAlreadyInitialized:		printf("AQPlayer error: kAudioSessionAlreadyInitialized.");		break;
		case kAudioSessionInitializationError:		printf("AQPlayer error: kAudioSessionInitializationError.")		;break;
		case kAudioSessionUnsupportedPropertyError:	printf("AQPlayer error: kAudioSessionUnsupportedPropertyError.");	break;
		case kAudioSessionBadPropertySizeError:		printf("AQPlayer error: kAudioSessionBadPropertySizeError.");		break;
		case kAudioSessionNotActiveError:			printf("AQPlayer error: kAudioSessionNotActiveError.");			break;
		case kAudioServicesNoHardwareError:			printf("AQPlayer error: kAudioServicesNoHardwareError.");			break;
		case kAudioSessionNoCategorySet:			printf("AQPlayer error: kAudioSessionNoCategorySet.");			break;			
		case kAudioSessionIncompatibleCategory:		printf("AQPlayer error: kAudioSessionIncompatibleCategory.");		break;
		default:																										break;
	}
	
	if (error)
		printf("After method: %s.\n",methodName);
	
	return result;
}

OSStatus AQPlayer::StartQueue(Boolean inResume)
{	
	
	OSStatus result;
	UInt32 category;
	
	// if we have a file but no queue, create one now
	if ((mQueue == NULL) && (mFilePath != NULL))
		CreateQueueForFile(mFilePath,0);
		
	category = kAudioSessionCategory_MediaPlayback;
	
	result = AudioSessionInitialize(CFRunLoopGetCurrent(),kCFRunLoopDefaultMode,NULL,NULL);
	
	AQPlayer::CheckResult(result,"AudioSessionSetProperty") ;
	
	result = AudioSessionSetProperty(kAudioSessionProperty_AudioCategory, sizeof(category), &category);
	
	
	
	AQPlayer::CheckResult(result,"AudioSessionSetProperty") ;
	
	result = AudioSessionSetActive(true);

	AQPlayer::CheckResult(result,"AudioSessionSetActive") ;
	
	mIsDone = false;
	
	// if we are not resuming, we also should restart the file read index
	//if (!inResume)
	//	mCurrentPacket = 0;	

	// prime the queue with some data before starting
	for (int i = 0; i < kNumberBuffers; ++i) 
	{
		AQBufferCallback (this, mQueue, mBuffers[i]);			
	}
	
	return AudioQueueStart(mQueue, NULL);
}

OSStatus AQPlayer::StopQueue()
{
	OSStatus result = AudioQueueStop(mQueue, true);
	if (result) printf("ERROR STOPPING QUEUE!\n");
	else
	{
		result = AudioSessionSetActive(false);
		if (result) printf("ERROR SETTING AUDIO SESSION INACTIVE!\n");
	}
	return result;
}

void	AQPlayer::Pause()
{
	AudioQueuePause(mQueue);
}

void	AQPlayer::Resume()
{
	AudioQueueStart(mQueue, NULL);
}



void AQPlayer::CreateQueueForFile(CFStringRef inFilePath, int startAt) 
{	
	CFURLRef sndFile = NULL; 
	
	try {					
		if (mFilePath == NULL)
		{
			mIsLooping = false;
			
			sndFile = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, inFilePath, kCFURLPOSIXPathStyle, false);
			if (!sndFile) { printf("can't parse file path\n"); return; }
			
			XThrowIfError(AudioFileOpenURL (sndFile, kAudioFileReadPermission, 0/*inFileTypeHint*/, &mAudioFile), "[Audio Queue] Can't open sound file");
		
			UInt32 size = sizeof(mDataFormat);
			XThrowIfError(AudioFileGetProperty(mAudioFile, kAudioFilePropertyDataFormat, &size, &mDataFormat), "couldn't get file's data format");
			
			//Added by fabien
			/*
			UInt64 nPackets;
			UInt32 propsize = sizeof(nPackets);
			AudioFileGetProperty(mAudioFile, kAudioFilePropertyAudioDataPacketCount, &propsize , &nPackets);
			*/
			
			UInt64 nPackets;
			UInt32 propsize = sizeof(nPackets);
			
			XThrowIfError (AudioFileGetProperty(mAudioFile, kAudioFilePropertyAudioDataPacketCount, &propsize, &nPackets), "kAudioFilePropertyAudioDataPacketCount");
			Float64 fileDuration = (nPackets * mDataFormat.mFramesPerPacket) / mDataFormat.mSampleRate;

			
			//printf("[AQPlayer] This soundtrack has %llu packets.\n",nPackets);
			//printf("[AQPlayer] This soundtrack has mFramesPerPacket=%lu.\n",mDataFormat.mFramesPerPacket);
			//printf("[AQPlayer] This soundtrack has mSampleRate=%.2f.\n",mDataFormat.mSampleRate);
			//printf("[AQPlayer] Duration: %.2f.\n",fileDuration);
//			AQPlayer *THIS = (AQPlayer *)inUserData;
			
			float f = startAt/fileDuration;
			f=  MAX(0,f);
			f=  MIN(1,f);
			
			this->mCurrentPacket = f*fileDuration/fileDuration*nPackets;
//			printf("[AQBufferCallback] starting packet: %d.\n",(int)THIS->mCurrentPacket);
			//printf("[CreateQueueForFile] just   mCurrentPacket=%d for object %p.\n",(int)this->mCurrentPacket,this);
			
			mFilePath = CFStringCreateCopy(kCFAllocatorDefault, inFilePath);
		}
		SetupNewQueue();		
	}
	catch (CAXException e) {
		char buf[256];
		fprintf(stderr, "Error: %s (%s) with file '%s' \n", e.mOperation, e.FormatError(buf),(char*)inFilePath);
	}
	if (sndFile)
		CFRelease(sndFile);
}

void AQPlayer::SetupNewQueue() 
{
	XThrowIfError(AudioQueueNewOutput(&mDataFormat, AQPlayer::AQBufferCallback, this, 
										CFRunLoopGetCurrent(), kCFRunLoopCommonModes, 0, &mQueue), "AudioQueueNew failed");
	UInt32 bufferByteSize;		
	// we need to calculate how many packets we read at a time, and how big a buffer we need
	// we base this on the size of the packets in the file and an approximate duration for each buffer
	// first check to see what the max size of a packet is - if it is bigger
	// than our allocation default size, that needs to become larger
	UInt32 maxPacketSize;
	UInt32 size = sizeof(maxPacketSize);
	XThrowIfError(AudioFileGetProperty(mAudioFile, 
									   kAudioFilePropertyPacketSizeUpperBound, &size, &maxPacketSize), "couldn't get file's max packet size");
	
	// adjust buffer size to represent about a half second of audio based on this format
	CalculateBytesForTime (mDataFormat, maxPacketSize, kBufferDurationSeconds, &bufferByteSize, &mNumPacketsToRead);

		//printf ("Buffer Byte Size: %d, Num Packets to Read: %d\n", (int)bufferByteSize, (int)mNumPacketsToRead);
	
	// (2) If the file has a cookie, we should get it and set it on the AQ
	size = sizeof(UInt32);
	OSStatus result = AudioFileGetPropertyInfo (mAudioFile, kAudioFilePropertyMagicCookieData, &size, NULL);
	
	if (!result && size) {
		char* cookie = new char [size];		
		XThrowIfError (AudioFileGetProperty (mAudioFile, kAudioFilePropertyMagicCookieData, &size, cookie), "get cookie from file");
		XThrowIfError (AudioQueueSetProperty(mQueue, kAudioQueueProperty_MagicCookie, cookie, size), "set cookie on queue");
		delete [] cookie;
	}
	
	// channel layout?
	result = AudioFileGetPropertyInfo(mAudioFile, kAudioFilePropertyChannelLayout, &size, NULL);
	if (result == noErr && size > 0) {
		AudioChannelLayout *acl = (AudioChannelLayout *)malloc(size);
		XThrowIfError(AudioFileGetProperty(mAudioFile, kAudioFilePropertyChannelLayout, &size, acl), "get audio file's channel layout");
		XThrowIfError(AudioQueueSetProperty(mQueue, kAudioQueueProperty_ChannelLayout, acl, size), "set channel layout on queue");
		free(acl);
	}
	
	XThrowIfError(AudioQueueAddPropertyListener(mQueue, kAudioQueueProperty_IsRunning, isRunningProc, this), "adding property listener");
	
	bool isFormatVBR = (mDataFormat.mBytesPerPacket == 0 || mDataFormat.mFramesPerPacket == 0);
	for (int i = 0; i < kNumberBuffers; ++i) {
		XThrowIfError(AudioQueueAllocateBufferWithPacketDescriptions(mQueue, bufferByteSize, (isFormatVBR ? mNumPacketsToRead : 0), &mBuffers[i]), "AudioQueueAllocateBuffer failed");
	}	

	// set the volume of the queue
	XThrowIfError (AudioQueueSetParameter(mQueue, kAudioQueueParam_Volume, 1.0), "set queue volume");
	
	mIsInitialized = true;
}

void AQPlayer::DisposeQueue(Boolean inDisposeFile)
{
	if (mQueue)
	{
		AudioQueueDispose(mQueue, true);
		mQueue = NULL;
	}
	if (inDisposeFile)
	{
		if (mAudioFile)
		{		
			AudioFileClose(mAudioFile);
			mAudioFile = 0;
		}
		if (mFilePath)
		{
			CFRelease(mFilePath);
			mFilePath = NULL;
		}
	}
	mIsInitialized = false;
}