#include <gtest/gtest.h>
#include <emb_ext_flash.h>

// Class for facilitating embedded external flash memory tests
class emb_ext_flash_test : public ::testing::Test
{
public:
  emb_ext_flash_test()
  {
    // initialization code here
  }

  void SetUp()
  {
    // code here will execute just before the test ensues
  }

  void TearDown()
  {
    // code here will be called just after the test completes
    // ok to through exceptions from here if need be
  }

  ~emb_ext_flash_test()
  {
    // cleanup any pending stuff, but no exceptions allowed
  }

  // put in any custom data members that you need
};

TEST_F(emb_ext_flash_test, test1)
{
  // test code here
  ASSERT_TRUE(true);
}