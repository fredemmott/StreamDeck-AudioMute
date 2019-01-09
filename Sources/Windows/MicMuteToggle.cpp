#include "windows.h"
#include "mmdeviceapi.h"
#include "mmsystem.h"
#include "endpointvolume.h"

#include "resource.h"
#include "../MicMuteToggle.h"

IAudioEndpointVolume* GetMicrophoneEndpoint() {
	IMMDeviceEnumerator* de;
	CoCreateInstance(
		__uuidof(MMDeviceEnumerator), NULL,
		CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),
		(void**)&de
	);

	IMMDevice* micDevicePtr;
	de->GetDefaultAudioEndpoint(EDataFlow::eCapture, ERole::eCommunications, &micDevicePtr);

	IAudioEndpointVolume* micVolume;
	micDevicePtr->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, nullptr, (void**)&micVolume);
	micDevicePtr->Release();
	de->Release();
	return micVolume;
}

bool IsMuted() {
	BOOL muted;
	auto me = GetMicrophoneEndpoint();
	me->GetMute(&muted);
	me->Release();
	return muted;
}

void SetMuted(MuteBehavior SetTo) {
	BOOL wasMuted;
	auto micVolume = GetMicrophoneEndpoint();
	micVolume->GetMute(&wasMuted);
	if (wasMuted && SetTo == MuteBehavior::MUTE) {
		return;
	}
	if (!wasMuted && SetTo == MuteBehavior::UNMUTE) {
		return;
	}

	const auto muteWav = MAKEINTRESOURCE(IDR_MUTE);
	const auto unmuteWav = MAKEINTRESOURCE(IDR_UNMUTE);
	const auto feedbackWav = wasMuted ? unmuteWav : muteWav;

	micVolume->SetMute(!wasMuted, nullptr);
	micVolume->Release();
}
