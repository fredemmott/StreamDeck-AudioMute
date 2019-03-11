#pragma once

#include <map>
#include <string>

enum class Role {
	DEFAULT,
	COMMUNICATION,
};

enum class Direction {
	OUTPUT,
	INPUT,
};

enum class MuteAction {
	UNMUTE,
	MUTE,
	TOGGLE,
};

std::map<std::string, std::string> GetAudioDeviceList(Direction);

std::string GetDefaultAudioDeviceID(Direction, Role);
void SetDefaultAudioDeviceID(Direction, Role, const std::string& deviceID);

bool IsAudioDeviceMuted(const std::string& deviceID);
void SetIsAudioDeviceMuted(const std::string& deviceID, MuteAction);