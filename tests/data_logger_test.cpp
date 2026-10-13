#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>

extern "C" {
#include "data_logger.h"
}

TEST(DataLogger, OpensAppendsAndExportsVitalSamples)
{
    const std::filesystem::path directory =
        std::filesystem::temp_directory_path() / "cardio_data_logger_test";
    const std::filesystem::path exported = directory / "export.csv";
    std::filesystem::create_directories(directory);
    std::filesystem::remove(directory / "vitals_P-001.csv");
    std::filesystem::remove(exported);

    DataLogger_t logger = {};
    const VitalSample_t sample = {1234, 72, 98, 120, 80, 16};
    ASSERT_TRUE(data_logger_open(&logger, directory.c_str(), "P-001"));
    ASSERT_TRUE(data_logger_append(&logger, &sample));
    ASSERT_TRUE(data_logger_flush(&logger));
    ASSERT_TRUE(data_logger_export(&logger, exported.c_str()));
    data_logger_close(&logger);

    std::ifstream contents(exported);
    std::string text((std::istreambuf_iterator<char>(contents)), std::istreambuf_iterator<char>());
    EXPECT_NE(text.find("tick_ms,hr,spo2,sys,dia,resp\n"), std::string::npos);
    EXPECT_NE(text.find("1234,72,98,120,80,16\n"), std::string::npos);
    std::filesystem::remove_all(directory);
}

TEST(DataLogger, FailsToOpenAnUnavailableDirectory)
{
    DataLogger_t logger = {};

    EXPECT_FALSE(data_logger_open(&logger, "/path/that/does/not/exist", "P-001"));
    EXPECT_FALSE(logger.is_open);
    EXPECT_EQ(logger.file_handle, nullptr);
}
