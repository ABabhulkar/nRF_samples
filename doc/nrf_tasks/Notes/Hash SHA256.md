#### Using PSA api
There are multiple hashing algorithms supported by PSA which can be found [here](https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/libraries/security/nrf_security/doc/driver_config.html#hash_support) 
There are 2 ways to use the hashing which are single part hashing and multi part hashing.
1. Single part hashing
	- This function will give you hash in provided buffer calculated by using algorithm provided in parameters.
	- This way is suitable for small data and where sync operations are expected.
```c
// define buffer for to get hash
uint8_t hash[PSA_HASH_MAX_SIZE];
size_t hash_len;

// Initialize PSA Crypto
status = psa_crypto_init();

// Calculate hash using SHA256 algorithm
psa_status_t status = psa_hash_compute(PSA_ALG_SHA_256, in_data, sizeof(in_data), hash, sizeof(hash), &hash_len);
if (status != PSA_SUCCESS) {
   LOG_ERR("psa hash failed: %d", status);
   return;
}

// Hash verification
status = psa_hash_compare(PSA_ALG_SHA_256, in_data, sizeof(in_data), hash, sizeof(hash));
if (status != PSA_SUCCESS) {
  LOG_ERR("hash compare failed: %d", status);
  return;
}
```

2. Using multi part hashing
	- This approach is useful in case we need have data stream or there is large data which can't be put in ram at a time.
	- In case we need to calculate hash in async way then this approach can be used
	- However user has to define the module which will provide async api to calculate hash and module it self defines the thread and main function which can be scheduled at low priority.
	- Verification of hash is same as generation just last function changes.
```c
// allocate operation stuct with intial state
psa_hash_operation_t operation = PSA_HASH_OPERATION_INIT;

// define buffer for to get hash
uint8_t hash[PSA_HASH_MAX_SIZE];
size_t hash_len;

// Initialize PSA Crypto
status = psa_crypto_init();

// Setup a multipart hash operation specify the hash algorithm.
status = psa_hash_setup(&hash_operation, PSA_ALG_SHA_256);

// Feed the chunks of the input data to the PSA driver 
status = psa_hash_update(&hash_operation, input_ptr, 42);
...
// Calculate the hash
status = psa_hash_finish(&hash_operation, hash, sizeof(hash), &hash_len);

// in case of verification all following function
status = psa_hash_verify(&operation, expected_hash, expected_hash_len);
if (status != PSA_SUCCESS) {
    LOG_ERR("Failed to verify hash\n");
    // in case of error call following function
    psa_hash_abort(&operation);
    return;
}
```