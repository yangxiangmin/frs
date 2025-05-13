// md5.h: interface for the MD5 class.

#ifndef Md5H
#define Md5H


// MD5 context
struct  MD5_CTX
{
    unsigned int    state[4];       // state (ABCD)
    unsigned int    count[2];       // number of bits, modulo 2^64 (lsb first)
    unsigned char   buffer[64];     // input buffer
};


class MD5
{
public:
    MD5();
    virtual ~MD5();

public:
    void    MD5Init         (MD5_CTX *);
    void    MD5Update       (MD5_CTX *,  unsigned char *, unsigned int);
    void    MD5Final        (unsigned char [16], MD5_CTX *);

private:
    void    MD5Transform    (unsigned int [4],  unsigned char [64]);
    void    Encode          (unsigned char *,   unsigned int  *, unsigned int);
    void    Decode          (unsigned int *,    unsigned char *, unsigned int);
};


#endif
