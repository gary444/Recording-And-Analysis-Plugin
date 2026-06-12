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

#include "../Utils/Utils.h"


namespace fs = std::filesystem;

using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::duration;
using std::chrono::milliseconds;

#include <cmath>
#include <iostream>

int main(int argc, char* argv[]) {


    std::string base_path(argv[1]);

    std::cout << "Will search for recording files for conversion in directory: " << base_path << std::endl;

    fs::path directory = base_path;
    if (!fs::exists(directory) || !fs::is_directory(directory)) {
        std::cerr << "The specified path does not exist or is not a directory: " << base_path << std::endl;
        return 1;
    }


    //std::string search_suffix = "experimentcontroller.recordmeta";

    std::string search_suffix = ".transform";
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

                        // head objects
                        //Utils::export_transform_data_to_CSV(filename, "/Player_0 [Remote]/TrackingSpace/CenterEyeAnchor", "head_p0");
                        //Utils::export_transform_data_to_CSV(filename, "/Player_1 [Remote]/TrackingSpace/CenterEyeAnchor", "head_p1");
                        //Utils::export_transform_data_to_CSV(filename, "/Player_2 [Remote]/TrackingSpace/CenterEyeAnchor", "head_p2");
                        //Utils::export_transform_data_to_CSV(filename, "/Player_3 [Remote]/TrackingSpace/CenterEyeAnchor", "head_p3");

                        // gaze objects
                        Utils::export_transform_data_to_CSV(filename, "/Player_0 [Remote]/TrackingSpace/CenterEyeAnchor/GazeDirectionObject", "gaze_p0");
                        Utils::export_transform_data_to_CSV(filename, "/Player_1 [Remote]/TrackingSpace/CenterEyeAnchor/GazeDirectionObject", "gaze_p1");
                        Utils::export_transform_data_to_CSV(filename, "/Player_2 [Remote]/TrackingSpace/CenterEyeAnchor/GazeDirectionObject", "gaze_p2");
                        Utils::export_transform_data_to_CSV(filename, "/Player_3 [Remote]/TrackingSpace/CenterEyeAnchor/GazeDirectionObject", "gaze_p3");

                        //Utils::export_transform_data_to_CSV(filename);



                    }
                }



                ++dirs_searched;

            }
            //break;

        }
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
