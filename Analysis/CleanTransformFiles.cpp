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

#include "Recording/Transform/TransformData.h"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

// A backward time jump larger than this (in seconds) signals the end of a
// stale prefix from a previous recording session.
static constexpr float kJumpThreshold = 5.0f;

struct CleanResult {
    size_t total;
    size_t prefix_skipped;
    size_t nan_skipped;
    size_t written;
};

CleanResult clean_transform_file(const std::string& input_path, const std::string& output_path) {
    CleanResult result{};

    std::ifstream in(input_path, std::ios::binary);
    if (!in.good()) {
        std::cerr << "  Cannot open input file: " << input_path << std::endl;
        return result;
    }

    in.seekg(0, std::ios::end);
    const size_t file_size = static_cast<size_t>(in.tellg());
    result.total = file_size / sizeof(TransformDTO);
    in.seekg(0, std::ios::beg);

    // First pass: find the stale-prefix boundary.
    // The stale prefix consists of records with a high timecode left over from
    // a previous session. It ends at the first large backward jump in t where
    // the preceding t was itself above the jump threshold (ruling out the
    // trivial case where t just starts near zero).
    size_t start_idx = 0;
    float prev_t = std::numeric_limits<float>::quiet_NaN();

    TransformDTO dto{};
    for (size_t i = 0; i < result.total; ++i) {
        in.read(reinterpret_cast<char*>(&dto), sizeof(TransformDTO));
        const float cur_t = dto.t;

        if (!std::isnan(prev_t) && !std::isnan(cur_t)) {
            if (cur_t < prev_t - kJumpThreshold && prev_t > kJumpThreshold) {
                start_idx = i;
                break;
            }
        }

        if (!std::isnan(cur_t))
            prev_t = cur_t;
    }

    result.prefix_skipped = start_idx;

    // Second pass: write records from start_idx onward, skipping NaN timestamps.
    in.clear();

    in.seekg(static_cast<std::streamoff>(start_idx * sizeof(TransformDTO)), std::ios::beg);

    std::ofstream out(output_path, std::ios::binary | std::ios::trunc);
    if (!out.good()) {
        std::cerr << "  Cannot open output file: " << output_path << std::endl;
        return result;
    }

    for (size_t i = start_idx; i < result.total; ++i) {
        in.read(reinterpret_cast<char*>(&dto), sizeof(TransformDTO));
        if (std::isnan(dto.t)) {
            ++result.nan_skipped;
        } else {
            out.write(reinterpret_cast<const char*>(&dto), sizeof(TransformDTO));
            ++result.written;
        }
    }

    return result;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: clean_transform_files <directory>" << std::endl;
        return 1;
    }

    const std::string base_path(argv[1]);
    const fs::path directory = base_path;

    if (!fs::exists(directory) || !fs::is_directory(directory)) {
        std::cerr << "Not a valid directory: " << base_path << std::endl;
        return 1;
    }

    const std::string ext          = ".transform";
    const std::string exclude_str  = "excluded";
    const std::string cleaned_str  = "_cleaned";

    try {
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (!fs::is_directory(entry.status()))
                continue;

            std::cout << "Processing directory: " << entry.path().filename().string() << std::endl;

            for (const auto& sub : fs::directory_iterator(entry.path())) {
                const std::string filename = sub.path().string();

                const bool has_ext     = filename.size() >= ext.size()
                                         && filename.compare(filename.size() - ext.size(), ext.size(), ext) == 0;
                const bool is_excluded = filename.find(exclude_str) != std::string::npos;
                const bool is_cleaned  = filename.find(cleaned_str) != std::string::npos;

                if (!has_ext || is_excluded || is_cleaned)
                    continue;

                // Build output path: insert _cleaned before .transform
                std::string output_path = filename;
                output_path.insert(output_path.size() - ext.size(), cleaned_str);

                std::cout << "  Cleaning: " << sub.path().filename().string() << std::endl;

                const CleanResult r = clean_transform_file(filename, output_path);

                std::cout << "    Total records  : " << r.total          << std::endl;
                std::cout << "    Prefix skipped : " << r.prefix_skipped << std::endl;
                std::cout << "    NaN skipped    : " << r.nan_skipped    << std::endl;
                std::cout << "    Written        : " << r.written         << std::endl;
                std::cout << "    Output         : " << output_path       << std::endl;
            }
        }
    }
    catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
        return 1;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "Finished." << std::endl;
    return 0;
}
