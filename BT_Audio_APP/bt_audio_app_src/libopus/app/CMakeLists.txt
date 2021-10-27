# vim: et ts=4 sts=4 sw=4 tw=0
cmake_minimum_required(VERSION 3.4.1)
PROJECT(opus)

#add_definitions(-D__EMX__ -DOPUS_BUILD -DFIXED_POINT -DARM_NEON_INTR_CFLAGS -ffast-math -O3 -funroll-loops
#                -DDISABLE_FLOAT_API -DUSE_ALLOCA -DHAVE_LRINT -DHAVE_LRINTF -D__OPTIMIZE__)

add_definitions(-g -ffunction-sections -funwind-tables -fstack-protector-strong -no-canonical-prefixes -D__EMX__ -DOPUS_BUILD -DFIXED_POINT -DARM_NEON_INTR_CFLAGS -ffast-math -O3 -funroll-loops
                -DDISABLE_FLOAT_API -DUSE_ALLOCA -DHAVE_LRINT -DHAVE_LRINTF -D__OPTIMIZE__)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fvisibility=hidden")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fvisibility=hidden")

set(opus_src_path  ${CMAKE_CURRENT_SOURCE_DIR}/../libopus/)

include_directories(
    ${opus_src_path}/include
    ${opus_src_path}/silk
    ${opus_src_path}/silk/fixed
    ${opus_src_path}/celt
)

set(PUBLIC_HEADERS
    ${opus_src_path}/include/opus.h
    ${opus_src_path}/include/opus_types.h
    ${opus_src_path}/include/opus_defines.h
    )

IF (NOT TARGET_ARCH)
    message(WARNING  "TARGET_ARCH is not set.use x86 as default")
    set(TARGET_ARCH x86)
ENDIF()

#message(WARNING "########${TARGET_ARCH}##############")

