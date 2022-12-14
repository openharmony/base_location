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

#include "common_utils.h"
#include "work_record.h"

using namespace testing::ext;
namespace OHOS {
namespace Location {
const int WRONG_UID = 1001;
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
    EXPECT_EQ(SYSTEM_UID, parcel.ReadInt32()); // uid
    EXPECT_EQ(0, parcel.ReadInt32()); // pid
    EXPECT_EQ("name", parcel.ReadString()); // name
    EXPECT_EQ("deviceId", parcel.ReadString()); // device id
}

HWTEST_F(WorkRecordTest, AddAndRemoveWorkRecord001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "WorkRecordTest, AddAndRemoveWorkRecord001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[WorkRecordTest] AddAndRemoveWorkRecord001 begin");
    std::unique_ptr<WorkRecord> emptyWorkrecord = std::make_unique<WorkRecord>();
    EXPECT_EQ(false, emptyWorkrecord->Remove("emptyRecord"));
    EXPECT_EQ(false, emptyWorkrecord->Remove(SYSTEM_UID, 0, "emptyRecord"));

    MessageParcel parcel;
    parcel.WriteInt32(1); // workrecord size
    parcel.WriteInt32(SYSTEM_UID); // uid
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

    EXPECT_EQ(true, workrecord->Add(SYSTEM_UID, 0, "name"));
    EXPECT_EQ(1, workrecord->Size());
    EXPECT_EQ(false, workrecord->Remove(SYSTEM_UID, 0, "WrongName"));
    EXPECT_EQ(1, workrecord->Size());
    EXPECT_EQ(false, workrecord->Remove(999, 0, "name"));
    EXPECT_EQ(1, workrecord->Size());
    EXPECT_EQ(false, workrecord->Remove(999, 0, "WrongName"));
    EXPECT_EQ(1, workrecord->Size());
    EXPECT_EQ(false, workrecord->Remove(999, 1, "name"));
    EXPECT_EQ(1, workrecord->Size());
    EXPECT_EQ(false, workrecord->Remove(999, 1, "WrongName"));
    EXPECT_EQ(1, workrecord->Size());
    EXPECT_EQ(false, workrecord->Remove(SYSTEM_UID, 1, "WrongName"));
    EXPECT_EQ(1, workrecord->Size());

    EXPECT_EQ(true, workrecord->Remove(SYSTEM_UID, 0, "name"));
    EXPECT_EQ(0, workrecord->Size()); // remove successfully
    LBSLOGI(LOCATOR, "[WorkRecordTest] AddAndRemoveWorkRecord001 end");
}

HWTEST_F(WorkRecordTest, FindWorkRecord001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "WorkRecordTest, FindWorkRecord001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[WorkRecordTest] FindWorkRecord001 begin");
    std::unique_ptr<WorkRecord> workrecord = std::make_unique<WorkRecord>();
    EXPECT_EQ(false, workrecord->Find(SYSTEM_UID, "name"));
    EXPECT_EQ(true, workrecord->Add(SYSTEM_UID, 0, "name"));
    EXPECT_EQ("name", workrecord->GetName(0));
    EXPECT_EQ("", workrecord->GetName(1)); // out of range
    EXPECT_EQ("", workrecord->GetName(-1)); // out of range
    EXPECT_EQ(SYSTEM_UID, workrecord->GetUid(0));
    EXPECT_EQ(-1, workrecord->GetUid(1)); // out of range
    EXPECT_EQ(-1, workrecord->GetUid(-1)); // out of range
    EXPECT_EQ(0, workrecord->GetPid(0));
    EXPECT_EQ(-1, workrecord->GetPid(1)); // out of range
    EXPECT_EQ(-1, workrecord->GetPid(-1)); // out of range
    EXPECT_EQ(false, workrecord->Find(999, "name"));
    EXPECT_EQ(false, workrecord->Find(999, "WrongName"));
    EXPECT_EQ(true, workrecord->Find(SYSTEM_UID, "name"));
    EXPECT_EQ(false, workrecord->Find(SYSTEM_UID, "WrongName"));
    LBSLOGI(LOCATOR, "[WorkRecordTest] FindWorkRecord001 end");
}

