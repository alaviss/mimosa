#include <iostream>
#include <cmath>

#include "decoder.hh"

namespace mimosa
{
  namespace json
  {
    Decoder::Decoder(stream::Stream::Ptr input)
      : input_(input),
        has_c_(false)
    {
    }

    Decoder::Token
    Decoder::pullNull()
    {
      int  ret;
      char buffer[3];

      ret = input_->loopRead(buffer, sizeof (buffer));
      if (ret != sizeof (buffer))
        throw ReadError();
      if (buffer[0] != 'u' || buffer[1] != 'l' || buffer[2] != 'l')
        throw SyntaxError();
      return kNull;
    }

    Decoder::Token
    Decoder::pullTrue()
    {
      int  ret;
      char buffer[3];

      ret = input_->loopRead(buffer, sizeof (buffer));
      if (ret != sizeof (buffer))
        throw ReadError();
      if (buffer[0] != 'r' || buffer[1] != 'u' || buffer[2] != 'e')
        throw SyntaxError();
      boolean_ = true;
      return kBoolean;
    }

    Decoder::Token
    Decoder::pullFalse()
    {
      int  ret;
      char buffer[4];

      ret = input_->loopRead(buffer, sizeof (buffer));
      if (ret != sizeof (buffer))
        throw ReadError();
      if (buffer[0] != 'a' || buffer[1] != 'l' || buffer[2] != 's' || buffer[3] != 'e')
        throw SyntaxError();
      boolean_ = false;
      return kBoolean;
    }

    Decoder::Token
    Decoder::pullString()
    {
      char c;

      string_.clear();
      while (input_->loopRead(&c, sizeof (c)) == 1) {
        if (c == '"')
          return kString;
        if (c != '\\') {
          string_.append(1, c);
          continue;
        }

        if (input_->loopRead(&c, sizeof (c)) != 1)
          throw ReadError();
        switch (c) {
        case '"': string_.append(1, '"'); break;
        case '\\': string_.append(1, '\\'); break;
        case '/': string_.append(1, '/'); break;
        case 'b': string_.append(1, '\b'); break;
        case 'f': string_.append(1, '\f'); break;
        case 'n': string_.append(1, '\n'); break;
        case 'r': string_.append(1, '\r'); break;
        case 't': string_.append(1, '\t'); break;
        case 'u':
          char u[4];
          if (input_->loopRead(u, sizeof (u)) != 4)
            throw ReadError();
          // XXX
          break;

        default:
          throw SyntaxError();
        }
      }
      throw ReadError();
    }

    Decoder::Token
    Decoder::pullRationalExp()
    {
      int sign = 1;
      int64_t exp = 0;
      char c;
      bool got_number = false;

      while (true) {
        if (!getc(&c)) {
          if (!got_number)
            throw SyntaxError();
          goto ret;
        }

        switch (c) {
        case '-':
          if (sign == -1 || got_number)
            throw SyntaxError();
          sign = -1;
          break;

        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
          got_number = true;
          exp = 10 * exp + c - '0';
          break;

        default:
          ungetc(c);
          goto ret;
        }
      }

      ret:
      rational_ *= pow(10, sign * exp);
      return kRational;
    }

    Decoder::Token
    Decoder::pullRationalDot(int sign)
    {
      char c;
      bool got_number = false;
      double div = 10;

      while (true) {
        if (!getc(&c)) {
          if (!got_number)
            throw SyntaxError();
          goto ret;
        }

        switch (c) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
          got_number = true;
          rational_ += (((double)(c - '0')) / div);
          div *= 10;
          break;

        case 'E':
        case 'e':
          if (!got_number)
            throw SyntaxError();
          rational_ *= sign;
          return pullRationalExp();

        default:
          ungetc(c);
          goto ret;
        }
      }

      ret:
      rational_ *= sign;
      return kRational;
    }

