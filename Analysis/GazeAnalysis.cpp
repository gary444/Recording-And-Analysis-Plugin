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
#include "Analysis/QuantitativeAnalysis/TransformAnalysis/QuantitativeTramsformAnalysisRequest.h"
#include "Analysis/QuantitativeAnalysis/TransformAnalysis/QuantitativeVelocityAnalysisRequest.h"
#include "Analysis/QuantitativeAnalysis/TransformAnalysis/QuantitativeGazeAnalysisRequest.h"
#include "Analysis/QuantitativeAnalysis/TransformAnalysis/QuantitativeDistanceAnalysisRequest.h"
#include "Analysis/QuantitativeAnalysis/TransformAnalysis/QuantitativeRotationAnalysisRequest.h"
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::duration;
using std::chrono::milliseconds;

#include <cmath>
#include <iostream>

void gaze_analysis(std::vector<std::string> rec_files, const std::string& output_dir) {
    //void gaze_analysis(std::string rec_file, const std::string& output_dir) {

    AnalysisManager& manager = AnalysisManager::getInstance();
    manager.set_analysis_results_output_directory(output_dir);

    for (auto f : rec_files) {
        manager.add_recording_path(f.data(), f.length());
    }



    std::string primary_file = manager.get_primary_file();
    MetaInformation meta_information{ primary_file + ".recordmeta" };

    // identify heads of participants
    std::vector<int> participant_head_uuids(4, -1);

    for (size_t i = 0; i < participant_head_uuids.size(); i++)
    {
        std::stringstream game_object_path_ss;
        game_object_path_ss << "/__SCENE__";
        game_object_path_ss << (i < 2 ? "/S143TrackingArea" : "/DBLTrackingArea" );
        game_object_path_ss << "/Participant" << std::to_string(i) << " [Remote]/Head";

        participant_head_uuids[i] = meta_information.get_old_uuid(game_object_path_ss.str());

        if (-1 == participant_head_uuids[i]) {
            std::cerr << "Error: Could not find UUID for specified path " << game_object_path_ss.str() << std::endl;
            return;
        }
        std::cout << "UUID: " << participant_head_uuids[i] << std::endl;
    }


    // TODO adjust cone size

    float gaze_cone_height = 10.0f;
    float gaze_cone_fov_angle = 60.0f;
    float gaze_cone_angle = gaze_cone_fov_angle / 2.0f;
    float gaze_cone_radius = std::tan(gaze_cone_angle * (3.14159f / 180.0f)) * gaze_cone_height;

    // note: add final argument to specify that forward direction is positive Z axis!
    std::shared_ptr<IntervalGazeAnalysisRequest> gaze_request_1 = std::make_shared<IntervalGazeAnalysisRequest>(participant_head_uuids[0], participant_head_uuids[1], gaze_cone_fov_angle, gaze_cone_height, 1);
    std::shared_ptr<IntervalGazeAnalysisRequest> gaze_request_2 = std::make_shared<IntervalGazeAnalysisRequest>(participant_head_uuids[0], participant_head_uuids[2], gaze_cone_fov_angle, gaze_cone_height, 1);
    std::shared_ptr<IntervalGazeAnalysisRequest> gaze_request_3 = std::make_shared<IntervalGazeAnalysisRequest>(participant_head_uuids[0], participant_head_uuids[3], gaze_cone_fov_angle, gaze_cone_height, 1);

    manager.add_interval_analysis_request(gaze_request_1);
    manager.add_interval_analysis_request(gaze_request_2);
    manager.add_interval_analysis_request(gaze_request_3);

    manager.process_interval_analysis_requests_for_all_files();



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


    std::string search_suffix = "experimentcontroller.recordmeta";

    std::cout << "Found input subdirectories:\n" << std::endl;

    try {

        int dirs_searched = 0;

        for (const auto& entry : fs::directory_iterator(directory)) {
            if (fs::is_directory(entry.status())) {
                std::cout << "Processing: " << entry.path().filename().string() << std::endl;


                // find recording files inside subdirectory 
                fs::path subdir = entry.path();
                std::vector<std::string> rec_files_from_group;

                for (const auto& subd_entry : fs::directory_iterator(subdir)) {

                    std::string filename = subd_entry.path().string();

                    if (filename.size() >= search_suffix.size() &&
                        filename.compare(filename.size() - search_suffix.size(), search_suffix.size(), search_suffix) == 0) {

                        fs::path p = subd_entry.path();
                        std::cout << "Found recording file" << p.replace_extension().string() << std::endl;

                        rec_files_from_group.push_back(p.replace_extension().string());
                    }
                }

                if (0 != rec_files_from_group.size()) {
                    // create subdirectory in output dir
                    fs::path out_subdirectory = out_directory / entry.path().filename();
                    if (!fs::exists(out_subdirectory)) {
                        if (!fs::create_directory(out_subdirectory)) {
                            std::cerr << "Failed to create output subdirectory." << std::endl;
                            return 1;
                        }
                    }

                    gaze_analysis(rec_files_from_group, out_subdirectory.string());

                    // TODO remove 
                    break;

                }

            }



        }
    }
    catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "General exception: " << e.what() << std::endl;
    }





    /*




    std::vector<std::string> recordings_to_analyse;


    std::string rec_file(argv[1]);
    recordings_to_analyse.push_back(rec_file);




    const std::string filter_str("experimentcontroller");

    for (auto r : recordings_to_analyse) {

        if ((r.find(filter_str) != std::string::npos)) {
            gaze_analysis(r, results_output_dir);
        }
        else {
            std::cout << "Did not analyse file " << r << " because filter string (" << filter_str << ") was not found " << std::endl;
        }
    }
    */

    std::cout << "Finished Analysis" << std::endl;

    return 0;
}