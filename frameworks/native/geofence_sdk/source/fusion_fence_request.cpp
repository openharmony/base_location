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

#include "fusion_fence_request.h"
#include <parcel.h>
#include "location_log.h"

namespace OHOS {
namespace Location {
bool FusionFencePoint::Marshalling(Parcel& parcel) const
{
    return parcel.WriteDouble(latitude) && parcel.WriteDouble(longitude);
}

bool FusionFencePoint::ReadFromParcel(Parcel& parcel)
{
    latitude = parcel.ReadDouble();
    longitude = parcel.ReadDouble();
    return true;
}

bool FusionFenceCellInfo::Marshalling(Parcel& parcel) const
{
    return parcel.WriteInt64(timeSinceBoot) &&
        parcel.WriteInt64(cellId) &&
        parcel.WriteInt32(lac) &&
        parcel.WriteInt32(mcc) &&
        parcel.WriteInt32(mnc) &&
        parcel.WriteInt32(rat) &&
        parcel.WriteInt32(signalIntensity) &&
        parcel.WriteInt32(arfcn) &&
        parcel.WriteInt32(pci) &&
        parcel.WriteInt32(tac) &&
        parcel.WriteInt32(static_cast<int32_t>(additionsMap.size())) &&
        [this, &parcel]() {
            for (const auto& [key, value] : additionsMap) {
                if (!parcel.WriteString(key) || !parcel.WriteString(value)) {
                    return false;
                }
            }
            return true;
        }();
}

bool FusionFenceCellInfo::ReadFromParcel(Parcel& parcel)
{
    timeSinceBoot = parcel.ReadInt64();
    cellId = parcel.ReadInt64();
    lac = parcel.ReadInt32();
    mcc = parcel.ReadInt32();
    mnc = parcel.ReadInt32();
    rat = parcel.ReadInt32();
    signalIntensity = parcel.ReadInt32();
    arfcn = parcel.ReadInt32();
    pci = parcel.ReadInt32();
    tac = parcel.ReadInt32();
    int32_t mapSize = parcel.ReadInt32();
    constexpr int32_t MAX_ADDITIONS_MAP_SIZE = 100;
    if (mapSize > MAX_ADDITIONS_MAP_SIZE) {
        mapSize = MAX_ADDITIONS_MAP_SIZE;
    }
    for (int32_t i = 0; i < mapSize; i++) {
        std::string key = parcel.ReadString();
        std::string value = parcel.ReadString();
        additionsMap[key] = value;
    }
    return true;
}

bool FusionFenceCell::Marshalling(Parcel& parcel) const
{
    if (!parcel.WriteInt32(static_cast<int32_t>(cellInfos.size()))) {
        return false;
    }
    for (const auto& cellInfo : cellInfos) {
        if (!cellInfo.Marshalling(parcel)) {
            return false;
        }
    }
    return true;
}

bool FusionFenceCell::ReadFromParcel(Parcel& parcel)
{
    int32_t size = parcel.ReadInt32();
    constexpr int32_t MAX_CELL_FENCES_COUNT = 1000;
    if (size > MAX_CELL_FENCES_COUNT) {
        LBSLOGE(FUSION_FENCE, "cellInfos size %{public}d exceeds max %{public}d", size, MAX_CELL_FENCES_COUNT);
        return false;
    }
    for (int32_t i = 0; i < size; i++) {
        FusionFenceCellInfo cellInfo;
        if (!cellInfo.ReadFromParcel(parcel)) {
            return false;
        }
        cellInfos.push_back(cellInfo);
    }
    return true;
}

bool WirelessSignalFeature::Marshalling(Parcel& parcel) const
{
    if (!parcel.WriteInt32(rssiAvg)) {
        return false;
    }
    if (!parcel.WriteDouble(rssiStandardDeviation)) {
        return false;
    }
    if (!parcel.WriteInt32(static_cast<int32_t>(mac.size()))) {
        return false;
    }
    for (const auto& macAddr : mac) {
        if (!parcel.WriteString(macAddr)) {
            return false;
        }
    }
    return true;
}

bool WirelessSignalFeature::ReadFromParcel(Parcel& parcel)
{
    rssiAvg = parcel.ReadInt32();
    rssiStandardDeviation = parcel.ReadDouble();
    int32_t macSize = parcel.ReadInt32();
    constexpr int32_t MAX_MAC_SIZE = 150;
    if (macSize > MAX_MAC_SIZE) {
        LBSLOGE(FUSION_FENCE, "mac size %{public}d exceeds max %{public}d", macSize, MAX_MAC_SIZE);
        return false;
    }
    for (int32_t i = 0; i < macSize; i++) {
        mac.push_back(parcel.ReadString());
    }
    return true;
}

bool FusionFenceWifi::Marshalling(Parcel& parcel) const
{
    if (!parcel.WriteInt32(static_cast<int32_t>(type))) {
        return false;
    }
    if (!parcel.WriteInt32(static_cast<int32_t>(wifiFeatures.size()))) {
        return false;
    }
    for (const auto& feature : wifiFeatures) {
        if (!feature.Marshalling(parcel)) {
            return false;
        }
    }
    return true;
}

bool FusionFenceWifi::ReadFromParcel(Parcel& parcel)
{
    type = static_cast<WifiFingerprintType>(parcel.ReadInt32());
    int32_t size = parcel.ReadInt32();
    constexpr int32_t MAX_WIFI_FEATURES_COUNT = 150;
    if (size > MAX_WIFI_FEATURES_COUNT) {
        LBSLOGE(FUSION_FENCE, "wifiFeatures size %{public}d exceeds max %{public}d", size, MAX_WIFI_FEATURES_COUNT);
        return false;
    }
    for (int32_t i = 0; i < size; i++) {
        WirelessSignalFeature feature;
        if (!feature.ReadFromParcel(parcel)) {
            return false;
        }
        wifiFeatures.push_back(feature);
    }
    return true;
}

bool FusionFenceGnss::Marshalling(Parcel& parcel) const
{
    if (!parcel.WriteInt32(static_cast<int32_t>(gnssFenceType))) {
        return false;
    }
    if (circularFence != nullptr) {
        if (!parcel.WriteInt32(1) || !circularFence->Marshalling(parcel)) {
            return false;
        }
    } else {
        if (!parcel.WriteInt32(0)) {
            return false;
        }
    }
    if (!parcel.WriteInt32(static_cast<int32_t>(polygon.size()))) {
        return false;
    }
    for (const auto& point : polygon) {
        if (!point.Marshalling(parcel)) {
            return false;
        }
    }
    return true;
}

bool FusionFenceGnss::ReadFromParcel(Parcel& parcel)
{
    gnssFenceType = static_cast<GnssFenceType>(parcel.ReadInt32());
    int32_t hasCircular = parcel.ReadInt32();
    if (hasCircular == 1) {
        circularFence = Geofence::UnmarshallingMakeUnique(parcel);
        if (circularFence == nullptr) {
            LBSLOGE(FUSION_FENCE, "FusionFenceGnss::ReadFromParcel: circularFence unmarshalling failed");
            return false;
        }
    }
    int32_t polygonSize = parcel.ReadInt32();
    constexpr int32_t MAX_POLYGON_POINTS_COUNT = 10;
    if (polygonSize > MAX_POLYGON_POINTS_COUNT) {
        LBSLOGE(FUSION_FENCE, "polygon size %{public}d exceeds max %{public}d", polygonSize, MAX_POLYGON_POINTS_COUNT);
        return false;
    }
    for (int32_t i = 0; i < polygonSize; i++) {
        FusionFencePoint point;
        if (!point.ReadFromParcel(parcel)) {
            return false;
        }
        polygon.push_back(point);
    }
    return true;
}

bool FusionFenceTransition::Marshalling(Parcel& parcel) const
{
    return parcel.WriteString(identifier) &&
        parcel.WriteInt32(static_cast<int32_t>(scene)) &&
        parcel.WriteInt32(static_cast<int32_t>(transitionEvent));
}

bool FusionFenceTransition::ReadFromParcel(Parcel& parcel)
{
    identifier = parcel.ReadString();
    scene = static_cast<FusionFenceScene>(parcel.ReadInt32());
    transitionEvent = static_cast<GeofenceTransitionEvent>(parcel.ReadInt32());
    return true;
}

FusionFenceTransition* FusionFenceTransition::Unmarshalling(Parcel& parcel)
{
    FusionFenceTransition* transition = new FusionFenceTransition();
    if (!transition->ReadFromParcel(parcel)) {
        delete transition;
        return nullptr;
    }
    return transition;
}

FusionFenceRequest::FusionFenceRequest()
{
    fenceType_ = 0;
    poiLocation_ = {0.0, 0.0};
    monitorTransitionEvents_ = 0;
    loiterTimeMs_ = 0;
    expirationMs_ = 0;
    transitionCallback_ = nullptr;
    uid_ = 0;
    pid_ = 0;
    tokenId_ = 0;
    tokenIdEx_ = 0;
    firstTokenId_ = 0;
}

FusionFenceRequest::FusionFenceRequest(const FusionFenceRequest& request)
{
    identifier_ = request.identifier_;
    scene_ = request.scene_;
    fenceType_ = request.fenceType_;
    poiType_ = request.poiType_;
    poiLocation_ = request.poiLocation_;
    monitorTransitionEvents_ = request.monitorTransitionEvents_;
    loiterTimeMs_ = request.loiterTimeMs_;
    gnssFences_ = request.gnssFences_;
    cellFences_ = request.cellFences_;
    wifiFences_ = request.wifiFences_;
    expirationMs_ = request.expirationMs_;
    transitionCallback_ = request.transitionCallback_;
    bundleName_ = request.bundleName_;
    uid_ = request.uid_;
    pid_ = request.pid_;
    tokenId_ = request.tokenId_;
    tokenIdEx_ = request.tokenIdEx_;
    firstTokenId_ = request.firstTokenId_;
}

FusionFenceRequest::~FusionFenceRequest() {}

std::string FusionFenceRequest::GetIdentifier() const
{
    return identifier_;
}

void FusionFenceRequest::SetIdentifier(const std::string& identifier)
{
    identifier_ = identifier;
}

FusionFenceScene FusionFenceRequest::GetScene() const
{
    return scene_;
}

void FusionFenceRequest::SetScene(FusionFenceScene scene)
{
    scene_ = scene;
}

int32_t FusionFenceRequest::GetFenceType() const
{
    return fenceType_;
}

void FusionFenceRequest::SetFenceType(int32_t fenceType)
{
    fenceType_ = fenceType;
}

std::string FusionFenceRequest::GetPoiType() const
{
    return poiType_;
}

void FusionFenceRequest::SetPoiType(const std::string& poiType)
{
    poiType_ = poiType;
}

FusionFencePoint FusionFenceRequest::GetPoiLocation() const
{
    return poiLocation_;
}

void FusionFenceRequest::SetPoiLocation(const FusionFencePoint& poiLocation)
{
    poiLocation_ = poiLocation;
}

int32_t FusionFenceRequest::GetMonitorTransitionEvents() const
{
    return monitorTransitionEvents_;
}

void FusionFenceRequest::SetMonitorTransitionEvents(int32_t monitorTransitionEvents)
{
    monitorTransitionEvents_ = monitorTransitionEvents;
}

int64_t FusionFenceRequest::GetLoiterTimeMs() const
{
    return loiterTimeMs_;
}

void FusionFenceRequest::SetLoiterTimeMs(int64_t loiterTimeMs)
{
    loiterTimeMs_ = loiterTimeMs;
}

const std::vector<std::shared_ptr<FusionFenceGnss>>& FusionFenceRequest::GetGnssFences() const
{
    return gnssFences_;
}

void FusionFenceRequest::SetGnssFences(const std::vector<std::shared_ptr<FusionFenceGnss>>& gnssFences)
{
    gnssFences_ = gnssFences;
}

const std::vector<std::shared_ptr<FusionFenceCell>>& FusionFenceRequest::GetCellFences() const
{
    return cellFences_;
}

void FusionFenceRequest::SetCellFences(const std::vector<std::shared_ptr<FusionFenceCell>>& cellFences)
{
    cellFences_ = cellFences;
}

const std::vector<std::shared_ptr<FusionFenceWifi>>& FusionFenceRequest::GetWifiFences() const
{
    return wifiFences_;
}

void FusionFenceRequest::SetWifiFences(const std::vector<std::shared_ptr<FusionFenceWifi>>& wifiFences)
{
    wifiFences_ = wifiFences;
}

int64_t FusionFenceRequest::GetExpirationMs() const
{
    return expirationMs_;
}

void FusionFenceRequest::SetExpirationMs(int64_t expirationMs)
{
    expirationMs_ = expirationMs;
}

sptr<IRemoteObject> FusionFenceRequest::GetTransitionCallback()
{
    return transitionCallback_;
}

void FusionFenceRequest::SetTransitionCallback(sptr<IRemoteObject> callback)
{
    transitionCallback_ = callback;
}

std::string FusionFenceRequest::GetBundleName() const
{
    return bundleName_;
}

void FusionFenceRequest::SetBundleName(const std::string& bundleName)
{
    bundleName_ = bundleName;
}

int32_t FusionFenceRequest::GetUid() const
{
    return uid_;
}

void FusionFenceRequest::SetUid(int32_t uid)
{
    uid_ = uid;
}

int32_t FusionFenceRequest::GetPid() const
{
    return pid_;
}

void FusionFenceRequest::SetPid(int32_t pid)
{
    pid_ = pid;
}

int64_t FusionFenceRequest::GetTokenId() const
{
    return tokenId_;
}

void FusionFenceRequest::SetTokenId(int64_t tokenId)
{
    tokenId_ = tokenId;
}

int64_t FusionFenceRequest::GetTokenIdEx() const
{
    return tokenIdEx_;
}

void FusionFenceRequest::SetTokenIdEx(int64_t tokenIdEx)
{
    tokenIdEx_ = tokenIdEx;
}

int64_t FusionFenceRequest::GetFirstTokenId() const
{
    return firstTokenId_;
}

void FusionFenceRequest::SetFirstTokenId(int64_t firstTokenId)
{
    firstTokenId_ = firstTokenId;
}

template<typename T>
static bool MarshallingFenceArray(Parcel& parcel, const std::vector<std::shared_ptr<T>>& fences)
{
    if (!parcel.WriteInt32(static_cast<int32_t>(fences.size()))) {
        return false;
    }
    for (const auto& fence : fences) {
        int32_t hasFence = (fence != nullptr) ? 1 : 0;
        if (!parcel.WriteInt32(hasFence)) {
            return false;
        }
        if (fence != nullptr && !fence->Marshalling(parcel)) {
            return false;
        }
    }
    return true;
}

template<typename T>
static bool ReadFromParcelFenceArray(Parcel& parcel, std::vector<std::shared_ptr<T>>& fences)
{
    int32_t fenceSize = parcel.ReadInt32();
    constexpr int32_t MAX_FENCE_ARRAY_SIZE = 30;
    if (fenceSize > MAX_FENCE_ARRAY_SIZE) {
        LBSLOGE(FUSION_FENCE, "fence array size %{public}d exceeds max %{public}d", fenceSize, MAX_FENCE_ARRAY_SIZE);
        return false;
    }
    for (int32_t i = 0; i < fenceSize; i++) {
        int32_t hasFence = parcel.ReadInt32();
        if (hasFence == 1) {
            auto fence = std::make_shared<T>();
            if (!fence->ReadFromParcel(parcel)) {
                LBSLOGE(FUSION_FENCE, "fence ReadFromParcel failed at index %{public}d", i);
                return false;
            }
            fences.push_back(fence);
        }
    }
    return true;
}

static bool MarshallingBasicFields(Parcel& parcel, const FusionFenceRequest& request)
{
    return parcel.WriteString(request.GetIdentifier()) &&
        parcel.WriteInt32(static_cast<int32_t>(request.GetScene())) &&
        parcel.WriteInt32(request.GetFenceType()) &&
        parcel.WriteString(request.GetPoiType()) &&
        request.GetPoiLocation().Marshalling(parcel) &&
        parcel.WriteInt32(request.GetMonitorTransitionEvents()) &&
        parcel.WriteInt64(request.GetLoiterTimeMs());
}

bool FusionFenceRequest::Marshalling(Parcel& parcel) const
{
    return MarshallingBasicFields(parcel, *this) &&
        MarshallingFenceArray(parcel, gnssFences_) &&
        MarshallingFenceArray(parcel, cellFences_) &&
        MarshallingFenceArray(parcel, wifiFences_) &&
        parcel.WriteInt64(expirationMs_) &&
        parcel.WriteRemoteObject(transitionCallback_);
}

static bool ReadFromParcelBasicFields(Parcel& parcel, FusionFenceRequest& request)
{
    request.SetIdentifier(parcel.ReadString());
    request.SetScene(static_cast<FusionFenceScene>(parcel.ReadInt32()));
    request.SetFenceType(parcel.ReadInt32());
    request.SetPoiType(parcel.ReadString());
    FusionFencePoint poiLocation;
    poiLocation.ReadFromParcel(parcel);
    request.SetPoiLocation(poiLocation);
    request.SetMonitorTransitionEvents(parcel.ReadInt32());
    request.SetLoiterTimeMs(parcel.ReadInt64());
    return true;
}

bool FusionFenceRequest::ReadFromParcel(Parcel& parcel)
{
    if (!ReadFromParcelBasicFields(parcel, *this)) {
        return false;
    }
    if (!ReadFromParcelFenceArray(parcel, gnssFences_)) {
        return false;
    }
    if (!ReadFromParcelFenceArray(parcel, cellFences_)) {
        return false;
    }
    if (!ReadFromParcelFenceArray(parcel, wifiFences_)) {
        return false;
    }
    expirationMs_ = parcel.ReadInt64();
    transitionCallback_ = parcel.ReadObject<IRemoteObject>();
    return true;
}

FusionFenceRequest* FusionFenceRequest::Unmarshalling(Parcel& parcel)
{
    FusionFenceRequest* request = new FusionFenceRequest();
    if (!request->ReadFromParcel(parcel)) {
        delete request;
        return nullptr;
    }
    return request;
}
} // namespace Location
} // namespace OHOS