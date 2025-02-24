#include <string>
#include <thread>

namespace {

using namespace s6i_sync;

TEST(MutexTest, BasicFunctionality) {
  // 基本的な作成と値の取得
  auto mutex_result = Mutex<int>::make(42);
  ASSERT_TRUE(mutex_result.is_ok());
  auto mutex = std::move(mutex_result.unwrap());

  {
    auto guard_result = mutex.lock();
    ASSERT_TRUE(guard_result.is_ok());
    auto& guard = guard_result.unwrap();
    EXPECT_EQ(*guard, 42);
    *guard = 100;
  }  // ここでアンロック

  // 値が変更されていることを確認
  {
    auto guard_result = mutex.lock();
    ASSERT_TRUE(guard_result.is_ok());
    auto& guard = guard_result.unwrap();
    EXPECT_EQ(*guard, 100);
  }
}

TEST(MutexTest, ComplexType) {
  // 複雑な型（std::string）でのテスト
  auto mutex_result = Mutex<std::string>::make("hello");
  ASSERT_TRUE(mutex_result.is_ok());
  auto mutex = std::move(mutex_result.unwrap());

  {
    auto guard_result = mutex.lock();
    ASSERT_TRUE(guard_result.is_ok());
    auto& guard = guard_result.unwrap();
    EXPECT_EQ(*guard, "hello");
    guard->append(" world");
  }

  {
    auto guard_result = mutex.lock();
    ASSERT_TRUE(guard_result.is_ok());
    auto& guard = guard_result.unwrap();
    EXPECT_EQ(*guard, "hello world");
  }
}

TEST(MutexTest, MoveSemantics) {
  // ムーブコンストラクタのテスト
  auto mutex_result = Mutex<int>::make(42);
  ASSERT_TRUE(mutex_result.is_ok());
  auto mutex1 = std::move(mutex_result.unwrap());
  Mutex<int> mutex2 = std::move(mutex1);

  {
    auto guard_result = mutex2.lock();
    ASSERT_TRUE(guard_result.is_ok());
    auto& guard = guard_result.unwrap();
    EXPECT_EQ(*guard, 42);
  }

  // ムーブ代入のテスト
  auto mutex3_result = Mutex<int>::make(100);
  ASSERT_TRUE(mutex3_result.is_ok());
  auto mutex3 = std::move(mutex3_result.unwrap());
  mutex2 = std::move(mutex3);

  {
    auto guard_result = mutex2.lock();
    ASSERT_TRUE(guard_result.is_ok());
    auto& guard = guard_result.unwrap();
    EXPECT_EQ(*guard, 100);
  }
}

TEST(MutexTest, ThreadSafety) {
  auto mutex_result = Mutex<int>::make(0);
  ASSERT_TRUE(mutex_result.is_ok());
  auto mutex = std::move(mutex_result.unwrap());

  // 複数スレッドでカウンターをインクリメント
  const int num_threads = 10;
  const int iterations = 1000;
  std::vector<std::thread> threads;

  for (int i = 0; i < num_threads; ++i) {
    threads.emplace_back([&mutex, iterations]() {
      for (int j = 0; j < iterations; ++j) {
        mutex.lock()
            .inspect_ok(
                [](MutexGuard<int>& guard_result) { *guard_result += 1; })
            .inspect_err([](SyncError) { FAIL(); });
      }
    });
  }

  // 全スレッドの完了を待つ
  for (auto& thread : threads) {
    thread.join();
  }

  // 最終的な値を確認
  {
    auto guard_result = mutex.lock();
    ASSERT_TRUE(guard_result.is_ok());
    auto& guard = guard_result.unwrap();
    EXPECT_EQ(*guard, num_threads * iterations);
  }
}

TEST(MutexTest, ErrorHandling) {
  // 無効なMutexでのロック操作
  auto mutex_result = Mutex<int>::make(42);
  ASSERT_TRUE(mutex_result.is_ok());
  auto mutex1 = std::move(mutex_result.unwrap());
  Mutex<int> mutex2 = std::move(mutex1);  // mutex1は無効に

  // 無効化されたmutex1でのロック試行
  auto guard_result = mutex1.lock();
  ASSERT_TRUE(guard_result.is_err());
  EXPECT_EQ(guard_result.unwrap_err(), SyncError::InvalidMutexError);

  // 有効なmutex2でのロック試行
  auto guard_result2 = mutex2.lock();
  ASSERT_TRUE(guard_result2.is_ok());
  auto& guard = guard_result2.unwrap();
  EXPECT_EQ(*guard, 42);
}

}  // namespace
