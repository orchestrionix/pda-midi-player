#include "ucl_dcmp.h"

#define UCL_UINT32_C(c)  c ## U

#define getbit(bb)      getbit_8(bb,src,ilen)

#define getbit_8(bb, src, ilen) \
    (((bb = bb & 0x7f ? bb*2 : ((unsigned)src[ilen++]*2+1)) >> 8) & 1)

#ifdef SAFE
#define fail(x,r)   if (x) { *dst_len = olen; return r; }
#else
#define fail(x,r)
#endif

int ucl_nrv2e_decompress_le32(const unsigned char *src, unsigned int src_len, unsigned char *dst, unsigned int *dst_len)
{
    unsigned int bb = 0;
#ifdef TEST_OVERLAP
    unsigned int ilen = src_off, olen = 0, last_m_off = 1;
#else
    unsigned int ilen = 0, olen = 0, last_m_off = 1;
#endif
#ifdef SAFE
    const unsigned int oend = *dst_len;
#endif

#ifdef TEST_OVERLAP
    src_len += src_off;
    fail(oend >= src_len, UCL_E_OVERLAP_OVERRUN);
#endif

    for (;;)
    {
        unsigned int m_off, m_len;

        while (getbit(bb))
        {
            fail(ilen >= src_len, UCL_E_INPUT_OVERRUN);
            fail(olen >= oend, UCL_E_OUTPUT_OVERRUN);
#ifdef TEST_OVERLAP
            fail(olen > ilen, UCL_E_OVERLAP_OVERRUN);
            olen++; ilen++;
#else
            dst[olen++] = src[ilen++];
#endif
        }
        m_off = 1;
        for (;;)
        {
            m_off = m_off*2 + getbit(bb);
            fail(ilen >= src_len, UCL_E_INPUT_OVERRUN);
            fail(m_off > UCL_UINT32_C(0xffffff) + 3, UCL_E_LOOKBEHIND_OVERRUN);
            if (getbit(bb)) break;
            m_off = (m_off-1)*2 + getbit(bb);
        }
        if (m_off == 2)
        {
            m_off = last_m_off;
            m_len = getbit(bb);
        }
        else
        {
            fail(ilen >= src_len, UCL_E_INPUT_OVERRUN);
            m_off = (m_off-3)*256 + src[ilen++];
            if (m_off == UCL_UINT32_C(0xffffffff))
                break;
            m_len = (m_off ^ UCL_UINT32_C(0xffffffff)) & 1;
            m_off >>= 1;
            last_m_off = ++m_off;
        }
        if (m_len)
            m_len = 1 + getbit(bb);
        else if (getbit(bb))
            m_len = 3 + getbit(bb);
        else
        {
            m_len++;
            do {
                m_len = m_len*2 + getbit(bb);
                fail(ilen >= src_len, UCL_E_INPUT_OVERRUN);
                fail(m_len >= oend, UCL_E_OUTPUT_OVERRUN);
            } while (!getbit(bb));
            m_len += 3;
        }
        m_len += (m_off > 0x500);
        fail(olen + m_len > oend, UCL_E_OUTPUT_OVERRUN);
        fail(m_off > olen, UCL_E_LOOKBEHIND_OVERRUN);
#ifdef TEST_OVERLAP
        olen += m_len + 1;
        fail(olen > ilen, UCL_E_OVERLAP_OVERRUN);
#else
        {
            const unsigned char *m_pos;
            m_pos = dst + olen - m_off;
            dst[olen++] = *m_pos++;
            do dst[olen++] = *m_pos++; while (--m_len > 0);
        }
#endif
    }
    *dst_len = olen;
    return ilen == src_len ? UCL_E_OK : (ilen < src_len ? UCL_E_INPUT_NOT_CONSUMED : UCL_E_INPUT_OVERRUN);
}
