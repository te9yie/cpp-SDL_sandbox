#include <deque>
#include <string>
#include <thread>

#include "pch.h"

namespace {

using namespace s6i_sync;

TEST(CondVarTest, BasicFunctionality) {
  // 条件変数の作成
  auto cond_result = CondVar::make();
  ASSERT_TRUE(cond_result.is_ok());
  auto cond = std::move(cond_result.unwrap());

  // Mutexの作成
  auto mutex_result = Mutex<bool>::make(false);
  ASSERT_TRUE(mutex_result.is_ok());
  auto mutex = std::move(mutex_result.unwrap());

  bool thread_finished = false;
  std::thread worker([&]() {
    auto guard_result = mutex.lock();
    ASSERT_TRUE(guard_result.is_ok());
    auto guard = guard_result.unwrap();

    // 条件が満たされるまで待機
    while (!*guard) {
      auto wait_result = cond.wait(guard);
      ASSERT_TRUE(wait_result.is_ok());
    }

    thread_finished = true;
  });

  // メインスレッドで条件を設定してシグナルを送信
  {
    auto guard_result = mutex.lock();
    ASSERT_TRUE(guard_result.is_ok());
    auto guard = guard_result.unwrap();
    *guard = true;
    auto signal_result = cond.signal(guard);
    ASSERT_TRUE(signal_result.is_ok());
  }

  worker.join();
  EXPECT_TRUE(thread_finished);
}

TEST(CondVarTest, ProducerConsumer) {
  auto cond_result = CondVar::make();
  ASSERT_TRUE(cond_result.is_ok());
  auto cond = std::move(cond_result.unwrap());

  auto mutex_result = Mutex<std::deque<int>>::make(std::deque<int>{});
  ASSERT_TRUE(mutex_result.is_ok());
  auto mutex = std::move(mutex_result.unwrap());

  const int num_items = 100;
  std::vector<int> consumed_items;

  // 消費者スレッド
  std::thread consumer([&]() {
    while (consumed_items.size() < num_items) {
      auto guard_result = mutex.lock();
      ASSERT_TRUE(guard_result.is_ok());
      auto guard = guard_result.unwrap();

      while (guard->empty()) {
        auto wait_result = cond.wait(guard);
        ASSERT_TRUE(wait_result.is_ok());
      }

      consumed_items.push_back(guard->front());
      guard->pop_front();
    }
  });

  // 生産者スレッド
  std::thread producer([&]() {
    for (int i = 0; i < num_items; ++i) {
      auto guard_result = mutex.lock();
      ASSERT_TRUE(guard_result.is_ok());
      auto guard = guard_result.unwrap();

      guard->push_back(i);
      auto signal_result = cond.signal(guard);
      ASSERT_TRUE(signal_result.is_ok());
    }
  });

  producer.join();
  consumer.join();

  // 全てのアイテムが順番通りに消費されたことを確認
  ASSERT_EQ(consumed_items.size(), num_items);
  for (int i = 0; i < num_items; ++i) {
    EXPECT_EQ(consumed_items[i], i);
  }
}

TEST(CondVarTest, MoveSemantics) {
  // ムーブコンストラクタのテスト
  auto cond_result = CondVar::make();
  ASSERT_TRUE(cond_result.is_ok());
  auto cond1 = std::move(cond_result.unwrap());
  CondVar cond2 = std::move(cond1);

  auto mutex_result = Mutex<bool>::make(false);
  ASSERT_TRUE(mutex_result.is_ok());
  auto mutex = std::move(mutex_result.unwrap());

  // 移動後のcond2が正常に動作することを確認
  bool thread_finished = false;
  std::thread worker([&]() {
    auto guard_result = mutex.lock();
    ASSERT_TRUE(guard_result.is_ok());
    auto guard = guard_result.unwrap();

    while (!*guard) {
      auto wait_result = cond2.wait(guard);
      ASSERT_TRUE(wait_result.is_ok());
    }

    thread_finished = true;
  });

  {
    auto guard_result = mutex.lock();
    ASSERT_TRUE(guard_result.is_ok());
    auto guard = guard_result.unwrap();
    *guard = true;
    auto signal_result = cond2.signal(guard);
    ASSERT_TRUE(signal_result.is_ok());
  }

  worker.join();
  EXPECT_TRUE(thread_finished);

  // 移動後の無効なcond1でのoperationが適切なエラーを返すことを確認
  auto guard_result = mutex.lock();
  ASSERT_TRUE(guard_result.is_ok());
  auto guard = guard_result.unwrap();

  auto wait_result = cond1.wait(guard);
  ASSERT_TRUE(wait_result.is_err());
  EXPECT_EQ(wait_result.unwrap_err(), SyncError::InvalidCondVarError);

  auto signal_result = cond1.signal(guard);
  ASSERT_TRUE(signal_result.is_err());
  EXPECT_EQ(signal_result.unwrap_err(), SyncError::InvalidCondVarError);

  auto broadcast_result = cond1.broadcast(guard);
  ASSERT_TRUE(broadcast_result.is_err());
  EXPECT_EQ(broadcast_result.unwrap_err(), SyncError::InvalidCondVarError);
}

TEST(CondVarTest, Broadcast) {
  auto cond_result = CondVar::make();
  ASSERT_TRUE(cond_result.is_ok());
  auto cond = std::move(cond_result.unwrap());

  auto mutex_result = Mutex<bool>::make(false);
  ASSERT_TRUE(mutex_result.is_ok());
  auto mutex = std::move(mutex_result.unwrap());

  const int num_threads = 5;
  std::atomic<int> woken_threads{0};

  std::vector<std::thread> workers;
  for (int i = 0; i < num_threads; ++i) {
    workers.emplace_back([&]() {
      auto guard_result = mutex.lock();
      ASSERT_TRUE(guard_result.is_ok());
      auto guard = guard_result.unwrap();

      while (!*guard) {
        auto wait_result = cond.wait(guard);
        ASSERT_TRUE(wait_result.is_ok());
      }

      woken_threads++;
    });
  }

  // 全スレッドが起動するのを少し待つ
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // broadcastで全スレッドを起こす
  {
    auto guard_result = mutex.lock();
    ASSERT_TRUE(guard_result.is_ok());
    auto guard = guard_result.unwrap();
    *guard = true;
    auto broadcast_result = cond.broadcast(guard);
    ASSERT_TRUE(broadcast_result.is_ok());
  }

  for (auto& worker : workers) {
    worker.join();
  }

  EXPECT_EQ(woken_threads.load(), num_threads);
}

}  // namespace
