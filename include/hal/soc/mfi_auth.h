/*
 * Copyright (C) 2016 YunOS Project. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file hal/soc/mfi_auth.h
 * @brief MFI UATH HAL
 * @version since 5.5.0
 */

#ifndef YOS_MFI_AUTH_H
#define YOS_MFI_AUTH_H

/** @brief     PlatformMFiAuthInitialize
 *
 *  @abstract Performs any platform-specific initialization needed.
 *            Example: Bring up I2C interface for communication with
 *            the Apple Authentication Coprocessor.
 *
 *  @param   i2c            :  mico_i2c_t context
 *
 *  @return    kNoErr        : on success.
 *  @return    kGeneralErr   : if an error occurred with any step
 */
int32_t hal_mfi_auth_init(uint8_t i2c);


/** @brief    PlatformMFiAuthFinalize
 *
 *  @abstract Performs any platform-specific cleanup needed.
 *            Example: Bringing down the I2C interface for communication with
 *            the Apple Authentication Coprocessor.
 *
 *  @param    i2c            :  mico_i2c_t context
 *
 *  @return   none
 */
void hal_mfi_auth_finalize(void);



/** @brief    PlatformMFiAuthCreateSignature
 *
 *  @abstract Create an RSA signature from the specified SHA-1 digest
 *            using the Apple Authentication Coprocessor.
 *
 *  @param    inDigestPtr     :    Pointer to 20-byte SHA-1 digest.
 *  @param    inDigestLen     :    Number of bytes in the digest. Must be 20.
 *  @param    outSignaturePtr :    Receives malloc()'d ptr to RSA signature. Caller must free() on success.
 *  @param    outSignatureLen :    Receives number of bytes in RSA signature.
 *
 *  @return    kNoErr         :    on success.
 *  @return    kGeneralErr    :    if an error occurred with any step
 */
int hal_mfi_auth_create_signature(const void *in_digest_ptr,
                                  size_t      in_digest_len,
                                  uint8_t   **out_signature_ptr,
                                  size_t     *out_signature_len);



/** @brief    PlatformMFiAuthCopyCertificate
 *
 *  @abstract Copy the certificate from the Apple Authentication Coprocessor.
 *
 *  @param    outCertificatePtr:   Receives malloc()'d ptr to a DER-encoded PKCS#7 message containing the certificate.
                                    Caller must free() on success.
 *  @param    outCertificateLen:   Number of bytes in the DER-encoded certificate.
 *
 *  @return    kNoErr         :    on success.
 *  @return    kGeneralErr    :    if an error occurred with any step
 */
int hal_mfi_auth_copy_certificate(uint8_t **out_cert_ptr, size_t *out_cert_len);

/** @} */
/** @} */
#endif // __MICODRIVERMFIAUTH_H__


