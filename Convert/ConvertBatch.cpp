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

#include "Recording/RecorderManager.h"
#include "catch2/catch_test_macros.hpp"
#include "catch2/catch_all.hpp"
#include "../Utils/Utils.h"

#include <filesystem>
#include <iostream>
#include <chrono>
#include <random>


namespace fs = std::filesystem;



int main(int argc, char *argv[]) {
    //RecorderManager &manager = RecorderManager::getInstance();
    //manager.set_debug_mode(0,true);

    //void export_sound_data_to_WAV(std::string const& sound_file_path);

    std::string base_path(argv[1]);
    std::string output_base_path (argv[2]);
    std::cout << "Will search for sound .txt files for conversion in directory: " << base_path << std::endl;
    std::cout << "Will create audio files in directory: " << output_base_path << std::endl;

    fs::path out_directory = output_base_path;

    if (!fs::exists(out_directory)) {
        if (fs::create_directory(out_directory)) {
            std::cout << "Output directory created successfully." << std::endl;
        }
        else {
            std::cerr << "Failed to create output directory." << std::endl;
            return 1;
        }
    }

    std::cout << "Found input subdirectories:\n" << std::endl;

    fs::path directory = base_path;

    std::string sound_suffix = "_sound.txt";


    try {
        if (fs::exists(directory) && fs::is_directory(directory)) {

            int dirs_searched = 0;

            for (const auto& entry : fs::directory_iterator(directory)) {
                if (fs::is_directory(entry.status())) {
                    std::cout << "Processing: " << entry.path().filename().string() << std::endl;

                    // create subdirectory in output dir
                    fs::path out_subdirectory = out_directory / entry.path().filename();

                    if (!fs::exists(out_subdirectory)) {
                        if (!fs::create_directory(out_subdirectory)) {
                            std::cerr << "Failed to create output subdirectory." << std::endl;
                            return 1;
                        }
                    }

                    int files_processed = 0;

                    // get sound files inside subdirectory 
                    fs::path subdir = entry.path();
                    for (const auto& subd_entry : fs::directory_iterator(subdir)) {

                        std::string filename = subd_entry.path().string();

                        if (filename.size() >= sound_suffix.size() &&
                            filename.compare(filename.size() - sound_suffix.size(), sound_suffix.size(), sound_suffix) == 0) {
                            std::cout << "Processing sound file ending with '" << sound_suffix << "': " << filename << std::endl;

                            std::string out_filename_base = (out_subdirectory / subd_entry.path().filename()).string();

                            std::cout << "Output sound file base " << out_filename_base << std::endl;


                            Utils::export_sound_data_to_WAV(filename, out_filename_base);

                            //if (++files_processed >= 1)
                            //    break;

                        }
                    }
                }

                //if (dirs_searched++ > 2)
                //    break;


            }

        }
        else {
            std::cerr << "The specified path does not exist or is not a directory." << std::endl;
        }
    }
    catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "General exception: " << e.what() << std::endl;
    }
     
    //std::string sound_file_path = "./aplausemr_group7_date20240715_15_09_trial1_participant1_sound.txt";
    //std::string sound_file_path = "Z:/Gary/Research/APlausE - MR_MR4_CollaborativeTelepresenceStudy/unityRecordings/excluded/group7/interrupted/aplausemr_group7_date20240715_15_00_trial1_participant1_sound.txt";
    //Utils::export_sound_data_to_WAV(sound_file_path);

    return 0;
}


