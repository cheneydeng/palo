// Copyright (c) 2017, Baidu.com, Inc. All Rights Reserved

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

#include <gtest/gtest.h>
#include <sstream>

#include "olap/field.h"
#include "olap/olap_cond.h"
#include "olap/olap_define.h"
#include "olap/olap_engine.h"
#include "olap/olap_header.h"
#include "olap/olap_table.h"
#include "olap/olap_common.h"
#include "olap/row_cursor.h"
#include "olap/column_file/stream_index_common.h"
#include "olap/column_file/stream_index_writer.h"
#include "olap/column_file/stream_index_reader.h"
#include "olap/column_file/file_stream.h"
#include "util/logging.h"

using namespace std;

namespace palo {
namespace column_file {

class TestStreamIndex : public testing::Test {
public:
    virtual ~TestStreamIndex() {
    }

    virtual void SetUp() {     
    }
    
    virtual void TearDown() {
    }

};

TEST_F(TestStreamIndex, index_write) {
    StreamIndexWriter writer(OLAP_FIELD_TYPE_INT);
    PositionEntryWriter entry;

    static const uint32_t loop = 10;
    uint32_t i = 0;
    for (; i < loop; i++) {
        entry.add_position(i);
        entry.add_position(i*2);
        entry.add_position(i*3);

        writer.add_index_entry(entry);
        entry.reset_write_offset();
    }

    size_t output_size = sizeof(StreamIndexHeader) + i * sizeof(uint32_t) * 3;
    ASSERT_EQ(output_size, writer.output_size());

    char* buffer = new char[output_size];

    ASSERT_EQ(OLAP_SUCCESS, writer.write_to_buffer(buffer, output_size));

    StreamIndexReader reader;
    ASSERT_EQ(OLAP_SUCCESS, reader.init(buffer, output_size, OLAP_FIELD_TYPE_NONE, true, false));

    ASSERT_EQ(loop, reader.entry_count());

    for (i = 0; i < loop; i++) {
        const PositionEntryReader& e = reader.entry(i);
        ASSERT_EQ(e.positions(0), i);
        ASSERT_EQ(e.positions(1), i * 2);
        ASSERT_EQ(e.positions(2), i * 3);
    }
}

TEST_F(TestStreamIndex, remove_written_position) {
    StreamIndexWriter writer(OLAP_FIELD_TYPE_INT);
    PositionEntryWriter entry;

    static const uint32_t loop = 10;
    //test 1
    {
        uint32_t i = 0;
        for (; i < loop; i++) {
            entry.add_position(i);
            entry.add_position(i*2);
            entry.add_position(i*3);
            entry.add_position(i*4);
            entry.add_position(i*5);
            entry.add_position(i*6);
            entry.add_position(i*7);

            writer.add_index_entry(entry);
            entry.reset_write_offset();
        }

        for (i = 0; i < loop; i++) {
            PositionEntryWriter* e = writer.mutable_entry(i);
            e->remove_written_position(0, 4);
        }

        size_t output_size = writer.output_size();

        char* buffer = new char[output_size];

        ASSERT_EQ(OLAP_SUCCESS, writer.write_to_buffer(buffer, output_size));

        StreamIndexReader reader;
        ASSERT_EQ(OLAP_SUCCESS, 
            reader.init(buffer, output_size, OLAP_FIELD_TYPE_NONE, true, false));

        ASSERT_EQ(loop, reader.entry_count());

        for (i = 0; i < loop; i++) {
            const PositionEntryReader& e = reader.entry(i);
            ASSERT_EQ(e.positions(0), i * 5);
            ASSERT_EQ(e.positions(1), i * 6);
            ASSERT_EQ(e.positions(2), i * 7);
        }
    }
    writer.reset();

    // test 2
    {
        uint32_t i = 0;
        for (; i < loop; i++) {
            entry.add_position(i);
            entry.add_position(i*2);
            entry.add_position(i*3);
            entry.add_position(i*4);
            entry.add_position(i*5);
            entry.add_position(i*6);
            entry.add_position(i*7);

            writer.add_index_entry(entry);
            entry.reset_write_offset();
        }

        for (i = 0; i < loop; i++) {
            PositionEntryWriter* e = writer.mutable_entry(i);
            e->remove_written_position(0, 2);
        }

        size_t output_size = writer.output_size();

        char* buffer = new char[output_size];

        ASSERT_EQ(OLAP_SUCCESS, writer.write_to_buffer(buffer, output_size));

        StreamIndexReader reader;
        ASSERT_EQ(OLAP_SUCCESS, 
            reader.init(buffer, output_size, OLAP_FIELD_TYPE_NONE, true, false));

        ASSERT_EQ(loop, reader.entry_count());

        for (i = 0; i < loop; i++) {
            const PositionEntryReader& e = reader.entry(i);
            ASSERT_EQ(e.positions(0), i * 3);
            ASSERT_EQ(e.positions(1), i * 4);
            ASSERT_EQ(e.positions(2), i * 5);
            ASSERT_EQ(e.positions(3), i * 6);
            ASSERT_EQ(e.positions(4), i * 7);
        }
    }
    writer.reset();
    // test 3
    {
        uint32_t i = 0;
        for (; i < loop; i++) {
            entry.add_position(i);
            entry.add_position(i*2);
            entry.add_position(i*3);
            entry.add_position(i*4);
            entry.add_position(i*5);
            entry.add_position(i*6);
            entry.add_position(i*7);

            writer.add_index_entry(entry);
            entry.reset_write_offset();
        }

        for (i = 0; i < loop; i++) {
            PositionEntryWriter* e = writer.mutable_entry(i);
            e->remove_written_position(3, 2);
        }

        size_t output_size = writer.output_size();

        char* buffer = new char[output_size];

        ASSERT_EQ(OLAP_SUCCESS, writer.write_to_buffer(buffer, output_size));

        StreamIndexReader reader;
        ASSERT_EQ(OLAP_SUCCESS, 
            reader.init(buffer, output_size, OLAP_FIELD_TYPE_NONE, true, false));

        ASSERT_EQ(loop, reader.entry_count());

        for (i = 0; i < loop; i++) {
            const PositionEntryReader& e = reader.entry(i);
            ASSERT_EQ(e.positions(0), i * 1);
            ASSERT_EQ(e.positions(1), i * 2);
            ASSERT_EQ(e.positions(2), i * 3);
            ASSERT_EQ(e.positions(3), i * 6);
            ASSERT_EQ(e.positions(4), i * 7);
        }
    }
    writer.reset();
    // test 4
    {
        uint32_t i = 0;
        for (; i < loop; i++) {
            entry.add_position(i);
            entry.add_position(i*2);
            entry.add_position(i*3);
            entry.add_position(i*4);
            entry.add_position(i*5);
            entry.add_position(i*6);
            entry.add_position(i*7);

            writer.add_index_entry(entry);
            entry.reset_write_offset();
        }

        for (i = 0; i < loop; i++) {
            PositionEntryWriter* e = writer.mutable_entry(i);
            e->remove_written_position(4, 3);
        }

        size_t output_size = writer.output_size();

        char* buffer = new char[output_size];

        ASSERT_EQ(OLAP_SUCCESS, writer.write_to_buffer(buffer, output_size));

        StreamIndexReader reader;
        ASSERT_EQ(OLAP_SUCCESS, 
            reader.init(buffer, output_size, OLAP_FIELD_TYPE_NONE, true, false));

        ASSERT_EQ(loop, reader.entry_count());

        for (i = 0; i < loop; i++) {
            const PositionEntryReader& e = reader.entry(i);
            ASSERT_EQ(e.positions(0), i * 1);
            ASSERT_EQ(e.positions(1), i * 2);
            ASSERT_EQ(e.positions(2), i * 3);
            ASSERT_EQ(e.positions(3 ), i * 4);
        }
    }
    writer.reset();
}

TEST_F(TestStreamIndex, test_statistic) {
    ColumnStatistics stat;
    ASSERT_EQ(OLAP_SUCCESS, stat.init(OLAP_FIELD_TYPE_INT, true));

    Field* field = Field::create_by_type(OLAP_FIELD_TYPE_INT);
    ASSERT_TRUE(field->allocate());

    // start
    ASSERT_STREQ(stat.minimum()->to_string().c_str(), "2147483647");
    ASSERT_STREQ(stat.maximum()->to_string().c_str(), "-2147483648");

    // 1 
    field->from_string("3");
    stat.add(field);
    ASSERT_STREQ(stat.minimum()->to_string().c_str(), "3");
    ASSERT_STREQ(stat.maximum()->to_string().c_str(), "3");

    // 2
    field->from_string("5");
    stat.add(field);
    ASSERT_STREQ(stat.minimum()->to_string().c_str(), "3");
    ASSERT_STREQ(stat.maximum()->to_string().c_str(), "5");

    // 3
    field->from_string("899");
    stat.add(field);
    ASSERT_STREQ(stat.minimum()->to_string().c_str(), "3");
    ASSERT_STREQ(stat.maximum()->to_string().c_str(), "899");

    // 4
    field->from_string("-111");
    stat.add(field);
    ASSERT_STREQ(stat.minimum()->to_string().c_str(), "-111");
    ASSERT_STREQ(stat.maximum()->to_string().c_str(), "899");

    stat.reset();
   // start
    ASSERT_STREQ(stat.minimum()->to_string().c_str(), "2147483647");
    ASSERT_STREQ(stat.maximum()->to_string().c_str(), "-2147483648");

    field->from_string("3");
    stat.add(field);
    field->from_string("6");
    stat.add(field);
    ASSERT_STREQ(stat.minimum()->to_string().c_str(), "3");
    ASSERT_STREQ(stat.maximum()->to_string().c_str(), "6");

    ColumnStatistics stat2;
    ASSERT_EQ(OLAP_SUCCESS, stat2.init(OLAP_FIELD_TYPE_INT, true));

    char buf[256];
    stat.write_to_buffer(buf, sizeof(buf));
    stat2.attach(buf);

    ASSERT_STREQ(stat2.minimum()->to_string().c_str(), "3");
    ASSERT_STREQ(stat2.maximum()->to_string().c_str(), "6");
}

TEST_F(TestStreamIndex, statistic) {
    StreamIndexWriter writer(OLAP_FIELD_TYPE_INT);
    PositionEntryWriter entry;
    ColumnStatistics stat;

    ASSERT_EQ(OLAP_SUCCESS, stat.init(OLAP_FIELD_TYPE_INT, true));

    Field* field = Field::create_by_type(OLAP_FIELD_TYPE_INT);
    ASSERT_TRUE(NULL != field);
    ASSERT_TRUE(field->allocate());
    char string_buffer[256];

    static const uint32_t loop = 10;
    uint32_t i = 0;
    for (; i < loop; i++) {
        entry.add_position(i);
        entry.add_position(i*2);
        entry.add_position(i*3);

        snprintf(string_buffer, sizeof(string_buffer), "%d", i * 9);
        field->from_string(string_buffer);
        stat.add(field);

        snprintf(string_buffer, sizeof(string_buffer), "%d", i * 2);
        field->from_string(string_buffer);
        stat.add(field);

        printf("%d. max %s\n", i, stat.maximum()->to_string().c_str());
        printf("%d. min %s\n", i, stat.minimum()->to_string().c_str());
        entry.set_statistic(&stat);

        printf("%d. min -> %d\n", i, entry._statistics_buffer[0]);
        printf("%d. max -> %d\n", i, entry._statistics_buffer[1]);

        writer.add_index_entry(entry);
        entry.reset_write_offset();
    }

    size_t output_size = sizeof(StreamIndexHeader) + 
                        loop * sizeof(uint32_t) * 3 + 
                        (1 + sizeof(int32_t)) * loop * 2;
    ASSERT_EQ(output_size, writer.output_size());

    char* buffer = new char[output_size];

    ASSERT_EQ(OLAP_SUCCESS, writer.write_to_buffer(buffer, output_size));

    StreamIndexReader reader;
    ASSERT_EQ(OLAP_SUCCESS, reader.init(buffer, output_size, OLAP_FIELD_TYPE_INT, true, true));

    ASSERT_EQ(loop, reader.entry_count());

    for (i = 0; i < loop; i++) {
        const PositionEntryReader& e = reader.entry(i);
        ASSERT_EQ(e.positions(0), i);
        ASSERT_EQ(e.positions(1), i * 2);
        ASSERT_EQ(e.positions(2), i * 3);

        if (!e.column_statistic().ignored()) {
            printf("%d. max %s\n", i, e.column_statistic().maximum()->to_string().c_str());
            printf("%d. min %s\n", i, e.column_statistic().minimum()->to_string().c_str());
        }
    }
}

}
}

int main(int argc, char** argv) {
    std::string conffile = std::string(getenv("PALO_HOME")) + "/conf/be.conf";
    if (!palo::config::init(conffile.c_str(), false)) {
        fprintf(stderr, "error read config file. \n");
        return -1;
    }
    palo::init_glog("be-test");
    int ret = palo::OLAP_SUCCESS;
    testing::InitGoogleTest(&argc, argv);
    ret = RUN_ALL_TESTS();
    google::protobuf::ShutdownProtobufLibrary();
    return ret;
}