file(GLOB opus   ${opus_src_path}/celt/bands.c
                 ${opus_src_path}/celt/celt.c
                 ${opus_src_path}/celt/celt_encoder.c
                 ${opus_src_path}/celt/celt_decoder.c
                 ${opus_src_path}/celt/cwrs.c
                 ${opus_src_path}/celt/entcode.c
                 ${opus_src_path}/celt/entdec.c
                 ${opus_src_path}/celt/entenc.c
                 ${opus_src_path}/celt/kiss_fft.c
                 ${opus_src_path}/celt/laplace.c
                 ${opus_src_path}/celt/mathops.c
                 ${opus_src_path}/celt/mdct.c
                 ${opus_src_path}/celt/modes_opus.c
                 ${opus_src_path}/celt/pitch.c
                 ${opus_src_path}/celt/celt_lpc.c
                 ${opus_src_path}/celt/quant_bands.c
                 ${opus_src_path}/celt/rate.c
                 ${opus_src_path}/celt/vq.c
                 ${opus_src_path}/silk/CNG.c
                 ${opus_src_path}/silk/code_signs.c
                 ${opus_src_path}/silk/init_decoder.c
                 ${opus_src_path}/silk/decode_core.c
                 ${opus_src_path}/silk/decode_frame.c
                 ${opus_src_path}/silk/decode_parameters.c
                 ${opus_src_path}/silk/decode_indices.c
                 ${opus_src_path}/silk/decode_pulses.c
                 ${opus_src_path}/silk/decoder_set_fs.c
                 ${opus_src_path}/silk/dec_API.c
                 ${opus_src_path}/silk/enc_API.c
                 ${opus_src_path}/silk/encode_indices.c
                 ${opus_src_path}/silk/encode_pulses.c
                 ${opus_src_path}/silk/gain_quant.c
                 ${opus_src_path}/silk/interpolate.c
                 ${opus_src_path}/silk/LP_variable_cutoff.c
                 ${opus_src_path}/silk/NLSF_decode.c
                 ${opus_src_path}/silk/NSQ.c
                 ${opus_src_path}/silk/NSQ_del_dec.c
                 ${opus_src_path}/silk/PLC.c
                 ${opus_src_path}/silk/shell_coder.c
                 ${opus_src_path}/silk/tables_gain.c
                 ${opus_src_path}/silk/tables_LTP.c
                 ${opus_src_path}/silk/tables_NLSF_CB_NB_MB.c
                 ${opus_src_path}/silk/tables_NLSF_CB_WB.c
                 ${opus_src_path}/silk/tables_other.c
                 ${opus_src_path}/silk/tables_pitch_lag.c
                 ${opus_src_path}/silk/tables_pulses_per_block.c
                 ${opus_src_path}/silk/VAD.c
                 ${opus_src_path}/silk/control_audio_bandwidth.c
                 ${opus_src_path}/silk/quant_LTP_gains.c
                 ${opus_src_path}/silk/VQ_WMat_EC.c
                 ${opus_src_path}/silk/HP_variable_cutoff.c
                 ${opus_src_path}/silk/NLSF_encode.c
                 ${opus_src_path}/silk/NLSF_VQ.c
                 ${opus_src_path}/silk/NLSF_unpack.c
                 ${opus_src_path}/silk/NLSF_del_dec_quant.c
                 ${opus_src_path}/silk/process_NLSFs.c
                 ${opus_src_path}/silk/stereo_LR_to_MS.c
                 ${opus_src_path}/silk/stereo_MS_to_LR.c
                 ${opus_src_path}/silk/check_control_input.c
                 ${opus_src_path}/silk/control_SNR.c
                 ${opus_src_path}/silk/init_encoder.c
                 ${opus_src_path}/silk/control_codec.c
                 ${opus_src_path}/silk/A2NLSF.c
                 ${opus_src_path}/silk/ana_filt_bank_1.c
                 ${opus_src_path}/silk/biquad_alt.c
                 ${opus_src_path}/silk/bwexpander_32.c
                 ${opus_src_path}/silk/bwexpander.c
                 ${opus_src_path}/silk/debug.c
                 ${opus_src_path}/silk/decode_pitch.c
                 ${opus_src_path}/silk/inner_prod_aligned.c
                 ${opus_src_path}/silk/lin2log.c
                 ${opus_src_path}/silk/log2lin.c
                 ${opus_src_path}/silk/LPC_analysis_filter.c
                 ${opus_src_path}/silk/LPC_inv_pred_gain.c
                 ${opus_src_path}/silk/table_LSF_cos.c
                 ${opus_src_path}/silk/NLSF2A.c
                 ${opus_src_path}/silk/NLSF_stabilize.c
                 ${opus_src_path}/silk/NLSF_VQ_weights_laroia.c
                 ${opus_src_path}/silk/pitch_est_tables.c
                 ${opus_src_path}/silk/resampler.c
                 ${opus_src_path}/silk/resampler_down2_3.c
                 ${opus_src_path}/silk/resampler_down2.c
                 ${opus_src_path}/silk/resampler_private_AR2.c
                 ${opus_src_path}/silk/resampler_private_down_FIR.c
                 ${opus_src_path}/silk/resampler_private_IIR_FIR.c
                 ${opus_src_path}/silk/resampler_private_up2_HQ.c
                 ${opus_src_path}/silk/resampler_rom.c
                 ${opus_src_path}/silk/sigm_Q15.c
                 ${opus_src_path}/silk/sort.c
                 ${opus_src_path}/silk/sum_sqr_shift.c
                 ${opus_src_path}/silk/stereo_decode_pred.c
                 ${opus_src_path}/silk/stereo_encode_pred.c
                 ${opus_src_path}/silk/stereo_find_predictor.c
                 ${opus_src_path}/silk/stereo_quant_pred.c
                 ${opus_src_path}/silk/LPC_fit.c
                 ${opus_src_path}/silk/fixed/LTP_analysis_filter_FIX.c
                 ${opus_src_path}/silk/fixed/LTP_scale_ctrl_FIX.c
                 ${opus_src_path}/silk/fixed/corrMatrix_FIX.c
                 ${opus_src_path}/silk/fixed/encode_frame_FIX.c
                 ${opus_src_path}/silk/fixed/find_LPC_FIX.c
                 ${opus_src_path}/silk/fixed/find_LTP_FIX.c
                 ${opus_src_path}/silk/fixed/find_pitch_lags_FIX.c
                 ${opus_src_path}/silk/fixed/find_pred_coefs_FIX.c
                 ${opus_src_path}/silk/fixed/noise_shape_analysis_FIX.c
                 ${opus_src_path}/silk/fixed/process_gains_FIX.c
                 ${opus_src_path}/silk/fixed/regularize_correlations_FIX.c
                 ${opus_src_path}/silk/fixed/residual_energy16_FIX.c
                 ${opus_src_path}/silk/fixed/residual_energy_FIX.c
                 ${opus_src_path}/silk/fixed/warped_autocorrelation_FIX.c
                 ${opus_src_path}/silk/fixed/apply_sine_window_FIX.c
                 ${opus_src_path}/silk/fixed/autocorr_FIX.c
                 ${opus_src_path}/silk/fixed/burg_modified_FIX.c
                 ${opus_src_path}/silk/fixed/k2a_FIX.c
                 ${opus_src_path}/silk/fixed/k2a_Q16_FIX.c
                 ${opus_src_path}/silk/fixed/pitch_analysis_core_FIX.c
                 ${opus_src_path}/silk/fixed/vector_ops_FIX.c
                 ${opus_src_path}/silk/fixed/schur64_FIX.c
                 ${opus_src_path}/silk/fixed/schur_FIX.c
                 ${opus_src_path}/src/opus.c
                 ${opus_src_path}/src/opus_decoder.c
                 ${opus_src_path}/src/opus_encoder.c
#                 ${opus_src_path}/src/opus_multistream.c
#                 ${opus_src_path}/src/opus_multistream_encoder.c
#                 ${opus_src_path}/src/opus_multistream_decoder.c
                 ${opus_src_path}/src/repacketizer.c
                 ${opus_src_path}/../app/aivs_encode.c)

IF(${TARGET_ARCH} MATCHES  "arm")
    file(GLOB opus_arch   ${opus_src_path}/celt/arm/armcpu.c
                          ${opus_src_path}/celt/arm/arm_celt_map.c)
    list(APPEND opus ${opus_arch})
ENDIF()

IF(${TARGET_ARCH} MATCHES  "x86")
    file(GLOB opus_arch   ${opus_src_path}/celt/x86/pitch_sse2.c
                          ${opus_src_path}/celt/x86/vq_sse2.c)
    list(APPEND opus ${opus_arch})
ENDIF()

IF(${TARGET_ARCH} MATCHES "mips")
    add_compile_options(-Wall -W -Wstrict-prototypes -Wextra -Wcast-align -Wnested-externs -Wshadow)
    add_compile_options(-mips32r2 -mno-mips16 -std=gnu99 -O2 -g -DENABLE_ASSERTIONS -DMIPSr1_ASM -DOPUS_BUILD -mdspr2 -march=74kc -mtune=74kc -mmt -mgp32)
    #for user specify
    add_compile_options(-EL -mhard-float)
    message(WARNING "compile mips opus")
ENDIF()

ADD_EXECUTABLE(opus ${opus})

