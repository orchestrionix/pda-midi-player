#ifndef __UCL_DCMP__
#define __UCL_DCMP__

#ifdef __cplusplus
extern "C" {
#endif


#define UCL_E_OK                    0
#define UCL_E_ERROR                 (-1)
#define UCL_E_INVALID_ARGUMENT      (-2)
#define UCL_E_OUT_OF_MEMORY         (-3)
/* compression errors */
#define UCL_E_NOT_COMPRESSIBLE      (-101)
/* decompression errors */
#define UCL_E_INPUT_OVERRUN         (-201)
#define UCL_E_OUTPUT_OVERRUN        (-202)
#define UCL_E_LOOKBEHIND_OVERRUN    (-203)
#define UCL_E_EOF_NOT_FOUND         (-204)
#define UCL_E_INPUT_NOT_CONSUMED    (-205)
#define UCL_E_OVERLAP_OVERRUN       (-206)

int ucl_nrv2e_decompress_le32(const unsigned char *src, unsigned int src_len, unsigned char *dst, unsigned int *dst_len);

#ifdef __cplusplus
}
#endif

#endif