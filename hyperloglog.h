#include <cmath>
#include <cstdint>
#include <vector>

class HyperLogLog {
  public:
    // 4 <= b <= 16
    explicit HyperLogLog(int b) : b_(b), m_(1 << b), ms_(1 << b) {}

    void update(uint32_t value) {
        int j = value & ((1 << b_) - 1);
        uint32_t zeros = nlz5(value >> b_) - b_ + 1;
        ms_[j] = std::max(ms_[j], zeros);
    }

    double estimate() const {
        double e = raw_estimate();

        if (e <= 2.5*m_) {
            int zeros = num_zeros();
            if (zeros != 0) {
                e = m_*log((double)m_/zeros);
            }
        } else if (e > pow(2, 32)/30.0) {
            e = log(1 - e/pow(2, 32))*pow(2, 32)*-1;
        }
        return e;
    }

    double raw_estimate() const {
        double e = 0;

        for (auto m : ms_) {
            e += 1.0/(1 << m);
        }
        e = 1.0/e;
        e *= alpha()*m_*m_;

        return e;
    }

    double alpha() const {
        switch (m_) {
            case 16:
                return 0.673;
            case 32:
                return 0.697;
            case 64:
                return 0.709;
            default:
                return 0.7213/(1 + 1.079/m_);
        }
    }

    int num_zeros() const {
        int count = 0;
        for (auto m : ms_) {
            if (m == 0)
                count++;
        }
        return count;
    }

    int b() const { return b_; }

  private:
    // These from the book "Hacker's Delight"
    uint32_t pop(uint32_t x) {
        x = x - ((x >> 1) & 0x55555555);
        x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
        x = (x + (x >> 4)) & 0x0F0F0F0F;
        x = x + (x << 8);
        x = x + (x << 16);
        return x >> 24;
    }

    uint32_t nlz5(uint32_t x) {
        x = x | (x >> 1);
        x = x | (x >> 2);
        x = x | (x >> 4);
        x = x | (x >> 8);
        x = x | (x >>16);
        return pop(~x);
    }

    std::vector<uint32_t> ms_;
    int b_;
    int m_;
};
