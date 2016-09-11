#pragma once

#include "egolib/Script/Traits.hpp"
#include "egolib/Script/Buffer.hpp"
#include "egolib/Script/TextInputFile.hpp"

namespace Ego {
namespace Script {

template <typename Traits = Traits<char>>
struct AbstractReader {
public:
    /**
     * @brief The line number.
     */
    size_t _lineNumber;

    /**
     * @brief The lexeme accumulation buffer.
     */
    Buffer _buffer;

    /**
     * @brief The text file or @a nullptr.
     */
    std::shared_ptr<TextInputFile<Traits>> _source;

    /**
     * @brief Construct this reader.
     * @param initialBufferCapacity the initial capacity of the lexeme accumulation buffer
     */
    AbstractReader(size_t initialBufferCapacity) :
        _source(nullptr),
        _buffer(initialBufferCapacity),
        _lineNumber(1) {}

    /**
     * @brief Destruct this reader.
     */
    virtual ~AbstractReader() {
        _source = nullptr;
    }

public:
    /**
     * @brief Get the current line number.
     * @return the current line number
     */
    size_t getLineNumber() const {
        return _lineNumber;
    }

    /**
     * @brief Get the current extended character.
     * @return the current extended character
     */
    typename Traits::ExtendedType current() const {
        return _source->get();
    }

public:
    /**
     * @brief Advance to the next extended character.
     */
    void next() {
        _source->advance();
    }

    /**
     * @brief Write the specified extended character.
     * @param echr the extended character
     */
    inline void write(const typename Traits::ExtendedType& echr) {
        assert(Traits::isValid(echr));
        _buffer.append(static_cast<typename Traits::Type>(echr));
    }

    /**
     * @brief Write the specified extended character and advance to the next extended character.
     * @param echr the extended character
     */
    inline void writeAndNext(const typename Traits::ExtendedType& echr) {
        write(echr);
        next();
    }

    /**
     * @brief Save the current extended character.
     */
    inline void save() {
        write(current());
    }

    /**
     * @brief Save the current extended character and advance to the next extended character.
     */
    inline void saveAndNext() {
        save();
        next();
    }

    /**
     * @brief Convert the contents of the lexeme accumulation buffer to a string value.
     * @return the string value
     */
    std::string toString() const {
        return _buffer.toString();
    }

public:
    /**
     * @brief Get if the current extended character equals another extended character.
     * @param other the other extended character
     * @return @a true if the current extended character equals the other extended character, @a false otherwise
     */
    inline bool is(const typename Traits::ExtendedType& echr) const {
        return echr == current();
    }

    /**
     * @brief Get if the current extended character is a whitespace character.
     * @return @a true if the current extended character is a whitespace character, @a false otherwise
     */
    inline bool isWhiteSpace() const {
        return Traits::isWhiteSpace(current());
    }

    /**
     * @brief Get if the current extended character is a new line character.
     * @return @a true if the current extended character is a new line character, @a false otherwise
     */
    inline bool isNewLine() const {
        return Traits::isNewLine(current());
    }

    /**
     * @brief Get if the current extended character is an alphabetic character.
     * @return @a true if the current extended character is an alphabetic character, @a false otherwise
     */
    inline bool isAlpha() const {
        return Traits::isAlphabetic(current());
    }

    /**
     * @brief Get if the current extended character is a digit character.
     * @return @a true if the current extended character is a digit character, @a false otherwise
     */
    inline bool isDigit() const {
        return Traits::isDigit(current());
    }

public:
    /**
     * @brief Skip zero or one newline sequences.
     * @remark Proper line counting is performed.
     */
    void skipNewLine() {
        if (isNewLine()) {
            typename Traits::ExtendedType old = current();
            next();
            if (isNewLine() && old != current()) {
                next();
            }
            _lineNumber++;
        }
    }

    /**
     * @brief Skip zero or more newline sequences.
     * @remark Proper line counting is performed.
     */
    void skipNewLines() {
        while (isNewLine()) {
            typename Traits::ExtendedType old = current();
            next();
            if (isNewLine() && old != current()) {
                next();
            }
            _lineNumber++;
        }
    }

};

} // namespace Script
} // namespace Ego
