#pragma once

#include <SDL.h>
#include <s6i_result/result.h>
#include <utility>
#include "error.h"

namespace s6i_sync {

template <typename T>
class MutexGuard;

/**
 * @brief スレッドセーフな値を保持するMutexクラス
 * @tparam T 保護する値の型
 */
template <typename T>
class Mutex {
 public:
  /**
   * @brief 新しいMutexを作成
   * @param args Tのコンストラクタに渡す引数
   * @return 成功時: 作成されたMutex、失敗時: エラー
   */
  template <typename... Args>
  static s6i_result::Result<Mutex<T>, SyncError> make(Args&&... args) {
    SDL_mutex* mutex = SDL_CreateMutex();
    if (!mutex) {
      return s6i_result::make_err(SyncError::MutexCreationError);
    }
    return s6i_result::make_ok(Mutex(mutex, std::forward<Args>(args)...));
  }

  // コピー禁止
  Mutex(const Mutex&) = delete;
  Mutex& operator=(const Mutex&) = delete;

  // ムーブ可能
  Mutex(Mutex&& other)
      : m_mutex(other.m_mutex), m_value(std::move(other.m_value)) {
    other.m_mutex = nullptr;
  }

  Mutex& operator=(Mutex&& other) {
    Mutex(std::move(other)).swap(*this);
    return *this;
  }

  ~Mutex() { SDL_DestroyMutex(m_mutex); }

  /**
   * @brief Mutexをロックし、保護された値へのアクセスを提供
   * @return 成功時: MutexGuard、失敗時: エラー
   */
  s6i_result::Result<MutexGuard<T>, SyncError> lock() {
    if (!m_mutex) {
      return s6i_result::make_err(SyncError::InvalidMutexError);
    }
    if (SDL_LockMutex(m_mutex) < 0) {
      return s6i_result::make_err(SyncError::MutexLockError);
    }
    return s6i_result::make_ok(MutexGuard<T>(m_mutex, m_value));
  }

  void swap(Mutex& other) {
    using std::swap;
    swap(m_mutex, other.m_mutex);
    swap(m_value, other.m_value);
  }

 private:
  template <typename... Args>
  Mutex(SDL_mutex* mutex, Args&&... args)
      : m_mutex(mutex), m_value(std::forward<Args>(args)...) {}

  SDL_mutex* m_mutex = nullptr;
  T m_value;

  friend class MutexGuard<T>;
};

template <typename T>
inline void swap(Mutex<T>& lhs, Mutex<T>& rhs) {
  lhs.swap(rhs);
}

/**
 * @brief RAIIでMutexのロック/アンロックを管理するガードクラス
 * @tparam T 保護する値の型
 */
template <typename T>
class MutexGuard {
 public:
  MutexGuard(MutexGuard&& other)
      : m_mutex(other.m_mutex), m_value(other.m_value) {
    other.m_mutex = nullptr;
    other.m_value = nullptr;
  }

  ~MutexGuard() { SDL_UnlockMutex(m_mutex); }

  // コピー禁止
  MutexGuard(const MutexGuard&) = delete;
  MutexGuard& operator=(const MutexGuard&) = delete;

  MutexGuard& operator=(MutexGuard&& rhs) {
    MutexGuard(std::move(rhs)).swap(*this);
    return *this;
  }

  T& operator*() { return *m_value; }
  T* operator->() { return m_value; }
  const T& operator*() const { return *m_value; }
  const T* operator->() const { return m_value; }

  void swap(MutexGuard& other) {
    using std::swap;
    swap(m_mutex, other.m_mutex);
    swap(m_value, other.m_value);
  }

 private:
  MutexGuard(SDL_mutex* mutex, T& value) : m_mutex(mutex), m_value(&value) {}

  SDL_mutex* m_mutex = nullptr;
  T* m_value = nullptr;

  friend class Mutex<T>;
};

template <typename T>
inline void swap(MutexGuard<T>& lhs, MutexGuard<T>& rhs) {
  lhs.swap(rhs);
}

}  // namespace s6i_sync
