/*
 * hidl interface for wpa_supplicant daemon
 * Copyright (c) 2004-2016, Jouni Malinen <j@w1.fi>
 * Copyright (c) 2004-2016, Roshan Pius <rpius@google.com>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef WPA_SUPPLICANT_HIDL_HIDL_MANAGER_H
#define WPA_SUPPLICANT_HIDL_HIDL_MANAGER_H

#include <map>
#include <string>

#include <android/hardware/wifi/supplicant/1.0/ISupplicantCallback.h>
#include <android/hardware/wifi/supplicant/1.0/ISupplicantP2pIfaceCallback.h>
#include <android/hardware/wifi/supplicant/1.0/ISupplicantP2pNetworkCallback.h>
#include <android/hardware/wifi/supplicant/1.4/ISupplicantStaIfaceCallback.h>
#include <android/hardware/wifi/supplicant/1.0/ISupplicantStaNetworkCallback.h>
#include <android/hardware/wifi/supplicant/1.4/ISupplicantStaNetworkCallback.h>

#include "p2p_iface.h"
#include "p2p_network.h"
#include "rsn_supp/pmksa_cache.h"
#include "sta_iface.h"
#include "sta_network.h"
#include "supplicant.h"

#ifdef SUPPLICANT_VENDOR_HIDL
#include "vendorsta_iface.h"
#include "vendorp2p_iface.h"
#include "vendorsta_network.h"
#include "supplicantvendor.h"
#endif

extern "C"
{
#include "utils/common.h"
#include "utils/includes.h"
#include "wpa_supplicant_i.h"
#include "driver_i.h"
}

class DeathNotifier : public android::hardware::hidl_death_recipient
{
public:
	DeathNotifier(struct wpa_global *wpa_global)
	    : wpa_global_(wpa_global)
	{}

	void serviceDied(
	    uint64_t /*cookie*/,
	    const android::wp<android::hidl::base::V1_0::IBase>
		& /* who */) override
	{
		wpa_printf(MSG_ERROR, "Client died. Terminating...");
		wpa_supplicant_terminate_proc(wpa_global_);
	}

private:
	struct wpa_global *wpa_global_;
};

namespace android {
namespace hardware {
namespace wifi {
namespace supplicant {
namespace V1_4 {
namespace implementation {
using V1_0::ISupplicant;
using V1_0::ISupplicantP2pIface;
using V1_0::ISupplicantStaIface;
using V1_0::ISupplicantStaIfaceCallback;
using V1_0::P2pGroupCapabilityMask;
using V1_0::WpsConfigMethods;
#ifdef SUPPLICANT_VENDOR_HIDL
using namespace vendor::qti::hardware::wifi::supplicantvendor::V2_2::Implementation;
using vendor::qti::hardware::wifi::supplicant::V2_2::ISupplicantVendorStaIface;
#endif

/**
 * HidlManager is responsible for managing the lifetime of all
 * hidl objects created by wpa_supplicant. This is a singleton
 * class which is created by the supplicant core and can be used
 * to get references to the hidl objects.
 */
class HidlManager
{
public:
	static HidlManager *getInstance();
	static void destroyInstance();

