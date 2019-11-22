#include <parser.hpp>
#include <gtest/gtest.h>
#include <k_means.h>
int main(int argc, char * argv [])
{

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

TEST(KMeansTest, direct_cast_2Dim)
{
    int lineCount = 15;

    std::ofstream testStream("test");
    ASSERT_TRUE(testStream.is_open());

    for(int i=0; i<(lineCount*2); i++,i++)
    {
        testStream <<i+1 << " " << i+2<<"\n";
    }
    testStream.close();

    ProgramOptions options;
    options.centroidsCount = 2;
    options.threadPoolSize = 1;

    options.fstream =  std::ifstream("test");

    CentroidsType testCentroids;
    testCentroids.push_back({3,4});
    testCentroids.push_back({10,11});


    KMeans means(options, [&testCentroids](CentroidsType & centroids, ProgramOptions & , int)
    {
        centroids = testCentroids;
        return true;
    });

    CentroidsType resultCentroids;
    ASSERT_TRUE(means.clustering(resultCentroids));

    ASSERT_EQ(7, resultCentroids.front().front());
    ASSERT_EQ(8, resultCentroids.front().back());

    ASSERT_EQ(22, resultCentroids.back().front());
    ASSERT_EQ(23, resultCentroids.back().back());

}

TEST(ParserTest, direct_negative_number1)
{
    std::vector<double> outVect;
    std::string str = "-1.111";
    ASSERT_TRUE(parsePoint(str.c_str(), outVect));

   ASSERT_EQ(-1.111, outVect.front());
}

TEST(ParserTest, direct_case1)
{
    std::vector<double> outVect;
    std::string str = "1.1";
    ASSERT_TRUE(parsePoint(str.c_str(), outVect));

   ASSERT_EQ(1.1, outVect.front());
}

TEST(ParserTest, direct_case2)
{
    std::vector<double> outVect;
    const char str[] = "1.1 200\0";
    ASSERT_TRUE(parsePoint(str, outVect));

   ASSERT_EQ(1.1, outVect.front());
   ASSERT_EQ(200, outVect.back());
}

TEST(ParserTest, direct_case1000)
{
    std::vector<double> outVect;

    std::string str;
    for(int i = 0; i < 1000; i++)
    {
        str += std::to_string(static_cast<double>(i)+static_cast<double>(i)*0.1);
        str += " ";
    }
    ASSERT_TRUE(parsePoint(str.c_str(), outVect));

}

TEST(ParserTest, negative_case1001)
{
    std::vector<double> outVect;

    std::string str;
    for(int i = 0; i < 1001; i++)
    {
        str += std::to_string(static_cast<double>(i)+static_cast<double>(i)*0.1);
        str += " ";
    }
    ASSERT_FALSE(parsePointWithChecking(str.c_str(), outVect));

}

TEST(ParserTest, negative_case0)
{
    std::vector<double> outVect;
    std::string str = "";
    ASSERT_FALSE(parsePointWithChecking(str.c_str(), outVect));
}



