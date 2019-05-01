#include <stdio.h>
#include <string.h>
#include <sodium.h>
#include <vector>
class G1;
class ZR;

class G1
{
    public:
    unsigned char * point;
    G1();
    ~G1();
    G1(const G1& other);
    void rand(void);
    void assign_sum(unsigned char *a, unsigned char *b);
    void assign_diff(unsigned char *a, unsigned char *b);
    void assign_prod(unsigned char *a, unsigned char *b);
    void print(void);
    G1 operator + (G1 &other);
    G1 operator - (G1 &other);
    G1 operator * (ZR &other);
    G1& operator = (const G1 &other);
    friend void swap(G1& first, G1& second)
    {
        using std::swap;
        swap(first.point, second.point);
    }
};

class ZR
{
    public:
    unsigned char * val;
    ZR();
    ~ZR();
    ZR(int i);
    ZR(const ZR& other);
    void from_char(char c);
    void from_int(int i);
    void rand(void);
    void assign_sum(ZR &a, ZR &b);
    void assign_sum(unsigned char *a, unsigned char *b);
    void print(void);
    ZR operator + (ZR &other);
    ZR& operator = (const ZR &other);
    friend void swap(ZR& first, ZR& second)
    {
        using std::swap;
        swap(first.val, second.val);
    }
};

//BEGIN G1 FUNCTIONS *****************************************
G1::G1(void)
{
    point = new unsigned char[crypto_core_ed25519_BYTES];
    memset(point, 0, crypto_core_ed25519_BYTES);
}

G1::~G1() 
{ 
    delete[] point;
}

G1::G1(const G1& other)
{
    point = new unsigned char[crypto_core_ed25519_BYTES];
    memcpy(point, other.point, crypto_core_ed25519_BYTES);
}

G1& G1::operator = (const G1 &other)
{ 
    //Could possibly be sped up further: https://stackoverflow.com/questions/3279543/what-is-the-copy-and-swap-idiom
    G1 temp(other);
    swap(*this, temp);
    return *this;
}

G1 G1::operator + (G1 &other)
{ 
    G1 out = G1(); 
    out.assign_sum(point, other.point);
    return out; 
}

G1 G1::operator - (G1 &other)
{ 
    G1 out = G1(); 
    out.assign_diff(point, other.point);
    return out; 
}

G1 G1::operator * (ZR &other)
{ 
    G1 out = G1(); 
    out.assign_prod(other.val, point);
    return out; 
}

void G1::rand()
{
    unsigned char rand [crypto_core_ed25519_SCALARBYTES];
    randombytes_buf(rand, sizeof rand);
    crypto_core_ed25519_from_uniform(point, rand);
}

void G1::assign_sum(unsigned char *a, unsigned char *b)
{
    if (crypto_core_ed25519_add(point, a, b) < 0)
    {
        printf("Can it be that there's some sort of error?\n");
    }
}

void G1::assign_diff(unsigned char *a, unsigned char *b)
{
    if (crypto_core_ed25519_sub(point, a, b) < 0)
    {
        printf("Can it be that there's some sort of error?\n");
    }
}

void G1::assign_prod(unsigned char *a, unsigned char *b)
{
    if (crypto_scalarmult_ed25519_noclamp(point, a, b) < 0)
    {
        printf("Can it be that there's some sort of error?\n");
    }
}

void G1::print(void)
{
    static char hex[crypto_core_ed25519_BYTES * 2 + 1];
    sodium_bin2hex(hex, sizeof hex, point, crypto_core_ed25519_BYTES);
    printf("%s\n", hex);

}

G1 rand_point(void)
{
    G1 a = G1();
    a.rand();
    return a;
}

// BEGIN ZR FUNCTIONS ****************************
ZR::ZR(void)
{
    val = new unsigned char[crypto_core_ed25519_SCALARBYTES];
}

ZR::~ZR() 
{ 
    delete[] val;
}

ZR::ZR(int i)
{
    val = new unsigned char[crypto_core_ed25519_SCALARBYTES];
    from_int(i);
}

ZR::ZR(const ZR& other)
{
    val = new unsigned char[crypto_core_ed25519_SCALARBYTES];
    memcpy(val, other.val, crypto_core_ed25519_SCALARBYTES);
}

ZR ZR::operator + (ZR &other)
{ 
    ZR out = ZR(); 
    out.assign_sum(val, other.val);
    return out; 
}

ZR& ZR::operator = (const ZR &other)
{ 
    //Could possibly be sped up further: https://stackoverflow.com/questions/3279543/what-is-the-copy-and-swap-idiom
    ZR temp(other);
    swap(*this, temp);
    return *this;
}

void ZR::from_char(char c)
{
    memset(val, 0, crypto_core_ed25519_SCALARBYTES);
    val[0] = c;
}