	// Methods called from wpa_supplicant core.
	int registerHidlService(struct wpa_global *global);
	int registerInterface(struct wpa_supplicant *wpa_s);
	int unregisterInterface(struct wpa_supplicant *wpa_s);
	int registerNetwork(
	    struct wpa_supplicant *wpa_s, struct wpa_ssid *ssid);
	int unregisterNetwork(
	    struct wpa_supplicant *wpa_s, struct wpa_ssid *ssid);
	int notifyStateChange(struct wpa_supplicant *wpa_s);
	int notifyNetworkRequest(
	    struct wpa_supplicant *wpa_s, struct wpa_ssid *ssid, int type,
	    const char *param);
	void notifyAnqpQueryDone(
	    struct wpa_supplicant *wpa_s, const u8 *bssid, const char *result,
	    const struct wpa_bss_anqp *anqp);
	void notifyHs20IconQueryDone(
	    struct wpa_supplicant *wpa_s, const u8 *bssid,
	    const char *file_name, const u8 *image, u32 image_length);
	void notifyHs20RxSubscriptionRemediation(
	    struct wpa_supplicant *wpa_s, const char *url, u8 osu_method);
	void notifyHs20RxDeauthImminentNotice(
	    struct wpa_supplicant *wpa_s, u8 code, u16 reauth_delay,
		const char *url);
	void notifyHs20RxTermsAndConditionsAcceptance(
			struct wpa_supplicant *wpa_s, const char *url);
	void notifyDisconnectReason(struct wpa_supplicant *wpa_s);
	void notifyAssocReject(struct wpa_supplicant *wpa_s,
	    const u8 *bssid, u8 timed_out);
	void notifyAuthTimeout(struct wpa_supplicant *wpa_s);
	void notifyBssidChanged(struct wpa_supplicant *wpa_s);
	void notifyWpsEventFail(
	    struct wpa_supplicant *wpa_s, uint8_t *peer_macaddr,
	    uint16_t config_error, uint16_t error_indication);
	void notifyWpsEventSuccess(struct wpa_supplicant *wpa_s);
	void notifyWpsEventPbcOverlap(struct wpa_supplicant *wpa_s);
	void notifyP2pDeviceFound(
	    struct wpa_supplicant *wpa_s, const u8 *addr,
	    const struct p2p_peer_info *info, const u8 *peer_wfd_device_info,
	    u8 peer_wfd_device_info_len, const u8 *peer_wfd_r2_device_info,
	    u8 peer_wfd_r2_device_info_len);
	void notifyP2pDeviceLost(
	    struct wpa_supplicant *wpa_s, const u8 *p2p_device_addr);
	void notifyP2pFindStopped(struct wpa_supplicant *wpa_s);
	void notifyP2pGoNegReq(
	    struct wpa_supplicant *wpa_s, const u8 *src_addr, u16 dev_passwd_id,
	    u8 go_intent);
	void notifyP2pGoNegCompleted(
	    struct wpa_supplicant *wpa_s, const struct p2p_go_neg_results *res);
	void notifyP2pGroupFormationFailure(
	    struct wpa_supplicant *wpa_s, const char *reason);
	void notifyP2pGroupStarted(
	    struct wpa_supplicant *wpa_group_s, const struct wpa_ssid *ssid,
	    int persistent, int client);
	void notifyP2pGroupRemoved(
	    struct wpa_supplicant *wpa_group_s, const struct wpa_ssid *ssid,
	    const char *role);
	void notifyP2pInvitationReceived(
	    struct wpa_supplicant *wpa_s, const u8 *sa, const u8 *go_dev_addr,
	    const u8 *bssid, int id, int op_freq);
	void notifyP2pInvitationResult(
	    struct wpa_supplicant *wpa_s, int status, const u8 *bssid);
	void notifyP2pProvisionDiscovery(
	    struct wpa_supplicant *wpa_s, const u8 *dev_addr, int request,
	    enum p2p_prov_disc_status status, u16 config_methods,
	    unsigned int generated_pin);
	void notifyP2pSdResponse(
	    struct wpa_supplicant *wpa_s, const u8 *sa, u16 update_indic,
	    const u8 *tlvs, size_t tlvs_len);
	void notifyApStaAuthorized(
	    struct wpa_supplicant *wpa_s, const u8 *sta,
	    const u8 *p2p_dev_addr);
	void notifyApStaDeauthorized(
	    struct wpa_supplicant *wpa_s, const u8 *sta,
	    const u8 *p2p_dev_addr);
	void notifyEapError(struct wpa_supplicant *wpa_s, int error_code);
	void notifyDppConfigReceived(struct wpa_supplicant *wpa_s,
			struct wpa_ssid *config);
	void notifyDppConfigSent(struct wpa_supplicant *wpa_s);
	void notifyDppSuccess(struct wpa_supplicant *wpa_s, V1_3::DppSuccessCode code);
	void notifyDppFailure(struct wpa_supplicant *wpa_s,
			android::hardware::wifi::supplicant::V1_3::DppFailureCode code);
	void notifyDppFailure(struct wpa_supplicant *wpa_s,
			android::hardware::wifi::supplicant::V1_3::DppFailureCode code,
			const char *ssid, const char *channel_list, unsigned short band_list[],
			int size);
	void notifyDppProgress(struct wpa_supplicant *wpa_s,
			android::hardware::wifi::supplicant::V1_3::DppProgressCode code);
	void notifyPmkCacheAdded(struct wpa_supplicant *wpa_s,
			struct rsn_pmksa_cache_entry *pmksa_entry);
	void notifyBssTmStatus(struct wpa_supplicant *wpa_s);
	void notifyTransitionDisable(struct wpa_supplicant *wpa_s,
			struct wpa_ssid *ssid,
			u8 bitmap);
	void notifyNetworkNotFound(struct wpa_supplicant *wpa_s);

	// Methods called from hidl objects.
	void notifyExtRadioWorkStart(struct wpa_supplicant *wpa_s, uint32_t id);
	void notifyExtRadioWorkTimeout(
	    struct wpa_supplicant *wpa_s, uint32_t id);

#ifdef SUPPLICANT_VENDOR_HIDL
	// DPP Callbacks
	void notifyDppAuthSuccess(
	    struct wpa_supplicant *wpa_s, int initiator);
	void notifyDppConf(
	    struct wpa_supplicant *wpa_s, u8 type, u8* ssid, u8 ssid_len,
	    const char *connector, struct wpabuf *c_sign, struct wpabuf *net_access,
	    uint32_t net_access_expiry, const char *passphrase, uint32_t psk_set,
	    u8 *psk);
#endif

	int getP2pIfaceHidlObjectByIfname(
	    const std::string &ifname,
	    android::sp<ISupplicantP2pIface> *iface_object);
	int getStaIfaceHidlObjectByIfname(
	    const std::string &ifname,
	    android::sp<V1_1::ISupplicantStaIface> *iface_object);
	int getP2pNetworkHidlObjectByIfnameAndNetworkId(
	    const std::string &ifname, int network_id,
	    android::sp<ISupplicantP2pNetwork> *network_object);
	int getStaNetworkHidlObjectByIfnameAndNetworkId(
	    const std::string &ifname, int network_id,
	    android::sp<V1_3::ISupplicantStaNetwork> *network_object);
	int addSupplicantCallbackHidlObject(
	    const android::sp<ISupplicantCallback> &callback);
	int addP2pIfaceCallbackHidlObject(
	    const std::string &ifname,
	    const android::sp<ISupplicantP2pIfaceCallback> &callback);
	int addStaIfaceCallbackHidlObject(
	    const std::string &ifname,
	    const android::sp<ISupplicantStaIfaceCallback> &callback);
	int addP2pNetworkCallbackHidlObject(
	    const std::string &ifname, int network_id,
	    const android::sp<ISupplicantP2pNetworkCallback> &callback);
	int addStaNetworkCallbackHidlObject(
	    const std::string &ifname, int network_id,
	    const android::sp<ISupplicantStaNetworkCallback> &callback);
#ifdef SUPPLICANT_VENDOR_HIDL
	// method for qti.hardware.wifi.supplicant@2.0
	int registerVendorHidlService(struct wpa_global *global);
	int getVendorStaIfaceHidlObjectByIfname(
	    const std::string &ifname,
	    android::sp<ISupplicantVendorStaIface> *iface_object);
	int getVendorStaNetworkHidlObjectByIfnameAndNetworkId(
	    const std::string &ifname, int network_id,
	    android::sp<ISupplicantVendorStaNetwork> *network_object);
	int addVendorStaIfaceCallbackHidlObject(
	    const std::string &ifname,
	    const android::sp<ISupplicantVendorStaIfaceCallback> &callback);
	int addVendorP2pIfaceCallbackHidlObject(
	    const std::string &ifname,
	    const android::sp<ISupplicantVendorP2PIfaceCallback> &callback);
	int getVendorP2pIfaceHidlObjectByIfname(
	    const std::string &ifname,
	    android::sp<ISupplicantVendorP2PIface> *iface_object);
#endif

private:
	HidlManager() = default;
	~HidlManager() = default;
	HidlManager(const HidlManager &) = default;
	HidlManager &operator=(const HidlManager &) = default;

