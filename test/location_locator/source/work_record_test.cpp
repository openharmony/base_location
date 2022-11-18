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

#include "work_record_test.h"

#include "string_ex.h"

#include "message_parcel.h"

#include "work_record.h"

using namespace testing::ext;
namespace OHOS {
namespace Location {
void WorkRecordTest::SetUp()
{
}

void WorkRecordTest::TearDown()
{
}

void WorkRecordTest::VerifyMarshalling(std::unique_ptr<WorkRecord>& workrecord)
{
    MessageParcel parcel;
    workrecord->Marshalling(parcel);
    EXPECT_EQ(1, parcel.ReadInt32()); // num
    EXPECT_EQ(1000, parcel.ReadInt32()); // uid
    EXPECT_EQ(0, parcel.ReadInt32()); // pid
    EXPECT_EQ("name", parcel.ReadString()); //name
    EXPECT_EQ("deviceId", parcel.ReadString()); // device id
}

HWTEST_F(WorkRecordTest, AddAndRemoveWorkRecord001, TestSize.Level1)
{
    std::unique_ptr<WorkRecord> emptyWorkrecord = std::make_unique<WorkRecord>();
    EXPECT_EQ(false, emptyWorkrecord->Remove("emptyRecord"));
    EXPECT_EQ(false, emptyWorkrecord->Remove(1000, 0, "emptyRecord"));

    MessageParcel parcel;
    parcel.WriteInt32(1); // workrecord size
    parcel.WriteInt32(1000); // uid
    parcel.WriteInt32(0); // pid
    parcel.WriteString("name");
    parcel.WriteString("deviceId");
    std::unique_ptr<WorkRecord> workrecord = WorkRecord::Unmarshalling(parcel);
    VerifyMarshalling(workrecord);
    EXPECT_EQ(1, workrecord->Size());
    EXPECT_EQ(false, workrecord->Remove("WrongName"));
    EXPECT_EQ(1, workrecord->Size());
    EXPECT_EQ(true, workrecord->Remove("name"));
    EXPECT_EQ(0, workrecord->Size()); // remove successfully

    EXPECT_EQ(true, workrecord->Add(1000, 0, "name"));
    EXPECT_EQ(1, workrecord->Size());
    EXPECT_EQ(false, workrecord->Remove(1000, 0, "WrongName"));
    EXPECT_EQ(1, workrecord->Size());
    EXPECT_EQ(false, workrecord->Remove(999, 0, "name"));
    EXPECT_EQ(1, workrecord->Size());
    EXPECT_EQ(false, workrecord->Remove(999, 0, "WrongName"));
    EXPECT_EQ(1, workrecord->Size());
    EXPECT_EQ(false, workrecord->Remove(999, 1, "name"));
    EXPECT_EQ(1, workrecord->Size());
    EXPECT_EQ(false, workrecord->Remove(999, 1, "WrongName"));
    EXPECT_EQ(1, workrecord->Size());
    EXPECT_EQ(false, workrecord->Remove(1000, 1, "WrongName"));
    EXPECT_EQ(1, workrecord->Size());

    EXPECT_EQ(true, workrecord->Remove(1000, 0, "name"));
    EXPECT_EQ(0, workrecord->Size()); // remove successfully
}

HWTEST_F(WorkRecordTest, FindWorkRecord001, TestSize.Level1)
{
    std::unique_ptr<WorkRecord> workrecord = std::make_unique<WorkRecord>();
    EXPECT_EQ(false, workrecord->Find(1000, "name"));
    EXPECT_EQ(true, workrecord->Add(1000, 0, "name"));
    EXPECT_EQ("name", workrecord->GetName(0));
    EXPECT_EQ("", workrecord->GetName(1));
    EXPECT_EQ(1000, workrecord->GetUid(0));
    EXPECT_EQ(-1, workrecord->GetUid(1));
    EXPECT_EQ(0, workrecord->GetPid(0));
    EXPECT_EQ(-1, workrecord->GetPid(1));
    EXPECT_EQ(false, workrecord->Find(999, "name"));
    EXPECT_EQ(false, workrecord->Find(999, "WrongName"));
    EXPECT_EQ(true, workrecord->Find(1000, "name"));
}

HWTEST_F(WorkRecordTest, ClearWorkRecord001, TestSize.Level1)
{
    std::unique_ptr<WorkRecord> workrecord = std::make_unique<WorkRecord>();
    EXPECT_EQ(true, workrecord->Add(1000, 0, "name"));
    EXPECT_EQ(false, workrecord->IsEmpty());
    workrecord->Clear();
    EXPECT_EQ(true, workrecord->IsEmpty());
}

HWTEST_F(WorkRecordTest, SetDeviceId001, TestSize.Level1)
{
    std::unique_ptr<WorkRecord> workrecord = std::make_unique<WorkRecord>();
    workrecord->SetDeviceId("deviceId");
    EXPECT_EQ("deviceId", workrecord->GetDeviceId());
}

HWTEST_F(WorkRecordTest, WorkRecordToString001, TestSize.Level1)
{
    std::unique_ptr<WorkRecord> workrecord = std::make_unique<WorkRecord>();
    EXPECT_EQ("[]", workrecord->ToString());
    EXPECT_EQ(true, workrecord->Add(1000, 0, "name"));
    EXPECT_NE("", workrecord->ToString());
}

HWTEST_F(WorkRecordTest, MarshallingWorkRecord001, TestSize.Level1)
{
    std::unique_ptr<WorkRecord> workrecord = std::make_unique<WorkRecord>();
    workrecord->Add(1000, 0, "name1");
    workrecord->Add(1001, 0, "name2");
    MessageParcel parcel;
    workrecord->MarshallingWorkRecord(parcel);

    EXPECT_EQ(2, parcel.ReadInt32()); // number
    EXPECT_EQ(2, parcel.ReadInt32()); // uids number
    EXPECT_EQ(1000, parcel.ReadInt32());
    EXPECT_EQ(1001, parcel.ReadInt32());
    EXPECT_EQ(2, parcel.ReadInt32()); // names number
    EXPECT_EQ("name1", Str16ToStr8(parcel.ReadString16()));
    EXPECT_EQ("name2", Str16ToStr8(parcel.ReadString16()));
}
} // namespace Location
} // namespace OHOS