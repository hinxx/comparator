#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
// htobe64
#include <endian.h>


// use 16 bit aligned size
#define KEY_SIZE        32
#define MAX_KEY_SIZE    128

struct Slice
{
    Slice(const char* d, size_t n) : size_(n) { memcpy(data_, d, n); data_[size_] = 0; }
    const char* data() const { return data_; }
    size_t size() const { return size_; }

    char data_[MAX_KEY_SIZE];
    size_t size_;
};

void hexdump(const void *ptr, int buflen) {
  unsigned char *buf = (unsigned char*)ptr;
  int i, j;
  for (i=0; i<buflen; i+=32) {
    printf("%06x: ", i);
    for (j=0; j<32; j++)
      if (i+j < buflen)
        printf("%02x ", unsigned(buf[i+j]));
      else
        printf("   ");
    printf(" ");
    for (j=0; j<32; j++)
      if (i+j < buflen)
        printf("%c", isprint(buf[i+j]) ? buf[i+j] : '.');
    printf("\n");
  }
}

#if 0
int comparator0(const Slice &a, const Slice &b) {

    printf("'%s' ?? '%s' ::: ", a.data(), b.data());

    uint64_t ts1, ts2;
    // char *pv1, *pv2;
    memcpy(&ts1, a.data(), sizeof(uint64_t));
    memcpy(&ts2, b.data(), sizeof(uint64_t));
    const char *pv1 = a.data() + sizeof(uint64_t);
    const char *pv2 = b.data() + sizeof(uint64_t);
    if (ts1 < ts2) {
        printf("ts1 < ts2: -1\n");
        return -1;
    }
    if (ts1 > ts2) {
        printf("ts1 > ts2: +1\n");
        return +1;
    }

    // a or b key is only timestamp (no pv name)
    if (sizeof(uint64_t) == a.size()) {
        printf("sizeof(uint64_t) == a.size(): -1\n");
        return -1;
    }
    if (sizeof(uint64_t) == b.size()) {
        printf("sizeof(uint64_t) == b.size(): +1\n");
        return +1;
    }
    if ((sizeof(uint64_t) == a.size()) && (sizeof(uint64_t) == b.size())) {
        printf("(sizeof(uint64_t) == a.size()) && (sizeof(uint64_t) == b.size()): 0\n");
        return 0;
    }

    // at this point ts1 == ts2 and we have pv part in both keys
    int len = ((a.size() - sizeof(uint64_t)) < (a.size() - sizeof(uint64_t))) ? (a.size() - sizeof(uint64_t)) : (b.size() - sizeof(uint64_t));
    int ret = memcmp(pv1, pv2, len);
    if (ret < 0) {
        printf("pv1 < pv2: -1\n");
        return -1;
    }
    if (ret > 0) {
        printf("pv1 > pv2: +1\n");
        return +1;
    }

    printf("ts1 == ts2 && pv1 == pv2: 0\n");
    return 0;
}


int comparator1(const Slice &a, const Slice &b) {
    printf("---------------------------------------------------------------------------------\n");
    printf("A [%3ld]: ", a.size());
    hexdump((const void *)a.data(), a.size());
    printf("B [%3ld]: ", b.size());
    hexdump((const void *)b.data(), b.size());

    // int len = (a.size() < b.size()) ? a.size() : b.size();
    // int ret = memcmp(a.data(), b.data(), len);
    // compare the timestamps only first
    int ret = memcmp(a.data(), b.data(), sizeof(uint64_t));
    if (ret < 0) {
        printf("TS A < TS B: -1\n");
        return -1;
    }
    if (ret > 0) {
        printf("TS A > TS B: +1\n");
        return +1;
    }
    // timestamps are the same
    printf("TS A == TS B, ");

    // a or b key is only timestamp (no pv name)
    if ((sizeof(uint64_t) == a.size()) && (sizeof(uint64_t) == b.size())) {
        printf("no PV A && no PV B: 0\n");
        return 0;
    }
    if (sizeof(uint64_t) == a.size()) {
        printf("no PV A: -1\n");
        return -1;
    }
    if (sizeof(uint64_t) == b.size()) {
        printf("no PV B: +1\n");
        return +1;
    }

    // compare the pv names if any
    int len = ((a.size() - sizeof(uint64_t)) < (b.size() - sizeof(uint64_t)))
               ? (a.size() - sizeof(uint64_t)) : (b.size() - sizeof(uint64_t));
    ret = memcmp(a.data() + sizeof(uint64_t), b.data() + sizeof(uint64_t), len);
    if (ret < 0) {
        printf("PV A < PV B: -1\n");
        return -1;
    }
    if (ret > 0) {
        printf("PV A > PV B: +1\n");
        return +1;
    }

    printf("PV A == PV B: 0\n");
    return 0;
}

