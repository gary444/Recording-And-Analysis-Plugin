// VRSYS plugin of Virtual Reality and Visualization Group (Bauhaus-University Weimar)
//  _    ______  _______  _______
// | |  / / __ \/ ___/\ \/ / ___/
// | | / / /_/ /\__ \  \  /\__ \
// | |/ / _, _/___/ /  / /___/ /
// |___/_/ |_|/____/  /_//____/
//
//  __                            __                       __   __   __    ___ .  . ___
// |__)  /\  |  | |__|  /\  |  | /__`    |  | |\ | | \  / |__  |__) /__` |  |   /\   |
// |__) /~~\ \__/ |  | /~~\ \__/ .__/    \__/ | \| |  \/  |___ |  \ .__/ |  |  /~~\  |
//
//       ___               __
// |  | |__  |  |\/|  /\  |__)
// |/\| |___ |  |  | /~~\ |  \
//
// Copyright (c) 2024 Virtual Reality and Visualization Group
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//-----------------------------------------------------------------
//   Authors:        Anton Lammert
//   Date:           2024
//-----------------------------------------------------------------

#include "Analysis/AnalysisManager.h"
#include "Analysis/IntervalAnalysis/TransformAnalysis/IntervalPositionAdjustmentAnalysisRequest.h"
#include "Analysis/IntervalAnalysis/TransformAnalysis/IntervalGazeIntersectionAnalysisRequest.h"
#include "Analysis/QuantitativeAnalysis/TransformAnalysis/QuantitativeTramsformAnalysisRequest.h"
#include "Analysis/QuantitativeAnalysis/TransformAnalysis/QuantitativeVelocityAnalysisRequest.h"
#include "Analysis/QuantitativeAnalysis/TransformAnalysis/QuantitativeGazeAnalysisRequest.h"
#include "Analysis/QuantitativeAnalysis/TransformAnalysis/QuantitativeDistanceAnalysisRequest.h"
#include "Analysis/QuantitativeAnalysis/TransformAnalysis/QuantitativeRotationAnalysisRequest.h"

#include <filesystem>
#include <string>

#include "../Utils/Utils.h"


namespace fs = std::filesystem;

using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::duration;
using std::chrono::milliseconds;

#include <cmath>
#include <iostream>

#define INTERSECTION_BASED_ANALYSIS


//#define USE_HEAD_ORIENTATION_ONLY



