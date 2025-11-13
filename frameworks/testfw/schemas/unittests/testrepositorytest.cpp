/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "testrepository.h"

#include "ng/log.h"

#include "gtest/gtest.h"

#include <stdexcept>



const char* const JSON =
"{                                                                           \n"
"    \"version\": 1,                                                         \n"
"    \"featured\": \"gl_alu\",                                               \n"
"    \"tests\": [                                                            \n"
"        {                                                                   \n"
"            \"test_name\": \"gl_alu\",                                      \n"
"            \"group_name\": \"group_low_level\",                            \n"
"            \"run_order\": 2,                                               \n"
"            \"variant_postfixes\": [                                        \n"
"                \"\",                                                       \n"
"                \"_off\"                                                    \n"
"            ],                                                              \n"
"            \"minimum_api\": [                                              \n"
"                {\"type\": \"GL\", \"major\": 3, \"minor\" : 0},            \n"
"                {\"type\": \"ES\", \"major\": 2, \"minor\" : 0}             \n"
"            ],                                                              \n"
"            \"requirements\": [                                             \n"
"            ],                                                              \n"
"            \"results\": [                                                  \n"
"                {\"result_postfix\": \"\", \"unit\": \"Frames\"}            \n"
"            ]                                                               \n"
"        },                                                                  \n"
"        {                                                                   \n"
"            \"test_name\": \"gl_runalone\",                                 \n"
"            \"group_name\": \"group_special\",                              \n"
"            \"run_order\": 0,                                               \n"
"            \"variant_postfixes\": [                                        \n"
"                \"\"                                                        \n"
"            ],                                                              \n"
"            \"minimum_api\": [                                              \n"
"                {\"type\": \"GL\", \"major\": 3, \"minor\" : 0},            \n"
"                {\"type\": \"ES\", \"major\": 2, \"minor\" : 0}             \n"
"            ],                                                              \n"
"            \"requirements\": [                                             \n"
"                \"runalone\"                                                \n"
"            ],                                                              \n"
"            \"results\": [                                                  \n"
"                {\"result_postfix\": \"\", \"unit\": \"Frames\"}            \n"
"            ]                                                               \n"
"        },                                                                  \n"
"        {                                                                   \n"
"            \"test_name\": \"gl_trex_battery\",                             \n"
"            \"group_name\": \"group_special\",                              \n"
"            \"run_order\": 1,                                               \n"
"            \"variant_postfixes\": [                                        \n"
"                \"\"                                                        \n"
"            ],                                                              \n"
"            \"minimum_api\": [                                              \n"
"                {\"type\": \"GL\", \"major\": 3, \"minor\" : 0},            \n"
"                {\"type\": \"ES\", \"major\": 2, \"minor\" : 0}             \n"
"            ],                                                              \n"
"            \"requirements\": [                                             \n"
"                \"battery\"                                                 \n"
"            ],                                                              \n"
"            \"results\": [                                                  \n"
"                {\"result_postfix\": \"_performance\", \"unit\":\"Frame\"}, \n"
"                {\"result_postfix\": \"_lifetime\", \"unit\":\"min\"},      \n"
"                {\"result_postfix\": \"_diagram\", \"unit\":\"chart\"}      \n"
"            ]                                                               \n"
"        }                                                                   \n"
"    ]                                                                       \n"
"}                                                                           \n";



const char* const COMPUTE_JSON =
"{                                                                           \n"
"    \"version\": 1,                                                         \n"
"    \"featured\": \"gl_alu\",                                               \n"
"    \"tests\": [                                                            \n"
"        {                                                                   \n"
"            \"test_name\": \"gl_alu\",                                      \n"
"            \"group_name\": \"group_low_level\",                            \n"
"            \"run_order\": 2,                                               \n"
"            \"variant_postfixes\": [                                        \n"
"                \"\",                                                       \n"
"                \"_off\"                                                    \n"
"            ],                                                              \n"
"            \"minimum_api\": [                                              \n"
"                {\"type\": \"GL\", \"major\": 3, \"minor\" : 0},            \n"
"                {\"type\": \"ES\", \"major\": 2, \"minor\" : 0}             \n"
"            ],                                                              \n"
"            \"minimum_compute_api\": [                                      \n"
"                {\"type\": \"CL\", \"major\": 1, \"minor\" : 0}             \n"
"            ],                                                              \n"
"            \"requirements\": [                                             \n"
"            ],                                                              \n"
"            \"results\": [                                                  \n"
"                {\"result_postfix\": \"\", \"unit\": \"Frames\"}            \n"
"            ]                                                               \n"
"        }                                                                   \n"
"    ]                                                                       \n"
"}                                                                           \n";



