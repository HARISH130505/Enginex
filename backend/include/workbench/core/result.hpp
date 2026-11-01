#pragma once

#include <string>
#include <variant>
#include <utility>
#include <optional>

namespace enginex::core {

enum class StatusCode {
    Ok = 0,
    InvalidArgument,
    NotFound,
    AlreadyExists,
    Timeout,
    Cancelled,
    InternalError,
    SimulatedFailure,
    ResourceExhausted,
    Unavailable
};

struct Error {
    StatusCode code;
    std::string message;

    Error(StatusCode c, std::string msg) : code(c), message(std::move(msg)) {}
};

template <typename T = void>
class Result {
public:
    Result(const T& val) : m_data(val) {}
    Result(T&& val) : m_data(std::move(val)) {}
    Result(Error err) : m_data(std::move(err)) {}

    bool is_ok() const noexcept {
        return std::holds_alternative<T>(m_data);
    }

    bool is_err() const noexcept {
        return std::holds_alternative<Error>(m_data);
    }

    explicit operator bool() const noexcept {
        return is_ok();
    }

    const T& value() const {
        return std::get<T>(m_data);
    }

    T& value() {
        return std::get<T>(m_data);
    }

    const Error& error() const {
        return std::get<Error>(m_data);
    }

    static Result<T> ok(T val) {
        return Result<T>(std::move(val));
    }

    static Result<T> fail(StatusCode code, std::string message) {
        return Result<T>(Error(code, std::move(message)));
    }

private:
    std::variant<T, Error> m_data;
};

// Specialization for void return type
template <>
class Result<void> {
public:
    Result() : m_error(std::nullopt) {}
    Result(Error err) : m_error(std::move(err)) {}

    bool is_ok() const noexcept {
        return !m_error.has_value();
    }

    bool is_err() const noexcept {
        return m_error.has_value();
    }

    explicit operator bool() const noexcept {
        return is_ok();
    }

    const Error& error() const {
        return m_error.value();
    }

    static Result<void> ok() {
        return Result<void>();
    }

    static Result<void> fail(StatusCode code, std::string message) {
        return Result<void>(Error(code, std::move(message)));
    }

private:
    std::optional<Error> m_error;
};

} // namespace enginex::core