	struct wpa_supplicant *getTargetP2pIfaceForGroup(
	    struct wpa_supplicant *wpa_s);
	void removeSupplicantCallbackHidlObject(
	    const android::sp<ISupplicantCallback> &callback);
	void removeP2pIfaceCallbackHidlObject(
	    const std::string &ifname,
	    const android::sp<ISupplicantP2pIfaceCallback> &callback);
	void removeStaIfaceCallbackHidlObject(
	    const std::string &ifname,
	    const android::sp<ISupplicantStaIfaceCallback> &callback);
	void removeP2pNetworkCallbackHidlObject(
	    const std::string &ifname, int network_id,
	    const android::sp<ISupplicantP2pNetworkCallback> &callback);
	void removeStaNetworkCallbackHidlObject(
	    const std::string &ifname, int network_id,
	    const android::sp<ISupplicantStaNetworkCallback> &callback);

	void callWithEachSupplicantCallback(
	    const std::function<android::hardware::Return<void>(
		android::sp<ISupplicantCallback>)> &method);
	void callWithEachP2pIfaceCallback(
	    const std::string &ifname,
	    const std::function<android::hardware::Return<void>(
		android::sp<ISupplicantP2pIfaceCallback>)> &method);
	template <class CallbackTypeDerived>
	void callWithEachP2pIfaceCallbackDerived(
	    const std::string &ifname,
	    const std::function<
		Return<void>(android::sp<CallbackTypeDerived>)> &method);
	void callWithEachStaIfaceCallback(
	    const std::string &ifname,
	    const std::function<android::hardware::Return<void>(
		android::sp<ISupplicantStaIfaceCallback>)> &method);
	void callWithEachStaIfaceCallback_1_1(
	    const std::string &ifname,
	    const std::function<android::hardware::Return<void>(
		android::sp<V1_1::ISupplicantStaIfaceCallback>)> &method);
	void callWithEachStaIfaceCallback_1_2(
	    const std::string &ifname,
	    const std::function<android::hardware::Return<void>(
	    android::sp<V1_2::ISupplicantStaIfaceCallback>)> &method);
	void callWithEachStaIfaceCallback_1_3(
	    const std::string &ifname,
	    const std::function<android::hardware::Return<void>(
	    android::sp<V1_3::ISupplicantStaIfaceCallback>)> &method);
	void callWithEachStaIfaceCallback_1_4(
	    const std::string &ifname,
	    const std::function<android::hardware::Return<void>(
	    android::sp<V1_4::ISupplicantStaIfaceCallback>)> &method);
	template <class CallbackTypeDerived>
	void callWithEachStaIfaceCallbackDerived(
	    const std::string &ifname,
	    const std::function<
		Return<void>(android::sp<CallbackTypeDerived>)> &method);
	void callWithEachP2pNetworkCallback(
	    const std::string &ifname, int network_id,
	    const std::function<android::hardware::Return<void>(
		android::sp<ISupplicantP2pNetworkCallback>)> &method);
	void callWithEachStaNetworkCallback(
	    const std::string &ifname, int network_id,
	    const std::function<android::hardware::Return<void>(
		android::sp<ISupplicantStaNetworkCallback>)> &method);
	template <class CallbackTypeDerived>
	void callWithEachStaNetworkCallbackDerived(
	    const std::string &ifname, int network_id,
	    const std::function<
		Return<void>(android::sp<CallbackTypeDerived>)> &method);
#ifdef SUPPLICANT_VENDOR_HIDL
	void removeVendorStaIfaceCallbackHidlObject(
	    const std::string &ifname,
	    const android::sp<ISupplicantVendorStaIfaceCallback> &callback);
	bool checkForVendorStaIfaceCallback(const std::string &ifname);
	void callWithEachVendorStaIfaceCallback(
	    const std::string &ifname,
	    const std::function<android::hardware::Return<void>(
		android::sp<ISupplicantVendorStaIfaceCallback>)> &method);
	void removeVendorP2pIfaceCallbackHidlObject(
	    const std::string &ifname,
	    const android::sp<ISupplicantVendorP2PIfaceCallback> &callback);
	bool checkForVendorP2pIfaceCallback(const std::string &ifname);
	void callWithEachVendorP2pIfaceCallback(
	    const std::string &ifname,
	    const std::function<android::hardware::Return<void>(
		android::sp<ISupplicantVendorP2PIfaceCallback>)> &method);
#endif

