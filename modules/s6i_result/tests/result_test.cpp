#include "s6i_result/result.h"
#include <gtest/gtest.h>
#include <string>

namespace {

// コピー不可・ムーブ可能なクラス
class MoveOnlyType {
 public:
  explicit MoveOnlyType(int value) : m_value(value) {}

  // コピー禁止
  MoveOnlyType(const MoveOnlyType&) = delete;
  MoveOnlyType& operator=(const MoveOnlyType&) = delete;

  // ムーブ可能
  MoveOnlyType(MoveOnlyType&& other) noexcept : m_value(other.m_value) {
    other.m_value = 0;
  }
  MoveOnlyType& operator=(MoveOnlyType&& other) noexcept {
    if (this != &other) {
      m_value = other.m_value;
      other.m_value = 0;
    }
    return *this;
  }

  int value() const { return m_value; }

 private:
  int m_value;
};

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

// 成功値の取得機能のテスト（ムーブ）
TEST(ResultTest, UnwrapOkTest) {
  auto ok = s6i_result::make_ok(42);
  s6i_result::Result<int, std::string> ok_result(ok);
  int value = ok_result.unwrap();  // ムーブ
  EXPECT_EQ(value, 42);
}

// 成功値の参照取得テスト
TEST(ResultTest, RefOkTest) {
  auto ok = s6i_result::make_ok(42);
  s6i_result::Result<int, std::string> ok_result(ok);

  // 参照経由で値を変更
  ok_result.ref_ok() = 100;
  EXPECT_EQ(ok_result.ref_ok(), 100);

  // const参照での読み取り
  const auto& const_result = ok_result;
  EXPECT_EQ(const_result.ref_ok(), 100);
}

// 失敗値の参照取得テスト
TEST(ResultTest, RefErrTest) {
  auto err = s6i_result::make_err<std::string>("error");
  s6i_result::Result<int, std::string> err_result(err);

  // 参照経由で値を変更
  err_result.ref_err() += "!";
  EXPECT_EQ(err_result.ref_err(), "error!");

  // const参照での読み取り
  const auto& const_result = err_result;
  EXPECT_EQ(const_result.ref_err(), "error!");
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

// inspect_okのテスト
TEST(ResultTest, InspectOkTest) {
  bool was_called = false;
  int inspected_value = 0;

  // Ok値の場合
  auto ok = s6i_result::make_ok(42);
  s6i_result::Result<int, std::string> ok_result(ok);
  auto inspect_result = ok_result.inspect_ok([&](int value) {
    was_called = true;
    inspected_value = value;
  });

  // コールバックが呼ばれたことを確認
  EXPECT_TRUE(was_called);
  EXPECT_EQ(inspected_value, 42);
  // 元のResultが返されることを確認
  EXPECT_TRUE(inspect_result.is_ok());
  EXPECT_EQ(inspect_result.unwrap(), 42);

  // Err値の場合
  was_called = false;
  auto err = s6i_result::make_err<std::string>("error");
  s6i_result::Result<int, std::string> err_result(err);
  auto inspect_err_result =
      err_result.inspect_ok([&](int) { was_called = true; });

  // コールバックが呼ばれないことを確認
  EXPECT_FALSE(was_called);
  // 元のResultが返されることを確認
  EXPECT_TRUE(inspect_err_result.is_err());
  EXPECT_EQ(inspect_err_result.unwrap_err(), "error");
}

// inspect_errのテスト
TEST(ResultTest, InspectErrTest) {
  bool was_called = false;
  std::string inspected_error;

  // Err値の場合
  auto err = s6i_result::make_err<std::string>("error");
  s6i_result::Result<int, std::string> err_result(err);
  auto inspect_result = err_result.inspect_err([&](const std::string& error) {
    was_called = true;
    inspected_error = error;
  });

  // コールバックが呼ばれたことを確認
  EXPECT_TRUE(was_called);
  EXPECT_EQ(inspected_error, "error");
  // 元のResultが返されることを確認
  EXPECT_TRUE(inspect_result.is_err());
  EXPECT_EQ(inspect_result.unwrap_err(), "error");

  // Ok値の場合
  was_called = false;
  auto ok = s6i_result::make_ok(42);
  s6i_result::Result<int, std::string> ok_result(ok);
  auto inspect_ok_result =
      ok_result.inspect_err([&](const std::string&) { was_called = true; });

  // コールバックが呼ばれないことを確認
  EXPECT_FALSE(was_called);
  // 元のResultが返されることを確認
  EXPECT_TRUE(inspect_ok_result.is_ok());
  EXPECT_EQ(inspect_ok_result.unwrap(), 42);
}

// ムーブオンリー型のテスト
TEST(ResultTest, MoveOnlyTypeTest) {
  // Ok値のテスト
  auto ok = s6i_result::make_ok(MoveOnlyType(42));
  s6i_result::Result<MoveOnlyType, std::string> ok_result(std::move(ok));
  EXPECT_TRUE(ok_result.is_ok());
  EXPECT_EQ(ok_result.ref_ok().value(), 42);

  // ムーブ後の値の確認
  MoveOnlyType moved_value = ok_result.unwrap();
  EXPECT_EQ(moved_value.value(), 42);

  // Err値のテスト
  auto err = s6i_result::make_err<std::string>("error");
  s6i_result::Result<MoveOnlyType, std::string> err_result(std::move(err));
  EXPECT_TRUE(err_result.is_err());
  EXPECT_EQ(err_result.unwrap_err(), "error");

  // map関数のテスト
  auto ok2 = s6i_result::make_ok(MoveOnlyType(10));
  s6i_result::Result<MoveOnlyType, std::string> ok_result2(std::move(ok2));

  auto mapped = ok_result2.map(
      [](MoveOnlyType& x) { return MoveOnlyType(x.value() * 2); });
  EXPECT_TRUE(mapped.is_ok());
  EXPECT_EQ(mapped.ref_ok().value(), 20);
}

}  // namespace
