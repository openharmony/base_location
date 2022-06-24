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

#include "agnss_event_callback.h"
#include <singleton.h>
#include "common_utils.h"
#include "gnss_ability.h"
#include "location_log.h"
#include "locator_ability.h"

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
    int slotId = Telephony::CellularDataClient::GetInstance().GetDefaultCellularDataSlotId();
    std::string imsi =
        CommonUtils::Str16ToStr8(DelayedRefSingleton<Telephony::CoreServiceClient>::GetInstance().GetIMSI(slotId));
    SubscriberSetId setId;
    setId.type = HDI::Location::Agnss::V1_0::SETID_TYPE_IMSI;
    setId.id = imsi;
    DelayedSingleton<GnssAbility>::GetInstance().get()->SetSetId(setId);
    return ERR_OK;
}

int32_t AGnssEventCallback::RequestAgnssRefInfo()
{
    int slotId = Telephony::CellularDataClient::GetInstance().GetDefaultCellularDataSlotId();
    std::vector<sptr<CellInformation>> cellInformations =
        DelayedRefSingleton<Telephony::CoreServiceClient>::GetInstance().GetCellInfoList(slotId);

    LBSLOGI(GNSS, "RequestAgnssRefInfo,cellInformations.");
    for (sptr<CellInformation> infoItem : cellInformations) {
        if (!infoItem->GetIsCamped()) {
            LBSLOGE(GNSS, "GetIsCamped return false");
            continue;
        }
        AGnssRefInfo refInfo;
        CellInformation::CellType cellType = infoItem->GetNetworkType();
        refInfo.type = HDI::Location::Agnss::V1_0::ANSS_REF_INFO_TYPE_CELLID;
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
        DelayedSingleton<GnssAbility>::GetInstance().get()->SetRefInfo(refInfo);
        break;
    }
    return ERR_OK;
}

void AGnssEventCallback::JudgmentDataGsm(AGnssRefInfo& refInfo, sptr<CellInformation> infoItem)
{
    auto gsmCellInfo = static_cast<Telephony::GsmCellInformation *>(infoItem.GetRefPtr());
    if (gsmCellInfo != nullptr) {
        refInfo.cellId.type = HDI::Location::Agnss::V1_0::CELLID_TYPE_GSM;
        refInfo.cellId.mcc = std::stoi(gsmCellInfo->GetMcc());
        refInfo.cellId.mnc = std::stoi(gsmCellInfo->GetMnc());
        refInfo.cellId.lac = gsmCellInfo->GetLac();
        refInfo.cellId.cid = gsmCellInfo->GetCellId();
    }
}

void AGnssEventCallback::JudgmentDataLte(AGnssRefInfo& refInfo, sptr<CellInformation> infoItem)
{
    auto lteCellInfo = static_cast<Telephony::LteCellInformation *>(infoItem.GetRefPtr());
    if (lteCellInfo != nullptr) {
        refInfo.cellId.type = HDI::Location::Agnss::V1_0::CELLID_TYPE_LTE;
        refInfo.cellId.mcc = std::stoi(lteCellInfo->GetMcc());
        refInfo.cellId.mnc = std::stoi(lteCellInfo->GetMnc());
        refInfo.cellId.tac = lteCellInfo->GetTac();
        refInfo.cellId.cid = lteCellInfo->GetCellId();
        refInfo.cellId.pcid = lteCellInfo->GetPci();
    }
}

void AGnssEventCallback::JudgmentDataNr(AGnssRefInfo& refInfo, sptr<CellInformation> infoItem)
{
    auto nrCellInfo = static_cast<Telephony::NrCellInformation *>(infoItem.GetRefPtr());
    if (nrCellInfo != nullptr) {
        refInfo.cellId.type = HDI::Location::Agnss::V1_0::CELLID_TYPE_NR;
        refInfo.cellId.mcc = std::stoi(nrCellInfo->GetMcc());
        refInfo.cellId.mnc = std::stoi(nrCellInfo->GetMnc());
        refInfo.cellId.tac = nrCellInfo->GetTac();
        refInfo.cellId.cid = nrCellInfo->GetCellId();
        refInfo.cellId.pcid = nrCellInfo->GetPci();
        refInfo.cellId.nci = nrCellInfo->GetNci();
    }
}

void AGnssEventCallback::JudgmentDataUmts(AGnssRefInfo& refInfo, sptr<CellInformation> infoItem)
{
    auto wcdmaCellInfo = static_cast<Telephony::WcdmaCellInformation *>(infoItem.GetRefPtr());
    if (wcdmaCellInfo != nullptr) {
        refInfo.cellId.type = HDI::Location::Agnss::V1_0::CELLID_TYPE_UMTS;
        refInfo.cellId.mcc = std::stoi(wcdmaCellInfo->GetMcc());
        refInfo.cellId.mnc = std::stoi(wcdmaCellInfo->GetMnc());
        refInfo.cellId.lac = wcdmaCellInfo->GetLac();
        refInfo.cellId.cid = wcdmaCellInfo->GetCellId();
    }
}
}  // namespace Location
}  // namespace OHOS