void gaze_analysis(std::vector<std::string> rec_files, const std::string& output_dir) {
    //void gaze_analysis(std::string rec_file, const std::string& output_dir) {

    AnalysisManager& manager = AnalysisManager::getInstance();

    //manager.clear_recording_paths();
    //manager.clear_requests();

    manager.set_analysis_results_output_directory(output_dir);

    for (auto f : rec_files) {
        manager.add_recording_path(f.data(), f.length());
    }


    std::string primary_file = manager.get_primary_file();
    MetaInformation::set_strings_to_remove_from_object_names({"[rejoin]" });
    //MetaInformation::set_strings_to_remove_from_object_names({" [Local]", " [Remote]", "[rejoin]"});
    MetaInformation meta_information{ primary_file + ".recordmeta" };

    // identify heads of participants
    std::vector<int> participant_head_uuids(4, -1);

    for (size_t i = 0; i < participant_head_uuids.size(); i++)
    {
        std::stringstream game_object_path_ss;
        game_object_path_ss << "/Player_" << std::to_string(i) << " [Remote]/TrackingSpace/CenterEyeAnchor";
        participant_head_uuids[i] = meta_information.get_old_uuid(game_object_path_ss.str());

        if (-1 == participant_head_uuids[i]) {
            std::cerr << "Error: Could not find UUID for specified path " << game_object_path_ss.str() << std::endl;
            return;
        }
        std::cout << "Head object UUID: " << participant_head_uuids[i] << std::endl;
    }


#ifndef USE_HEAD_ORIENTATION_ONLY
    std::vector<int> participant_gaze_obj_uuids(4, -1);

    for (size_t i = 0; i < participant_gaze_obj_uuids.size(); i++)
    {
        std::stringstream game_object_path_ss;
        game_object_path_ss << "/Player_" << std::to_string(i) << " [Remote]/TrackingSpace/CenterEyeAnchor/GazeDirectionObject";
        participant_gaze_obj_uuids[i] = meta_information.get_old_uuid(game_object_path_ss.str());

        if (-1 == participant_gaze_obj_uuids[i]) {
            std::cerr << "Error: Could not find UUID for specified path " << game_object_path_ss.str() << std::endl;
            return;
        }
        std::cout << "Gaze object UUID: " << participant_gaze_obj_uuids[i] << std::endl;
    }
#endif


    // manually defined workspace (table) and description positions, set from Unity transforms
    glm::vec3 workspace_position{ -0.372 , 0.5, 0.0};
    glm::vec3 workspace_scale{ 1.7, 1, 1.2 };
    glm::vec3 description_position{ 1.478 , 1.85, 0.0 };
    glm::vec3 description_scale{ 0.1, 2.4, 1.68 };



#ifdef INTERSECTION_BASED_ANALYSIS

    for (size_t i = 0; i < 4; i++)
    {
        for (size_t j = 0; j < 4; j++)
        {
            if (j != i) {

                // check gaze to other participants
                std::shared_ptr<IntervalGazeIntersectionAnalysisRequest> interval_intersection_request = std::make_shared<IntervalGazeIntersectionAnalysisRequest>(
#ifdef USE_HEAD_ORIENTATION_ONLY
                    participant_head_uuids[i],
#else            
                    participant_gaze_obj_uuids[i],
#endif
                    participant_head_uuids[j],
                    1, // view direction: z axis
                    IntersectionSphere,
                    0.3f, // radius,
                    glm::vec3(0), //glm::vec3 half_extents,
                    0.25f, //float capsule_half_height,
                    1,//int capsule_axis
                    -0.36f // float capsule_offset_along_axis
                );

                manager.add_interval_analysis_request(interval_intersection_request);

            }
        }

        // check gaze at workspace / table
        std::shared_ptr<IntervalGazeStaticIntersectionAnalysisRequest> table_interval_intersection_request = std::make_shared<IntervalGazeStaticIntersectionAnalysisRequest>(
#ifdef USE_HEAD_ORIENTATION_ONLY
            participant_head_uuids[i],
#else            
            participant_gaze_obj_uuids[i],
#endif                
            1, // view direction: z axis
            IntersectionCuboid,
            0.0f, 
            glm::vec3(0.5),
            0.0f, 0, 0.0f, // capsule data, not relevant here
            workspace_position, glm::quat(glm::vec3(0)), workspace_scale,
            "Workspace"
        );
        manager.add_interval_analysis_request(table_interval_intersection_request);



        // check gaze at description
        std::shared_ptr<IntervalGazeStaticIntersectionAnalysisRequest> description_interval_intersection_request = std::make_shared<IntervalGazeStaticIntersectionAnalysisRequest>(
#ifdef USE_HEAD_ORIENTATION_ONLY
            participant_head_uuids[i],
#else            
            participant_gaze_obj_uuids[i],
#endif                
            1, // view direction: z axis
            IntersectionCuboid,
            0.0f,
            glm::vec3(0.5),
            0.0f, 0, 0.0f, // capsule data, not relevant here
            description_position, glm::quat(glm::vec3(0)), description_scale,
            "ScenarioDescription"
        );

            manager.add_interval_analysis_request(description_interval_intersection_request);

    }

    manager.process_interval_analysis_requests_for_all_files();


#else 

    //-------------------------------------------------------------------
    // quantitative head rotation queries 
    //------------------------------------------------------------------- 


    float rotation_analysis_sampling_rate = 10.f;

    for (size_t i = 0; i < 4; i++)
    {
#ifdef USE_HEAD_ORIENTATION_ONLY
        std::shared_ptr<QuantitativeTransformAnalysisRequest> quantitative_rotation_request = std::make_shared<QuantitativeRotationAnalysisRequest>(participant_head_uuids[i], rotation_analysis_sampling_rate);
#else        
        std::shared_ptr<QuantitativeTransformAnalysisRequest> quantitative_rotation_request = std::make_shared<QuantitativeRotationAnalysisRequest>(participant_gaze_obj_uuids[i], rotation_analysis_sampling_rate);
#endif        
        manager.add_quantitative_analysis_request(quantitative_rotation_request);
    }



    //-------------------------------------------------------------------
    // quantitative gaze queries 
    //------------------------------------------------------------------- 

    for (size_t i = 0; i < 4; i++)
    {
        for (size_t j = 0; j < 4; j++)
        {
            if (j != i) {

                // note: add final argument to specify that forward direction is positive Z axis!
                // a (first argument) is gazer, b (second argument) is gaze target
#ifdef USE_HEAD_ORIENTATION_ONLY
                std::shared_ptr<QuantitativeTransformAnalysisRequest> gaze_request = std::make_shared<QuantitativeGazeAnalysisRequest>(participant_head_uuids[i], participant_head_uuids[j], rotation_analysis_sampling_rate);
#else
                std::shared_ptr<QuantitativeTransformAnalysisRequest> gaze_request = std::make_shared<QuantitativeGazeAnalysisRequest>(participant_gaze_obj_uuids[i], participant_head_uuids[j], rotation_analysis_sampling_rate);
#endif
                manager.add_quantitative_analysis_request(gaze_request);
            }
        }
    }

    manager.process_quantitative_analysis_requests_for_all_files();

#endif


}

