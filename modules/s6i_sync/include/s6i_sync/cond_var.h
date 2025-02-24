#pragma once

#include <SDL.h>
#include <s6i_result/result.h>
#include <cassert>
#include <variant>  // std::monostateのため
#include "error.h"
#include "mutex.h"

namespace s6i_sync {

/**
 * @brief 条件変数クラス
 * スレッド間の同期を制御するための条件変数を提供します
 */
class CondVar {
 public:
  /**
   * @brief 新しい条件変数を作成
   * @return 成功時: 作成された条件変数、失敗時: エラー
   */
  static s6i_result::Result<CondVar, SyncError> make() {
    SDL_cond* cond = SDL_CreateCond();
    SDL_LogInfo(SDL_LOG_CATEGORY_SYSTEM, "Create condition variable.");
    if (!cond) {
      SDL_LogError(SDL_LOG_CATEGORY_SYSTEM,
                   "Failed to create condition variable: %s", SDL_GetError());
      return s6i_result::make_err(SyncError::CondVarCreationError);
    }
    return s6i_result::make_ok(CondVar(cond));
  }

  // コピー禁止
  CondVar(const CondVar&) = delete;
  CondVar& operator=(const CondVar&) = delete;

  // ムーブ可能
  CondVar(CondVar&& other) : m_cond(other.m_cond) { other.m_cond = nullptr; }

  CondVar& operator=(CondVar&& other) {
    CondVar(std::move(other)).swap(*this);
    return *this;
  }

  ~CondVar() {
    SDL_LogInfo(SDL_LOG_CATEGORY_SYSTEM, "Destroy condition variable.");
    SDL_DestroyCond(m_cond);
  }

  /**
   * @brief 条件変数で待機
   * @param guard ロック済みのMutexGuard
   * @return 成功時: void、失敗時: エラー
   */
  template <typename T>
  s6i_result::Result<std::monostate, SyncError> wait(MutexGuard<T>& guard) {
    if (!m_cond) {
      return s6i_result::make_err(SyncError::InvalidCondVarError);
    }
    if (SDL_CondWait(m_cond, guard.get_raw()) < 0) {
      SDL_LogError(SDL_LOG_CATEGORY_SYSTEM,
                   "Failed to wait on condition variable: %s", SDL_GetError());
      return s6i_result::make_err(SyncError::CondVarWaitError);
    }
    return s6i_result::make_ok(std::monostate{});
  }

  /**
   * @brief 待機中のスレッドの1つを起こす
   * @param guard ロック済みのMutexGuard
   * (データの競合を防ぐため、Mutexをロックした状態で呼び出す必要があります)
   * @return 成功時: void、失敗時: エラー
   */
  template <typename T>
  s6i_result::Result<std::monostate, SyncError> signal(
      [[maybe_unused]] MutexGuard<T>& guard) {
    if (!m_cond) {
      return s6i_result::make_err(SyncError::InvalidCondVarError);
    }
    if (SDL_CondSignal(m_cond) < 0) {
      SDL_LogError(SDL_LOG_CATEGORY_SYSTEM,
                   "Failed to signal condition variable: %s", SDL_GetError());
      return s6i_result::make_err(SyncError::CondVarSignalError);
    }
    return s6i_result::make_ok(std::monostate{});
  }

  /**
   * @brief 待機中のすべてのスレッドを起こす
   * @param guard ロック済みのMutexGuard
   * (データの競合を防ぐため、Mutexをロックした状態で呼び出す必要があります)
   * @return 成功時: void、失敗時: エラー
   */
  template <typename T>
  s6i_result::Result<std::monostate, SyncError> broadcast(
      [[maybe_unused]] MutexGuard<T>& guard) {
    if (!m_cond) {
      return s6i_result::make_err(SyncError::InvalidCondVarError);
    }
    if (SDL_CondBroadcast(m_cond) < 0) {
      SDL_LogError(SDL_LOG_CATEGORY_SYSTEM,
                   "Failed to broadcast condition variable: %s",
                   SDL_GetError());
      return s6i_result::make_err(SyncError::CondVarBroadcastError);
    }
    return s6i_result::make_ok(std::monostate{});
  }

  void swap(CondVar& other) {
    using std::swap;
    swap(m_cond, other.m_cond);
  }

 private:
  explicit CondVar(SDL_cond* cond) : m_cond(cond) { assert(cond); }

  SDL_cond* m_cond = nullptr;
};

inline void swap(CondVar& lhs, CondVar& rhs) {
  lhs.swap(rhs);
}

}  // namespace s6i_sync
