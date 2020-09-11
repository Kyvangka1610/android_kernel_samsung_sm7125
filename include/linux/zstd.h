/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) Yann Collet, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under both the BSD-style license (found in the
 * LICENSE file in the root directory of https://github.com/facebook/zstd) and
 * the GPLv2 (found in the COPYING file in the root directory of
 * https://github.com/facebook/zstd). You may select, at your option, one of the
 * above-listed licenses.
 */

#ifndef LINUX_ZSTD_H
#define LINUX_ZSTD_H

/**
 * This is a kernel-style API that wraps the upstream zstd API, which cannot be
 * used directly because the symbols aren't exported. It exposes the minimal
 * functionality which is currently required by users of zstd in the kernel.
 * Expose extra functions from lib/zstd/zstd.h as needed.
 */

/* ======   Dependency   ====== */
#include <linux/types.h>
#include <linux/zstd_lib.h>

/* ======   Helper Functions   ====== */
/**
 * zstd_compress_bound() - maximum compressed size in worst case scenario
 * @src_size: The size of the data to compress.
 *
 * Return:    The maximum compressed size in the worst case scenario.
 */
size_t zstd_compress_bound(size_t src_size);

/**
 * zstd_is_error() - tells if a size_t function result is an error code
 * @code:  The function result to check for error.
 *
 * Return: Non-zero iff the code is an error.
 */
unsigned int zstd_is_error(size_t code);

/**
 * enum zstd_error_code - zstd error codes
 */
typedef ZSTD_ErrorCode zstd_error_code;

/**
 * zstd_get_error_code() - translates an error function result to an error code
 * @code:  The function result for which zstd_is_error(code) is true.
 *
 * Return: A unique error code for this error.
 */
zstd_error_code zstd_get_error_code(size_t code);

/**
 * zstd_get_error_name() - translates an error function result to a string
 * @code:  The function result for which zstd_is_error(code) is true.
 *
 * Return: An error string corresponding to the error code.
 */
const char *zstd_get_error_name(size_t code);

/**
 * zstd_min_clevel() - minimum allowed compression level
 *
 * Return: The minimum allowed compression level.
 */
int zstd_min_clevel(void);

/**
 * zstd_max_clevel() - maximum allowed compression level
 *
 * Return: The maximum allowed compression level.
 */
int zstd_max_clevel(void);

/* ======   Parameter Selection   ====== */

/**
 * enum zstd_strategy - zstd compression search strategy
 *
 * From faster to stronger. See zstd_lib.h.
 */
typedef ZSTD_strategy zstd_strategy;

/**
 * struct zstd_compression_parameters - zstd compression parameters
 * @windowLog:    Log of the largest match distance. Larger means more
 *                compression, and more memory needed during decompression.
 * @chainLog:     Fully searched segment. Larger means more compression,
 *                slower, and more memory (useless for fast).
 * @hashLog:      Dispatch table. Larger means more compression,
 *                slower, and more memory.
 * @searchLog:    Number of searches. Larger means more compression and slower.
 * @searchLength: Match length searched. Larger means faster decompression,
 *                sometimes less compression.
 * @targetLength: Acceptable match size for optimal parser (only). Larger means
 *                more compression, and slower.
 * @strategy:     The zstd compression strategy.
 *
 * See zstd_lib.h.
 */
typedef ZSTD_compressionParameters zstd_compression_parameters;

/**
 * struct zstd_frame_parameters - zstd frame parameters
 * @contentSizeFlag: Controls whether content size will be present in the
 *                   frame header (when known).
 * @checksumFlag:    Controls whether a 32-bit checksum is generated at the
 *                   end of the frame for error detection.
 * @noDictIDFlag:    Controls whether dictID will be saved into the frame
 *                   header when using dictionary compression.
 *
 * The default value is all fields set to 0. See zstd_lib.h.
 */
typedef ZSTD_frameParameters zstd_frame_parameters;

/**
 * struct zstd_parameters - zstd parameters
 * @cParams: The compression parameters.
 * @fParams: The frame parameters.
 */
typedef ZSTD_parameters zstd_parameters;

/**
 * zstd_get_params() - returns zstd_parameters for selected level
 * @level:              The compression level
 * @estimated_src_size: The estimated source size to compress or 0
 *                      if unknown.
 *
 * Return:              The selected zstd_parameters.
 */
zstd_parameters zstd_get_params(int level,
	unsigned long long estimated_src_size);

/* ======   Single-pass Compression   ====== */

typedef ZSTD_CCtx zstd_cctx;

/**
 * zstd_cctx_workspace_bound() - max memory needed to initialize a zstd_cctx
 * @parameters: The compression parameters to be used.
 *
 * If multiple compression parameters might be used, the caller must call
 * zstd_cctx_workspace_bound() for each set of parameters and use the maximum
 * size.
 *
 * Return:      A lower bound on the size of the workspace that is passed to
 *              zstd_init_cctx().
 */
size_t zstd_cctx_workspace_bound(const zstd_compression_parameters *parameters);

/**
 * zstd_init_cctx() - initialize a zstd compression context
 * @workspace:      The workspace to emplace the context into. It must outlive
 *                  the returned context.
 * @workspace_size: The size of workspace. Use zstd_cctx_workspace_bound() to
 *                  determine how large the workspace must be.
 *
 * Return:          A zstd compression context or NULL on error.
 */
zstd_cctx *zstd_init_cctx(void *workspace, size_t workspace_size);

/**
 * zstd_compress_cctx() - compress src into dst with the initialized parameters
 * @cctx:         The context. Must have been initialized with zstd_init_cctx().
 * @dst:          The buffer to compress src into.
 * @dst_capacity: The size of the destination buffer. May be any size, but
 *                ZSTD_compressBound(srcSize) is guaranteed to be large enough.
 * @src:          The data to compress.
 * @src_size:     The size of the data to compress.
 * @parameters:   The compression parameters to be used.
 *
 * Return:        The compressed size or an error, which can be checked using
 *                zstd_is_error().
 */
size_t zstd_compress_cctx(zstd_cctx *cctx, void *dst, size_t dst_capacity,
	const void *src, size_t src_size, const zstd_parameters *parameters);

/* ======   Single-pass Decompression   ====== */

typedef ZSTD_DCtx zstd_dctx;

/**
 * zstd_dctx_workspace_bound() - max memory needed to initialize a zstd_dctx
 *
 * Return: A lower bound on the size of the workspace that is passed to
 *         zstd_init_dctx().
 */
size_t zstd_dctx_workspace_bound(void);

/**
 * zstd_init_dctx() - initialize a zstd decompression context
 * @workspace:      The workspace to emplace the context into. It must outlive
 *                  the returned context.
 * @workspace_size: The size of workspace. Use zstd_dctx_workspace_bound() to
 *                  determine how large the workspace must be.
 *
 * Return:          A zstd decompression context or NULL on error.
 */
zstd_dctx *zstd_init_dctx(void *workspace, size_t workspace_size);

/**
 * zstd_decompress_dctx() - decompress zstd compressed src into dst
 * @dctx:         The decompression context.
 * @dst:          The buffer to decompress src into.
 * @dst_capacity: The size of the destination buffer. Must be at least as large
 *                as the decompressed size. If the caller cannot upper bound the
 *                decompressed size, then it's better to use the streaming API.
 * @src:          The zstd compressed data to decompress. Multiple concatenated
 *                frames and skippable frames are allowed.
 * @src_size:     The exact size of the data to decompress.
 *
 * Return:        The decompressed size or an error, which can be checked using
 *                zstd_is_error().
 */
size_t zstd_decompress_dctx(zstd_dctx *dctx, void *dst, size_t dst_capacity,
	const void *src, size_t src_size);

/* ======   Streaming Buffers   ====== */

/**
 * struct zstd_in_buffer - input buffer for streaming
 * @src:  Start of the input buffer.
 * @size: Size of the input buffer.
 * @pos:  Position where reading stopped. Will be updated.
 *        Necessarily 0 <= pos <= size.
 *
 * See zstd_lib.h.
 */
typedef ZSTD_inBuffer zstd_in_buffer;

/**
 * struct zstd_out_buffer - output buffer for streaming
 * @dst:  Start of the output buffer.
 * @size: Size of the output buffer.
 * @pos:  Position where writing stopped. Will be updated.
 *        Necessarily 0 <= pos <= size.
 *
 * See zstd_lib.h.
 */
typedef ZSTD_outBuffer zstd_out_buffer;

/* ======   Streaming Compression   ====== */

typedef ZSTD_CStream zstd_cstream;

/**
 * zstd_cstream_workspace_bound() - memory needed to initialize a zstd_cstream
 * @cparams: The compression parameters to be used for compression.
 *
 * Return:   A lower bound on the size of the workspace that is passed to
 *           zstd_init_cstream().
 */
size_t zstd_cstream_workspace_bound(const zstd_compression_parameters *cparams);

/**
 * zstd_init_cstream() - initialize a zstd streaming compression context
 * @parameters        The zstd parameters to use for compression.
 * @pledged_src_size: If params.fParams.contentSizeFlag == 1 then the caller
 *                    must pass the source size (zero means empty source).
 *                    Otherwise, the caller may optionally pass the source
 *                    size, or zero if unknown.
 * @workspace:        The workspace to emplace the context into. It must outlive
 *                    the returned context.
 * @workspace_size:   The size of workspace.
 *                    Use zstd_cstream_workspace_bound(params->cparams) to
 *                    determine how large the workspace must be.
 *
 * Return:            The zstd streaming compression context or NULL on error.
 */
zstd_cstream *zstd_init_cstream(const zstd_parameters *parameters,
	unsigned long long pledged_src_size, void *workspace, size_t workspace_size);

/**
 * zstd_reset_cstream() - reset the context using parameters from creation
 * @cstream:          The zstd streaming compression context to reset.
 * @pledged_src_size: Optionally the source size, or zero if unknown.
 *
 * Resets the context using the parameters from creation. Skips dictionary
 * loading, since it can be reused. If `pledged_src_size` is non-zero the frame
 * content size is always written into the frame header.
 *
 * Return:            Zero or an error, which can be checked using
 *                    zstd_is_error().
 */
size_t zstd_reset_cstream(zstd_cstream *cstream,
	unsigned long long pledged_src_size);

/**
 * zstd_compress_stream() - streaming compress some of input into output
 * @cstream: The zstd streaming compression context.
 * @output:  Destination buffer. `output->pos` is updated to indicate how much
 *           compressed data was written.
 * @input:   Source buffer. `input->pos` is updated to indicate how much data
 *           was read. Note that it may not consume the entire input, in which
 *           case `input->pos < input->size`, and it's up to the caller to
 *           present remaining data again.
 *
 * The `input` and `output` buffers may be any size. Guaranteed to make some
 * forward progress if `input` and `output` are not empty.
 *
 * Return:   A hint for the number of bytes to use as the input for the next
 *           function call or an error, which can be checked using
 *           zstd_is_error().
 */
size_t zstd_compress_stream(zstd_cstream *cstream, zstd_out_buffer *output,
	zstd_in_buffer *input);

/**
 * zstd_flush_stream() - flush internal buffers into output
 * @cstream: The zstd streaming compression context.
 * @output:  Destination buffer. `output->pos` is updated to indicate how much
 *           compressed data was written.
 *
 * zstd_flush_stream() must be called until it returns 0, meaning all the data
 * has been flushed. Since zstd_flush_stream() causes a block to be ended,
 * calling it too often will degrade the compression ratio.
 *
 * Return:   The number of bytes still present within internal buffers or an
 *           error, which can be checked using zstd_is_error().
 */
size_t zstd_flush_stream(zstd_cstream *cstream, zstd_out_buffer *output);

/**
 * zstd_end_stream() - flush internal buffers into output and end the frame
 * @cstream: The zstd streaming compression context.
 * @output:  Destination buffer. `output->pos` is updated to indicate how much
 *           compressed data was written.
 *
 * zstd_end_stream() must be called until it returns 0, meaning all the data has
 * been flushed and the frame epilogue has been written.
 *
 * Return:   The number of bytes still present within internal buffers or an
 *           error, which can be checked using zstd_is_error().
 */
size_t zstd_end_stream(zstd_cstream *cstream, zstd_out_buffer *output);

/* ======   Streaming Decompression   ====== */

typedef ZSTD_DStream zstd_dstream;

/**
 * zstd_dstream_workspace_bound() - memory needed to initialize a zstd_dstream
 * @max_window_size: The maximum window size allowed for compressed frames.
 *
 * Return:           A lower bound on the size of the workspace that is passed
 *                   to zstd_init_dstream().
 */
size_t zstd_dstream_workspace_bound(size_t max_window_size);

/**
 * zstd_init_dstream() - initialize a zstd streaming decompression context
 * @max_window_size: The maximum window size allowed for compressed frames.
 * @workspace:       The workspace to emplace the context into. It must outlive
 *                   the returned context.
 * @workspaceSize:   The size of workspace.
 *                   Use zstd_dstream_workspace_bound(max_window_size) to
 *                   determine how large the workspace must be.
 *
 * Return:           The zstd streaming decompression context.
 */
zstd_dstream *zstd_init_dstream(size_t max_window_size, void *workspace,
	size_t workspace_size);

/**
 * zstd_reset_dstream() - reset the context using parameters from creation
 * @dstream: The zstd streaming decompression context to reset.
 *
 * Resets the context using the parameters from creation. Skips dictionary
 * loading, since it can be reused.
 *
 * Return:   Zero or an error, which can be checked using zstd_is_error().
 */
size_t zstd_reset_dstream(zstd_dstream *dstream);

/**
 * zstd_decompress_stream() - streaming decompress some of input into output
 * @dstream: The zstd streaming decompression context.
 * @output:  Destination buffer. `output.pos` is updated to indicate how much
 *           decompressed data was written.
 * @input:   Source buffer. `input.pos` is updated to indicate how much data was
 *           read. Note that it may not consume the entire input, in which case
 *           `input.pos < input.size`, and it's up to the caller to present
 *           remaining data again.
 *
 * The `input` and `output` buffers may be any size. Guaranteed to make some
 * forward progress if `input` and `output` are not empty.
 * zstd_decompress_stream() will not consume the last byte of the frame until
 * the entire frame is flushed.
 *
 * Return:   Returns 0 iff a frame is completely decoded and fully flushed.
 *           Otherwise returns a hint for the number of bytes to use as the
 *           input for the next function call or an error, which can be checked
 *           using zstd_is_error(). The size hint will never load more than the
 *           frame.
 */
size_t zstd_decompress_stream(zstd_dstream *dstream, zstd_out_buffer *output,
	zstd_in_buffer *input);

/* ======   Frame Inspection Functions ====== */

/**
 * zstd_find_frame_compressed_size() - returns the size of a compressed frame
 * @src:      Source buffer. It should point to the start of a zstd encoded
 *            frame or a skippable frame.
 * @src_size: The size of the source buffer. It must be at least as large as the
 *            size of the frame.
 *
 * Return:    The compressed size of the frame pointed to by `src` or an error,
 *            which can be check with zstd_is_error().
 *            Suitable to pass to ZSTD_decompress() or similar functions.
 */
size_t zstd_find_frame_compressed_size(const void *src, size_t src_size);

/**
 * struct zstd_frame_params - zstd frame parameters stored in the frame header
 * @frameContentSize: The frame content size, or 0 if not present.
 * @windowSize:       The window size, or 0 if the frame is a skippable frame.
 * @dictID:           The dictionary id, or 0 if not present.
 * @checksumFlag:     Whether a checksum was used.
 */
typedef ZSTD_frameParams zstd_frame_header;

/**
 * zstd_get_frame_header() - extracts parameters from a zstd or skippable frame
 * @params:   On success the frame parameters are written here.
 * @src:      The source buffer. It must point to a zstd or skippable frame.
 * @src_size: The size of the source buffer.
 *
 * Return:    0 on success. If more data is required it returns how many bytes
 *            must be provided to make forward progress. Otherwise it returns
 *            an error, which can be checked using zstd_is_error().
 */
<<<<<<< HEAD
size_t ZSTD_getFrameParams(ZSTD_frameParams *fparamsPtr, const void *src,
	size_t srcSize);

/*-*****************************************************************************
 * Buffer-less and synchronous inner streaming functions
 *
 * This is an advanced API, giving full control over buffer management, for
 * users which need direct control over memory.
 * But it's also a complex one, with many restrictions (documented below).
 * Prefer using normal streaming API for an easier experience
 ******************************************************************************/

/*-*****************************************************************************
 * Buffer-less streaming compression (synchronous mode)
 *
 * A ZSTD_CCtx object is required to track streaming operations.
 * Use ZSTD_initCCtx() to initialize a context.
 * ZSTD_CCtx object can be re-used multiple times within successive compression
 * operations.
 *
 * Start by initializing a context.
 * Use ZSTD_compressBegin(), or ZSTD_compressBegin_usingDict() for dictionary
 * compression,
 * or ZSTD_compressBegin_advanced(), for finer parameter control.
 * It's also possible to duplicate a reference context which has already been
 * initialized, using ZSTD_copyCCtx()
 *
 * Then, consume your input using ZSTD_compressContinue().
 * There are some important considerations to keep in mind when using this
 * advanced function :
 * - ZSTD_compressContinue() has no internal buffer. It uses externally provided
 *   buffer only.
 * - Interface is synchronous : input is consumed entirely and produce 1+
 *   (or more) compressed blocks.
 * - Caller must ensure there is enough space in `dst` to store compressed data
 *   under worst case scenario. Worst case evaluation is provided by
 *   ZSTD_compressBound().
 *   ZSTD_compressContinue() doesn't guarantee recover after a failed
 *   compression.
 * - ZSTD_compressContinue() presumes prior input ***is still accessible and
 *   unmodified*** (up to maximum distance size, see WindowLog).
 *   It remembers all previous contiguous blocks, plus one separated memory
 *   segment (which can itself consists of multiple contiguous blocks)
 * - ZSTD_compressContinue() detects that prior input has been overwritten when
 *   `src` buffer overlaps. In which case, it will "discard" the relevant memory
 *   section from its history.
 *
 * Finish a frame with ZSTD_compressEnd(), which will write the last block(s)
 * and optional checksum. It's possible to use srcSize==0, in which case, it
 * will write a final empty block to end the frame. Without last block mark,
 * frames will be considered unfinished (corrupted) by decoders.
 *
 * `ZSTD_CCtx` object can be re-used (ZSTD_compressBegin()) to compress some new
 * frame.
 ******************************************************************************/

/*=====   Buffer-less streaming compression functions  =====*/
size_t ZSTD_compressBegin(ZSTD_CCtx *cctx, int compressionLevel);
size_t ZSTD_compressBegin_usingDict(ZSTD_CCtx *cctx, const void *dict,
	size_t dictSize, int compressionLevel);
size_t ZSTD_compressBegin_advanced(ZSTD_CCtx *cctx, const void *dict,
	size_t dictSize, ZSTD_parameters params,
	unsigned long long pledgedSrcSize);
size_t ZSTD_copyCCtx(ZSTD_CCtx *cctx, const ZSTD_CCtx *preparedCCtx,
	unsigned long long pledgedSrcSize);
size_t ZSTD_compressBegin_usingCDict(ZSTD_CCtx *cctx, const ZSTD_CDict *cdict,
	unsigned long long pledgedSrcSize);
size_t ZSTD_compressContinue(ZSTD_CCtx *cctx, void *dst, size_t dstCapacity,
	const void *src, size_t srcSize);
size_t ZSTD_compressEnd(ZSTD_CCtx *cctx, void *dst, size_t dstCapacity,
	const void *src, size_t srcSize);



/*-*****************************************************************************
 * Buffer-less streaming decompression (synchronous mode)
 *
 * A ZSTD_DCtx object is required to track streaming operations.
 * Use ZSTD_initDCtx() to initialize a context.
 * A ZSTD_DCtx object can be re-used multiple times.
 *
 * First typical operation is to retrieve frame parameters, using
 * ZSTD_getFrameParams(). It fills a ZSTD_frameParams structure which provide
 * important information to correctly decode the frame, such as the minimum
 * rolling buffer size to allocate to decompress data (`windowSize`), and the
 * dictionary ID used.
 * Note: content size is optional, it may not be present. 0 means unknown.
 * Note that these values could be wrong, either because of data malformation,
 * or because an attacker is spoofing deliberate false information. As a
 * consequence, check that values remain within valid application range,
 * especially `windowSize`, before allocation. Each application can set its own
 * limit, depending on local restrictions. For extended interoperability, it is
 * recommended to support at least 8 MB.
 * Frame parameters are extracted from the beginning of the compressed frame.
 * Data fragment must be large enough to ensure successful decoding, typically
 * `ZSTD_frameHeaderSize_max` bytes.
 * Result: 0: successful decoding, the `ZSTD_frameParams` structure is filled.
 *        >0: `srcSize` is too small, provide at least this many bytes.
 *        errorCode, which can be tested using ZSTD_isError().
 *
 * Start decompression, with ZSTD_decompressBegin() or
 * ZSTD_decompressBegin_usingDict(). Alternatively, you can copy a prepared
 * context, using ZSTD_copyDCtx().
 *
 * Then use ZSTD_nextSrcSizeToDecompress() and ZSTD_decompressContinue()
 * alternatively.
 * ZSTD_nextSrcSizeToDecompress() tells how many bytes to provide as 'srcSize'
 * to ZSTD_decompressContinue().
 * ZSTD_decompressContinue() requires this _exact_ amount of bytes, or it will
 * fail.
 *
 * The result of ZSTD_decompressContinue() is the number of bytes regenerated
 * within 'dst' (necessarily <= dstCapacity). It can be zero, which is not an
 * error; it just means ZSTD_decompressContinue() has decoded some metadata
 * item. It can also be an error code, which can be tested with ZSTD_isError().
 *
 * ZSTD_decompressContinue() needs previous data blocks during decompression, up
 * to `windowSize`. They should preferably be located contiguously, prior to
 * current block. Alternatively, a round buffer of sufficient size is also
 * possible. Sufficient size is determined by frame parameters.
 * ZSTD_decompressContinue() is very sensitive to contiguity, if 2 blocks don't
 * follow each other, make sure that either the compressor breaks contiguity at
 * the same place, or that previous contiguous segment is large enough to
 * properly handle maximum back-reference.
 *
 * A frame is fully decoded when ZSTD_nextSrcSizeToDecompress() returns zero.
 * Context can then be reset to start a new decompression.
 *
 * Note: it's possible to know if next input to present is a header or a block,
 * using ZSTD_nextInputType(). This information is not required to properly
 * decode a frame.
 *
 * == Special case: skippable frames ==
 *
 * Skippable frames allow integration of user-defined data into a flow of
 * concatenated frames. Skippable frames will be ignored (skipped) by a
 * decompressor. The format of skippable frames is as follows:
 * a) Skippable frame ID - 4 Bytes, Little endian format, any value from
 *    0x184D2A50 to 0x184D2A5F
 * b) Frame Size - 4 Bytes, Little endian format, unsigned 32-bits
 * c) Frame Content - any content (User Data) of length equal to Frame Size
 * For skippable frames ZSTD_decompressContinue() always returns 0.
 * For skippable frames ZSTD_getFrameParams() returns fparamsPtr->windowLog==0
 * what means that a frame is skippable.
 * Note: If fparamsPtr->frameContentSize==0, it is ambiguous: the frame might
 *       actually be a zstd encoded frame with no content. For purposes of
 *       decompression, it is valid in both cases to skip the frame using
 *       ZSTD_findFrameCompressedSize() to find its size in bytes.
 * It also returns frame size as fparamsPtr->frameContentSize.
 ******************************************************************************/

/*=====   Buffer-less streaming decompression functions  =====*/
size_t ZSTD_decompressBegin(ZSTD_DCtx *dctx);
size_t ZSTD_decompressBegin_usingDict(ZSTD_DCtx *dctx, const void *dict,
	size_t dictSize);
void   ZSTD_copyDCtx(ZSTD_DCtx *dctx, const ZSTD_DCtx *preparedDCtx);
size_t ZSTD_nextSrcSizeToDecompress(ZSTD_DCtx *dctx);
size_t ZSTD_decompressContinue(ZSTD_DCtx *dctx, void *dst, size_t dstCapacity,
	const void *src, size_t srcSize);
typedef enum {
	ZSTDnit_frameHeader,
	ZSTDnit_blockHeader,
	ZSTDnit_block,
	ZSTDnit_lastBlock,
	ZSTDnit_checksum,
	ZSTDnit_skippableFrame
} ZSTD_nextInputType_e;
ZSTD_nextInputType_e ZSTD_nextInputType(ZSTD_DCtx *dctx);

/*-*****************************************************************************
 * Block functions
 *
 * Block functions produce and decode raw zstd blocks, without frame metadata.
 * Frame metadata cost is typically ~18 bytes, which can be non-negligible for
 * very small blocks (< 100 bytes). User will have to take in charge required
 * information to regenerate data, such as compressed and content sizes.
 *
 * A few rules to respect:
 * - Compressing and decompressing require a context structure
 *   + Use ZSTD_initCCtx() and ZSTD_initDCtx()
 * - It is necessary to init context before starting
 *   + compression : ZSTD_compressBegin()
 *   + decompression : ZSTD_decompressBegin()
 *   + variants _usingDict() are also allowed
 *   + copyCCtx() and copyDCtx() work too
 * - Block size is limited, it must be <= ZSTD_getBlockSizeMax()
 *   + If you need to compress more, cut data into multiple blocks
 *   + Consider using the regular ZSTD_compress() instead, as frame metadata
 *     costs become negligible when source size is large.
 * - When a block is considered not compressible enough, ZSTD_compressBlock()
 *   result will be zero. In which case, nothing is produced into `dst`.
 *   + User must test for such outcome and deal directly with uncompressed data
 *   + ZSTD_decompressBlock() doesn't accept uncompressed data as input!!!
 *   + In case of multiple successive blocks, decoder must be informed of
 *     uncompressed block existence to follow proper history. Use
 *     ZSTD_insertBlock() in such a case.
 ******************************************************************************/

/* Define for static allocation */
#define ZSTD_BLOCKSIZE_ABSOLUTEMAX (4 * 1024)
/*=====   Raw zstd block functions  =====*/
size_t ZSTD_getBlockSizeMax(ZSTD_CCtx *cctx);
size_t ZSTD_compressBlock(ZSTD_CCtx *cctx, void *dst, size_t dstCapacity,
	const void *src, size_t srcSize);
size_t ZSTD_decompressBlock(ZSTD_DCtx *dctx, void *dst, size_t dstCapacity,
	const void *src, size_t srcSize);
size_t ZSTD_insertBlock(ZSTD_DCtx *dctx, const void *blockStart,
	size_t blockSize);
=======
size_t zstd_get_frame_header(zstd_frame_header *params, const void *src,
	size_t src_size);
>>>>>>> 6a27ec63c637... BACKPORT: lib: zstd: Add kernel-specific API

#endif  /* LINUX_ZSTD_H */