HWTEST_F(WorkRecordTest, ClearWorkRecord001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "WorkRecordTest, ClearWorkRecord001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[WorkRecordTest] ClearWorkRecord001 begin");
    std::unique_ptr<WorkRecord> workrecord = std::make_unique<WorkRecord>();
    EXPECT_EQ(true, workrecord->Add(SYSTEM_UID, 0, "name"));
    EXPECT_EQ(false, workrecord->IsEmpty());
    workrecord->Clear();
    EXPECT_EQ(true, workrecord->IsEmpty());
    LBSLOGI(LOCATOR, "[WorkRecordTest] ClearWorkRecord001 end");
}

HWTEST_F(WorkRecordTest, SetDeviceId001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "WorkRecordTest, SetDeviceId001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[WorkRecordTest] SetDeviceId001 begin");
    std::unique_ptr<WorkRecord> workrecord = std::make_unique<WorkRecord>();
    workrecord->SetDeviceId("deviceId");
    EXPECT_EQ("deviceId", workrecord->GetDeviceId());
    LBSLOGI(LOCATOR, "[WorkRecordTest] SetDeviceId001 end");
}

HWTEST_F(WorkRecordTest, WorkRecordToString001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "WorkRecordTest, WorkRecordToString001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[WorkRecordTest] WorkRecordToString001 begin");
    std::unique_ptr<WorkRecord> workrecord = std::make_unique<WorkRecord>();
    EXPECT_EQ("[]", workrecord->ToString());
    EXPECT_EQ(true, workrecord->Add(SYSTEM_UID, 0, "name"));
    EXPECT_NE("", workrecord->ToString());
    LBSLOGI(LOCATOR, "[WorkRecordTest] WorkRecordToString001 end");
}

HWTEST_F(WorkRecordTest, MarshallingWorkRecord001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "WorkRecordTest, MarshallingWorkRecord001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[WorkRecordTest] MarshallingWorkRecord001 begin");
    std::unique_ptr<WorkRecord> workrecord = std::make_unique<WorkRecord>();
    workrecord->Add(SYSTEM_UID, 0, "name1");
    workrecord->Add(WRONG_UID, 0, "name2");
    MessageParcel parcel;
    workrecord->MarshallingWorkRecord(parcel);

    EXPECT_EQ(2, parcel.ReadInt32()); // number
    EXPECT_EQ(2, parcel.ReadInt32()); // uids number
    EXPECT_EQ(SYSTEM_UID, parcel.ReadInt32());
    EXPECT_EQ(WRONG_UID, parcel.ReadInt32());
    EXPECT_EQ(2, parcel.ReadInt32()); // names number
    EXPECT_EQ("name1", Str16ToStr8(parcel.ReadString16()));
    EXPECT_EQ("name2", Str16ToStr8(parcel.ReadString16()));
    LBSLOGI(LOCATOR, "[WorkRecordTest] MarshallingWorkRecord001 end");
}

HWTEST_F(WorkRecordTest, AddWorkRecord001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "WorkRecordTest, AddWorkRecord001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[WorkRecordTest] AddWorkRecord001 begin");
    std::unique_ptr<WorkRecord> workrecord = std::make_unique<WorkRecord>();
    EXPECT_EQ(true, workrecord->Add(SYSTEM_UID + 1, 0, "name"));
    EXPECT_EQ(true, workrecord->Add(SYSTEM_UID, 0, "name")); // diff uid, add to record
    EXPECT_EQ(false, workrecord->Add(SYSTEM_UID, 0, "name")); // the same name in record
    EXPECT_EQ(true, workrecord->Add(SYSTEM_UID, 0, "DiffName")); // diff name, add to record
    LBSLOGI(LOCATOR, "[WorkRecordTest] AddWorkRecord001 end");
}
} // namespace Location
} // namespace OHOS