void ZR::from_int(int i)
{
    memset(val, 0, crypto_core_ed25519_SCALARBYTES);
    int negate = 0;
    if(i < 0)
    {
        i *=-1;
        negate = 1;
    }
    //smol endian
    for(int j = 0; i > 0; j++)
    {
        val[j] = i % 256;
        i /= 256;
    }
    if(negate) crypto_core_ed25519_scalar_negate(val, val);
}

void ZR::rand(void)
{
    crypto_core_ed25519_scalar_random(val);
}

void ZR::assign_sum(ZR &a, ZR &b)
{
    crypto_core_ed25519_scalar_add(val, a.val, b.val);
}

void ZR::assign_sum(unsigned char *a, unsigned char *b)
{
    crypto_core_ed25519_scalar_add(val, a, b);
}

void ZR::print(void)
{
    static char hex[crypto_core_ed25519_SCALARBYTES * 2 + 1];
    sodium_bin2hex(hex, sizeof hex, val, crypto_core_ed25519_SCALARBYTES);
    printf("%s\n", hex);
}

ZR rand_scalar(void)
{
    ZR a = ZR();
    a.rand();
    return a;
}

std::vector<G1> precompute(unsigned int level, G1 g)
{
    unsigned long long int size = 1;
    for (unsigned int i = 0; i < level; i++)
    {
        size *= 2;
    }
    unsigned int d = 252 / level;
    if (252%level != 0)
    {
        d++;
    }
    std::vector<G1> powers;
    G1 zero1 = g-g;
    powers.push_back(zero1);
    G1 base = g;
    powers.push_back(base);
    for(unsigned int i = 1; i < level; i++)
    {
        G1 next = G1(powers[i]);
        for(unsigned int j = 0; j < d; j++)
        {
            next = next + next;
        }
        powers.push_back(next);
    }
    std::vector<G1> precomp;
    G1 zero = g - g;
    //Fill precomp with every possible combination of entries in powers
    for (unsigned long long i = 0; i<size; i++)
    {
        unsigned long long j = 1;
        G1 next = G1(zero);
        for(unsigned long long k = i; k > 0; k = k/2)
        {
            if (k%2 == 1)
            {
                next = next + powers[j];
            }
            j++;
        }
        precomp.push_back(next);
    }
    return precomp;
}
int calc_comb(unsigned char * arr, int offset, int c_len, int c_width)
{
    int out = 0;
    //252 bits only needs 31.5 bytes, but ZR is given 32 bytes. Adjust the offset by half a byte.
    int pos = offset + 4;
    if (pos < 0)
    {
        c_len--;
        pos += c_width;
    }
    unsigned char byte;
    int bitselect;
    //printf("Combing ");
    for(int j = 0; j < c_len; j++)
    {
        //printf("%i, ", pos -4);
        out *= 2;
        byte = arr[pos/8];
        bitselect = 7 - pos%8;
        if ((byte >> bitselect)  & 0x01)
        {
            out +=1;
        }
        pos += c_width;
    }
    //printf("\n");
    return out;
}

G1 ppmult(G1 g, ZR a, std::vector<G1> pp, int level)
{
    int d = 252 / level;
    if (252%level != 0)
    {
        d++;
    }
    unsigned char big_end [crypto_core_ed25519_SCALARBYTES];
    for(int i = 0; i < crypto_core_ed25519_SCALARBYTES; i++)
    {
        big_end[i] = a.val[crypto_core_ed25519_SCALARBYTES - 1 - i];
    }
    int lookup;
    G1 out = g-g;
    int padding = level*d - 252;
    for(int offset = -1 * padding; offset < d - padding; offset++)
    {
        out = out+out;
        lookup = calc_comb(big_end, offset, level, d);
        out = out + pp[lookup];
    }
    return out;
    
}

int main(void)
{
    if (sodium_init() < 0) {
        /* panic! the library couldn't be initialized, it is not safe to use */
        printf("The library couldn't be initialized, it is not safe to use Q.Q\n");
        return 0;
    }
    int level = 8;
    G1 g = rand_point();
    ZR a = rand_scalar();
    //ZR a = ZR(12);
    std::vector<G1> pp = precompute(level, g);
    G1 prod = ppmult(g, a, pp, level);
    G1 check = g*a;
    prod.print();
    check.print();
    G1 result = g;
    /*for(unsigned long i = 0; i < 10000; i++)
    {
        crypto_scalarmult_ed25519_noclamp(result.point, a.val, g.point);
        crypto_core_ed25519_add(result.point, g.point, g.point);
        ZR rand = rand_scalar();
        result = g*rand;
        result = ppmult(g, a, pp, level);
        result = g*a;
        result = g+g;
    }*/
    
    return 0;
}