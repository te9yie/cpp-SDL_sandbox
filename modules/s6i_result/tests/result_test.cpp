#include "s6i_result/result.h"
#include <gtest/gtest.h>
#include <string>

namespace {

// ヘルパー関数のテスト
TEST(ResultTest, MakeOkErrTest) {
  auto ok = s6i_result::make_ok(42);
  s6i_result::Result<int, std::string> ok_result(ok);
  EXPECT_TRUE(ok_result.is_ok());
  EXPECT_EQ(ok_result.unwrap(), 42);

  auto err = s6i_result::make_err<std::string>("error");
  s6i_result::Result<int, std::string> err_result(err);
  EXPECT_TRUE(err_result.is_err());
  EXPECT_EQ(err_result.unwrap_err(), "error");
}

// TとEが同じ型の場合でも、OkとErrで明確に区別できることを確認
TEST(ResultTest, SameTypeTest) {
  auto ok = s6i_result::make_ok(42);
  auto err = s6i_result::make_err(-1);
  s6i_result::Result<int, int> ok_result(ok);
  s6i_result::Result<int, int> err_result(err);

  EXPECT_TRUE(ok_result.is_ok());
  EXPECT_FALSE(ok_result.is_err());
  EXPECT_FALSE(err_result.is_ok());
  EXPECT_TRUE(err_result.is_err());

  EXPECT_EQ(ok_result.unwrap(), 42);
  EXPECT_EQ(err_result.unwrap_err(), -1);
}

// 基本的な状態チェック機能のテスト
TEST(ResultTest, IsOkTest) {
  auto ok = s6i_result::make_ok(42);
  auto err = s6i_result::make_err<std::string>("error");
  s6i_result::Result<int, std::string> ok_result(ok);
  s6i_result::Result<int, std::string> err_result(err);

  EXPECT_TRUE(ok_result.is_ok());
  EXPECT_FALSE(ok_result.is_err());
  EXPECT_FALSE(err_result.is_ok());
  EXPECT_TRUE(err_result.is_err());
}

// 成功値の取得機能のテスト
TEST(ResultTest, UnwrapOkTest) {
  auto ok = s6i_result::make_ok(42);
  s6i_result::Result<int, std::string> ok_result(ok);
  EXPECT_EQ(ok_result.unwrap(), 42);
}

// デフォルト値を使用した値の取得機能のテスト
TEST(ResultTest, UnwrapOrTest) {
  auto ok = s6i_result::make_ok(42);
  auto err = s6i_result::make_err<std::string>("error");
  s6i_result::Result<int, std::string> ok_result(ok);
  s6i_result::Result<int, std::string> err_result(err);

  EXPECT_EQ(ok_result.unwrap_or(0), 42);
  EXPECT_EQ(err_result.unwrap_or(0), 0);
}

// 成功値に対する変換機能のテスト
TEST(ResultTest, MapTest) {
  auto ok = s6i_result::make_ok(42);
  auto err = s6i_result::make_err<std::string>("error");
  s6i_result::Result<int, std::string> ok_result(ok);
  s6i_result::Result<int, std::string> err_result(err);

  auto mapped_ok = ok_result.map([](int x) { return x * 2; });
  EXPECT_TRUE(mapped_ok.is_ok());
  EXPECT_EQ(mapped_ok.unwrap(), 84);

  auto mapped_err = err_result.map([](int x) { return x * 2; });
  EXPECT_TRUE(mapped_err.is_err());
  EXPECT_EQ(mapped_err.unwrap_err(), "error");
}

// エラー値に対する変換機能のテスト
TEST(ResultTest, MapErrTest) {
  auto ok = s6i_result::make_ok(42);
  auto err = s6i_result::make_err<std::string>("error");
  s6i_result::Result<int, std::string> ok_result(ok);
  s6i_result::Result<int, std::string> err_result(err);

  auto mapped_ok =
      ok_result.map_err([](const std::string& e) { return e + "!"; });
  EXPECT_TRUE(mapped_ok.is_ok());
  EXPECT_EQ(mapped_ok.unwrap(), 42);

  auto mapped_err =
      err_result.map_err([](const std::string& e) { return e + "!"; });
  EXPECT_TRUE(mapped_err.is_err());
  EXPECT_EQ(mapped_err.unwrap_err(), "error!");
}

// 成功値に対する連鎖的な処理のテスト
TEST(ResultTest, AndThenTest) {
  auto ok = s6i_result::make_ok(42);
  auto err = s6i_result::make_err<std::string>("error");
  s6i_result::Result<int, std::string> ok_result(ok);
  s6i_result::Result<int, std::string> err_result(err);

  auto chained_ok = ok_result.and_then(
      [](int x) -> s6i_result::Result<std::string, std::string> {
        return s6i_result::make_ok(std::to_string(x));
      });
  EXPECT_TRUE(chained_ok.is_ok());
  EXPECT_EQ(chained_ok.unwrap(), "42");

  auto chained_err = err_result.and_then(
      [](int x) -> s6i_result::Result<std::string, std::string> {
        return s6i_result::make_ok(std::to_string(x));
      });
  EXPECT_TRUE(chained_err.is_err());
  EXPECT_EQ(chained_err.unwrap_err(), "error");
}

}  // namespace
