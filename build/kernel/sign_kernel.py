# Copyright 2022 Nikita Melekhin. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import hashlib
import sys
import codecs

# Test Key Set.
PRIVATE_D = 0x38df48893d28df263487987da506d0c56d5f817573bc091071c5cd798d78ba4c996f946a2d695b9b4428794c59500ba2e5b2ed383aa791b18f56fd90875010eddb98c37113f9717511a2845edbcc85e9c559f74d474c107767666f894357c1439217ce82d0181a58ce9a8ad75e3a37ccb3aeba5ba07dd2f8cb8b92ae2735e275
PUBLIC_N = 0xa65f3fc1e6e850b5c7174fb7c0b30f36507ad9a67e83abe707b176fb7f44230711ffe8291119d921e0e483024dc6e603a628996ec2c875069aef21f1b9e9cfbcd3f7ee6f4ecf1bc67d9d7239adef94596038b4b5833ee34b4ddad75b17ad8f8fbb2cd5149115602aa57aecdfdacbea8a7fe735c98784a4b1868986dced517e1d
PUBLIC_E = 0x10001

kernel_path = sys.argv[1]
outpath = sys.argv[2]

sha256_hash = hashlib.sha256()
with open(kernel_path, "rb") as f:
    for byte_block in iter(lambda: f.read(4096), b""):
        sha256_hash.update(byte_block)


def int_to_bytes(n, nsize=128):
    xs = bytearray()
    while (n):
        by = n % 256
        xs.append(by)
        n //= 256

    while len(xs) < nsize:
        xs.append(0)

    return xs


hash = int(codecs.encode(sha256_hash.digest(), 'hex'), 16)
signature = pow(hash, PRIVATE_D, PUBLIC_N)

signature_file = open(outpath, "wb")
signature_file.write(int_to_bytes(signature))
signature_file.close()
