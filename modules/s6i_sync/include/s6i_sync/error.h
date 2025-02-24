#pragma once

namespace s6i_sync {

/**
 * @brief 同期処理に関するエラー型
 */
enum class SyncError {
  // Mutex関連エラー
  MutexCreationError,  ///< Mutexの作成に失敗
  MutexLockError,      ///< Mutexのロックに失敗
  InvalidMutexError,   ///< 無効なMutexへの操作

  // Thread関連エラー
  ThreadCreationError,  ///< スレッドの作成に失敗
  ThreadJoinError,      ///< スレッドのjoinに失敗
  InvalidThreadError,   ///< 無効なスレッドへの操作

  // CondVar関連エラー
  CondVarCreationError,  ///< 条件変数の作成に失敗
  CondVarWaitError,      ///< 条件変数のwaitに失敗
  CondVarSignalError,    ///< 条件変数のsignal/broadcastに失敗
};

}  // namespace s6i_sync
