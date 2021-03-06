/*
 * hidl interface for wpa_hostapd daemon
 * Copyright (c) 2004-2018, Jouni Malinen <j@w1.fi>
 * Copyright (c) 2004-2018, Roshan Pius <rpius@google.com>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef HOSTAPD_HIDL_SUPPLICANT_H
#define HOSTAPD_HIDL_SUPPLICANT_H

#include <string>

#include <android-base/macros.h>

#include <android/hardware/wifi/hostapd/1.3/IHostapd.h>
#include <android/hardware/wifi/hostapd/1.3/IHostapdCallback.h>

extern "C"
{
#include "utils/common.h"
#include "utils/eloop.h"
#include "utils/includes.h"
#include "utils/wpa_debug.h"
#include "ap/hostapd.h"
#include "ap/sta_info.h"
}

class DeathNotifier : public android::hardware::hidl_death_recipient
{
public:
	void serviceDied(
	    uint64_t /*cookie*/,
	    const android::wp<android::hidl::base::V1_0::IBase>
		& /* who */) override
	{
		wpa_printf(MSG_ERROR, "Client died. Terminating...");
		eloop_terminate();
	}
};

namespace android {
namespace hardware {
namespace wifi {
namespace hostapd {
namespace V1_3 {
namespace implementation {
using namespace android::hardware::wifi::hostapd::V1_0;

/**
 * Implementation of the hostapd hidl object. This hidl
 * object is used core for global control operations on
 * hostapd.
 */
class Hostapd : public V1_3::IHostapd
{
public:
	Hostapd(hapd_interfaces* interfaces);
	~Hostapd() override = default;

	// Hidl methods exposed.
	Return<void> addAccessPoint(
	    const V1_0::IHostapd::IfaceParams& iface_params,
	    const V1_0::IHostapd::NetworkParams& nw_params, addAccessPoint_cb _hidl_cb) override;
	Return<void> addAccessPoint_1_1(
	    const V1_1::IHostapd::IfaceParams& iface_params,
	    const V1_0::IHostapd::NetworkParams& nw_params, addAccessPoint_cb _hidl_cb) override;
	Return<void> addAccessPoint_1_2(
	    const V1_2::IHostapd::IfaceParams& iface_params,
	    const V1_2::IHostapd::NetworkParams& nw_params,
	    addAccessPoint_1_2_cb _hidl_cb) override;
	Return<void> addAccessPoint_1_3(
	    const V1_3::IHostapd::IfaceParams& iface_params,
	    const V1_3::IHostapd::NetworkParams& nw_params,
	    addAccessPoint_1_3_cb _hidl_cb) override;
	Return<void> removeAccessPoint(
	    const hidl_string& iface_name,
	    removeAccessPoint_cb _hidl_cb) override;
	Return<void> terminate() override;
	Return<void> registerCallback(
	    const sp<V1_1::IHostapdCallback>& callback,
	    registerCallback_cb _hidl_cb) override;
	Return<void> registerCallback_1_3(
	    const sp<V1_3::IHostapdCallback>& callback,
	    registerCallback_1_3_cb _hidl_cb) override;
	Return<void>forceClientDisconnect(
	    const hidl_string& iface_name,
	    const hidl_array<uint8_t, 6>& client_address,
	    V1_2::Ieee80211ReasonCode reason_code, forceClientDisconnect_cb _hidl_cb) override;
	Return<void> setDebugParams(
	    V1_2::DebugLevel level, setDebugParams_cb _hidl_cb) override;
private:
	// Corresponding worker functions for the HIDL methods.
	V1_0::HostapdStatus addAccessPointInternal(
	    const V1_0::IHostapd::IfaceParams& iface_params,
	    const V1_0::IHostapd::NetworkParams& nw_params);
	V1_0::HostapdStatus addAccessPointInternal_1_1(
	    const V1_1::IHostapd::IfaceParams& IfaceParams,
	    const V1_0::IHostapd::NetworkParams& nw_params);
	V1_2::HostapdStatus addAccessPointInternal_1_2(
	    const V1_2::IHostapd::IfaceParams& IfaceParams,
	    const V1_2::IHostapd::NetworkParams& nw_params);
	V1_2::HostapdStatus addAccessPointInternal_1_3(
	    const V1_3::IHostapd::IfaceParams& IfaceParams,
	    const V1_3::IHostapd::NetworkParams& nw_params);
	V1_2::HostapdStatus addSingleAccessPoint(
	    const V1_3::IHostapd::IfaceParams& IfaceParams,
	    const V1_3::IHostapd::ChannelParams& channelParams,
	    const V1_3::IHostapd::NetworkParams& nw_params,
	    std::string br_name);
	V1_2::HostapdStatus addConcurrentAccessPoints(
	    const V1_3::IHostapd::IfaceParams& IfaceParams,
	    const V1_3::IHostapd::NetworkParams& nw_params);
	V1_0::HostapdStatus removeAccessPointInternal(const std::string& iface_name);
	V1_0::HostapdStatus registerCallbackInternal(
	    const sp<V1_1::IHostapdCallback>& callback);
	V1_2::HostapdStatus registerCallbackInternal_1_3(
	    const sp<V1_3::IHostapdCallback>& callback);
	V1_2::HostapdStatus forceClientDisconnectInternal(
	    const std::string& iface_name,
	    const std::array<uint8_t, 6>& client_address,
	    V1_2::Ieee80211ReasonCode reason_code);
	V1_2::HostapdStatus setDebugParamsInternal(V1_2::DebugLevel level);
	// Raw pointer to the global structure maintained by the core.
	struct hapd_interfaces* interfaces_;
	// Callbacks registered.
	std::vector<sp<V1_3::IHostapdCallback>> callbacks_;
	// Death notifier.
	android::sp<DeathNotifier> death_notifier_;
	// Bridge and its managed interfaces.
	std::map<std::string, std::vector<std::string>> br_interfaces_;
	DISALLOW_COPY_AND_ASSIGN(Hostapd);
};
}  // namespace implementation
}  // namespace V1_3
}  // namespace hostapd
}  // namespace wifi
}  // namespace hardware
}  // namespace android

#endif  // HOSTAPD_HIDL_SUPPLICANT_H
