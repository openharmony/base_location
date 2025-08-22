/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifdef FEATURE_GNSS_SUPPORT
#ifdef HDF_DRIVERS_INTERFACE_AGNSS_ENABLE
#include "agnss_event_callback.h"

#include <singleton.h>

#ifdef TEL_CELLULAR_DATA_ENABLE
#include "cellular_data_client.h"
#endif
#ifdef TEL_CORE_SERVICE_ENABLE
#include "core_service_client.h"
#endif

#include "common_utils.h"
#include "gnss_ability.h"
#include "location_log.h"
#include "securec.h"

#ifdef WIFI_ENABLE
#include "wifi_scan.h"
#endif

namespace OHOS {
namespace Location {
int32_t AGnssEventCallback::RequestSetUpAgnssDataLink(const AGnssDataLinkRequest& request)
{
    LBSLOGI(GNSS, "AGnssEventCallback::RequestSetUpAgnssDataLink. agnsstype:%{public}d, setUpType:%{public}d",
        static_cast<int>(request.agnssType), static_cast<int>(request.setUpType));
    return ERR_OK;
}

int32_t AGnssEventCallback::RequestSubscriberSetId(SubscriberSetIdType type)
{
    LBSLOGI(GNSS, "AGnssEventCallback::RequestSubscriberSetId. type:%{public}d", static_cast<int>(type));
    std::string imsi;
#if defined(TEL_CORE_SERVICE_ENABLE) && defined(TEL_CELLULAR_DATA_ENABLE)
    int slotId = Telephony::CellularDataClient::GetInstance().GetDefaultCellularDataSlotId();
    std::u16string tempImsi;
    DelayedRefSingleton<Telephony::CoreServiceClient>::GetInstance().GetIMSI(slotId, tempImsi);
    imsi = CommonUtils::Str16ToStr8(tempImsi);
#endif
    SubscriberSetId setId;
    setId.type = HDI::Location::Agnss::V2_0::AGNSS_SETID_TYPE_IMSI;
    setId.id = imsi;
    auto gnssAbility = GnssAbility::GetInstance();
    gnssAbility->SetSetId(setId);
    return ERR_OK;
}

__attribute__((no_sanitize("cfi"))) void AGnssEventCallback::GetWiFiRefInfo(AGnssRefInfo& refInfo)
{
#ifdef WIFI_ENABLE
    std::vector<Wifi::WifiScanInfo> wifiScanInfo;
    std::shared_ptr<Wifi::WifiScan> ptrWifiScan = Wifi::WifiScan::GetInstance(WIFI_SCAN_ABILITY_ID);
    if (ptrWifiScan == nullptr) {
        LBSLOGE(GNSS, "%{public}s WifiScan get instance failed", __func__);
        return;
    }
    int ret = ptrWifiScan->GetScanInfoList(wifiScanInfo);
    if (ret != Wifi::WIFI_OPT_SUCCESS) {
        LBSLOGE(GNSS, "GetScanInfoList failed");
        return;
    }
    if (wifiScanInfo.size() == 0) {
        LBSLOGE(GNSS, "empty mac.");
        return;
    }
    uint8_t macArray[MAC_LEN];
    if (CommonUtils::GetMacArray(wifiScanInfo[0].bssid, macArray) != EOK) {
        LBSLOGE(GNSS, "GetMacArray failed.");
        return;
    }
    for (size_t i = 0; i < MAC_LEN; i++) {
        refInfo.mac.mac.push_back(macArray[i]);
    }
#endif
}

void AGnssEventCallback::GetCellRefInfo(AGnssRefInfo& refInfo)
{
#if defined(TEL_CORE_SERVICE_ENABLE) && defined(TEL_CELLULAR_DATA_ENABLE)
    int slotId = Telephony::CellularDataClient::GetInstance().GetDefaultCellularDataSlotId();
    std::vector<sptr<CellInformation>> cellInformations;
    DelayedRefSingleton<Telephony::CoreServiceClient>::GetInstance().GetCellInfoList(slotId, cellInformations);
    if (cellInformations.size() == 0) {
        LBSLOGE(GNSS, "empty cell info list.");
        return;
    }
    for (sptr<CellInformation> infoItem : cellInformations) {
        if (!infoItem->GetIsCamped()) {
            LBSLOGE(GNSS, "GetIsCamped return false");
            continue;
        }
        CellInformation::CellType cellType = infoItem->GetNetworkType();
        switch (cellType) {
            case CellInformation::CellType::CELL_TYPE_GSM: {
                JudgmentDataGsm(refInfo, infoItem);
                break;
            }
            case CellInformation::CellType::CELL_TYPE_LTE: {
                JudgmentDataLte(refInfo, infoItem);
                break;
            }
            case CellInformation::CellType::CELL_TYPE_CDMA:
            case CellInformation::CellType::CELL_TYPE_WCDMA:
            case CellInformation::CellType::CELL_TYPE_TDSCDMA: {
                JudgmentDataUmts(refInfo, infoItem);
                break;
            }
            case CellInformation::CellType::CELL_TYPE_NR: {
                JudgmentDataNr(refInfo, infoItem);
                break;
            }
            default:
                break;
        }
        break;
    }
#endif
}

int32_t AGnssEventCallback::RequestAgnssRefInfo(AGnssRefInfoType type)
{
    AGnssRefInfo refInfo;
    refInfo.type = type;
    GetWiFiRefInfo(refInfo);
    GetCellRefInfo(refInfo);
    auto gnssAbility = GnssAbility::GetInstance();
    if (gnssAbility == nullptr) {
        LBSLOGE(GNSS, "RequestAgnssRefInfo: gnss ability is nullptr");
        return ERR_OK;
    }
    gnssAbility->SetRefInfo(refInfo);
    return ERR_OK;
}

#ifdef TEL_CORE_SERVICE_ENABLE
void AGnssEventCallback::JudgmentDataGsm(AGnssRefInfo& refInfo, sptr<CellInformation> infoItem)
{
    auto gsmCellInfo = static_cast<Telephony::GsmCellInformation *>(infoItem.GetRefPtr());
    if (gsmCellInfo != nullptr) {
        refInfo.cellId.type = HDI::Location::Agnss::V2_0::CELLID_TYPE_GSM;
        if (CommonUtils::IsStrValidForStoi(gsmCellInfo->GetMcc())) {
            refInfo.cellId.mcc = static_cast<unsigned short>(std::stoi(gsmCellInfo->GetMcc()));
        }
        if (CommonUtils::IsStrValidForStoi(gsmCellInfo->GetMnc())) {
            refInfo.cellId.mnc = static_cast<unsigned short>(std::stoi(gsmCellInfo->GetMnc()));
        }
        refInfo.cellId.lac = static_cast<unsigned short>(gsmCellInfo->GetLac());
        refInfo.cellId.cid = static_cast<unsigned int>(gsmCellInfo->GetCellId());
    }
}

void AGnssEventCallback::JudgmentDataLte(AGnssRefInfo& refInfo, sptr<CellInformation> infoItem)
{
    auto lteCellInfo = static_cast<Telephony::LteCellInformation *>(infoItem.GetRefPtr());
    if (lteCellInfo != nullptr) {
        refInfo.cellId.type = HDI::Location::Agnss::V2_0::CELLID_TYPE_LTE;
        if (CommonUtils::IsStrValidForStoi(lteCellInfo->GetMcc())) {
            refInfo.cellId.mcc = static_cast<unsigned short>(std::stoi(lteCellInfo->GetMcc()));
        }
        if (CommonUtils::IsStrValidForStoi(lteCellInfo->GetMnc())) {
            refInfo.cellId.mnc = static_cast<unsigned short>(std::stoi(lteCellInfo->GetMnc()));
        }
        refInfo.cellId.tac = static_cast<unsigned short>(lteCellInfo->GetTac());
        refInfo.cellId.cid = static_cast<unsigned int>(lteCellInfo->GetCellId());
        refInfo.cellId.pcid = static_cast<unsigned short>(lteCellInfo->GetPci());
    }
}

void AGnssEventCallback::JudgmentDataNr(AGnssRefInfo& refInfo, sptr<CellInformation> infoItem)
{
    auto nrCellInfo = static_cast<Telephony::NrCellInformation *>(infoItem.GetRefPtr());
    if (nrCellInfo != nullptr) {
        refInfo.cellId.type = HDI::Location::Agnss::V2_0::CELLID_TYPE_NR;
        if (CommonUtils::IsStrValidForStoi(nrCellInfo->GetMcc())) {
            refInfo.cellId.mcc = static_cast<unsigned short>(std::stoi(nrCellInfo->GetMcc()));
        }
        if (CommonUtils::IsStrValidForStoi(nrCellInfo->GetMnc())) {
            refInfo.cellId.mnc = static_cast<unsigned short>(std::stoi(nrCellInfo->GetMnc()));
        }
        refInfo.cellId.tac = static_cast<unsigned short>(nrCellInfo->GetTac());
        refInfo.cellId.cid = static_cast<unsigned int>(nrCellInfo->GetCellId());
        refInfo.cellId.pcid = static_cast<unsigned short>(nrCellInfo->GetPci());
        refInfo.cellId.nci = static_cast<unsigned int>(nrCellInfo->GetNci());
    }
}

void AGnssEventCallback::JudgmentDataUmts(AGnssRefInfo& refInfo, sptr<CellInformation> infoItem)
{
    auto wcdmaCellInfo = static_cast<Telephony::WcdmaCellInformation *>(infoItem.GetRefPtr());
    if (wcdmaCellInfo != nullptr) {
        refInfo.cellId.type = HDI::Location::Agnss::V2_0::CELLID_TYPE_UMTS;
        if (CommonUtils::IsStrValidForStoi(wcdmaCellInfo->GetMcc())) {
            refInfo.cellId.mcc = static_cast<unsigned short>(std::stoi(wcdmaCellInfo->GetMcc()));
        }
        if (CommonUtils::IsStrValidForStoi(wcdmaCellInfo->GetMnc())) {
            refInfo.cellId.mnc = static_cast<unsigned short>(std::stoi(wcdmaCellInfo->GetMnc()));
        }
        refInfo.cellId.lac = static_cast<unsigned short>(wcdmaCellInfo->GetLac());
        refInfo.cellId.cid = static_cast<unsigned int>(wcdmaCellInfo->GetCellId());
    }
}
#endif
}  // namespace Location
}  // namespace OHOS
#endif
#endif
