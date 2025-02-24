#pragma once

#include <cassert>
#include <utility>
#include <variant>

namespace s6i_result {

/**
 * @brief 成功値を表す型
 *
 * @tparam T 成功値の型
 */
template <typename T>
struct Ok {
  T m_value;
  explicit Ok(const T& v) : m_value(v) {}
  explicit Ok(T&& v) : m_value(std::move(v)) {}
};

/**
 * @brief 型TからOk<T>を生成するヘルパー関数
 * @tparam T 成功値の型
 * @param value 成功値
 * @return Ok<T> 生成されたOk型
 */
template <typename T>
inline Ok<T> make_ok(T&& value) {
  return Ok<T>(std::forward<T>(value));
}

/**
 * @brief 失敗値を表す型
 *
 * @tparam E 失敗値の型
 */
template <typename E>
struct Err {
  E m_value;
  explicit Err(const E& e) : m_value(e) {}
  explicit Err(E&& e) : m_value(std::move(e)) {}
};

/**
 * @brief 型EからErr<E>を生成するヘルパー関数
 * @tparam E 失敗値の型
 * @param error 失敗値
 * @return Err<E> 生成されたErr型
 */
template <typename E>
inline Err<E> make_err(E&& error) {
  return Err<E>(std::forward<E>(error));
}

/**
 * @brief Result型
 *
 * 成功値(T)もしくは失敗値(E)のいずれかを保持する型です。
 *
 * @tparam T 成功値の型
 * @tparam E 失敗値の型
 */
template <typename T, typename E>
class Result {
 private:
  std::variant<Ok<T>, Err<E>> m_value;

 public:
  /** @brief 成功値からResultを構築 */
  Result(const Ok<T>& ok) : m_value(ok) {}
  /** @brief 成功値からResultを構築（ムーブ） */
  Result(Ok<T>&& ok) : m_value(std::move(ok)) {}
  /** @brief 失敗値からResultを構築 */
  Result(const Err<E>& err) : m_value(err) {}
  /** @brief 失敗値からResultを構築（ムーブ） */
  Result(Err<E>&& err) : m_value(std::move(err)) {}

  /** @brief 成功値を保持しているかどうかを判定 */
  bool is_ok() const { return std::holds_alternative<Ok<T>>(m_value); }

  /** @brief 失敗値を保持しているかどうかを判定 */
  bool is_err() const { return std::holds_alternative<Err<E>>(m_value); }

  /**
   * @brief 成功値を取得（ムーブ）
   * @note 失敗値を保持している場合はassertで停止
   */
  T unwrap() {
    assert(is_ok() && "Called unwrap on an Err value");
    return std::move(std::get<Ok<T>>(m_value).m_value);
  }

  /**
   * @brief 成功値もしくはデフォルト値を取得（ムーブ）
   * @param default_value 失敗時に返す値
   */
  T unwrap_or(T&& default_value) {
    if (is_ok()) {
      return std::move(std::get<Ok<T>>(m_value).m_value);
    }
    return std::forward<T>(default_value);
  }

  /**
   * @brief 成功値への参照を取得
   * @note 失敗値を保持している場合はassertで停止
   */
  T& ref_ok() {
    assert(is_ok() && "Called ref_ok on an Err value");
    return std::get<Ok<T>>(m_value).m_value;
  }

  /**
   * @brief 成功値へのconst参照を取得
   * @note 失敗値を保持している場合はassertで停止
   */
  const T& ref_ok() const {
    assert(is_ok() && "Called ref_ok on an Err value");
    return std::get<Ok<T>>(m_value).m_value;
  }

  /**
   * @brief 失敗値を取得（ムーブ）
   * @note 成功値を保持している場合はassertで停止
   */
  E unwrap_err() {
    assert(is_err() && "Called unwrap_err on an Ok value");
    return std::move(std::get<Err<E>>(m_value).m_value);
  }

  /**
   * @brief 失敗値への参照を取得
   * @note 成功値を保持している場合はassertで停止
   */
  E& ref_err() {
    assert(is_err() && "Called ref_err on an Ok value");
    return std::get<Err<E>>(m_value).m_value;
  }

  /**
   * @brief 失敗値へのconst参照を取得
   * @note 成功値を保持している場合はassertで停止
   */
  const E& ref_err() const {
    assert(is_err() && "Called ref_err on an Ok value");
    return std::get<Err<E>>(m_value).m_value;
  }

  /**
   * @brief 成功値に関数を適用
   * @param f 適用する関数
   */
  template <typename F>
  auto map(F&& f) -> Result<decltype(f(std::declval<T>())), E> {
    Result self = std::move(*this);
    if (self.is_ok()) {
      return make_ok(f(std::get<Ok<T>>(self.m_value).m_value));
    }
    return Err<E>(std::get<Err<E>>(self.m_value).m_value);
  }

  /**
   * @brief 失敗値に関数を適用
   * @param f 適用する関数
   */
  template <typename F>
  auto map_err(F&& f) -> Result<T, decltype(f(std::declval<E>()))> {
    Result self = std::move(*this);
    if (self.is_err()) {
      return make_err(f(std::get<Err<E>>(self.m_value).m_value));
    }
    return Ok<T>(std::get<Ok<T>>(self.m_value).m_value);
  }

  /**
   * @brief 成功値に関数を適用
   * @param f 適用する関数
   */
  template <typename F>
  Result inspect_ok(F&& f) {
    Result self = std::move(*this);
    if (self.is_ok()) {
      f(std::get<Ok<T>>(self.m_value).m_value);
    }
    return self;
  }

  /**
   * @brief 失敗値に関数を適用
   * @param f 適用する関数
   */
  template <typename F>
  Result inspect_err(F&& f) {
    Result self = std::move(*this);
    if (self.is_err()) {
      f(std::get<Err<E>>(self.m_value).m_value);
    }
    return self;
  }

  /**
   * @brief 成功値に関数を適用し、新しいResultを生成
   * @param f Result<U, E>を返す関数
   */
  template <typename F>
  auto and_then(F&& f) -> decltype(f(std::declval<T>())) {
    Result self = std::move(*this);
    if (self.is_ok()) {
      return f(std::get<Ok<T>>(self.m_value).m_value);
    }
    return Err<E>(std::get<Err<E>>(self.m_value).m_value);
  }
};

}  // namespace s6i_result