	// Singleton instance of this class.
	static HidlManager *instance_;
	// Raw pointer to the global structure maintained by the core.
	struct wpa_global *wpa_global_;
	// Death notifier.
	android::sp<DeathNotifier> death_notifier_;
	// The main hidl service object.
	android::sp<Supplicant> supplicant_object_;
	// Map of all the P2P interface specific hidl objects controlled by
	// wpa_supplicant. This map is keyed in by the corresponding
	// |ifname|.
	std::map<const std::string, android::sp<P2pIface>>
	    p2p_iface_object_map_;
	// Map of all the STA interface specific hidl objects controlled by
	// wpa_supplicant. This map is keyed in by the corresponding
	// |ifname|.
	std::map<const std::string, android::sp<StaIface>>
	    sta_iface_object_map_;
	// Map of all the P2P network specific hidl objects controlled by
	// wpa_supplicant. This map is keyed in by the corresponding
	// |ifname| & |network_id|.
	std::map<const std::string, android::sp<P2pNetwork>>
	    p2p_network_object_map_;
	// Map of all the STA network specific hidl objects controlled by
	// wpa_supplicant. This map is keyed in by the corresponding
	// |ifname| & |network_id|.
	std::map<const std::string, android::sp<StaNetwork>>
	    sta_network_object_map_;

