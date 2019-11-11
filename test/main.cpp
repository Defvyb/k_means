#include <parser.hpp>
#include <gtest/gtest.h>


int main(int argc, char * argv [])
{

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
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
    std::string str = "1.1 200";
    ASSERT_TRUE(parsePoint(str.c_str(), outVect));

   ASSERT_EQ(1.1, outVect.front());
   ASSERT_EQ(200, outVect.back());
}

TEST(ParserTest, direct_case1000)
{
    std::vector<double> outVect;

    std::string str;
    for(double i = 0; i < 1000; i++)
    {
        str += std::to_string(i+i*0.1);
        str += " ";
    }
    ASSERT_TRUE(parsePoint(str.c_str(), outVect));

}

TEST(ParserTest, negative_case1001)
{
    std::vector<double> outVect;

    std::string str;
    for(double i = 0; i < 1001; i++)
    {
        str += std::to_string(i+i*0.1);
        str += " ";
    }
    ASSERT_FALSE(parsePoint(str.c_str(), outVect));

}

TEST(ParserTest, negative_case0)
{
    std::vector<double> outVect;
    std::string str = "";
    ASSERT_TRUE(parsePoint(str.c_str(), outVect));
}


