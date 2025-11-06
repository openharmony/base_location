/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#ifndef APP_IDENTITY_H
#define APP_IDENTITY_H

#include <string>
#include <parcel.h>

namespace OHOS {
namespace Location {
class AppIdentity : public Parcelable {
public:
    AppIdentity();
    explicit AppIdentity(pid_t uid, pid_t pid, uint32_t tokenId, uint64_t tokenIdEx, uint32_t firstTokenId);
    virtual ~AppIdentity() = default;

    inline pid_t GetPid() const
    {
        return pid_;
    }

    inline void SetPid(pid_t pid)
    {
        pid_ = pid;
    }

    inline pid_t GetUid() const
    {
        return uid_;
    }

    inline void SetUid(pid_t uid)
    {
        uid_ = uid;
    }

    inline uint32_t GetTokenId() const
    {
        return tokenId_;
    }

    inline void SetTokenId(uint32_t tokenId)
    {
        tokenId_ = tokenId;
    }

    inline uint64_t GetTokenIdEx() const
    {
        return tokenIdEx_;
    }

    inline void SetTokenIdEx(uint64_t tokenIdEx)
    {
        tokenIdEx_ = tokenIdEx;
    }

    inline uint32_t GetFirstTokenId() const
    {
        return firstTokenId_;
    }

    inline void SetFirstTokenId(uint32_t firstTokenId)
    {
        firstTokenId_ = firstTokenId;
    }

    inline std::string GetBundleName() const
    {
        return bundleName_;
    }

    inline void SetBundleName(std::string bundleName)
    {
        bundleName_ = bundleName;
    }
    void ReadFromParcel(Parcel& parcel)
    {
        uid_ = parcel.ReadInt32();
        pid_ = parcel.ReadInt32();
        tokenId_ = static_cast<uint32_t>(parcel.ReadInt32());
        tokenIdEx_ = parcel.ReadInt64();
        firstTokenId_ = static_cast<uint32_t>(parcel.ReadInt32());
        bundleName_ = parcel.ReadString();
    }

    bool Marshalling(Parcel& parcel) const
    {
        return parcel.WriteInt32(uid_) &&
           parcel.WriteInt32(pid_) &&
           parcel.WriteInt32(tokenId_) &&
           parcel.WriteInt64(tokenIdEx_) &&
           parcel.WriteInt32(firstTokenId_) &&
           parcel.WriteString(bundleName_);
    }

    static std::shared_ptr<AppIdentity> UnmarshallingShared(Parcel& parcel)
    {
        auto identity = std::make_shared<AppIdentity>();
        identity->ReadFromParcel(parcel);
        return identity;
    }

    static AppIdentity* Unmarshalling(Parcel& parcel)
    {
        auto identity = std::make_shared<AppIdentity>();
        identity->ReadFromParcel(parcel);
        return identity;
    }

    std::string ToString() const;
private:
    pid_t uid_;
    pid_t pid_;
    uint32_t tokenId_;
    uint64_t tokenIdEx_;
    uint32_t firstTokenId_;
    std::string bundleName_;
};
} // namespace Location
} // namespace OHOS
#endif // APP_IDENTITY_H