	// Callback registered for the main hidl service object.
	std::vector<android::sp<ISupplicantCallback>> supplicant_callbacks_;
	// Map of all the callbacks registered for P2P interface specific
	// hidl objects controlled by wpa_supplicant.  This map is keyed in by
	// the corresponding |ifname|.
	std::map<
	    const std::string,
	    std::vector<android::sp<ISupplicantP2pIfaceCallback>>>
	    p2p_iface_callbacks_map_;
	// Map of all the callbacks registered for STA interface specific
	// hidl objects controlled by wpa_supplicant.  This map is keyed in by
	// the corresponding |ifname|.
	std::map<
	    const std::string,
	    std::vector<android::sp<ISupplicantStaIfaceCallback>>>
	    sta_iface_callbacks_map_;
	// Map of all the callbacks registered for P2P network specific
	// hidl objects controlled by wpa_supplicant.  This map is keyed in by
	// the corresponding |ifname| & |network_id|.
	std::map<
	    const std::string,
	    std::vector<android::sp<ISupplicantP2pNetworkCallback>>>
	    p2p_network_callbacks_map_;
	// Map of all the callbacks registered for STA network specific
	// hidl objects controlled by wpa_supplicant.  This map is keyed in by
	// the corresponding |ifname| & |network_id|.
	std::map<
	    const std::string,
	    std::vector<android::sp<ISupplicantStaNetworkCallback>>>
	    sta_network_callbacks_map_;

#ifdef SUPPLICANT_VENDOR_HIDL
	// The main vendor hidl service object.
	android::sp<SupplicantVendor> supplicantvendor_object_;
	// Map of all the Vendor STA interface specific hidl objects controlled by
	// wpa_supplicant. This map is keyed in by the corresponding
	// |ifname|.
	std::map<const std::string, android::sp<VendorStaIface>>
	    vendor_sta_iface_object_map_;
	// Map of all the Vendor STA network specific hidl objects controlled by
	// wpa_supplicant. This map is keyed in by the corresponding
	// |ifname| & |network_id|.
	std::map<const std::string, android::sp<VendorStaNetwork>>
	    vendor_sta_network_object_map_;
	// Map of all the vendor callbacks registered for STA interface specific
	// hidl objects controlled by wpa_supplicant.  This map is keyed in by
	// the corresponding |ifname|.
	std::map<
	    const std::string,
	    std::vector<android::sp<ISupplicantVendorStaIfaceCallback>>>
	    vendor_sta_iface_callbacks_map_;
	// Map of all the Vendor P2P interface specific hidl objects controlled by
	// wpa_supplicant. This map is keyed in by the corresponding
	// |ifname|.
	std::map<const std::string, android::sp<VendorP2pIface>>
	    vendor_p2p_iface_object_map_;
	// Map of all the vendor callbacks registered for P2P interface specific
	// hidl objects controlled by wpa_supplicant.  This map is keyed in by
	// the corresponding |ifname|.
	std::map<
	    const std::string,
	    std::vector<android::sp<ISupplicantVendorP2PIfaceCallback>>>
	    vendor_p2p_iface_callbacks_map_;
#endif
};

// The hidl interface uses some values which are the same as internal ones to
// avoid nasty runtime conversion functions. So, adding compile time asserts
// to guard against any internal changes breaking the hidl interface.
static_assert(
    static_cast<uint32_t>(ISupplicant::DebugLevel::EXCESSIVE) == MSG_EXCESSIVE,
    "Debug level value mismatch");
static_assert(
    static_cast<uint32_t>(ISupplicant::DebugLevel::ERROR) == MSG_ERROR,
    "Debug level value mismatch");

static_assert(
    static_cast<uint32_t>(V1_0::ISupplicantStaNetwork::KeyMgmtMask::NONE) ==
	WPA_KEY_MGMT_NONE,
    "KeyMgmt value mismatch");
static_assert(
    static_cast<uint32_t>(V1_0::ISupplicantStaNetwork::KeyMgmtMask::WPA_PSK) ==
	WPA_KEY_MGMT_PSK,
    "KeyMgmt value mismatch");
static_assert(
    static_cast<uint32_t>(V1_0::ISupplicantStaNetwork::KeyMgmtMask::WPA_EAP) ==
	WPA_KEY_MGMT_IEEE8021X,
    "KeyMgmt value mismatch");
static_assert(
    static_cast<uint32_t>(V1_0::ISupplicantStaNetwork::KeyMgmtMask::IEEE8021X) ==
	WPA_KEY_MGMT_IEEE8021X_NO_WPA,
    "KeyMgmt value mismatch");
static_assert(
    static_cast<uint32_t>(V1_0::ISupplicantStaNetwork::KeyMgmtMask::FT_EAP) ==
	WPA_KEY_MGMT_FT_IEEE8021X,
    "KeyMgmt value mismatch");
static_assert(
    static_cast<uint32_t>(V1_0::ISupplicantStaNetwork::KeyMgmtMask::FT_PSK) ==
	WPA_KEY_MGMT_FT_PSK,
    "KeyMgmt value mismatch");
static_assert(
    static_cast<uint32_t>(V1_0::ISupplicantStaNetwork::KeyMgmtMask::OSEN) ==
	WPA_KEY_MGMT_OSEN,
    "KeyMgmt value mismatch");
static_assert(
    static_cast<uint32_t>(V1_2::ISupplicantStaNetwork::KeyMgmtMask::SAE) ==
	WPA_KEY_MGMT_SAE,
    "KeyMgmt value mismatch");
static_assert(
    static_cast<uint32_t>(V1_2::ISupplicantStaNetwork::KeyMgmtMask::SUITE_B_192) ==
	WPA_KEY_MGMT_IEEE8021X_SUITE_B_192,
    "KeyMgmt value mismatch");
static_assert(
    static_cast<uint32_t>(V1_2::ISupplicantStaNetwork::KeyMgmtMask::OWE) ==
	WPA_KEY_MGMT_OWE,
    "KeyMgmt value mismatch");
static_assert(
    static_cast<uint32_t>(V1_2::ISupplicantStaNetwork::KeyMgmtMask::WPA_PSK_SHA256) ==
	WPA_KEY_MGMT_PSK_SHA256,
    "KeyMgmt value mismatch");
static_assert(
    static_cast<uint32_t>(V1_2::ISupplicantStaNetwork::KeyMgmtMask::WPA_EAP_SHA256) ==
	WPA_KEY_MGMT_IEEE8021X_SHA256,
    "KeyMgmt value mismatch");
static_assert(
    static_cast<uint32_t>(V1_3::ISupplicantStaNetwork::KeyMgmtMask::WAPI_PSK) ==
	WPA_KEY_MGMT_WAPI_PSK,
    "KeyMgmt value mismatch");
static_assert(
    static_cast<uint32_t>(V1_3::ISupplicantStaNetwork::KeyMgmtMask::WAPI_CERT) ==
	WPA_KEY_MGMT_WAPI_CERT,
    "KeyMgmt value mismatch");
static_assert(
    static_cast<uint32_t>(V1_0::ISupplicantStaNetwork::ProtoMask::WPA) ==
	WPA_PROTO_WPA,
    "Proto value mismatch");
static_assert(
    static_cast<uint32_t>(V1_0::ISupplicantStaNetwork::ProtoMask::RSN) ==
	WPA_PROTO_RSN,
    "Proto value mismatch");
static_assert(
    static_cast<uint32_t>(V1_0::ISupplicantStaNetwork::ProtoMask::OSEN) ==
	WPA_PROTO_OSEN,
    "Proto value mismatch");
static_assert(
    static_cast<uint32_t>(V1_3::ISupplicantStaNetwork::ProtoMask::WAPI) ==
	WPA_PROTO_WAPI,
    "Proto value mismatch");
static_assert(
    static_cast<uint32_t>(V1_0::ISupplicantStaNetwork::AuthAlgMask::OPEN) ==
	WPA_AUTH_ALG_OPEN,
    "AuthAlg value mismatch");
static_assert(
    static_cast<uint32_t>(V1_0::ISupplicantStaNetwork::AuthAlgMask::SHARED) ==
	WPA_AUTH_ALG_SHARED,
    "AuthAlg value mismatch");
static_assert(
    static_cast<uint32_t>(V1_0::ISupplicantStaNetwork::AuthAlgMask::LEAP) ==
	WPA_AUTH_ALG_LEAP,
    "AuthAlg value mismatch");
static_assert(
    static_cast<uint32_t>(V1_0::ISupplicantStaNetwork::GroupCipherMask::WEP40) ==
	WPA_CIPHER_WEP40,
    "GroupCipher value mismatch");
static_assert(
    static_cast<uint32_t>(V1_0::ISupplicantStaNetwork::GroupCipherMask::WEP104) ==
	WPA_CIPHER_WEP104,
    "GroupCipher value mismatch");
static_assert(
    static_cast<uint32_t>(V1_0::ISupplicantStaNetwork::GroupCipherMask::TKIP) ==
	WPA_CIPHER_TKIP,
    "GroupCipher value mismatch");
static_assert(
    static_cast<uint32_t>(V1_0::ISupplicantStaNetwork::GroupCipherMask::CCMP) ==
	WPA_CIPHER_CCMP,
    "GroupCipher value mismatch");
static_assert(
    static_cast<uint32_t>(V1_2::ISupplicantStaNetwork::GroupCipherMask::GCMP_256) ==
	WPA_CIPHER_GCMP_256,
    "GroupCipher value mismatch");
static_assert(
    static_cast<uint32_t>(V1_3::ISupplicantStaNetwork::GroupCipherMask::SMS4) ==
	WPA_CIPHER_SMS4,
    "GroupCipher value mismatch");
static_assert(
    static_cast<uint32_t>(
	V1_0::ISupplicantStaNetwork::GroupCipherMask::GTK_NOT_USED) ==
	WPA_CIPHER_GTK_NOT_USED,
    "GroupCipher value mismatch");
static_assert(
    static_cast<uint32_t>(V1_0::ISupplicantStaNetwork::PairwiseCipherMask::NONE) ==
	WPA_CIPHER_NONE,
    "PairwiseCipher value mismatch");
static_assert(
    static_cast<uint32_t>(V1_0::ISupplicantStaNetwork::PairwiseCipherMask::TKIP) ==
	WPA_CIPHER_TKIP,
    "PairwiseCipher value mismatch");
static_assert(
    static_cast<uint32_t>(V1_0::ISupplicantStaNetwork::PairwiseCipherMask::CCMP) ==
	WPA_CIPHER_CCMP,
    "PairwiseCipher value mismatch");
static_assert(
    static_cast<uint32_t>(
	V1_2::ISupplicantStaNetwork::PairwiseCipherMask::GCMP_256) ==
	WPA_CIPHER_GCMP_256,
    "PairwiseCipher value mismatch");
static_assert(
    static_cast<uint32_t>(
	V1_3::ISupplicantStaNetwork::PairwiseCipherMask::SMS4) ==
	WPA_CIPHER_SMS4,
    "PairwiseCipher value mismatch");
static_assert(
    static_cast<uint32_t>(ISupplicantStaIfaceCallback::State::DISCONNECTED) ==
	WPA_DISCONNECTED,
    "State value mismatch");
static_assert(
    static_cast<uint32_t>(ISupplicantStaIfaceCallback::State::COMPLETED) ==
	WPA_COMPLETED,
    "State value mismatch");

static_assert(
    static_cast<uint32_t>(ISupplicantStaIface::AnqpInfoId::VENUE_NAME) ==
	ANQP_VENUE_NAME,
    "ANQP ID value mismatch");
static_assert(
    static_cast<uint32_t>(
	ISupplicantStaIface::AnqpInfoId::ROAMING_CONSORTIUM) ==
	ANQP_ROAMING_CONSORTIUM,
    "ANQP ID value mismatch");
static_assert(
    static_cast<uint32_t>(ISupplicantStaIface::AnqpInfoId::NAI_REALM) ==
	ANQP_NAI_REALM,
    "ANQP ID value mismatch");
static_assert(
    static_cast<uint32_t>(
	ISupplicantStaIface::AnqpInfoId::IP_ADDR_TYPE_AVAILABILITY) ==
	ANQP_IP_ADDR_TYPE_AVAILABILITY,
    "ANQP ID value mismatch");
static_assert(
    static_cast<uint32_t>(
	ISupplicantStaIface::AnqpInfoId::ANQP_3GPP_CELLULAR_NETWORK) ==
	ANQP_3GPP_CELLULAR_NETWORK,
    "ANQP ID value mismatch");
static_assert(
    static_cast<uint32_t>(ISupplicantStaIface::AnqpInfoId::DOMAIN_NAME) ==
	ANQP_DOMAIN_NAME,
    "ANQP ID value mismatch");
static_assert(
    static_cast<uint32_t>(
	ISupplicantStaIface::Hs20AnqpSubtypes::OPERATOR_FRIENDLY_NAME) ==
	HS20_STYPE_OPERATOR_FRIENDLY_NAME,
    "HS Subtype value mismatch");
static_assert(
    static_cast<uint32_t>(ISupplicantStaIface::Hs20AnqpSubtypes::WAN_METRICS) ==
	HS20_STYPE_WAN_METRICS,
    "HS Subtype value mismatch");
static_assert(
    static_cast<uint32_t>(
	ISupplicantStaIface::Hs20AnqpSubtypes::CONNECTION_CAPABILITY) ==
	HS20_STYPE_CONNECTION_CAPABILITY,
    "HS Subtype value mismatch");
static_assert(
    static_cast<uint32_t>(
	ISupplicantStaIface::Hs20AnqpSubtypes::OSU_PROVIDERS_LIST) ==
	HS20_STYPE_OSU_PROVIDERS_LIST,
    "HS Subtype value mismatch");

static_assert(
    static_cast<uint16_t>(
	ISupplicantStaIfaceCallback::WpsConfigError::NO_ERROR) ==
	WPS_CFG_NO_ERROR,
    "Wps config error value mismatch");
static_assert(
    static_cast<uint16_t>(ISupplicantStaIfaceCallback::WpsConfigError::
			      PUBLIC_KEY_HASH_MISMATCH) ==
	WPS_CFG_PUBLIC_KEY_HASH_MISMATCH,
    "Wps config error value mismatch");
static_assert(
    static_cast<uint16_t>(
	ISupplicantStaIfaceCallback::WpsErrorIndication::NO_ERROR) ==
	WPS_EI_NO_ERROR,
    "Wps error indication value mismatch");
static_assert(
    static_cast<uint16_t>(
	ISupplicantStaIfaceCallback::WpsErrorIndication::AUTH_FAILURE) ==
	WPS_EI_AUTH_FAILURE,
    "Wps error indication value mismatch");

static_assert(
    static_cast<uint32_t>(WpsConfigMethods::USBA) == WPS_CONFIG_USBA,
    "Wps config value mismatch");
static_assert(
    static_cast<uint32_t>(WpsConfigMethods::ETHERNET) == WPS_CONFIG_ETHERNET,
    "Wps config value mismatch");
static_assert(
    static_cast<uint32_t>(WpsConfigMethods::LABEL) == WPS_CONFIG_LABEL,
    "Wps config value mismatch");
static_assert(
    static_cast<uint32_t>(WpsConfigMethods::DISPLAY) == WPS_CONFIG_DISPLAY,
    "Wps config value mismatch");
static_assert(
    static_cast<uint32_t>(WpsConfigMethods::INT_NFC_TOKEN) ==
	WPS_CONFIG_INT_NFC_TOKEN,
    "Wps config value mismatch");
static_assert(
    static_cast<uint32_t>(WpsConfigMethods::EXT_NFC_TOKEN) ==
	WPS_CONFIG_EXT_NFC_TOKEN,
    "Wps config value mismatch");
static_assert(
    static_cast<uint32_t>(WpsConfigMethods::NFC_INTERFACE) ==
	WPS_CONFIG_NFC_INTERFACE,
    "Wps config value mismatch");
static_assert(
    static_cast<uint32_t>(WpsConfigMethods::PUSHBUTTON) ==
	WPS_CONFIG_PUSHBUTTON,
    "Wps config value mismatch");
static_assert(
    static_cast<uint32_t>(WpsConfigMethods::KEYPAD) == WPS_CONFIG_KEYPAD,
    "Wps config value mismatch");
static_assert(
    static_cast<uint32_t>(WpsConfigMethods::VIRT_PUSHBUTTON) ==
	WPS_CONFIG_VIRT_PUSHBUTTON,
    "Wps config value mismatch");
static_assert(
    static_cast<uint32_t>(WpsConfigMethods::PHY_PUSHBUTTON) ==
	WPS_CONFIG_PHY_PUSHBUTTON,
    "Wps config value mismatch");
static_assert(
    static_cast<uint32_t>(WpsConfigMethods::P2PS) == WPS_CONFIG_P2PS,
    "Wps config value mismatch");
static_assert(
    static_cast<uint32_t>(WpsConfigMethods::VIRT_DISPLAY) ==
	WPS_CONFIG_VIRT_DISPLAY,
    "Wps config value mismatch");
static_assert(
    static_cast<uint32_t>(WpsConfigMethods::PHY_DISPLAY) ==
	WPS_CONFIG_PHY_DISPLAY,
    "Wps config value mismatch");

static_assert(
    static_cast<uint32_t>(P2pGroupCapabilityMask::GROUP_OWNER) ==
	P2P_GROUP_CAPAB_GROUP_OWNER,
    "P2P capability value mismatch");
static_assert(
    static_cast<uint32_t>(P2pGroupCapabilityMask::PERSISTENT_GROUP) ==
	P2P_GROUP_CAPAB_PERSISTENT_GROUP,
    "P2P capability value mismatch");
static_assert(
    static_cast<uint32_t>(P2pGroupCapabilityMask::GROUP_LIMIT) ==
	P2P_GROUP_CAPAB_GROUP_LIMIT,
    "P2P capability value mismatch");
static_assert(
    static_cast<uint32_t>(P2pGroupCapabilityMask::INTRA_BSS_DIST) ==
	P2P_GROUP_CAPAB_INTRA_BSS_DIST,
    "P2P capability value mismatch");
static_assert(
    static_cast<uint32_t>(P2pGroupCapabilityMask::CROSS_CONN) ==
	P2P_GROUP_CAPAB_CROSS_CONN,
    "P2P capability value mismatch");
static_assert(
    static_cast<uint32_t>(P2pGroupCapabilityMask::PERSISTENT_RECONN) ==
	P2P_GROUP_CAPAB_PERSISTENT_RECONN,
    "P2P capability value mismatch");
static_assert(
    static_cast<uint32_t>(P2pGroupCapabilityMask::GROUP_FORMATION) ==
	P2P_GROUP_CAPAB_GROUP_FORMATION,
    "P2P capability value mismatch");

static_assert(
    static_cast<uint16_t>(
	ISupplicantP2pIfaceCallback::WpsDevPasswordId::DEFAULT) ==
	DEV_PW_DEFAULT,
    "Wps dev password id value mismatch");
static_assert(
    static_cast<uint16_t>(
	ISupplicantP2pIfaceCallback::WpsDevPasswordId::USER_SPECIFIED) ==
	DEV_PW_USER_SPECIFIED,
    "Wps dev password id value mismatch");
static_assert(
    static_cast<uint16_t>(
	ISupplicantP2pIfaceCallback::WpsDevPasswordId::MACHINE_SPECIFIED) ==
	DEV_PW_MACHINE_SPECIFIED,
    "Wps dev password id value mismatch");
static_assert(
    static_cast<uint16_t>(
	ISupplicantP2pIfaceCallback::WpsDevPasswordId::REKEY) == DEV_PW_REKEY,
    "Wps dev password id value mismatch");
static_assert(
    static_cast<uint16_t>(
	ISupplicantP2pIfaceCallback::WpsDevPasswordId::PUSHBUTTON) ==
	DEV_PW_PUSHBUTTON,
    "Wps dev password id value mismatch");
static_assert(
    static_cast<uint16_t>(
	ISupplicantP2pIfaceCallback::WpsDevPasswordId::REGISTRAR_SPECIFIED) ==
	DEV_PW_REGISTRAR_SPECIFIED,
    "Wps dev password id value mismatch");
static_assert(
    static_cast<uint16_t>(ISupplicantP2pIfaceCallback::WpsDevPasswordId::
			      NFC_CONNECTION_HANDOVER) ==
	DEV_PW_NFC_CONNECTION_HANDOVER,
    "Wps dev password id value mismatch");
static_assert(
    static_cast<uint16_t>(
	ISupplicantP2pIfaceCallback::WpsDevPasswordId::P2PS_DEFAULT) ==
	DEV_PW_P2PS_DEFAULT,
    "Wps dev password id value mismatch");

static_assert(
    static_cast<uint16_t>(
	ISupplicantP2pIfaceCallback::P2pStatusCode::SUCCESS) == P2P_SC_SUCCESS,
    "P2P status code value mismatch");
static_assert(
    static_cast<uint16_t>(ISupplicantP2pIfaceCallback::P2pStatusCode::
			      FAIL_INFO_CURRENTLY_UNAVAILABLE) ==
	P2P_SC_FAIL_INFO_CURRENTLY_UNAVAILABLE,
    "P2P status code value mismatch");
static_assert(
    static_cast<uint16_t>(
	ISupplicantP2pIfaceCallback::P2pStatusCode::FAIL_INCOMPATIBLE_PARAMS) ==
	P2P_SC_FAIL_INCOMPATIBLE_PARAMS,
    "P2P status code value mismatch");
static_assert(
    static_cast<uint16_t>(
	ISupplicantP2pIfaceCallback::P2pStatusCode::FAIL_LIMIT_REACHED) ==
	P2P_SC_FAIL_LIMIT_REACHED,
    "P2P status code value mismatch");
static_assert(
    static_cast<uint16_t>(
	ISupplicantP2pIfaceCallback::P2pStatusCode::FAIL_INVALID_PARAMS) ==
	P2P_SC_FAIL_INVALID_PARAMS,
    "P2P status code value mismatch");
static_assert(
    static_cast<uint16_t>(ISupplicantP2pIfaceCallback::P2pStatusCode::
			      FAIL_UNABLE_TO_ACCOMMODATE) ==
	P2P_SC_FAIL_UNABLE_TO_ACCOMMODATE,
    "P2P status code value mismatch");
static_assert(
    static_cast<uint16_t>(
	ISupplicantP2pIfaceCallback::P2pStatusCode::FAIL_PREV_PROTOCOL_ERROR) ==
	P2P_SC_FAIL_PREV_PROTOCOL_ERROR,
    "P2P status code value mismatch");
static_assert(
    static_cast<uint16_t>(
	ISupplicantP2pIfaceCallback::P2pStatusCode::FAIL_NO_COMMON_CHANNELS) ==
	P2P_SC_FAIL_NO_COMMON_CHANNELS,
    "P2P status code value mismatch");
static_assert(
    static_cast<uint16_t>(
	ISupplicantP2pIfaceCallback::P2pStatusCode::FAIL_UNKNOWN_GROUP) ==
	P2P_SC_FAIL_UNKNOWN_GROUP,
    "P2P status code value mismatch");
static_assert(
    static_cast<uint16_t>(
	ISupplicantP2pIfaceCallback::P2pStatusCode::FAIL_BOTH_GO_INTENT_15) ==
	P2P_SC_FAIL_BOTH_GO_INTENT_15,
    "P2P status code value mismatch");
static_assert(
    static_cast<uint16_t>(ISupplicantP2pIfaceCallback::P2pStatusCode::
			      FAIL_INCOMPATIBLE_PROV_METHOD) ==
	P2P_SC_FAIL_INCOMPATIBLE_PROV_METHOD,
    "P2P status code value mismatch");
static_assert(
    static_cast<uint16_t>(
	ISupplicantP2pIfaceCallback::P2pStatusCode::FAIL_REJECTED_BY_USER) ==
	P2P_SC_FAIL_REJECTED_BY_USER,
    "P2P status code value mismatch");
static_assert(
    static_cast<uint16_t>(
	ISupplicantP2pIfaceCallback::P2pStatusCode::SUCCESS_DEFERRED) ==
	P2P_SC_SUCCESS_DEFERRED,
    "P2P status code value mismatch");

static_assert(
    static_cast<uint16_t>(
	ISupplicantP2pIfaceCallback::P2pProvDiscStatusCode::SUCCESS) ==
	P2P_PROV_DISC_SUCCESS,
    "P2P status code value mismatch");
static_assert(
    static_cast<uint16_t>(
	ISupplicantP2pIfaceCallback::P2pProvDiscStatusCode::TIMEOUT) ==
	P2P_PROV_DISC_TIMEOUT,
    "P2P status code value mismatch");
static_assert(
    static_cast<uint16_t>(
	ISupplicantP2pIfaceCallback::P2pProvDiscStatusCode::REJECTED) ==
	P2P_PROV_DISC_REJECTED,
    "P2P status code value mismatch");
static_assert(
    static_cast<uint16_t>(
	ISupplicantP2pIfaceCallback::P2pProvDiscStatusCode::TIMEOUT_JOIN) ==
	P2P_PROV_DISC_TIMEOUT_JOIN,
    "P2P status code value mismatch");
static_assert(
    static_cast<uint16_t>(
	ISupplicantP2pIfaceCallback::P2pProvDiscStatusCode::INFO_UNAVAILABLE) ==
	P2P_PROV_DISC_INFO_UNAVAILABLE,
    "P2P status code value mismatch");
}  // namespace implementation
}  // namespace V1_4
}  // namespace supplicant
}  // namespace wifi
}  // namespace hardware
}  // namespace android
#endif  // WPA_SUPPLICANT_HIDL_HIDL_MANAGER_H