// NOTE: this comparator does not sort these properly:
        // Slice a = make_slice(123456, "aaaa", 4);
        // Slice b = make_slice(123456, "bbb", 3);

        // Slice a = make_slice(123456, "key1000", 7);
        // Slice b = make_slice(123456, "key999", 6);

int comparator2(const Slice &a, const Slice &b) {
    printf("---------------------------------------------------------------------------------\n");
    char abuf[MAX_KEY_SIZE] = {0};
    char bbuf[MAX_KEY_SIZE] = {0};
    sprintf(abuf, "'%8ld%s'", htobe64(*(uint64_t *)a.data()), a.data() + sizeof(uint64_t));
    sprintf(bbuf, "'%8ld%s'", htobe64(*(uint64_t *)b.data()), b.data() + sizeof(uint64_t));
    printf("A [%3ld]: ", a.size());
    hexdump((const void *)a.data(), a.size());
    printf("B [%3ld]: ", b.size());
    hexdump((const void *)b.data(), b.size());

    assert(a.size() == b.size());

    int ret = memcmp(a.data(), b.data(), a.size());
    if (ret < 0) {
        printf("%s < %s: -1\n", abuf, bbuf);
        return -1;
    }
    if (ret > 0) {
        printf("%s > %s: +1\n", abuf, bbuf);
        return +1;
    }
    // timestamps are the same
    printf("%s == %s: 0\n", abuf, bbuf);

    return 0;
}

#endif


/*
util/comparator.cc

namespace {
class BytewiseComparatorImpl : public Comparator {
 public:
  BytewiseComparatorImpl() {}
  static const char* kClassName() { return "leveldb.BytewiseComparator"; }
  const char* Name() const override { return kClassName(); }

  int Compare(const Slice& a, const Slice& b) const override {
    return a.compare(b);
  }
...

include/rocksdb/slice.h

inline int Slice::compare(const Slice& b) const {
  assert(data_ != nullptr && b.data_ != nullptr);
  const size_t min_len = (size_ < b.size_) ? size_ : b.size_;
  int r = memcmp(data_, b.data_, min_len);
  if (r == 0) {
    if (size_ < b.size_)
      r = -1;
    else if (size_ > b.size_)
      r = +1;
  }
  return r;
}

*/
int comparator(const Slice &a, const Slice &b) {
    printf("---------------------------------------------------------------------------------\n");
    char abuf[MAX_KEY_SIZE] = {0};
    char bbuf[MAX_KEY_SIZE] = {0};
    size_t ts_sz = sizeof(uint64_t);

    sprintf(abuf, "'%8ld%s'", htobe64(*(uint64_t *)a.data()), a.data() + ts_sz);
    sprintf(bbuf, "'%8ld%s'", htobe64(*(uint64_t *)b.data()), b.data() + ts_sz);

    printf("A [%3ld]: ", a.size());
    hexdump((const void *)a.data(), a.size());
    printf("B [%3ld]: ", b.size());
    hexdump((const void *)b.data(), b.size());

    // assert(a.size() == b.size());
    const size_t min_len = (a.size() < b.size()) ? a.size() : b.size();

    int ret = memcmp(a.data(), b.data(), min_len);
    if (ret == 0) {
        // bytes up to min_len are the same, check sizes
        if (a.size() < b.size()) {
            printf("%s < %s: -1 (len)\n", abuf, bbuf);
            return -1;
        } else if (a.size() > b.size()) {
            printf("%s > %s: +1 (len)\n", abuf, bbuf);
            return +1;
        }
        // fall through for equal keys
    } else {
        // bytes up to min_len differ
        if (ret < 0) {
            printf("%s < %s: -1 (cmp)\n", abuf, bbuf);
            return -1;
        }
        if (ret > 0) {
            printf("%s > %s: +1 (cmp)\n", abuf, bbuf);
            return +1;
        }
        // should not get here!
        assert(1 == 0);
    }

    printf("%s == %s: 0 (cmp)\n", abuf, bbuf);
    return 0;
}


// inline void EncodeFixed64(char* buf, uint64_t value) {
// //   if (port::kLittleEndian) {
// //     memcpy(buf, &value, sizeof(value));
// //   } else {
//     buf[7] = value & 0xff;
//     buf[6] = (value >> 8) & 0xff;
//     buf[5] = (value >> 16) & 0xff;
//     buf[4] = (value >> 24) & 0xff;
//     buf[3] = (value >> 32) & 0xff;
//     buf[2] = (value >> 40) & 0xff;
//     buf[1] = (value >> 48) & 0xff;
//     buf[0] = (value >> 56) & 0xff;
// //   }
// }