const char* const MIXED_JSON =
"{                                                                           \n"
"    \"version\": 1,                                                         \n"
"    \"featured\": \"gl_alu\",                                               \n"
"    \"tests\": [                                                            \n"
"        {                                                                   \n"
"            \"test_name\": \"gl_alu\",                                      \n"
"            \"group_name\": \"group_low_level\",                            \n"
"            \"run_order\": 2,                                               \n"
"            \"variant_postfixes\": [                                        \n"
"                \"\",                                                       \n"
"                \"_off\"                                                    \n"
"            ],                                                              \n"
"            \"minimum_api\": [                                              \n"
"                {\"type\": \"GL\", \"major\": 3, \"minor\" : 0},            \n"
"                {\"type\": \"ES\", \"major\": 2, \"minor\" : 0}             \n"
"            ],                                                              \n"
"            \"minimum_compute_api\": [                                      \n"
"                {\"type\": \"CL\", \"major\": 1, \"minor\" : 0}             \n"
"            ],                                                              \n"
"            \"requirements\": [                                             \n"
"            ],                                                              \n"
"            \"results\": [                                                  \n"
"                {\"result_postfix\": \"\", \"unit\": \"Frames\"}            \n"
"            ]                                                               \n"
"        },                                                                  \n"
"        {                                                                   \n"
"            \"test_name\": \"gl_runalone\",                                 \n"
"            \"group_name\": \"group_special\",                              \n"
"            \"run_order\": 0,                                               \n"
"            \"variant_postfixes\": [                                        \n"
"                \"\"                                                        \n"
"            ],                                                              \n"
"            \"minimum_api\": [                                              \n"
"                {\"type\": \"GL\", \"major\": 3, \"minor\" : 0},            \n"
"                {\"type\": \"ES\", \"major\": 2, \"minor\" : 0}             \n"
"            ],                                                              \n"
"            \"requirements\": [                                             \n"
"                \"runalone\"                                                \n"
"            ],                                                              \n"
"            \"results\": [                                                  \n"
"                {\"result_postfix\": \"\", \"unit\": \"Frames\"}            \n"
"            ]                                                               \n"
"        }                                                                   \n"
"    ]                                                                       \n"
"}                                                                           \n";



class TestRepositoryTest: public testing::Test {
protected:
    tfw::Configuration gpuConfiguration;
    tfw::Configuration gpuAndBatteryConfiguration;
    std::vector<tfw::ResultGroup> results;
    std::vector<tfw::CompareResult> compareResults;

    static void SetUpTestCase() {
        DisableLogging();
    }

    static void DisableLogging() {
        static ng::LogSinkPrintf sink;
        sink.setMinSeverity(ng::log::Severity::warn);
        static ng::Logger logger;
        logger.addSink(&sink);
        ng::Logger::setTheGlobal(&logger);
    }

    void SetUp() {
        CreateConfigurations();
        CreateResults();
        CreateCompareResults();
    }

    void CreateConfigurations() {
        tfw::ApiDefinition apiDefinition;
        apiDefinition.setType(tfw::ApiDefinition::GL);
        apiDefinition.setMajor(4);
        apiDefinition.setMinor(3);

        std::vector<tfw::ApiDefinition> apiDefinitions;
        apiDefinitions.push_back(apiDefinition);

        tfw::Configuration configuration;
        configuration.setApiDefinitions(apiDefinitions);
        configuration.setName("Unit test GPU");
        configuration.setType("GPU");
        gpuConfiguration = configuration;

        std::vector<std::string> features;
        features.push_back("battery");
        configuration.setFeatures(features);
        gpuAndBatteryConfiguration = configuration;
    }

