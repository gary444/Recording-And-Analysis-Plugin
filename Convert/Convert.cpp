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

std::string recording_dir = std::filesystem::current_path().string() + "/";
std::string recording_file = "test";

int transform_buffer_size = 1000;
int sound_buffer_size = 100;
int replay_buffer_num = 3;
float replay_buffer_interval = 10.0f;
int recording_duration = 100;
int stop_time = 15;
int transform_count = 20;
int recording_steps_per_second = 10;

bool forward = true;
bool reverse = true;
bool random = true;

bool first = true;


int main(int argc, char *argv[]) {
    //RecorderManager &manager = RecorderManager::getInstance();
    //manager.set_debug_mode(0,true);

    //void export_sound_data_to_WAV(std::string const& sound_file_path);

    std::string base_path (argv[1]);
    std::cout << "Will search for audio files for conversion in directory: " << base_path << std::endl;

     
    //std::string sound_file_path = "./aplausemr_group7_date20240715_15_09_trial1_participant1_sound.txt";
    //std::string sound_file_path = "Z:/Gary/Research/APlausE - MR_MR4_CollaborativeTelepresenceStudy/unityRecordings/excluded/group7/interrupted/aplausemr_group7_date20240715_15_00_trial1_participant1_sound.txt";
    //Utils::export_sound_data_to_WAV(sound_file_path);

    return 0;
}