Slice make_slice(uint64_t value, const char *pv, size_t pv_len) {
    char buf[MAX_KEY_SIZE] = {0};
    size_t n = 0;
    // memcpy(buf, &value, sizeof(uint64_t));
    // EncodeFixed64(buf, value);
    
    // convert the value to big endian so that it can be compared with memcmp()
    *(uint64_t *)buf = htobe64(value);
    n += sizeof(uint64_t);
    if (pv_len > 0) {
        memcpy(buf+n, ":", 1);
        n += 1;
        memcpy(buf+n, pv, pv_len);
        n += pv_len;
    }

    // create a fixed len key size, with trailing \0; could be faster to compare
    // with memcmp(), but then we do not know the actual length of key and that
    // affects ordering of some key values. We could run strlen() on the key, but
    // that would degrade the performance, likely..
    // use comparator2() with these slices.
    // return Slice(buf, KEY_SIZE);
    
    // create arbitrary long key
    return Slice(buf, n);
}

int main(int argc, char const *argv[]) {

    //
    // Human vs. ASCII order
    //
    // consider these resources:
    // https://blog.codinghorror.com/sorting-for-humans-natural-sort-order/
    // https://github.com/sourcefrog/natsort
    //
     
    {
        // A == B
        Slice a = make_slice(123456, "key0", 4);
        Slice b = make_slice(123456, "key0", 4);
        int ret = comparator(a, b);
        assert(ret == 0);
    }

    {
        // A < B
        Slice a = make_slice(444, "key1", 4);
        Slice b = make_slice(444, "key2", 4);
        int ret = comparator(a, b);
        assert(ret == -1);
    }

    {
        // A > B
        Slice a = make_slice(555, "key4", 4);
        Slice b = make_slice(555, "key3", 4);
        int ret = comparator(a, b);
        assert(ret == +1);
    }

    {
        // A == B
        Slice a = make_slice(4321, "", 0);
        Slice b = make_slice(4321, "", 0);
        int ret = comparator(a, b);
        assert(ret == 0);
    }

    {
        // A < B
        Slice a = make_slice(666, "", 0);
        Slice b = make_slice(666, "key5", 4);
        int ret = comparator(a, b);
        assert(ret == -1);
    }

    {
        // A > B
        Slice a = make_slice(765, "key6", 4);
        Slice b = make_slice(765, "", 0);
        int ret = comparator(a, b);
        assert(ret == +1);
    }

    {
        // A > B
        Slice a = make_slice(987654, "", 0);
        Slice b = make_slice(123456, "", 0);
        int ret = comparator(a, b);
        assert(ret == +1);
    }

    {
        // A > B
        Slice a = make_slice(9876, "key8", 4);
        Slice b = make_slice(1234, "", 0);
        int ret = comparator(a, b);
        assert(ret == +1);
    }

    {
        // A > B
        Slice a = make_slice(987, "", 0);
        Slice b = make_slice(123, "key9", 4);
        int ret = comparator(a, b);
        assert(ret == +1);
    }

    {
        // A < B
        Slice a = make_slice(999, "key01", 5);
        Slice b = make_slice(999, "key9", 4);
        int ret = comparator(a, b);
        assert(ret == -1);
    }

    {
        // A > B
        Slice a = make_slice(111, "key01", 5);
        Slice b = make_slice(111, "key00", 5);
        int ret = comparator(a, b);
        assert(ret == +1);
    }

    {
        // A > B
        Slice a = make_slice(22, "key1000", 7);
        Slice b = make_slice(22, "key0999", 7);
        int ret = comparator(a, b);
        assert(ret == +1);
    }

    {
        // A > B
        Slice a = make_slice(33, "key1000", 7);
        Slice b = make_slice(33, "key999", 6);
        int ret = comparator(a, b);
        assert(ret == -1);
    }

    {
        // A < B
        Slice a = make_slice(45, "key100", 6);
        Slice b = make_slice(45, "key999", 6);
        int ret = comparator(a, b);
        assert(ret == -1);
    }

    {
        // A < B
        Slice a = make_slice(88, "key10", 5);
        Slice b = make_slice(88, "key999", 6);
        int ret = comparator(a, b);
        assert(ret == -1);
    }

    {
        // A < B
        Slice a = make_slice(77, "aey", 3);
        Slice b = make_slice(77, "bey", 3);
        int ret = comparator(a, b);
        assert(ret == -1);
    }

    {
        // A > B
        Slice a = make_slice(66, "aaaa", 4);
        Slice b = make_slice(66, "bbb", 3);
        int ret = comparator(a, b);
        assert(ret == -1);
    }

    {
        // A > B
        Slice a = make_slice(10, "a1", 2);
        Slice b = make_slice(10, "b2", 2);
        int ret = comparator(a, b);
        assert(ret == -1);
    }

    {
        // A > B
        Slice a = make_slice(10, "d1", 2);
        Slice b = make_slice(10, "z9", 2);
        int ret = comparator(a, b);
        assert(ret == -1);
    }

    {
        // A > B
        Slice a = make_slice(10, "z9", 2);
        Slice b = make_slice(11, "a1", 2);
        int ret = comparator(a, b);
        assert(ret == -1);
    }


    return 0;
}