    void CreateResults() {
        tfw::Result result;

        tfw::ResultGroup resultGroup;
        result.setTestId("gl_alu_off");
        result.setResultId("gl_alu_off");
        result.setScore(1000);
        resultGroup.addResult(result);
        results.push_back(resultGroup);

        resultGroup = tfw::ResultGroup();
        result.setTestId("gl_runalone");
        result.setResultId("gl_runalone");
        result.setScore(1500);
        resultGroup.addResult(result);
        results.push_back(resultGroup);
        
        resultGroup = tfw::ResultGroup();
        result.setTestId("gl_trex_battery");
        result.setResultId("gl_trex_battey_performance");
        result.setScore(1000);
        resultGroup.addResult(result);
        results.push_back(resultGroup);
    }

    void CreateCompareResults() {
        tfw::CompareResult compareResult;

        compareResult.setResultId("gl_alu");
        compareResult.setScore(1000);
        compareResults.push_back(compareResult);

        compareResult.setResultId("gl_runalone");
        compareResult.setScore(500);
        compareResults.push_back(compareResult);

        compareResult.setResultId("gl_trex_battey_performance");
        compareResult.setScore(2000);
        compareResults.push_back(compareResult);
    }
};


    
TEST_F(TestRepositoryTest, LoadInvalidJson)
{
    tfw::TestRepository testRepository;
    EXPECT_THROW(testRepository.loadTestsFromJsonString("Can't parse this!"), std::runtime_error);
}



TEST_F(TestRepositoryTest, SingleColumnTestLayout)
{
    tfw::TestRepository repo;
    repo.loadTestsFromJsonString(JSON);

    std::vector<std::vector<tfw::TestItem> > layout =
            tfw::TestRepository::singleColumnLayout(repo.tests());

    EXPECT_EQ(layout.size(), 4);

    EXPECT_EQ(layout.at(0).size(), 1);
    EXPECT_EQ(layout.at(0).front().testId(), "gl_alu");
    EXPECT_TRUE(layout.at(0).front().isFirstInGroup());

    EXPECT_EQ(layout.at(1).size(), 1);
    EXPECT_EQ(layout.at(1).front().testId(), "gl_alu_off");
    EXPECT_FALSE(layout.at(1).front().isFirstInGroup());

    EXPECT_EQ(layout.at(2).size(), 1);
    EXPECT_EQ(layout.at(2).front().testId(), "gl_runalone");
    EXPECT_TRUE(layout.at(2).front().isFirstInGroup());

    EXPECT_EQ(layout.at(3).size(), 1);
    EXPECT_EQ(layout.at(3).front().testId(), "gl_trex_battery");
    EXPECT_FALSE(layout.at(3).front().isFirstInGroup());
}



TEST_F(TestRepositoryTest, MultiColumnTestLayout)
{
    tfw::TestRepository repo;
    repo.loadTestsFromJsonString(JSON);

    std::vector<std::vector<tfw::TestItem> > layout = tfw::TestRepository::multiColumnLayout(
            repo.tests());

    EXPECT_EQ(layout.size(), 3);
    EXPECT_EQ(layout.at(0).size(), 2);
    EXPECT_EQ(layout.at(0).front().testId(), "gl_alu");
    EXPECT_TRUE(layout.at(0).front().isFirstInGroup());
    EXPECT_EQ(layout.at(0).back().testId(), "gl_alu_off");
    EXPECT_FALSE(layout.at(0).back().isFirstInGroup());

    EXPECT_EQ(layout.at(1).size(), 1);
    EXPECT_EQ(layout.at(1).front().testId(), "gl_runalone");
    EXPECT_TRUE(layout.at(1).front().isFirstInGroup());

    EXPECT_EQ(layout.at(2).size(), 1);
    EXPECT_EQ(layout.at(2).front().testId(), "gl_trex_battery");
    EXPECT_FALSE(layout.at(2).front().isFirstInGroup());
}



