//
// Created by Admin on 07.03.2024.
//

#include "QuantitativePoseRetrievalRequest.h"

QuantitativePoseRetrievalRequest::QuantitativePoseRetrievalRequest(int i, float t_sampling_rate) : id(i),
                                                                                                         QuantitativeTransformAnalysisRequest(
                                                                                                                 t_sampling_rate) {

}

void QuantitativePoseRetrievalRequest::update_parameters(MetaInformation &original_meta_file,
                                                            MetaInformation &new_meta_file) {
    id = new_meta_file.get_old_uuid(original_meta_file.get_object_name(id));
}

void QuantitativePoseRetrievalRequest::process_request(std::shared_ptr<TransformData> t_data,
                                                          std::shared_ptr<SoundData> s_data) {
    if (!t_data)
        return;
    if (t_data->id == id) {
        recent_data.emplace_back(t_data);

        //only keep a pair of recording structs to facilitate interpolation
        while (recent_data.size() > 2) {
            recent_data.pop_front();
        }

        const float sampling_interval = 1.0f / temporal_sampling_rate;
        const float time_for_next_sample = last_value_time + sampling_interval;

        if (t_data->time > time_for_next_sample) {

            glm::quat r = recent_data[0]->global_rotation;
            glm::vec3 p = recent_data[0]->global_position;

            if (recent_data.size() > 1) {

                glm::quat r0 = recent_data[0]->global_rotation;
                glm::quat r1 = recent_data[1]->global_rotation;


                glm::vec3 p0 = recent_data[0]->global_position;
                glm::vec3 p1 = recent_data[1]->global_position;

                float a = (time_for_next_sample - recent_data[0]->time) / (recent_data[1]->time - recent_data[0]->time);
                // todo get correct interp value with t_data->time
                r = glm::slerp(r0, r1, a);
                p = (a * p1) + (1.f - a) * p0;


            }

            glm::vec3 eulerAngles = glm::degrees(glm::eulerAngles(r));

            values.push_back(TimeBasedValue{ time_for_next_sample, {p.x, p.y, p.z, eulerAngles.x, eulerAngles.y, eulerAngles.z}});
            last_value_time = time_for_next_sample;
        }
    }
}

std::shared_ptr<QuantitativeAnalysisRequest> QuantitativePoseRetrievalRequest::clone() const {
    return std::make_shared<QuantitativePoseRetrievalRequest>(id, temporal_sampling_rate);
}

std::string QuantitativePoseRetrievalRequest::get_description(MetaInformation &meta_info) const {
    std::string s = "RotationAnalysis";
    s += " id: " + meta_info.get_object_name(id) + "-";
    s += " temporal sampling rate: " + std::to_string(temporal_sampling_rate);
    return s;
}

void QuantitativePoseRetrievalRequest::clear_recent_data() {
    last_value_time = -1;
    recent_data.clear();
}

TransformAnalysisType QuantitativePoseRetrievalRequest::get_type() const {
    return RotationAnalysis;
}
