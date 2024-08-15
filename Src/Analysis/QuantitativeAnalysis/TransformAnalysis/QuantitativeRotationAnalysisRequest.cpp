//
// Created by Admin on 07.03.2024.
//

#include "QuantitativeRotationAnalysisRequest.h"

QuantitativeRotationAnalysisRequest::QuantitativeRotationAnalysisRequest(int i, float t_sampling_rate) : id(i),
                                                                                                         QuantitativeTransformAnalysisRequest(
                                                                                                                 t_sampling_rate) {

}

void QuantitativeRotationAnalysisRequest::update_parameters(MetaInformation &original_meta_file,
                                                            MetaInformation &new_meta_file) {
    id = new_meta_file.get_old_uuid(original_meta_file.get_object_name(id));
}

void QuantitativeRotationAnalysisRequest::process_request(std::shared_ptr<TransformData> t_data,
                                                          std::shared_ptr<SoundData> s_data) {
    
    const float averaging_window_duration_sec = 0.2;
    const int max_buffer_size = averaging_window_duration_sec * 60; // assume no more than 60 Hz recording rate
    
    if (!t_data)
        return;
    if (t_data->id == id) {
        recent_data.emplace_back(t_data);

        while (recent_data.size() > max_buffer_size) {
            recent_data.pop_front();
        }

        const float sampling_interval = 1.0f / temporal_sampling_rate;
        const float time_for_next_sample = last_value_time + sampling_interval;

        const float window_start_time = std::max(0.f, time_for_next_sample - averaging_window_duration_sec);

        if (t_data->time > time_for_next_sample) {

            // get average rotation speed over last frames_to_average frames
            float accum_rot_speed = 0.f;
            int samples = 0;
            int read_pos = recent_data.size() - 1;
            while (read_pos >= 0 && recent_data[read_pos]->time > window_start_time)
            {
                glm::quat tmp = recent_data[read_pos]->global_rotation * glm::inverse(recent_data[read_pos-1]->global_rotation);
                float angle_diff = acos(std::min(1.f, abs(tmp.w))) * 2.0f;
                float angle_diff_degrees = angle_diff * (180.0f / 3.141f);
                float time_diff = recent_data[read_pos]->time - recent_data[read_pos-1]->time;

                if (time_diff > 0.0f)
                {
					accum_rot_speed += angle_diff_degrees / time_diff;
					samples++;
				}
				read_pos--;
            }
            if (samples) {
                accum_rot_speed /= float(samples);
            }
            values.push_back(TimeBasedValue{ time_for_next_sample, {accum_rot_speed}});
            last_value_time = time_for_next_sample;
        }
    }
}

std::shared_ptr<QuantitativeAnalysisRequest> QuantitativeRotationAnalysisRequest::clone() const {
    return std::make_shared<QuantitativeRotationAnalysisRequest>(id, temporal_sampling_rate);
}

std::string QuantitativeRotationAnalysisRequest::get_description(MetaInformation &meta_info) const {
    std::string s = "RotationAnalysis";
    s += " id: " + meta_info.get_object_name(id) + "-";
    s += " temporal sampling rate: " + std::to_string(temporal_sampling_rate);
    return s;
}

void QuantitativeRotationAnalysisRequest::clear_recent_data() {
    last_value_time = -1;
    values.clear();
}

TransformAnalysisType QuantitativeRotationAnalysisRequest::get_type() const {
    return RotationAnalysis;
}