TEST_F(TestRepositoryTest, AvailableTests)
{
    tfw::TestRepository repo;
    repo.loadTestsFromJsonString(JSON);
    repo.addConfiguration(gpuAndBatteryConfiguration);
    EXPECT_EQ(repo.allAvailableTests().size(), 3);
    EXPECT_EQ(repo.allAvailableTests().at(0).testId(), "gl_trex_battery");
    EXPECT_EQ(repo.allAvailableTests().at(1).testId(), "gl_alu");
    EXPECT_EQ(repo.allAvailableTests().at(2).testId(), "gl_alu_off");
    EXPECT_TRUE(repo.findTest("gl_alu").isAvailable());
    EXPECT_TRUE(repo.findTest("gl_alu_off").isAvailable());
    EXPECT_TRUE(repo.findTest("gl_runalone").isAvailable());
    EXPECT_TRUE(repo.findTest("gl_trex_battery").isAvailable());
}



TEST_F(TestRepositoryTest, AvailableTestsNoConfiguration)
{
    tfw::TestRepository repo;
    repo.loadTestsFromJsonString(JSON);
    EXPECT_TRUE(repo.allAvailableTests().empty());
    EXPECT_FALSE(repo.findTest("gl_alu").isAvailable());
    EXPECT_FALSE(repo.findTest("gl_alu_off").isAvailable());
    EXPECT_FALSE(repo.findTest("gl_runalone").isAvailable());
    EXPECT_FALSE(repo.findTest("gl_trex_battery").isAvailable());
}



TEST_F(TestRepositoryTest, AvailableTestsMissingFeature)
{
    tfw::TestRepository repo;
    repo.loadTestsFromJsonString(JSON);
    repo.addConfiguration(gpuConfiguration);
    EXPECT_EQ(repo.allAvailableTests().size(), 2);
    EXPECT_EQ(repo.allAvailableTests().at(0).testId(), "gl_alu");
    EXPECT_EQ(repo.allAvailableTests().at(1).testId(), "gl_alu_off");
    EXPECT_TRUE(repo.findTest("gl_alu").isAvailable());
    EXPECT_TRUE(repo.findTest("gl_alu_off").isAvailable());
    EXPECT_TRUE(repo.findTest("gl_runalone").isAvailable());
    EXPECT_FALSE(repo.findTest("gl_trex_battery").isAvailable());
}



TEST_F(TestRepositoryTest, GroupSelection)
{
    tfw::TestRepository repo;
    repo.loadTestsFromJsonString(JSON);
    repo.addConfiguration(gpuAndBatteryConfiguration);

    EXPECT_TRUE(repo.findTest("gl_alu").isGroupSelected());
    EXPECT_TRUE(repo.findTest("gl_alu_off").isGroupSelected());
    EXPECT_FALSE(repo.findTest("gl_runalone").isGroupSelected());
    EXPECT_FALSE(repo.findTest("gl_trex_battery").isGroupSelected());

    repo.setTestSelection("gl_alu_off", false);
    EXPECT_FALSE(repo.findTest("gl_alu").isGroupSelected());
    EXPECT_FALSE(repo.findTest("gl_alu_off").isGroupSelected());
    EXPECT_FALSE(repo.findTest("gl_runalone").isGroupSelected());
    EXPECT_FALSE(repo.findTest("gl_trex_battery").isGroupSelected());

    repo.setGroupSelection("group_low_level", false);
    EXPECT_FALSE(repo.findTest("gl_alu").isSelected());
    EXPECT_FALSE(repo.findTest("gl_alu_off").isSelected());

    repo.setGroupSelection("group_low_level", true);
    EXPECT_TRUE(repo.findTest("gl_alu").isSelected());
    EXPECT_TRUE(repo.findTest("gl_alu_off").isSelected());

    repo.setTestSelection("gl_trex_battery", false);
    EXPECT_FALSE(repo.findTest("gl_runalone").isGroupSelected());
    EXPECT_FALSE(repo.findTest("gl_trex_battery").isGroupSelected());

    repo.setTestSelection("gl_trex_battery", true);
    EXPECT_FALSE(repo.findTest("gl_runalone").isGroupSelected());
    EXPECT_FALSE(repo.findTest("gl_trex_battery").isGroupSelected());

    repo.setGroupSelection("group_special", false);
    EXPECT_FALSE(repo.findTest("gl_runalone").isSelected());
    EXPECT_FALSE(repo.findTest("gl_trex_battery").isSelected());

    repo.setGroupSelection("group_special", true);
    EXPECT_TRUE(repo.findTest("gl_runalone").isSelected());
    EXPECT_TRUE(repo.findTest("gl_trex_battery").isSelected());
}



