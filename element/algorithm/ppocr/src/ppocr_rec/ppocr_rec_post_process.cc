//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "ppocr_rec_post_process.h"

#include <cmath>

#include "common/logger.h"

namespace sophon_stream {
namespace element {
namespace ppocr_rec {

void PpocrRecPostProcess::init(std::shared_ptr<PpocrRecContext> context) {}

void PpocrRecPostProcess::postProcess(
    std::shared_ptr<PpocrRecContext> context,
    common::ObjectMetadatas& objectMetadatas) {
  if (objectMetadatas.size() == 0) return;

  for (auto obj : objectMetadatas) {
    if (obj->mFrame->mEndOfStream) break;
    if (obj->mOutputBMtensors->tensors.size() == 0) continue;
    // hm_data
    std::vector<std::shared_ptr<BMNNTensor>> outputTensors(
        obj->mOutputBMtensors->tensors.size());
    for (int i = 0; i < obj->mOutputBMtensors->tensors.size(); i++) {
      outputTensors[i] = std::make_shared<BMNNTensor>(
          obj->mOutputBMtensors->handle,
          context->bmNetwork->m_netinfo->output_names[0],
          context->bmNetwork->m_netinfo->output_scales[0],
          obj->mOutputBMtensors->tensors[i].get(), context->bmNetwork->is_soc);
    }

    auto output_shape = outputTensors[0]->get_shape();
    auto output_dims = output_shape->num_dims;
    int batch_num = output_shape->dims[0];
    int outputdim_1 = output_shape->dims[1];
    int outputdim_2 = output_shape->dims[2];

    float* predict_batch = nullptr;
    predict_batch = (float*)outputTensors[0]->get_cpu_data();

    for (int m = 0; m < batch_num; m++) {
      if (context->beam_search) {
        const int k = context->beam_width;  // Beam width

        std::vector<std::pair<std::string, float>>
            beam_search_results;  // 存储Beam Search的结果

        std::vector<BeamSearchCandidate> beams;
        beams.push_back(BeamSearchCandidate({}, 1.0f, {}));  // 初始的候选序列

        for (int t = 0; t < outputdim_1; t++) {
          std::vector<BeamSearchCandidate> new_beams;
          std::vector<float> next_char_probs;

          for (int c = 0; c < outputdim_2; c++) {
            float token_score =
                predict_batch[(m * outputdim_1 + t) * outputdim_2 + c];
            next_char_probs.push_back(token_score);
          }

          std::vector<int> top_candidates(next_char_probs.size());
          std::iota(top_candidates.begin(), top_candidates.end(), 0);
          std::sort(top_candidates.begin(), top_candidates.end(),
                    [&](int i, int j) {
                      return next_char_probs[i] > next_char_probs[j];
                    });

          top_candidates.resize(k);

          for (const BeamSearchCandidate& beam : beams) {
            for (int c = 0; c < top_candidates.size(); c++) {
              std::vector<int> new_prefix = beam.prefix;
              new_prefix.push_back(top_candidates[c]);
              float new_score = beam.score;
              new_score = new_score * next_char_probs[top_candidates[c]];
              std::vector<float> new_confs = beam.confs;
              new_confs.push_back(next_char_probs[top_candidates[c]]);
              new_beams.emplace_back(
                  BeamSearchCandidate(new_prefix, new_score, new_confs));
            }
          }
          // std::sort(new_beams.begin(), new_beams.end());
          std::sort(
              new_beams.begin(), new_beams.end(),
              [](const BeamSearchCandidate& a, const BeamSearchCandidate& b) {
                return a.score > b.score;
              });
          new_beams.resize(k);
          beams = new_beams;
        }
        BeamSearchCandidate best_beam = beams[0];

        std::string str_res;
        std::vector<float> conf_list;

        int pre_c = best_beam.prefix[0];
        if (pre_c != 0) {
          str_res = context->label_list_[pre_c];
          conf_list.push_back(best_beam.confs[0]);
        }
        for (int idx = 0; idx < best_beam.prefix.size(); idx++) {
          if (pre_c == best_beam.prefix[idx] || best_beam.prefix[idx] == 0) {
            if (best_beam.prefix[idx] == 0) {
              pre_c = best_beam.prefix[idx];
            }
            continue;
          }
          str_res += context->label_list_[best_beam.prefix[idx]];
          conf_list.push_back(best_beam.confs[idx]);
          pre_c = best_beam.prefix[idx];
        }
        float score = std::accumulate(conf_list.begin(), conf_list.end(), 0.0) /
                      conf_list.size();
        if (std::isnan(score)) {
          score = 0;
          str_res = "###";
        }
        std::shared_ptr<common::RecognizedObjectMetadata> recData =
            std::make_shared<common::RecognizedObjectMetadata>();
        recData->mLabelName = str_res;
        recData->mScores.push_back(score);
        obj->mRecognizedObjectMetadatas.push_back(recData);
      } else {
        std::string str_res;
        int* argmax_idx = new int[outputdim_1];
        float* max_value = new float[outputdim_1];
        for (int n = 0; n < outputdim_1; n++) {
          int char_start_indx = (m * outputdim_1 + n) * outputdim_2;
          int char_end_indx = (m * outputdim_1 + n + 1) * outputdim_2;
          argmax_idx[n] = char_start_indx;
          max_value[n] = predict_batch[char_start_indx];
          for (int j = char_start_indx; j < char_end_indx; j++)
            if (max_value[n] < predict_batch[j]) {
              argmax_idx[n] = j;
              max_value[n] = predict_batch[j];
            }
          argmax_idx[n] -= char_start_indx;
        }

        int last_index = 0;
        float score = 0.f;
        int count = 0;
        for (int n = 0; n < outputdim_1; n++) {
          if (argmax_idx[n] > 0 && (!(n > 0 && argmax_idx[n] == last_index))) {
            score += max_value[n];
            count += 1;
            str_res += context->label_list_[argmax_idx[n]];
          }
          last_index = argmax_idx[n];
        }
        score /= count;
        if (std::isnan(score)) {
          score = 0;
          str_res = "###";
        }
        std::shared_ptr<common::RecognizedObjectMetadata> recData =
            std::make_shared<common::RecognizedObjectMetadata>();
        recData->mLabelName = str_res;
        recData->mScores.push_back(score);
        obj->mRecognizedObjectMetadatas.push_back(recData);
        free(argmax_idx);
        free(max_value);
      }
    }
  }
}

}  // namespace ppocr_rec
}  // namespace element
}  // namespace sophon_stream