/* Copyright (C) 2026 FonSyHen / LightningMods */

#include "bootstrap_payload.h"

#include <onion/log.h>

#include <stdint.h>
#include <stdlib.h>

#include "LzmaLib.h"

#define LZMA_CLI_HEADER_SIZE 13

#ifndef FONSYHEN_BOOTSTRAPPER_LZMA
#define FONSYHEN_BOOTSTRAPPER_LZMA "../../bin/bootstrapper.elf.lzma"
#endif
#ifndef FONSYHEN_BOOTSTRAPPER_SIZE
#define FONSYHEN_BOOTSTRAPPER_SIZE "../../bin/bootstrapper.elf.lzma.size"
#endif
#ifndef FONSYHEN_BOOTSTRAPPER_SHA1
#define FONSYHEN_BOOTSTRAPPER_SHA1 "../../bin/bootstrapper.elf.sha1"
#endif

__asm__(".intel_syntax noprefix\n"
        ".section .data\n"
        ".global fonsyhen_compressed\n"
        ".type   fonsyhen_compressed, @object\n"
        ".align  16\n"
        "fonsyhen_compressed:\n"
        ".incbin \"" FONSYHEN_BOOTSTRAPPER_LZMA "\"\n"
        "fonsyhen_compressed_end:\n"
        ".global fonsyhen_compressed_size\n"
        ".type  fonsyhen_compressed_size, @object\n"
        ".align  4\n"
        "fonsyhen_compressed_size:\n"
        ".int    fonsyhen_compressed_end - fonsyhen_compressed\n"
        ".global fonsyhen_decompressed_size\n"
        ".type   fonsyhen_decompressed_size, @object\n"
        ".align  16\n"
        "fonsyhen_decompressed_size:\n"
        ".incbin \"" FONSYHEN_BOOTSTRAPPER_SIZE "\"\n"
        ".byte 0\n"
        ".global fonsyhen_sha1_hex\n"
        ".type   fonsyhen_sha1_hex, @object\n"
        ".align  16\n"
        "fonsyhen_sha1_hex:\n"
        ".incbin \"" FONSYHEN_BOOTSTRAPPER_SHA1 "\"\n"
        ".byte 0\n");

extern uint32_t fonsyhen_compressed_size;
extern uint8_t fonsyhen_compressed[];
extern uint8_t fonsyhen_decompressed_size[];
extern uint8_t fonsyhen_sha1_hex[];

size_t bootstrap_payload_expected_size(void) {
  char *end = NULL;
  const unsigned long n =
      strtoul((const char *)fonsyhen_decompressed_size, &end, 10);
  if (end == (char *)fonsyhen_decompressed_size || n == 0)
    return 0;
  return (size_t)n;
}

bool bootstrap_payload_expected_sha1(uint8_t digest[SHA1_DIGEST_SIZE]) {
  return sha1_parse_hex((const char *)fonsyhen_sha1_hex, digest);
}

bootstrap_payload_status bootstrap_payload_decompress(uint8_t **out,
                                                      size_t *out_size) {
  if (!out || !out_size)
    return BOOTSTRAP_PAYLOAD_INVALID;
  *out = NULL;
  *out_size = 0;

  if (fonsyhen_compressed_size <= LZMA_CLI_HEADER_SIZE)
    return BOOTSTRAP_PAYLOAD_INVALID;

  const size_t expected = bootstrap_payload_expected_size();
  if (expected == 0)
    return BOOTSTRAP_PAYLOAD_INVALID;

  uint8_t *buf = (uint8_t *)malloc(expected);
  if (!buf)
    return BOOTSTRAP_PAYLOAD_NOMEM;

  size_t dest_len = expected;
  size_t src_len = fonsyhen_compressed_size;
  const int res =
      LzmaUncompress(buf, &dest_len, fonsyhen_compressed + LZMA_CLI_HEADER_SIZE,
                     &src_len, fonsyhen_compressed, LZMA_PROPS_SIZE);
  if (res != 0 || dest_len == 0) {
    LOG_ERROR("LzmaUncompress failed: %d", res);
    free(buf);
    return BOOTSTRAP_PAYLOAD_LZMA;
  }

  *out = buf;
  *out_size = dest_len;
  return BOOTSTRAP_PAYLOAD_OK;
}
