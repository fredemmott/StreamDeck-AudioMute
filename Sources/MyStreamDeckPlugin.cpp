//==============================================================================
/**
@file       MyStreamDeckPlugin.cpp

@brief      CPU plugin

@copyright  (c) 2018, Corsair Memory, Inc.
			This source code is licensed under the MIT-style license found in the LICENSE file.

**/
//==============================================================================

#include "MyStreamDeckPlugin.h"
#include <atomic>

#include "AudioFunctions.h"
#include "Common/ESDConnectionManager.h"

class CallBackTimer
{
public:
	CallBackTimer() :_execute(false) { }

	~CallBackTimer()
	{
		if (_execute.load(std::memory_order_acquire))
		{
			stop();
		};
	}

	void stop()
	{
		_execute.store(false, std::memory_order_release);
		if (_thd.joinable())
			_thd.join();
	}

	void start(int interval, std::function<void(void)> func)
	{
		if (_execute.load(std::memory_order_acquire))
		{
			stop();
		};
		_execute.store(true, std::memory_order_release);
		_thd = std::thread([this, interval, func]()
		{
			CoInitialize(NULL); // initialize COM again for the timer thread
			while (_execute.load(std::memory_order_acquire))
			{
				func();
				std::this_thread::sleep_for(std::chrono::milliseconds(interval));
			}
		});
	}

	bool is_running() const noexcept
	{
		return (_execute.load(std::memory_order_acquire) && _thd.joinable());
	}

private:
	std::atomic<bool> _execute;
	std::thread _thd;
};

MyStreamDeckPlugin::MyStreamDeckPlugin()
{
	CoInitialize(NULL); // initialize COM for the main thread
	mTimer = new CallBackTimer();
	mTimer->start(500, [this]()
	{
		this->UpdateTimer();
	});
}

MyStreamDeckPlugin::~MyStreamDeckPlugin()
{
	if (mTimer != nullptr)
	{
		mTimer->stop();

		delete mTimer;
		mTimer = nullptr;
	}
}

void MyStreamDeckPlugin::UpdateTimer()
{
	//
	// Warning: UpdateTimer() is running in the timer thread
	//
	if (mConnectionManager != nullptr)
	{
		mVisibleContextsMutex.lock();
		const bool isMuted = IsMuted();
		for (const std::string& context : mVisibleContexts)
		{
			mConnectionManager->SetState(isMuted ? 0 : 1, context);
		}
		mVisibleContextsMutex.unlock();
	}
}

bool MyStreamDeckPlugin::IsMuted()
{
	return IsAudioDeviceMuted(GetDefaultAudioDeviceID(Direction::INPUT, Role::COMMUNICATION));
}

void MyStreamDeckPlugin::KeyDownForAction(const std::string& inAction, const std::string& inContext, const json &inPayload, const std::string& inDeviceID)
{
	// don't race the timer, which leads to flickering
	mVisibleContextsMutex.lock();
	SetIsAudioDeviceMuted(GetDefaultAudioDeviceID(Direction::INPUT, Role::COMMUNICATION), MuteAction::TOGGLE);
}

void MyStreamDeckPlugin::KeyUpForAction(const std::string& inAction, const std::string& inContext, const json &inPayload, const std::string& inDeviceID)
{
	mVisibleContextsMutex.unlock();
	// Nothing to do
}

void MyStreamDeckPlugin::WillAppearForAction(const std::string& inAction, const std::string& inContext, const json &inPayload, const std::string& inDeviceID)
{
	// Remember the context
	mVisibleContextsMutex.lock();
	mVisibleContexts.insert(inContext);
	mConnectionManager->SetState(IsMuted() ? 0 : 1, inContext);
	mVisibleContextsMutex.unlock();
}

void MyStreamDeckPlugin::WillDisappearForAction(const std::string& inAction, const std::string& inContext, const json &inPayload, const std::string& inDeviceID)
{
	// Remove the context
	mVisibleContextsMutex.lock();
	mVisibleContexts.erase(inContext);
	mVisibleContextsMutex.unlock();
}

void MyStreamDeckPlugin::DeviceDidConnect(const std::string& inDeviceID, const json &inDeviceInfo)
{
	// Nothing to do
}

void MyStreamDeckPlugin::DeviceDidDisconnect(const std::string& inDeviceID)
{
	// Nothing to do
}

void MyStreamDeckPlugin::DidReceiveGlobalSettings(const json &)
{
}

void MyStreamDeckPlugin::SendToPlugin(const std::string & inAction, const std::string & inContext, const json & inPayload, const std::string & inDevice)
{
}
