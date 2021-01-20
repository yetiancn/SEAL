// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "bench.h"
#include <benchmark/benchmark.h>
#include <iomanip>
#include <seal/seal.h>
using namespace benchmark;
using namespace seal;
using namespace sealbench;
using namespace std;

namespace sealbench
{
    /**
    Wraps benchmark::RegisterBenchmark to use microsecond and accepts std::string name.
    */
    template <class Lambda, class... Args>
    internal::Benchmark *register_bm(string name, Lambda &&fn, Args &&... args)
    {
        return RegisterBenchmark(name.c_str(), [=](State &st) { fn(st, args...); })
            ->Unit(benchmark::kMicrosecond)
            ->Iterations(10);
    }

#define SEAL_BENCHMARK_REGISTER(category, n, log_q, name, func, ...)                                                \
    register_bm(                                                                                                    \
        string("n=") + to_string(n) + string(" / log_q=") + to_string(log_q) + string(" / " #category " / " #name), \
        func, __VA_ARGS__)

    void register_bm_family(
        const pair<size_t, vector<Modulus>> &parms, unordered_map<EncryptionParameters, shared_ptr<BMEnv>> &bm_env_map)
    {
        // For BFV benchmark cases (default to 20-bit plain_modulus)
        EncryptionParameters parms_bfv(scheme_type::bfv);
        parms_bfv.set_poly_modulus_degree(parms.first);
        parms_bfv.set_coeff_modulus(parms.second);
        parms_bfv.set_plain_modulus(PlainModulus::Batching(parms.first, 20));
        shared_ptr<BMEnv> bm_env_bfv = bm_env_map.find(parms_bfv)->second;

        // For CKKS / KeyGen / Util benchmark cases
        EncryptionParameters parms_ckks(scheme_type::ckks);
        parms_ckks.set_poly_modulus_degree(parms.first);
        parms_ckks.set_coeff_modulus(parms.second);
        shared_ptr<BMEnv> bm_env_ckks = bm_env_map.find(parms_ckks)->second;

        // Registration / display order:
        // 1. KeyGen
        // 2. BFV
        // 3. CKKS
        // 4. Util
        int n = static_cast<int>(parms.first);
        int log_q = static_cast<int>(
            bm_env_map.find(parms_ckks)->second->context().key_context_data()->total_coeff_modulus_bit_count());
        SEAL_BENCHMARK_REGISTER(KeyGen, n, log_q, Secret, bm_keygen_secret, bm_env_ckks);
        SEAL_BENCHMARK_REGISTER(KeyGen, n, log_q, Public, bm_keygen_public, bm_env_ckks);
        if (bm_env_bfv->context().using_keyswitching())
        {
            SEAL_BENCHMARK_REGISTER(KeyGen, n, log_q, Relin, bm_keygen_relin, bm_env_ckks);
            SEAL_BENCHMARK_REGISTER(KeyGen, n, log_q, Galois, bm_keygen_galois, bm_env_ckks);
        }
        SEAL_BENCHMARK_REGISTER(BFV, n, log_q, EncryptSecret, bm_bfv_encrypt_secret, bm_env_bfv);
        SEAL_BENCHMARK_REGISTER(BFV, n, log_q, EncryptPublic, bm_bfv_encrypt_public, bm_env_bfv);
        SEAL_BENCHMARK_REGISTER(BFV, n, log_q, Decrypt, bm_bfv_decrypt, bm_env_bfv);
        SEAL_BENCHMARK_REGISTER(BFV, n, log_q, EncodeBatch, bm_bfv_encode_batch, bm_env_bfv);
        SEAL_BENCHMARK_REGISTER(BFV, n, log_q, DecodeBatch, bm_bfv_decode_batch, bm_env_bfv);
        SEAL_BENCHMARK_REGISTER(BFV, n, log_q, EvaluateAddCt, bm_bfv_add_ct, bm_env_bfv);
        SEAL_BENCHMARK_REGISTER(BFV, n, log_q, EvaluateAddPt, bm_bfv_add_pt, bm_env_bfv);
        SEAL_BENCHMARK_REGISTER(BFV, n, log_q, EvaluateMulCt, bm_bfv_mul_ct, bm_env_bfv);
        SEAL_BENCHMARK_REGISTER(BFV, n, log_q, EvaluateMulPt, bm_bfv_mul_pt, bm_env_bfv);
        SEAL_BENCHMARK_REGISTER(BFV, n, log_q, EvaluateSquare, bm_bfv_square, bm_env_bfv);
        if (bm_env_bfv->context().first_context_data()->parms().coeff_modulus().size() > 1)
        {
            SEAL_BENCHMARK_REGISTER(BFV, n, log_q, EvaluateModSwitchInplace, bm_bfv_modswitch_inplace, bm_env_bfv);
        }
        if (bm_env_bfv->context().using_keyswitching())
        {
            SEAL_BENCHMARK_REGISTER(BFV, n, log_q, EvaluateRelinInplace, bm_bfv_relin_inplace, bm_env_bfv);
            SEAL_BENCHMARK_REGISTER(BFV, n, log_q, EvaluateRotateRows, bm_bfv_rotate_rows, bm_env_bfv);
            SEAL_BENCHMARK_REGISTER(BFV, n, log_q, EvaluateRotateCols, bm_bfv_rotate_cols, bm_env_bfv);
        }

        SEAL_BENCHMARK_REGISTER(CKKS, n, log_q, EncryptSecret, bm_ckks_encrypt_secret, bm_env_ckks);
        SEAL_BENCHMARK_REGISTER(CKKS, n, log_q, EncryptPublic, bm_ckks_encrypt_public, bm_env_ckks);
        SEAL_BENCHMARK_REGISTER(CKKS, n, log_q, Decrypt, bm_ckks_decrypt, bm_env_ckks);
        SEAL_BENCHMARK_REGISTER(CKKS, n, log_q, EncodeDouble, bm_ckks_encode_double, bm_env_ckks);
        SEAL_BENCHMARK_REGISTER(CKKS, n, log_q, DecodeDouble, bm_ckks_decode_double, bm_env_ckks);
        SEAL_BENCHMARK_REGISTER(CKKS, n, log_q, EvaluateAddCt, bm_ckks_add_ct, bm_env_ckks);
        SEAL_BENCHMARK_REGISTER(CKKS, n, log_q, EvaluateAddPt, bm_ckks_add_pt, bm_env_ckks);
        SEAL_BENCHMARK_REGISTER(CKKS, n, log_q, EvaluateMulCt, bm_ckks_mul_ct, bm_env_ckks);
        SEAL_BENCHMARK_REGISTER(CKKS, n, log_q, EvaluateMulPt, bm_ckks_mul_ct, bm_env_ckks);
        SEAL_BENCHMARK_REGISTER(CKKS, n, log_q, EvaluateSquare, bm_ckks_square, bm_env_ckks);
        if (bm_env_bfv->context().first_context_data()->parms().coeff_modulus().size() > 1)
        {
            SEAL_BENCHMARK_REGISTER(CKKS, n, log_q, EvaluateRescaleInplace, bm_ckks_rescale_inplace, bm_env_ckks);
        }
        if (bm_env_bfv->context().using_keyswitching())
        {
            SEAL_BENCHMARK_REGISTER(CKKS, n, log_q, EvaluateRelinInplace, bm_ckks_relin_inplace, bm_env_ckks);
            SEAL_BENCHMARK_REGISTER(CKKS, n, log_q, EvaluateRotate, bm_ckks_rotate, bm_env_ckks);
        }
        //        SEAL_BENCHMARK_REGISTER(Util, n, log_q, NTTForward, bm_ntt_forward, bm_env_ckks);
        //        SEAL_BENCHMARK_REGISTER(Util, n, log_q, NTTBackward, bm_ntt_backward, bm_env_ckks);
        //        SEAL_BENCHMARK_REGISTER(Util, n, log_q, NTTLazyForward, bm_ntt_lazy_forward, bm_env_ckks);
        //        SEAL_BENCHMARK_REGISTER(Util, n, log_q, NTTLazyBackward, bm_ntt_lazy_backward, bm_env_ckks);
        //        SEAL_BENCHMARK_REGISTER(Util, n, log_q, FFTForward, bm_fft_forward, bm_env_ckks);
        //        SEAL_BENCHMARK_REGISTER(Util, n, log_q, FFTBackward, bm_fft_backward, bm_env_ckks);
    }

} // namespace sealbench

int main(int argc, char **argv)
{
    vector<pair<size_t, vector<Modulus>>> bm_parms_vec;

    unordered_map<EncryptionParameters, shared_ptr<BMEnv>> bm_env_map;

    // Initialize bm_parms_vec with BFV default paramaters with 128-bit security.
    // Advanced users may replace this section with custom parameters.
    // SEAL benchmark allow insecure parameters for experimental purpose.
    // DO NOT USE SEAL BENCHMARK AS EXAMPLE.
    auto default_parms = seal::util::global_variables::GetDefaultCoeffModulus128();

    cout << "Microsoft SEAL version: " << SEAL_VERSION << endl;
    cout << "SEALBenchmark is performing precomputation ..." << endl;

    for (auto &i : default_parms)
    {
        bm_parms_vec.emplace_back(i);
    }

    // Initialize bm_env_map with bm_parms_vec each of which creates two EncryptionParameters for BFV and CKKS.
    for (auto &i : default_parms)
    {
        EncryptionParameters parms_ckks(scheme_type::ckks);
        parms_ckks.set_poly_modulus_degree(i.first);
        parms_ckks.set_coeff_modulus(i.second);
        EncryptionParameters parms_bfv(scheme_type::bfv);
        parms_bfv.set_poly_modulus_degree(i.first);
        parms_bfv.set_coeff_modulus(i.second);
        parms_bfv.set_plain_modulus(PlainModulus::Batching(i.first, 20));

        if (bm_env_map.emplace(make_pair(parms_ckks, make_shared<BMEnv>(parms_ckks))).second == false)
        {
            throw invalid_argument("duplicate parameter sets");
        }
        if (bm_env_map.emplace(make_pair(parms_bfv, make_shared<BMEnv>(parms_bfv))).second == false)
        {
            throw invalid_argument("duplicate parameter sets");
        }
    }

    cout << "[" << setw(7) << right << (seal::MemoryManager::GetPool().alloc_byte_count() >> 20) << " MB] "
         << "Total allocation from the memory pool" << endl;

    // For each parameter set in bm_parms_vec, register a family of benchmark cases.
    for (auto &i : bm_parms_vec)
    {
        sealbench::register_bm_family(i, bm_env_map);
    }

    Initialize(&argc, argv);
    RunSpecifiedBenchmarks();

    cout << "[" << setw(7) << right << (seal::MemoryManager::GetPool().alloc_byte_count() >> 20) << " MB] "
         << "Total allocation from the memory pool" << endl;
}
