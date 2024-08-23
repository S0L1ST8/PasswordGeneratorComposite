#include <algorithm>
#include <array>
#include <iostream>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>

class PasswordGenerator {
  public:
    virtual std::string Generate() = 0;

    virtual std::string AllowedChars() const = 0;
    virtual size_t Length() const = 0;

    virtual void Add(std::unique_ptr<PasswordGenerator> generator) = 0;

    virtual ~PasswordGenerator() = default;
};

class BasicPasswordGenerator : public PasswordGenerator {
  public:
    explicit BasicPasswordGenerator(size_t len) noexcept : len_(len) {}

    size_t Length() const noexcept override {
        return len_;
    }

  private:
    size_t len_;

    std::string Generate() override {
        throw std::runtime_error("not implemented");
    }

    void Add(std::unique_ptr<PasswordGenerator>) override {
        throw std::runtime_error("not implemented");
    }
};

class DigitGenerator : public BasicPasswordGenerator {
  public:
    explicit DigitGenerator(size_t len) noexcept : BasicPasswordGenerator(len) {}

    std::string AllowedChars() const override {
        return "0123456789";
    }
};

class SymbolGenerator : public BasicPasswordGenerator {
  public:
    explicit SymbolGenerator(size_t len) noexcept : BasicPasswordGenerator(len) {}

    std::string AllowedChars() const override {
        return "!@#$%^&*()[]{}?<>";
    }
};

class UpperLetterGenerator : public BasicPasswordGenerator {
  public:
    explicit UpperLetterGenerator(size_t len) noexcept : BasicPasswordGenerator(len) {}

    std::string AllowedChars() const override {
        return "ABCDEFGHIJKLMNOPQRSTUVXYWZ";
    }
};

class LowerLetterGenerator : public BasicPasswordGenerator {
  public:
    explicit LowerLetterGenerator(size_t len) noexcept : BasicPasswordGenerator(len) {}

    std::string AllowedChars() const override {
        return "abcdefghijklmnopqrstuvxywz";
    }
};

class CompositePasswordGenerator : public PasswordGenerator {
  public:
    CompositePasswordGenerator() {
        auto seed_data = std::array<int, std::mt19937::state_size>{};
        std::generate(std::begin(seed_data), std::end(seed_data), std::ref(rd_));
        std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
        eng_.seed(seq);
    }

    std::string Generate() override {
        std::string password;
        for (auto& generator : generators_) {
            std::string chars = generator->AllowedChars();
            std::uniform_int_distribution<> ud(0, static_cast<int>(chars.length() - 1));

            for (size_t i = 0; i < generator->Length(); ++i) {
                password += chars[ud(eng_)];
            }
        }

        std::shuffle(std::begin(password), std::end(password), eng_);

        return password;
    }

    void Add(std::unique_ptr<PasswordGenerator> generator) override {
        generators_.push_back(std::move(generator));
    }

  private:
    std::mt19937 eng_;
    std::random_device rd_;
    std::vector<std::unique_ptr<PasswordGenerator>> generators_;

    size_t Length() const override {
        throw std::runtime_error("not implemented");
    }

    std::string AllowedChars() const override {
        throw std::runtime_error("not implemented");
    }
};

int main() {
    CompositePasswordGenerator generator;
    generator.Add(std::make_unique<SymbolGenerator>(2));
    generator.Add(std::make_unique<DigitGenerator>(2));
    generator.Add(std::make_unique<UpperLetterGenerator>(2));
    generator.Add(std::make_unique<LowerLetterGenerator>(4));

    auto password = generator.Generate();
    std::cout << password << std::endl;
}