int main(int argc, char* argv[]) {


    std::string base_path(argv[1]);
    std::string results_output_dir = (argv[2]);

    std::cout << "Will search for recording files for conversion in directory: " << base_path << std::endl;
    std::cout << "Will write analysis files in directory: " << results_output_dir << std::endl;

    // create output directory
    fs::path out_directory = results_output_dir;
    if (!fs::exists(out_directory)) {
        if (fs::create_directory(out_directory)) {
            std::cout << "Output directory created successfully." << std::endl;
        }
        else {
            std::cerr << "Failed to create output directory." << std::endl;
            return 1;
        }
    }

    fs::path directory = base_path;
    if (!fs::exists(directory) || !fs::is_directory(directory)) {
        std::cerr << "The specified path does not exist or is not a directory: " << base_path << std::endl;
        return 1;
    }


    
    std::string search_suffix = ".recordmeta";
    std::string search_string = "excluded";

    std::cout << "Found input subdirectories:\n" << std::endl;

    try {

        int dirs_searched = 0;

        std::vector<std::string> all_rec_files;


        for (const auto& entry : fs::directory_iterator(directory)) {
            if (fs::is_directory(entry.status())) {
                std::cout << "Processing: " << entry.path().filename().string() << std::endl;


                // find recording files inside subdirectory 
                fs::path subdir = entry.path();

                for (const auto& subd_entry : fs::directory_iterator(subdir)) {

                    std::string filename = subd_entry.path().string();

                    if (filename.size() >= search_suffix.size() 
                        && filename.compare(filename.size() - search_suffix.size(), search_suffix.size(), search_suffix) == 0 
                        && filename.find(search_string) == std::string::npos 
                        ) {



                        fs::path p = subd_entry.path();
                        std::cout << "Found recording file: " << p.replace_extension().string() << std::endl;
                        all_rec_files.push_back(p.replace_extension().string());

                        //break;

                    }
                }



                ++dirs_searched;

            }
            //break;

        }
        gaze_analysis(all_rec_files, out_directory.string());

    }
    catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "General exception: " << e.what() << std::endl;
    }

    std::cout << "Finished Analysis" << std::endl;

    return 0;
}