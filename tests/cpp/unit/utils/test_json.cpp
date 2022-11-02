/*******************************************************************************
* Copyright 2022 Intel Corporation
*
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
*******************************************************************************/
#include <cstdio>
#include <string>
#include "utils/json.hpp"
#include "gtest/gtest.h"

TEST(Json, WriterReader) {
    std::string filename = "test.txt";
    std::ofstream of(filename);
    dnnl::graph::impl::utils::json::json_writer_t writer(&of);
    const std::string test_v = "\"\\tr\nls\\kv\rm\"";
    ASSERT_NO_THROW(writer.write_string(test_v));
    of.close();
    std::ifstream fs("test.txt");
    dnnl::graph::impl::utils::json::json_reader_t read(&fs);
    std::string tmp;
    ASSERT_NO_THROW(read.read_string(&tmp));
    fs.close();
    ASSERT_EQ(std::remove("test.txt"), 0);
}