TEST_F(TestRepositoryTest, GetDuelResultsBestFirst)
{
    tfw::TestRepository repo;
    repo.loadTestsFromJsonString(JSON);
    repo.setBestResults(results);
    repo.setDuelResults(compareResults);

    EXPECT_EQ(repo.duelResults().at(2).resultId(), "gl_runalone");
    EXPECT_EQ(repo.duelResults().at(2).score(), 1500);
    EXPECT_EQ(repo.duelResults().at(2).compareScore(), 500);
}



TEST_F(TestRepositoryTest, GetDuelResultsDuelFirst)
{
    tfw::TestRepository repo;
    repo.loadTestsFromJsonString(JSON);
    repo.setDuelResults(compareResults);
    repo.setBestResults(results);

    EXPECT_EQ(repo.duelResults().at(2).resultId(), "gl_runalone");
    EXPECT_EQ(repo.duelResults().at(2).score(), 1500);
    EXPECT_EQ(repo.duelResults().at(2).compareScore(), 500);
}



TEST_F(TestRepositoryTest, NeedClConfigurations)
{
    tfw::TestRepository noClRepo;
    noClRepo.loadTestsFromJsonString(JSON);
    EXPECT_FALSE(noClRepo.needClConfigurations());

    tfw::TestRepository clRepo;
    clRepo.loadTestsFromJsonString(COMPUTE_JSON);
    EXPECT_TRUE(clRepo.needClConfigurations());
}



TEST_F(TestRepositoryTest, GetconfigsGl)
{
    tfw::TestRepository repo;
    repo.loadTestsFromJsonString(JSON);
    std::vector<unsigned> configs = repo.allowedConfigurations();

    EXPECT_EQ(configs.size(), 2);
    EXPECT_TRUE(std::find(configs.begin(), configs.end(), tfw::ApiDefinition::GL) != configs.end());
    EXPECT_TRUE(std::find(configs.begin(), configs.end(), tfw::ApiDefinition::ES) != configs.end());
}



TEST_F(TestRepositoryTest, GetconfigsCl)
{
    tfw::TestRepository repo;
    repo.loadTestsFromJsonString(COMPUTE_JSON);
    std::vector<unsigned> configs = repo.allowedConfigurations();

    EXPECT_EQ(configs.size(), 2);
    EXPECT_FALSE(std::find(configs.begin(), configs.end(), tfw::ApiDefinition::CL) != configs.end());
    EXPECT_FALSE(std::find(configs.begin(), configs.end(), tfw::ApiDefinition::GL) != configs.end());
    EXPECT_FALSE(std::find(configs.begin(), configs.end(), tfw::ApiDefinition::ES) != configs.end());
    EXPECT_TRUE(std::find(configs.begin(), configs.end(), tfw::ApiDefinition::GL | tfw::ApiDefinition::CL) != configs.end());
    EXPECT_TRUE(std::find(configs.begin(), configs.end(), tfw::ApiDefinition::ES | tfw::ApiDefinition::CL) != configs.end());
}



TEST_F(TestRepositoryTest, GetconfigsGlCl)
{
    tfw::TestRepository repo;
    repo.loadTestsFromJsonString(MIXED_JSON);
    std::vector<unsigned> configs = repo.allowedConfigurations();

    EXPECT_EQ(configs.size(), 4);
    EXPECT_FALSE(std::find(configs.begin(), configs.end(), tfw::ApiDefinition::CL) != configs.end());
    EXPECT_TRUE(std::find(configs.begin(), configs.end(), tfw::ApiDefinition::GL) != configs.end());
    EXPECT_TRUE(std::find(configs.begin(), configs.end(), tfw::ApiDefinition::ES) != configs.end());
    EXPECT_TRUE(std::find(configs.begin(), configs.end(), tfw::ApiDefinition::GL | tfw::ApiDefinition::CL) != configs.end());
    EXPECT_TRUE(std::find(configs.begin(), configs.end(), tfw::ApiDefinition::ES | tfw::ApiDefinition::CL) != configs.end());
}
