/*
* Copyright (C) 2026 Huawei Device Co., Ltd.
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef FUSION_FENCE_REQUEST_H
#define FUSION_FENCE_REQUEST_H

#include <parcel.h>
#include <string>
#include <vector>
#include "geofence.h"
#include "i_gnss_geofence_callback.h"

namespace OHOS {
namespace Location {
enum FusionFenceScene {
    AIRPORT = 1,
    TRAIN_STATION = 2,
    SUBWAY = 3,
    SHOP = 4
};

enum FusionFenceType {
    FUSION_FENCE_GNSS = 1,
    FUSION_FENCE_CELLULAR = 2,
    FUSION_FENCE_WIFI = 4,
    FUSION_FENCE_BLUETOOTH = 8
};

enum GnssFenceType {
    GNSS_FENCE_POLYGON = 1,
    GNSS_FENCE_CIRCULAR = 2
};

enum WifiFingerprintType {
    WIFI_FINGERPRINT_SEMANTIC = 1,
    WIFI_FINGERPRINT_LOCATION = 2
};

struct FusionFencePoint {
    double latitude = 0.0;
    double longitude = 0.0;
 
    bool Marshalling(Parcel& parcel) const;
    bool ReadFromParcel(Parcel& parcel);
};

struct FusionFenceCellInfo {
    int64_t timeSinceBoot;
    int64_t cellId;
    int32_t lac;
    int32_t mcc;
    int32_t mnc;
    int32_t rat;
    int32_t signalIntensity;
    int32_t arfcn;
    int32_t pci;
    int32_t tac;
    std::map<std::string, std::string> additionsMap;
 
    bool Marshalling(Parcel& parcel) const;
    bool ReadFromParcel(Parcel& parcel);
};

struct FusionFenceCell {
    std::vector<FusionFenceCellInfo> cellInfos;
 
    bool Marshalling(Parcel& parcel) const;
    bool ReadFromParcel(Parcel& parcel);
};

struct WirelessSignalFeature {
    int32_t rssiAvg;
    double rssiStandardDeviation;
    std::vector<std::string> mac;
 
    bool Marshalling(Parcel& parcel) const;
    bool ReadFromParcel(Parcel& parcel);
};

struct FusionFenceWifi {
    WifiFingerprintType type;
    std::vector<WirelessSignalFeature> wifiFeatures;
 
    bool Marshalling(Parcel& parcel) const;
    bool ReadFromParcel(Parcel& parcel);
};

struct FusionFenceGnss {
    GnssFenceType gnssFenceType;
    std::shared_ptr<Geofence> circularFence;
    std::vector<FusionFencePoint> polygon;
 
    bool Marshalling(Parcel& parcel) const;
    bool ReadFromParcel(Parcel& parcel);
};

struct FusionFenceTransition {
    std::string identifier;
    FusionFenceScene scene;
    GeofenceTransitionEvent transitionEvent;
 
    bool Marshalling(Parcel& parcel) const;
    bool ReadFromParcel(Parcel& parcel);
    static FusionFenceTransition* Unmarshalling(Parcel& parcel);
};

class FusionFenceRequest : public Parcelable {
public:
    FusionFenceRequest();
    FusionFenceRequest(const FusionFenceRequest& request);
    FusionFenceRequest& operator=(const FusionFenceRequest& other) = delete;
    ~FusionFenceRequest();
 
    std::string GetIdentifier() const;
    void SetIdentifier(const std::string& identifier);
 
    FusionFenceScene GetScene() const;
    void SetScene(FusionFenceScene scene);
 
    int32_t GetFenceType() const;
    void SetFenceType(int32_t fenceType);

    std::string GetPoiType() const;
    void SetPoiType(const std::string& poiType);
 
    FusionFencePoint GetPoiLocation() const;
    void SetPoiLocation(const FusionFencePoint& poiLocation);
 
    int32_t GetMonitorTransitionEvents() const;
    void SetMonitorTransitionEvents(int32_t monitorTransitionEvents);
 
    int64_t GetLoiterTimeMs() const;
    void SetLoiterTimeMs(int64_t loiterTimeMs);
 
    const std::vector<std::shared_ptr<FusionFenceGnss>>& GetGnssFences() const;
    void SetGnssFences(const std::vector<std::shared_ptr<FusionFenceGnss>>& gnssFences);
 
    const std::vector<std::shared_ptr<FusionFenceCell>>& GetCellFences() const;
    void SetCellFences(const std::vector<std::shared_ptr<FusionFenceCell>>& cellFences);
 
    const std::vector<std::shared_ptr<FusionFenceWifi>>& GetWifiFences() const;
    void SetWifiFences(const std::vector<std::shared_ptr<FusionFenceWifi>>& wifiFences);
 
    int64_t GetExpirationMs() const;
    void SetExpirationMs(int64_t expirationMs);
 
    sptr<IRemoteObject> GetTransitionCallback();
    void SetTransitionCallback(sptr<IRemoteObject> callback);
 
    std::string GetBundleName() const;
    void SetBundleName(const std::string& bundleName);
 
    int32_t GetUid() const;
    void SetUid(int32_t uid);
 
    int32_t GetPid() const;
    void SetPid(int32_t pid);
 
    int64_t GetTokenId() const;
    void SetTokenId(int64_t tokenId);
 
    int64_t GetTokenIdEx() const;
    void SetTokenIdEx(int64_t tokenIdEx);
 
    int64_t GetFirstTokenId() const;
    void SetFirstTokenId(int64_t firstTokenId);
 
    bool Marshalling(Parcel& parcel) const;
    bool ReadFromParcel(Parcel& parcel);
    static FusionFenceRequest* Unmarshalling(Parcel& parcel);

private:
    std::string identifier_;
    FusionFenceScene scene_;
    int32_t fenceType_;
    std::string poiType_;
    FusionFencePoint poiLocation_;
    int32_t monitorTransitionEvents_;
    int64_t loiterTimeMs_;
    std::vector<std::shared_ptr<FusionFenceGnss>> gnssFences_;
    std::vector<std::shared_ptr<FusionFenceCell>> cellFences_;
    std::vector<std::shared_ptr<FusionFenceWifi>> wifiFences_;
    int64_t expirationMs_;
    sptr<IRemoteObject> transitionCallback_;
    std::string bundleName_;
    int32_t uid_;
    int32_t pid_;
    int64_t tokenId_;
    int64_t tokenIdEx_;
    int64_t firstTokenId_;
};
} // namespace Location
} // namespace OHOS
#endif // FUSION_FENCE_REQUEST_H