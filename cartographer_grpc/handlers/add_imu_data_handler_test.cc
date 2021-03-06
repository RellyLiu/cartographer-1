/*
 * Copyright 2018 The Cartographer Authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "cartographer_grpc/handlers/add_imu_data_handler.h"
#include "cartographer_grpc/testing/handler_test.h"
#include "cartographer_grpc/testing/test_helpers.h"
#include "google/protobuf/text_format.h"
#include "gtest/gtest.h"

namespace cartographer_grpc {
namespace handlers {
namespace {

using ::testing::_;
using ::testing::Eq;
using ::testing::Pointee;
using ::testing::Truly;

const std::string kMessage = R"PROTO(
  sensor_metadata {
    trajectory_id: 1
    sensor_id: "sensor_id"
  }
  imu_data {
    timestamp: 2
    linear_acceleration {
      x: 3 y: 4 z: 5
    }
    angular_velocity {
      x: 6 y: 7 z: 8
    }
  })PROTO";

using AddImuDataHandlerTest = testing::HandlerTest<AddImuDataHandler>;

TEST_F(AddImuDataHandlerTest, NoLocalSlamUploader) {
  proto::AddImuDataRequest request;
  EXPECT_TRUE(
      google::protobuf::TextFormat::ParseFromString(kMessage, &request));
  SetNoLocalTrajectoryUploader();
  EXPECT_CALL(*mock_map_builder_context_,
              DoEnqueueSensorData(
                  Eq(request.sensor_metadata().trajectory_id()),
                  Pointee(Truly(testing::BuildDataPredicateEquals(request)))));
  test_server_->SendWrite(request);
  test_server_->SendWritesDone();
  test_server_->SendFinish();
}

TEST_F(AddImuDataHandlerTest, WithMockLocalSlamUploader) {
  proto::AddImuDataRequest request;
  EXPECT_TRUE(
      google::protobuf::TextFormat::ParseFromString(kMessage, &request));
  SetMockLocalTrajectoryUploader();
  EXPECT_CALL(*mock_map_builder_context_,
              DoEnqueueSensorData(
                  Eq(request.sensor_metadata().trajectory_id()),
                  Pointee(Truly(testing::BuildDataPredicateEquals(request)))));
  EXPECT_CALL(*mock_local_trajectory_uploader_,
              DoEnqueueDataRequest(Pointee(
                  Truly(testing::BuildProtoPredicateEquals(&request)))));
  test_server_->SendWrite(request);
  test_server_->SendWritesDone();
  test_server_->SendFinish();
}

}  // namespace
}  // namespace handlers
}  // namespace cartographer_grpc
