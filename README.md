
# hal5_cavp

This application performs some of the CAVP validation tests for Crypto functions in STM32H5.

Implemented CAVP tests:

- Secure Hash
    - SHA1 Short and Long Msg
    - SHA256 Short and Long Msg
    - SHA512 Short and Long Msg

# Usage

Compile and flash with:

```
$ CAVP_TEST=SHA1 make
```

Possible CAVP_TEST values are:

- SHA1
- SHA256
- SHA512


