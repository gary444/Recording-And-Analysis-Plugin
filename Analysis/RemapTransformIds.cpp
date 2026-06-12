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
#include "Recording/MetaInformation.h"

#include "../Utils/Utils.h"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <string>

namespace fs = std::filesystem;

static const std::string kRejoinTag = "[rejoin]";

std::string get_output_path(const std::string& input_path) {
    const std::string ext = ".transform";
    std::string out = input_path;
    const size_t pos = out.rfind(ext);
    if (pos != std::string::npos)
        out.insert(pos, "_remapped");
    return out;
}

// Reads the meta file and returns {rejoin_uuid -> original_uuid} for every
// game object whose name contains [rejoin]. Both id and p_id fields in each
// TransformDTO record must be remapped using this table.
//
// The meta file is loaded without stripping [rejoin] so that both the original
// and rejoin entries are present simultaneously, making the lookup straightforward.
std::map<int, int> build_remap_table(const std::string& meta_path) {
    // Ensure the static strip-list is empty so [rejoin] names are preserved.
    MetaInformation::set_strings_to_remove_from_object_names({});
    MetaInformation meta(meta_path);

    const std::map<int, std::string> uuid_name = meta.get_recording_gameobjects_map();
    std::map<int, int> remap;

    for (const auto& [rejoin_uuid, name] : uuid_name) {
        if (name.find(kRejoinTag) == std::string::npos)
            continue;

        std::string original_name = name;
        size_t pos = original_name.find(kRejoinTag);
        original_name.erase(pos, kRejoinTag.size());

        const int original_uuid = meta.get_old_uuid(original_name);
        if (original_uuid == -1) {
            std::cout << "  Warning: no original object found for rejoin object '"
                      << name << "' — skipping." << std::endl;
            continue;
        }

        remap[rejoin_uuid] = original_uuid;
        std::cout << "  " << rejoin_uuid << "  (" << name << ")" << std::endl;
        std::cout << "  -> " << original_uuid << "  (" << original_name << ")" << std::endl;
    }

    return remap;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: remap_transform_ids <transform_file>" << std::endl;
        return 1;
    }

    const std::string input_path(argv[1]);

    if (!fs::exists(input_path)) {
        std::cerr << "File not found: " << input_path << std::endl;
        return 1;
    }

    const std::string meta_path = Utils::get_meta_file_path_from_transform(input_path);
    if (!fs::exists(meta_path)) {
        std::cerr << "Meta file not found: " << meta_path << std::endl;
        return 1;
    }

    std::cout << "Building remap table from: " << meta_path << std::endl;
    const std::map<int, int> remap = build_remap_table(meta_path);

    if (remap.empty()) {
        std::cout << "No [rejoin] objects found in meta file — nothing to remap." << std::endl;
        return 0;
    }

    std::cout << "\nFound " << remap.size() << " UUID(s) to remap." << std::endl;

    std::ifstream in(input_path, std::ios::binary);
    if (!in.good()) {
        std::cerr << "Cannot open input file: " << input_path << std::endl;
        return 1;
    }

    in.seekg(0, std::ios::end);
    const size_t total = static_cast<size_t>(in.tellg()) / sizeof(TransformDTO);
    in.seekg(0, std::ios::beg);

    const std::string output_path = get_output_path(input_path);
    std::ofstream out(output_path, std::ios::binary | std::ios::trunc);
    if (!out.good()) {
        std::cerr << "Cannot open output file: " << output_path << std::endl;
        return 1;
    }

    // Count remapped records per original rejoin UUID for the summary.
    std::map<int, size_t> id_remap_counts;

    TransformDTO dto{};
    for (size_t i = 0; i < total; ++i) {
        in.read(reinterpret_cast<char*>(&dto), sizeof(TransformDTO));

        const auto id_it = remap.find(dto.id);
        if (id_it != remap.end()) {
            ++id_remap_counts[id_it->first];
            dto.id = id_it->second;
        }

        const auto pid_it = remap.find(dto.p_id);
        if (pid_it != remap.end()) {
            dto.p_id = pid_it->second;
        }

        out.write(reinterpret_cast<const char*>(&dto), sizeof(TransformDTO));
    }

    in.close();
    out.close();

    std::cout << "\nRemap summary:" << std::endl;
    for (const auto& [rejoin_uuid, count] : id_remap_counts) {
        std::cout << "  UUID " << rejoin_uuid
                  << " -> " << remap.at(rejoin_uuid)
                  << " : " << count << " record(s)" << std::endl;
    }

    const size_t total_remapped = [&] {
        size_t n = 0;
        for (const auto& [k, v] : id_remap_counts) n += v;
        return n;
    }();

    std::cout << "\nTotal records processed : " << total << std::endl;
    std::cout << "Total records remapped  : " << total_remapped << std::endl;
    std::cout << "Output                  : " << output_path << std::endl;

    return 0;
}