    Decoder::Token
    Decoder::pullNumber()
    {
      int sign = 1;
      char c;
      bool got_number = false;

      integer_ = 0;

      while (true) {
        if (!getc(&c)) {
          if (!got_number)
            throw SyntaxError();
          goto ret;
        }

        switch (c) {
        case '-':
          if (sign == -1 || got_number)
            throw SyntaxError();
          sign = -1;
          break;

        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
          got_number = true;
          integer_ = 10 * integer_ + c - '0';
          break;

        case '.':
          if (!got_number)
            throw SyntaxError();
          rational_ = integer_;
          return pullRationalDot(sign);

        case 'E':
        case 'e':
          if (!got_number)
            throw SyntaxError();
          rational_ = integer_ * sign;
          return pullRationalExp();

        default:
          ungetc(c);
          goto ret;
        }
      }

      ret:
      integer_ *= sign;
      return kInteger;
    }

    Decoder::Token
    Decoder::pullValue()
    {
      char c;

      if (!getcnows(&c)) {
        if (!state_.empty())
          throw SyntaxError();
        return kEof;
      }

      switch (c) {
      case '[':
        state_.push_back(kArray);
        return kArrayBegin;

      case '{':
        state_.push_back(kObjectKey);
        return kObjectBegin;

      case '"':
        return pullString();

      case 'n':
        return pullNull();

      case 't':
        return pullTrue();

      case 'f':
        return pullFalse();

      case '-':
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        ungetc(c);
        return pullNumber();

      default:
        throw SyntaxError();
      }
    }

    Decoder::Token
    Decoder::pullValueOrArrayEnd()
    {
      char c;

      if (!getcnows(&c))
        throw SyntaxError();

      switch (c) {
      case ']':
        return kArrayEnd;

      default:
        ungetc(c);
        return pullValue();
      }
    }

    Decoder::Token
    Decoder::pullCommaOrArrayEnd()
    {
      char c;

      if (!getcnows(&c))
        throw SyntaxError();

      switch (c) {
      case ']':
        return kArrayEnd;

      case ',':
        return pullValue();

      default:
        throw SyntaxError();
      }
    }

    Decoder::Token
    Decoder::pullObjectKey()
    {
      char c;

      if (!getcnows(&c))
        throw SyntaxError();

      switch (c) {
      case '"':
        return pullString();

      case '}':
        return kObjectEnd;

      default:
        throw SyntaxError();
      }
    }

    Decoder::Token
    Decoder::pullObjectValue()
    {
      char c;

      if (!getcnows(&c) || c != ',')
        throw SyntaxError();

      return pullValue();
    }

    Decoder::Token
    Decoder::pullObjectNext()
    {
      char c;

      if (!getcnows(&c))
        throw SyntaxError();

      switch (c) {
      case ',':
        return kObjectBegin;

      case '}':
        return kObjectEnd;

      default:
        throw SyntaxError();
      }
    }

    Decoder::Token
    Decoder::pull()
    {
      Decoder::Token token;

      if (state_.empty())
        return pullValue();

      auto & state = state_.back();
      switch (state) {
      case kArray:
        token = pullValueOrArrayEnd();
        if (token == kArrayEnd)
          state_.pop_back();
        else
          state = kArrayNext;
        return token;

      case kArrayNext:
        token = pullCommaOrArrayEnd();
        if (token == kArrayEnd)
          state_.pop_back();
        return token;

      case kObjectKey:
        token = pullObjectKey();
        if (token == kObjectEnd)
          state_.pop_back();
        else
          state = kObjectValue;
        return token;

      case kObjectValue:
        return pullObjectValue();

      case kObjectNext:
        token = pullObjectNext();
        if (token == kObjectEnd)
          state_.pop_back();
        else
          state = kObjectValue;
        return token;

      default:
        assert(false);
        return kEof;
      }
    }

    bool
    Decoder::getc(char * c)
    {
      if (has_c_) {
        *c = c_;
        has_c_ = false;
        return true;
      }
      int ret = input_->loopRead(c, 1);
      if (ret < 0)
        throw ReadError();
      return ret == 1;
    }

    bool
    Decoder::getcnows(char * c)
    {
      while (true) {
        if (!getc(c))
          return false;

        if (!::isspace(*c))
          return true;
      }
    }

    void
    Decoder::ungetc(char c)
    {
      assert(!has_c_);
      c_     = c;
      has_c_ = true;
    }
  }
}
