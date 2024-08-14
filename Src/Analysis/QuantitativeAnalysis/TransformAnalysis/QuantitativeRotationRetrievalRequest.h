//
// Created by Admin on 07.03.2024.
//

#ifndef RECORDINGPLUGIN_QuantitativeRotationRetrievalRequest_H
#define RECORDINGPLUGIN_QuantitativeRotationRetrievalRequest_H


#include <deque>
#include "QuantitativeTramsformAnalysisRequest.h"

class QuantitativeRotationRetrievalRequest : public QuantitativeTransformAnalysisRequest{

private:
    int id;

    std::deque<std::shared_ptr<TransformData>> recent_data;

public:
    QuantitativeRotationRetrievalRequest(int i, float t_sampling_rate);

    void update_parameters(MetaInformation& original_meta_file, MetaInformation& new_meta_file) override;

    void process_request(std::shared_ptr<TransformData> t_data, std::shared_ptr<SoundData> s_data) override;

    std::shared_ptr<QuantitativeAnalysisRequest> clone() const override;

    std::string get_description(MetaInformation& meta_info) const override;

    void clear_recent_data() override;

    TransformAnalysisType get_type() const override;
};


#endif //RECORDINGPLUGIN_QuantitativeRotationRetrievalRequest_H
